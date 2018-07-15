#include "TemporaryAllocator.hpp"
#include <foonathan/memory/temporary_allocator.hpp>
#include <iostream>

TemporaryAllocator::TemporaryAllocator() : impl(new foonathan::memory::temporary_allocator()) {}

TemporaryAllocator::~TemporaryAllocator() {
    impl->shrink_to_fit();
    delete impl;
}

void* TemporaryAllocator::Allocate(size_t len, size_t align) {
    return impl->allocate(len, align);
}

void TemporaryAllocator::Deallocate(void* addr, size_t len) {
    std::cerr << "Called deallocate on a temporary_allocator: this has no effect!\n";
}
