#pragma once
#ifndef APPLICATION_CONTEXT_ALLOCATOR_BASE_HPP
#define APPLICATION_CONTEXT_ALLOCATOR_BASE_HPP
#include <cstdint>

namespace allocator_trait_flags {
    enum e : uint32_t {
        NoFlags = 0,
        // Predefined types
        // Minimum interface to malloc() or OS-specific heap allocation functions, ultimately
        HeapAllocator = 1,
        // Allocates blocks of memory split into fixed node-size chunks. Allocations must be <= node_size
        FixedSizePoolAllocator = 2,
        // Creates a number of fixed pools and assigns allocate() requests to a pool for best-fit
        VariableSizePoolAllocator = 3,
        // Interfaces to OS-specific virtual memory system
        VirtualMemoryAllocator = 4,
        // Secure memory is kept in RAM, and is flushed to zero when deallocated
        SecureAllocator = 5,
        // Memory arenas are flushed all at once
        ArenaAllocator = 6,
        // Uses a stack of memory blocks, allocating merely moves pointer to "active" block
        BlockStackAllocator = 7,
        // Optimized for temporary transient allocations
        TemporaryAllocator = 8,
        // Guaranteed thread-safe allocator
        ThreadSafeAllocator = 9,
        // Flags that modify behavior of above
        ArrayOptimized = 0x0000ffff,
        UsesBlocks,
        UsesMemoryArena,
        Tracked,
        Aligned,
        ThreadSafe,
        FixedSize,
        StaticStorage,
        JointMemory
    };
}

struct AllocatorCreateInfo {
    uint32_t AllocatorType;
    uint32_t AllocatorFlags;
    void* pNext;
};

struct PoolAllocatorCreateInfo {
    uint32_t NodeSize;
    uint32_t PageSize;
};

struct TrackedAllocatorCallbacks {
    void (*OnNodeAllocation)(void* addr, size_t len, size_t align);
    void (*OnArrayAllocation)(void* addr, size_t count, size_t len, size_t align);
    void (*OnNodeDeallocation)(void* addr, size_t len, size_t align);
    void (*OnArrayDeallocation)(void* addr, size_t count, size_t len, size_t align);
};

class Allocator {
    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
public:
    Allocator(const AllocatorCreateInfo* create_info);
    ~Allocator();

    void* Allocate(size_t len, size_t align);
    void Deallocate(void* addr, size_t len);

private:
    const uint32_t allocatorType;
    void* impl;
};

#endif //!APPLICATION_CONTEXT_ALLOCATOR_BASE_HPP