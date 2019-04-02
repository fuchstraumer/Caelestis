#pragma once
#ifndef FOUNDATION_CONTENTION_FREE_MUTEX_HPP
#define FOUNDATION_CONTENTION_FREE_MUTEX_HPP
#include <cstddef> // size_t
#include <atomic>
#include <mutex>
#include <memory>
#include <array>
#include <unordered_map>
#include <cassert>

namespace foundation
{

    /*
        \brief Contention free shared mutex: significantly faster than std::shared_mutex with similar interface. Recursive, as well.

        Supports recursive locks for shared locks, but going from
        shared to exclusive recursively is undefined behavior
        and won't work

        This also won't show performance benefits until its
        accessed by ~8+ threads, most likely. Also recall
        that it should generally be used in read mode for
        longer periods if possible.

        More readers (vs writers) is greatly preferable, as well

        `ContentionFreeCount` is the maximum user count for threads,
        as well. Once more than `ContentionFreeCount` threads are using
        the object, following locks will always become exclusive until
        the oversubscribed threads are destroyed (and this is accomodated
        via using `thread_local` to clean up a cache of thread handles)

        Reference: https://github.com/AlexeyAB/object_threadsafe
    */
    struct contention_free_mutex
    {
    public:
        constexpr static size_t SupportedThreadCount = 64u;
    private:
        constexpr static auto memberAlignment = std::hardware_destructive_interference_size;

        struct contention_free_flag
        {
            contention_free_flag() noexcept { value = 0; }
            // false sharing would effectively cripple this
            alignas(std::hardware_destructive_interference_size) std::atomic<size_t> value;
        };

        std::atomic_bool wantExclusiveLock{ false };
        using shared_lock_array = std::array<contention_free_flag, SupportedThreadCount>;
        // stores each threads current recursion depth
        alignas(memberAlignment) const std::shared_ptr<shared_lock_array> sharedLocksArrayPtr;
        // this doesn't seem ideal
        alignas(memberAlignment) shared_lock_array& sharedLocksArray;
        // current recursive depth of lock
        size_t recursiveExclusiveLockCount{ 0u };
        std::atomic<std::thread::id> ownerThreadId;

        struct auto_unregister
        {
            size_t threadIdx{ std::numeric_limits<size_t>::max() };
            std::shared_ptr<shared_lock_array> sharedPtr;
            auto_unregister(size_t thread_idx, const std::shared_ptr<shared_lock_array>& shared_ptr) : threadIdx(std::move(thread_idx)),
                sharedPtr(shared_ptr) {}
            auto_unregister(auto_unregister&& other) noexcept : threadIdx(std::move(other.threadIdx)), sharedPtr(std::move(other.sharedPtr)) {}
            ~auto_unregister()
            {
                if (sharedPtr.use_count() > 0u)
                {
                    (*sharedPtr)[threadIdx].value--;
                }
            }
        };

        __forceinline std::thread::id fast_get_this_thread_id() const noexcept
        {
            return std::this_thread::get_id();
        }

        enum class index_operation : uint8_t
        {
            InvalidOp = 0u,
            UnregisterThread,
            GetIndex,
            RegisterThread
        };

        size_t getOrSetIndex(index_operation operation, size_t result_index = std::numeric_limits<size_t>::max()) noexcept
        {
            thread_local static std::unordered_map<void*, auto_unregister> threadLocalIdxMap;

            auto iter = threadLocalIdxMap.find(this);
            if (iter != std::cend(threadLocalIdxMap))
            {
                result_index = iter->second.threadIdx;
            }

            if (operation == index_operation::UnregisterThread)
            {
                if (sharedLocksArray[result_index].value == 1u)
                {
                    threadLocalIdxMap.erase(this);
                }
                else
                {
                    return std::numeric_limits<size_t>::max();
                }
            }
            else if (operation == index_operation::RegisterThread)
            {
                threadLocalIdxMap.emplace(this, result_index, sharedLocksArrayPtr);

                for (auto removal_iter = threadLocalIdxMap.begin(); removal_iter != threadLocalIdxMap.end();)
                {
                    if (removal_iter->second.sharedPtr->at(iter->second.threadIdx).value == std::numeric_limits<size_t>::max())
                    {
                        removal_iter = threadLocalIdxMap.erase(removal_iter);
                    }
                    else
                    {
                        ++removal_iter;
                    }
                }
            }

            return result_index;
        }

        /*
            \brief Without clearing the array like this, thread stuff can get weird. This also means parent class doesn't need a unique destructor.
        */
        struct shared_lock_array_deleter
        {
            void operator()(shared_lock_array* array)
            {
                for (auto& item : *array) // make sure all potential flags are reset
                {
                    item.value = std::numeric_limits<uint32_t>::max();
                }
                delete array;
            }
        };
    public:

