#include "PoolAllocator.hpp"
#include <foonathan/memory/memory_pool.hpp>
#include <foonathan/memory/memory_pool_collection.hpp>
#include <cassert>

struct FixedSizePoolAllocatorImpl {
    FixedSizePoolAllocatorImpl(size_t node_sz, size_t page_sz) : pool(node_sz, page_sz) {}
    foonathan::memory::memory_pool<> pool;
};

FixedSizePoolAllocator::FixedSizePoolAllocator(size_t node_size, size_t page_size) : AllocatorImpl(),
    impl(new FixedSizePoolAllocatorImpl(node_size, page_size)) {}

FixedSizePoolAllocator::~FixedSizePoolAllocator() {
    delete impl;
}

void* FixedSizePoolAllocator::Allocate(size_t len, size_t align) {
    assert(len <= impl->pool.node_size());
    return impl->pool.allocate_node();
}

void FixedSizePoolAllocator::Deallocate(void* addr, size_t len) {
    return impl->pool.deallocate_node(addr);
}

size_t FixedSizePoolAllocator::NodeSize() const noexcept {
    return impl->pool.node_size();
}

struct VariableSizePoolAllocatorImpl {
    VariableSizePoolAllocatorImpl(size_t node_sz, size_t page_sz) : pool(node_sz, page_sz) {}
    foonathan::memory::memory_pool_collection<foonathan::memory::node_pool, foonathan::memory::log2_buckets> pool;
};

VariableSizePoolAllocator::VariableSizePoolAllocator(size_t node_size, size_t page_size) : AllocatorImpl(),
    impl(new VariableSizePoolAllocatorImpl(node_size, page_size)) {}

VariableSizePoolAllocator::~VariableSizePoolAllocator() {
    delete impl;
}

VariableSizePoolAllocator::VariableSizePoolAllocator(VariableSizePoolAllocator&& other) noexcept : impl(std::move(other.impl)) {}

void* VariableSizePoolAllocator::Allocate(size_t len, size_t align) {
    assert(len <= impl->pool.max_node_size());
    return impl->pool.allocate_node(len);
}

void VariableSizePoolAllocator::Deallocate(void* addr, size_t len) {
    return impl->pool.deallocate_node(addr, len);
}

size_t VariableSizePoolAllocator::NodeSize() const noexcept {
    return impl->pool.max_node_size();
}
