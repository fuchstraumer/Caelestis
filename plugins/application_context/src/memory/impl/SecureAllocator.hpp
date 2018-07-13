#pragma once
#ifndef APPLICATION_CONTEXT_SECURE_MEMORY_HPP
#define APPLICATION_CONTEXT_SECURE_MEMORY_HPP

/*
    Secure memory is never written to disk, and is put into protected
    caches with elevated permission requirements on Windows. This memory
    is explicitly zeroed on free, too, so it should be hard to extract
    secure information stored here.
*/

class SecureAllocator {
    SecureAllocator(const SecureAllocator&) = delete;
    SecureAllocator& operator=(const SecureAllocator&) = delete;
public:

    SecureAllocator();
    ~SecureAllocator();

    void* Allocate(size_t len, size_t align);
    void Deallocate(void* addr, size_t len);

private:
};

#endif //!APPLICATION_CONTEXT_SECURE_MEMORY_HPP
