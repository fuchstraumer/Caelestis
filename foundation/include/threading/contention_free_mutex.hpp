#pragma once
#ifndef FOUNDATION_CONTENTION_FREE_MUTEX_HPP
#define FOUNDATION_CONTENTION_FREE_MUTEX_HPP
#include <cstddef> // size_t
#include <atomic>
#include <mutex>
#include <memory>

template<typename size_t ContentionFreeCount = 20u, bool SharedFlag = false>
struct contention_free_mutex
{
private:
    std::atomic_bool wantExclusiveLock{ false };

    struct contention_free_flag
    {
        contention_free_flag() noexcept { value = 0; }
        alignas(std::hardware_destructive_interference_size) std::atomic<size_t> value;
    };

public:
};

#endif //!FOUNDATION_CONTENTION_FREE_MUTEX_HPP
