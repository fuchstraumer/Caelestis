#include "SecureAllocator.hpp"
#ifdef _WIN32
#include "SecureMemoryWin32.hpp"
#else
#include "SecureMemoryUnix.hpp"
#endif
#include "process/ProcessInfo.hpp"
#include <unordered_map>
#include <memory>
#include <cassert>

class SecureAllocation {
    SecureAllocation(const SecureAllocation&) = delete;
    SecureAllocation& operator=(const SecureAllocation&) = delete;
public:
    explicit SecureAllocation(size_t initial_size);
    ~SecureAllocation();

    void* Allocate(size_t len, size_t align);
private:
    void* start;
    void* ptr;
    size_t size;
    size_t remaining;
};

SecureAllocation::SecureAllocation(size_t len) {
    static AllocationContext context;
    size_t page_size = ProcessInfo::GetPageSize();
    size_t remainder = len & page_size;
    remaining = remainder ? (len + page_size - remainder) : len;
    size = remaining;
    ptr = SysAlloc(len);
    start = ptr;
}

SecureAllocation::~SecureAllocation() {
    SysDealloc(start, size);
}

void* SecureAllocation::Allocate(size_t len, size_t align) {
    if (std::align(align, len, ptr, remaining)) {
        void* result = ptr;
        ptr = static_cast<char*>(ptr) + len;
        remaining -= len;
        return result;
    }
    return nullptr;
}

static std::mutex allocMutex;
// Ref count of shared_ptr is controlled by number of addresses using it
static std::unordered_map<void*, std::shared_ptr<SecureAllocation>> secureTable;
static std::shared_ptr<SecureAllocation> lastAlloc = nullptr;

void* SecureAllocate(size_t len, size_t align_of) {
    std::lock_guard<std::mutex> guard(allocMutex);
    if (lastAlloc) {
        void* result = lastAlloc->Allocate(len, align_of);
        if (result) {
            secureTable[result] = lastAlloc;
            return result;
        }
    }

    lastAlloc = std::make_shared<SecureAllocation>(len);
    void* result = lastAlloc->Allocate(len, align_of);
    secureTable[result] = lastAlloc;
    return result;
}

int SecureDeallocate(void* addr, size_t len) {
    PlatformSecureZeroMemory(addr, len);
    std::lock_guard<std::mutex> guard(allocMutex);
    secureTable.erase(addr);
    return 1;
}

SecureAllocator::SecureAllocator() {}

SecureAllocator::~SecureAllocator() {}

void* SecureAllocator::Allocate(size_t len, size_t align) {
    return SecureAllocate(len, align);
}

void SecureAllocator::Deallocate(void * addr, size_t len) {
#ifndef NDEBUG
    const int result = SecureDeallocate(addr, len);
    assert(result);
#else
    SecureDeallocate(addr, len);
#endif // !NDEBUG
}
