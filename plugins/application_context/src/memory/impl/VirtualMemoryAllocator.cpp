#include "VirtualMemoryAllocator.hpp"
#include <foonathan/memory/virtual_memory.hpp>

VirtualMemoryAllocator::VirtualMemoryAllocator() : impl(new foonathan::memory::virtual_memory_allocator()), AllocatorImpl() {}

VirtualMemoryAllocator::~VirtualMemoryAllocator() {
    delete impl;
}

VirtualMemoryAllocator::VirtualMemoryAllocator(VirtualMemoryAllocator&& other) noexcept {
    impl = std::move(other.impl);
}

VirtualMemoryAllocator& VirtualMemoryAllocator::operator=(VirtualMemoryAllocator&& other) noexcept {
    impl = std::move(other.impl);
    return *this;
}

void* VirtualMemoryAllocator::Allocate(size_t len, size_t alignment) {
    return impl->allocate_node(len, alignment);
}

void VirtualMemoryAllocator::Deallocate(void* addr, size_t len) {
    impl->deallocate_node(addr, len, 0);
}
