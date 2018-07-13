#pragma once
#ifndef APPLICATION_CONTEXT_POOL_ALLOCATOR_HPP
#define APPLICATION_CONTEXT_POOL_ALLOCATOR_HPP
#include "AllocatorImpl.hpp"
/*
    Pool allocator which keeps large pages split into node_size chunks.
    "len" in allocate must be equal to or less than the node_size
*/
struct FixedSizePoolAllocatorImpl;

class FixedSizePoolAllocator : public AllocatorImpl {
    FixedSizePoolAllocator(const FixedSizePoolAllocator&) = delete;
    FixedSizePoolAllocator& operator=(const FixedSizePoolAllocator&) = delete;
public:

    FixedSizePoolAllocator(size_t node_size, size_t page_size);
    ~FixedSizePoolAllocator();
    FixedSizePoolAllocator(FixedSizePoolAllocator&& other) noexcept;
    FixedSizePoolAllocator& operator=(FixedSizePoolAllocator&& other) noexcept;

    // Returns address of memory of node_size
    void* Allocate(size_t len, size_t align = 0) final;
    // Returns node to the pool
    void Deallocate(void* addr, size_t len = 0) final;

    size_t NodeSize() const noexcept;

private:
    FixedSizePoolAllocatorImpl* impl{ nullptr };
};

/*
    Keeps a number of memory pools, of sizes that are a log2 
    of the node_size used to initialize this pool. Helps reduce
    waste of nodes when used memory is smaller than the node size.
*/
struct VariableSizePoolAllocatorImpl;

class VariableSizePoolAllocator : public AllocatorImpl {
    VariableSizePoolAllocator(const VariableSizePoolAllocator&) = delete;
    VariableSizePoolAllocator& operator=(const VariableSizePoolAllocator&) = delete;
public:

    VariableSizePoolAllocator(size_t node_size, size_t page_size);
    ~VariableSizePoolAllocator();
    VariableSizePoolAllocator(VariableSizePoolAllocator&& other) noexcept;
    VariableSizePoolAllocator& operator=(VariableSizePoolAllocator&& other) noexcept;

    // Returns address of memory of node_size
    void* Allocate(size_t len, size_t align = 0) final;
    // Returns node to the pool
    void Deallocate(void* addr, size_t len = 0) final;
    // For variable size pool, returns maximum node size
    size_t NodeSize() const noexcept;

private:
    VariableSizePoolAllocatorImpl* impl;
};

#endif //!APPLICATION_CONTEXT_POOL_ALLOCATOR_HPP
