#include "VirtualMemoryAllocator.hpp"
#include <foonathan/memory/virtual_memory.hpp>

VirtualMemoryAllocator::VirtualMemoryAllocator() : impl(new foonathan::memory::virtual_memory_allocator()) {}

VirtualMemoryAllocator::~VirtualMemoryAllocator() {
    delete impl;
}

VirtualMemoryAllocator::VirtualMemoryAllocator(VirtualMemoryAllocator&& other) noexcept {

}

void* VirtualMemoryAllocator::Allocate(size_t len, size_t alignment) {
    return impl->allocate_node(len, alignment);
}

void VirtualMemoryAllocator::Deallocate(void* addr, size_t len, size_t alignment) {
    impl->deallocate_node(addr, len, alignment);
}
