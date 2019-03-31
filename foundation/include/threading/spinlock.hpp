#pragma once
#ifndef FUCHSTRAUMER_SPIN_LOCK_HPP
#define FUCHSTRAUMER_SPIN_LOCK_HPP
#include <atomic>

struct spin_lock
{
    spin_lock() = default;
    ~spin_lock() = default;

    bool try_lock() noexcept
    {
        return !flag.test_and_set(std::memory_order::memory_order_acquire);
    }

    void lock() noexcept
    {
        while (try_lock())
        {

        }
    }

    void unlock() noexcept
    {
        flag.clear(std::memory_order::memory_order_release);
    }

private:
    std::atomic_flag flag{ 0 };
};

struct spin_lock_guard
{
    spin_lock_guard(spin_lock& lock) noexcept : ref(lock)
    {
        ref.lock();
    }

    ~spin_lock_guard() noexcept
    {
        ref.unlock();
    }

    spin_lock_guard(const spin_lock_guard&) = delete;
    spin_lock_guard(spin_lock_guard&& other) = delete;
    spin_lock_guard& operator=(const spin_lock_guard&) = delete;
    spin_lock_guard& operator=(spin_lock_guard&& other) = delete;

private:
    spin_lock& ref;
};

#endif //!FUCHSTRAUMER_SPIN_LOCK_HPP
