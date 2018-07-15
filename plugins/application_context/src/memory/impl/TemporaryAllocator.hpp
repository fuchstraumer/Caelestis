#pragma once
#ifndef APPLICATION_CONTEXT_TEMPORARY_ALLOCATOR_IMPL_HPP
#define APPLICATION_CONTEXT_TEMPORARY_ALLOCATOR_IMPL_HPP
#include "AllocatorImpl.hpp"

namespace foonathan {
    namespace memory {
        class temporary_allocator;
    }
}

class TemporaryAllocator : public AllocatorImpl {
public:

    TemporaryAllocator();
    ~TemporaryAllocator();

    void* Allocate(size_t len, size_t align) final;
    void Deallocate(void* addr, size_t len) final;
private:
    foonathan::memory::temporary_allocator* impl;
};

#endif //!APPLICATION_CONTEXT_TEMPORARY_ALLOCATOR_IMPL_HPP
