#include "HeapAllocator.hpp"
#include <foonathan/memory/heap_allocator.hpp>

HeapAllocator::HeapAllocator() : AllocatorImpl() {}

HeapAllocator::~HeapAllocator() {}

void* HeapAllocator::Allocate(size_t len, size_t align) {
    return foonathan::memory::heap_alloc(len);
}

void HeapAllocator::Deallocate(void* addr, size_t len) {
    foonathan::memory::heap_dealloc(addr, len);
}