        contention_free_mutex() : sharedLocksArrayPtr(new shared_lock_array(), shared_lock_array_deleter()), sharedLocksArray(*sharedLocksArrayPtr),
            wantExclusiveLock(false), recursiveExclusiveLockCount(0u) {}

        ~contention_free_mutex() = default;

        bool unregister_thread() noexcept
        {
            return getOrSetIndex(index_operation::UnregisterThread) != std::numeric_limits<size_t>::max();
        }

        size_t register_thread() noexcept
        {
            size_t curr_index = getOrSetIndex(index_operation::GetIndex);
            if (curr_index == std::numeric_limits<size_t>::max())
            {
                // I chose early return, instead of reference implementations <= preceding entry to the for loop
                if (sharedLocksArrayPtr.use_count() > sharedLocksArray.size())
                {
                    return curr_index;
                }
                for (size_t i = 0u; i < sharedLocksArray.size(); ++i)
                {
                    size_t unregistered_value = 0u;
                    if (sharedLocksArray[i].value == 0u)
                    {
                        if (sharedLocksArray[i].value.compare_exchange_strong(unregistered_value, 1u))
                        {
                            curr_index = i;
                            getOrSetIndex(index_operation::RegisterThread, curr_index);
                            break;
                        }
                    }
                }
            }
            return curr_index;
        }

        void lock_shared() noexcept
        {
            const size_t register_index = register_thread();

            if (register_index != std::numeric_limits<size_t>::max())
            {
                size_t recursion_depth = sharedLocksArray[register_index].value.load(std::memory_order_acquire);
                assert(recursion_depth >= 1u);

                if (recursion_depth != std::numeric_limits<size_t>::max())
                {
                    sharedLocksArray[register_index].value.store(recursion_depth + 1, std::memory_order_release);  // recursive->release
                }
                else
                {
                    sharedLocksArray[register_index].value.store(recursion_depth + 1, std::memory_order_seq_cst); // first -> sequential
                    while (wantExclusiveLock.load(std::memory_order_seq_cst))
                    {
                        sharedLocksArray[register_index].value.store(recursion_depth, std::memory_order_seq_cst);
                        for (volatile size_t i = 0u; wantExclusiveLock.load(std::memory_order_seq_cst); ++i)
                        {
                            if (i % 100000u == 0)
                            {
                                std::this_thread::yield();
                            }
                        }
                        sharedLocksArray[register_index].value.store(recursion_depth + 1, std::memory_order_seq_cst);
                    }
                }
            }
            else
            {
                // exclusive lock section
                if (ownerThreadId.load(std::memory_order_acquire) != fast_get_this_thread_id())
                {
                    size_t i = 0u;
                    for (bool flag = false; !wantExclusiveLock.compare_exchange_weak(flag, true, std::memory_order_seq_cst); flag = false)
                    {
                        if (++i % 100000u == 0u)
                        {
                            // yield can be used by the OS/implementation to change current threads priority
                            std::this_thread::yield();
                        }
                    }
                    ownerThreadId.store(fast_get_this_thread_id(), std::memory_order_release);
                }
                ++recursiveExclusiveLockCount;
            }
        }

        void unlock_shared() noexcept
        {
            const size_t register_index = getOrSetIndex(index_operation::GetIndex);

            if (register_index != std::numeric_limits<size_t>::max())
            {
                const size_t recursion_depth = sharedLocksArray[register_index].value.load(std::memory_order_acquire);
                sharedLocksArray[register_index].value.store(recursion_depth - 1, std::memory_order_release);
            }
            else
            {
                if (--recursiveExclusiveLockCount == 0u)
                {
                    ownerThreadId.store(std::thread::id(), std::memory_order_release);
                    wantExclusiveLock.store(false, std::memory_order_release);
                }
            }
        }

        void lock() noexcept
        {
            const size_t register_index = register_thread();
            if (register_index != std::numeric_limits<size_t>::max())
            {
                assert(sharedLocksArray[register_index].value.load(std::memory_order_acquire) == 1u);

                if (ownerThreadId.load(std::memory_order_acquire) != fast_get_this_thread_id())
                {
                    size_t i = 0u; // spin/wait to acquire the lock
                    for (bool flag = false; !wantExclusiveLock.compare_exchange_weak(flag, true, std::memory_order_seq_cst); flag = false)
                    {
                        if (++i % 100000 == 0)
                        {
                            std::this_thread::yield();
                        }
                    }
                    ownerThreadId.store(fast_get_this_thread_id(), std::memory_order_release);
                }

                ++recursiveExclusiveLockCount;
            }
        }

        void unlock() noexcept
        {
            assert(recursiveExclusiveLockCount > 0u);
            if (--recursiveExclusiveLockCount == 0u)
            {
                ownerThreadId.store(std::thread::id(), std::memory_order_release);
                wantExclusiveLock.store(false, std::memory_order_release);
            }
        }

    };

}

#endif //!FOUNDATION_CONTENTION_FREE_MUTEX_HPP
