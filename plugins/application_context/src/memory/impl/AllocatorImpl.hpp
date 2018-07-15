#pragma once
#ifndef APPLICATION_CONTEXT_BASE_ALLOCATOR_IMPL_HPP
#define APPLICATION_CONTEXT_BASE_ALLOCATOR_IMPL_HPP

struct AllocatorImpl {
    AllocatorImpl() = default;
    virtual ~AllocatorImpl() = default;
    AllocatorImpl(AllocatorImpl&& other) noexcept {}
    AllocatorImpl& operator=(AllocatorImpl&& other) noexcept { return *this; }
    virtual void* Allocate(size_t len, size_t align) = 0;
    virtual void Deallocate(void* addr, size_t len) = 0;
};

#endif //!APPLICATION_CONTEXT_BASE_ALLOCATOR_IMPL_HPP