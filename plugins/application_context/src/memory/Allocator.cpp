#include "memory/Allocator.hpp"
#include "impl/AllocatorImpl.hpp"
#include "impl/HeapAllocator.hpp"
#include "impl/PoolAllocator.hpp"
#include "impl/VirtualMemoryAllocator.hpp"
#include "impl/SecureAllocator.hpp"
#include "impl/TrackedAllocator.hpp"
#include "impl/TemporaryAllocator.hpp"
#include <stdexcept>

template<typename T>
T* struct_ptr_cast(void* ptr) {
    return reinterpret_cast<T*>(ptr);
}

Allocator::Allocator(const AllocatorCreateInfo* create_info) : allocatorType(create_info->AllocatorType) {

    auto create_fixed_pool_alloc = [this, create_info]() {
        PoolAllocatorCreateInfo* pool_info = reinterpret_cast<PoolAllocatorCreateInfo*>(create_info->pNext);
        impl = new FixedSizePoolAllocator(pool_info->NodeSize, pool_info->PageSize);
    };
    
    auto create_variable_pool_alloc = [this, create_info]() {
        PoolAllocatorCreateInfo* pool_info = reinterpret_cast<PoolAllocatorCreateInfo*>(create_info->pNext);
        impl = new VariableSizePoolAllocator(pool_info->NodeSize, pool_info->PageSize);
    };

    switch (allocatorType) {
    case allocator_trait_flags::HeapAllocator:
        impl = new HeapAllocator();
        break;
    case allocator_trait_flags::FixedSizePoolAllocator:
        create_fixed_pool_alloc();
        break;
    case allocator_trait_flags::VariableSizePoolAllocator:
        create_variable_pool_alloc();
        break;
    case allocator_trait_flags::VirtualMemoryAllocator:
        impl = new VirtualMemoryAllocator();
        break;
    case allocator_trait_flags::SecureAllocator:
        impl = new SecureAllocator();
        break;
    case allocator_trait_flags::TemporaryAllocator:
        impl = new TemporaryAllocator();
        break;
    default:
        throw std::domain_error("Invalid allocator type");
    }
}

Allocator::~Allocator() {
    // Virtual destructor, should dispatch to right one?
    delete struct_ptr_cast<AllocatorImpl>(impl);
}

void* Allocator::Allocate(size_t len, size_t align) {
    return struct_ptr_cast<AllocatorImpl>(impl)->Allocate(len, align);
}

void Allocator::Deallocate(void* addr, size_t len) {
    struct_ptr_cast<AllocatorImpl>(impl)->Deallocate(addr, len);
}