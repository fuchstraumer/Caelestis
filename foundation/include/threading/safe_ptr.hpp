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

        template<typename Arg>
        auto operator[](Arg argument)->decltype((*ptr)[argument])
        {
            return (*ptr)[argument];
        }

    };

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
    template<class...MutexTypes>
    friend class std::lock_guard;

public:

    template<typename...Args>
    safe_ptr(Args&&...args) : ptr(std::make_shared<T>(std::forward<Args>(args...))), mutexPointer(std::make_shared<MutexType>()) {}

    auto_lock<ExclusiveLockType> operator->() noexcept
    {
        return auto_lock<ExclusiveLockType>(ptr.get(), *mutexPointer);
    }

    auto_lock<ExclusiveLockType> operator*() noexcept
    {
        return auto_lock<ExclusiveLockType>(ptr.get(), *mutexPointer);
    }

    const auto_lock<SharedLockType> operator->() noexcept
    {
        return auto_lock<SharedLockType>(ptr.get(), *mutexPointer);
    }

    const auto_lock<SharedLockType> operator*() noexcept
    {
        return auto_lock<SharedLockType>(ptr.get(), *mutexPointer);
    }

};

#endif //!FUCHSTRAUMER_SAFE_PTR_HPP
