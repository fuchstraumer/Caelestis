#pragma once
#ifndef APPLICATION_CONTEXT_HEAP_ALLOCATOR_HPP
#define APPLICATION_CONTEXT_HEAP_ALLOCATOR_HPP
#include "AllocatorImpl.hpp"

class HeapAllocator : AllocatorImpl {
public:

    HeapAllocator();
    ~HeapAllocator();
    HeapAllocator(HeapAllocator&& other) noexcept;
    HeapAllocator& operator=(HeapAllocator&& other) noexcept;

    void* Allocate(size_t len, size_t align) final;
    void Deallocate(void* addr, size_t len) final;

};

#endif //!APPLICATION_CONTEXT_HEAP_ALLOCATOR_HPP
