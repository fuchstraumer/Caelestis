#pragma once
#ifndef FUCHSTRAUMER_SAFE_PTR_HPP
#define FUCHSTRAUMER_SAFE_PTR_HPP
#include <mutex>
#include <atomic>
#include <memory>
#include <shared_mutex>

namespace foundation
{

    template<typename T, typename MutexType = std::recursive_mutex, typename ExclusiveLockType = std::unique_lock<MutexType>, typename SharedLockType = std::unique_lock<MutexType>>
    class safe_ptr
    {
        // TODO: Update with propagate_const once available
        const std::shared_ptr<T> pointer;
        std::shared_ptr<MutexType> mutexPointer;

        template<typename ReqLock>
        class auto_lock
        {
            T* const pointer{ nullptr };
            ReqLock lock;
        public:

            auto_lock(auto_lock&& other) noexcept : pointer(std::move(other.pointer)), lock(std::move(other.lock)) {}
            auto_lock(T* const _ptr, MutexType& mutex) noexcept : ptr(_ptr), lock(mutex) {}

            T* operator->() noexcept
            {
                return ptr;
            }

            const T* operator->() const noexcept
            {
                return ptr;
            }

        };

        template<typename ReqLock>
        class auto_lock_object
        {
            T* const pointer{ nullptr };
            ReqLock lock;
        public:

            auto_lock_object(auto_lock_object&& other) noexcept : pointer(std::move(other.pointer)), lock(std::move(other.lock)) {}
            auto_lock_object(const T* _ptr, MutexType& mutex) noexcept : pointer(_ptr), lock(mutex) {}

            template<typename Arg>
            auto operator[](Arg&& argument)->decltype((*pointer)[argument])
            {
                return (*ptr)[argument];
            }

        };

        struct no_lock
        {
            no_lock(no_lock&&) {}
            template<typename AnonType>
            no_lock(AnonType&) {}
        };

        // null lock type, as in an object that will take mutex ctor but won't mutate it
        using auto_null_lock = auto_lock_object<no_lock>;

        T* get() const noexcept
        {
            return pointer.get();
        }

        MutexType* get_mutex_ptr() const noexcept
        {
            return mutexPointer.get();
        }

        template<typename...Args>
        void lock_shared() const noexcept
        {
            mutexPointer->lock_shared();
        }

        template<typename...Args>
        void unlock_shared() const noexcept
        {
            mutexPointer->unlock_shared();
        }

        void lock()
        {
            mutexPointer->lock();
        }

        void unlock()
        {
            mutexPointer->unlock();
        }

        friend struct link_safe_ptrs;
        template<typename, typename, typename, typename>
        friend class safe_object;
        template<typename AnonType>
        friend struct exclusive_locked_safe_ptr;
        template<typename AnonType>
        friend struct share_locked_safe_ptr;
        template<typename, typename, size_t, size_t>
        friend class lock_timed_transation;
        template<class...MutexTypes>
        friend class std::lock_guard;
        template<typename AnonMutexType>
        friend class std::shared_lock;


    public:

        template<typename...Args>
        safe_ptr(Args&&...args) : ptr(std::make_shared<T>(std::forward<Args>(args...))), mutexPointer(std::make_shared<MutexType>()) {}

        auto_lock<ExclusiveLockType> operator->() noexcept
        {
            return auto_lock<ExclusiveLockType>(pointer.get(), *mutexPointer);
        }

        auto_lock<ExclusiveLockType> operator*() noexcept
        {
            return auto_lock<ExclusiveLockType>(pointer.get(), *mutexPointer);
        }

        const auto_lock<SharedLockType> operator->() const noexcept
        {
            return auto_lock<SharedLockType>(pointer.get(), *mutexPointer);
        }

        const auto_lock<SharedLockType> operator*() const noexcept
        {
            return auto_lock<SharedLockType>(pointer.get(), *mutexPointer);
        }

    };

    template<typename T, typename MutexType = std::recursive_mutex, typename ExclusiveLockType = std::unique_lock<MutexType>, typename SharedLockType = std::unique_lock<MutexType>>
    class safe_object
    {
    protected:
        T object;
        mutable MutexType mutex;

        T* get_object_pointer() const noexcept
        {
            return const_cast<T*>(&object);
        }

        template<typename RequestedLock>
        using auto_lock = typename safe_ptr<T, MutexType, ExclusiveLockType, SharedLockType>::template auto_lock<RequestedLock>;
        template<typename RequestedLock>
        using auto_lock_object = typename safe_ptr<T, MutexType, ExclusiveLockType, SharedLockType>::template auto_lock_object<RequestedLock>;
        using auto_null_lock = typename safe_ptr<T, MutexType, ExclusiveLockType, SharedLockType>::auto_null_lock;

        template<typename AnonType>
        friend struct exclusive_locked_safe_ptr;
        template<typename AnonType>
        friend struct share_locked_safe_ptr;


    public:

        static_assert(std::is_default_constructible_v<T>, "T used in safe object must be default constructible!");

        template<typename...Args>
        safe_object(Args&&...args) : object(std::forward<Args>(args)...) {}

        safe_object(const safe_object& other) noexcept
        {
            std::lock_guard<MutexType> copy_lock(other.mutex);
            object = other.object;
        }

        safe_object& operator=(const safe_object& other) noexcept
        {
            std::lock_guard<MutexType> copy_lock(other.mutex);
            object = other.object;
        }

        auto_lock<ExclusiveLockType> operator->() noexcept
        {
            return auto_lock<ExclusiveLockType>(get_object_pointer(), mutex);
        }

        auto_lock<SharedLockType> operator->() const noexcept
        {
            return auto_lock<SharedLockType>(get_object_pointer(), mutex);
        }

        auto_lock<ExclusiveLockType> operator*() noexcept
        {
            return auto_lock<ExclusiveLockType>(get_object_pointer(), mutex);
        }

        auto_lock<SharedLockType> operator->() const noexcept
        {
            return auto_lock<SharedLockType>(get_object_pointer(), mutex);
        }

        using exclusive_lock_type = ExclusiveLockType;
        using shared_lock_type = SharedLockType;

    };

    template<typename T>
    struct exclusive_locked_safe_ptr
    {
        T& safeRef;
        typename T::exclusive_lock_type lock;
        exclusive_locked_safe_ptr(const T& p) : safeRef(*const_cast<T*>(&p))
    };

}

#endif //!FUCHSTRAUMER_SAFE_PTR_HPP
