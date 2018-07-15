#pragma once
#ifndef SECURE_MEMORY_WIN32_HELPERS_HPP
#define SECURE_MEMORY_WIN32_HELPERS_HPP
#include <mutex>
#include <algorithm>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>

void enablePrivilege(const char* name) {
    LUID luid;
    if (!LookupPrivilegeValue(nullptr, (LPCTSTR)name, &luid)) {
        std::cerr << "Failed to lookup privilege value in SecureAlloc system\n";
        return;
    }

    HANDLE access_token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &access_token)) {
        std::cerr << "Failed to open process token for adjusting privileges\n";
        return;
    }

    TOKEN_PRIVILEGES privileges{ 0 };

    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Luid = luid;
    privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(access_token, false, &privileges, sizeof(privileges), nullptr, nullptr)) {
        std::cerr << "Failed to adjust token privileges.\n";
        CloseHandle(access_token);
        return;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        std::cerr << "Failed to adjust token privileges\n";
    }

    CloseHandle(access_token);
}

static std::mutex workingSizeMutex;

void growWorkingSize(size_t len) {
    size_t min_size;
    size_t max_size;

    std::lock_guard<std::mutex> size_lock(workingSizeMutex);

    if (!GetProcessWorkingSetSize(GetCurrentProcess(), &min_size, &max_size)) {
        std::cerr << "Couldn't access process working set size!\n";
        throw std::runtime_error("Severe error in retrieval or process working set size!");
    }

    max_size = std::max(min_size + len, max_size);

    if (!SetProcessWorkingSetSizeEx(GetCurrentProcess(), min_size + len, max_size, QUOTA_LIMITS_HARDWS_MIN_ENABLE | QUOTA_LIMITS_HARDWS_MAX_DISABLE)) {
        throw std::runtime_error("Failed to adjust process working set size!");
    }
}

void* SysAlloc(size_t len) {
    void* ptr = VirtualAlloc(nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!ptr) {
        std::cerr << "Failed to allocate page for secure memory in secure allocator!\n";
        return nullptr;
    }

    if (VirtualLock(ptr, len) == 0) {
        DWORD gle = GetLastError();

        if (gle == ERROR_WORKING_SET_QUOTA) {
            growWorkingSize(len);

            if (VirtualLock(ptr, len) != 0) {
                return ptr;
            }
        }

        throw std::runtime_error("Failed to VirtualLock fresh memory page for SecureAlloc");
    }

    return ptr;
}

void SysDealloc(void* addr, size_t len) {
    if (VirtualUnlock(addr, len) == 0) {
        throw std::runtime_error("Failed to VirtualUnlock memory page for SecureDealloc");
    }

    if (VirtualFree(addr, len, MEM_RELEASE) == 0) {
        throw std::runtime_error("Failed to VirtualFree memory page for SecureDealloc");
    }
}

void PlatformSecureZeroMemory(void* addr, size_t len) {
    if (!addr) {
        return;
    }

    SecureZeroMemory(addr, len);
}

// Used to make sure privilege is set first time secure memory constructor is called
struct AllocationContext {
    AllocationContext() {
        enablePrivilege(SE_INC_WORKING_SET_NAME);
    }
};

#endif //!SECURE_MEMORY_WIN32_HELPERS_HPP
