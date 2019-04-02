#pragma once
#ifndef FUCHSTRAUMER_SAFE_PTR_HPP
#define FUCHSTRAUMER_SAFE_PTR_HPP
#include <mutex>
#include <atomic>
#include <memory>

template<typename T, typename MutexType = std::recursive_mutex, typename ExclusiveLockType = std::unique_lock<MutexType>,
    typename SharedLockType = std::unique_lock<MutexType>>
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
    template<typename size_t, size_t>
    friend class lock_timed_transation;
    template<typename...MutexTypes>
    friend class std::lock_guard;

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

#endif //!FUCHSTRAUMER_SAFE_PTR_HPP
