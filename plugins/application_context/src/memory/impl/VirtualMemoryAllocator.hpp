#pragma once
#ifndef APPLICATION_CONTEXT_VIRTUAL_MEMORY_ALLOCATOR_HPP
#define APPLICATION_CONTEXT_VIRTUAL_MEMORY_ALLOCATOR_HPP
#include "AllocatorImpl.hpp"
namespace foonathan {
    namespace memory {
        class virtual_memory_allocator;
    }
}

class VirtualMemoryAllocator : public AllocatorImpl {
public:
    VirtualMemoryAllocator();
    ~VirtualMemoryAllocator();
    VirtualMemoryAllocator(VirtualMemoryAllocator&& other) noexcept;
    VirtualMemoryAllocator& operator=(VirtualMemoryAllocator&& other) noexcept;

    void* Allocate(size_t len, size_t alignment = 0) final;
    void Deallocate(void* addr, size_t len) final;

private:
    foonathan::memory::virtual_memory_allocator *impl;
};


#endif //!APPLICATION_CONTEXT_VIRTUAL_MEMORY_ALLOC_HPP
