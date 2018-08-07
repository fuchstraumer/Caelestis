#pragma once
#ifndef APPLICATION_CONTEXT_PROCESS_ID_HPP
#define APPLICATION_CONTEXT_PROCESS_ID_HPP
#include <cstdint>

#ifdef _WIN32
typedef unsigned long DWORD;
typedef DWORD PlatformProcessID;
#else
#include <sys/types.h>
#include <unistd.h>
typedef pid_t PlatformProcessID;
#endif

struct ProcessID {
    ProcessID() = default;
    ~ProcessID() = default;

    PlatformProcessID ToPlatformID() const;
    int64_t ToSI64() const;
    uint32_t ToUI32() const;

    bool operator==(const ProcessID& other) const noexcept;
    bool operator!=(const ProcessID& other) const noexcept;
    bool operator<(const ProcessID& other) const noexcept;

    static ProcessID GetCurrent();
    static ProcessID FromNative(PlatformProcessID _id);

private:
    explicit ProcessID(PlatformProcessID _id);
    PlatformProcessID pid{ 0 };
};

#endif //!APPLICATION_CONTEXT_PROCESS_ID_HPP
