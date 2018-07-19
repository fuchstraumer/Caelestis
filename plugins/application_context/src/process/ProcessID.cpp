#include "process/ProcessID.hpp"
#ifdef _WIN32
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#elif defined __unix__

#elif defined __apple__

#endif

ProcessID::ProcessID(PlatformProcessID id) : pid(id) {}

PlatformProcessID ProcessID::ToPlatformID() const
{
    return PlatformProcessID();
}

int64_t ProcessID::ToSI64() const {
    return static_cast<int64_t>(pid);
}

uint32_t ProcessID::ToUI32() const {
    return static_cast<uint32_t>(pid);
}

bool ProcessID::operator==(const ProcessID & other) const noexcept {
    return pid == other.pid;
}

bool ProcessID::operator!=(const ProcessID & other) const noexcept {
    return pid != other.pid;
}

bool ProcessID::operator<(const ProcessID & other) const noexcept {
    return pid < other.pid;
}

ProcessID ProcessID::GetCurrent() {
#ifdef _WIN32
    return ProcessID(GetCurrentProcessId());
#else
    return ProcessID();
#endif
}

ProcessID ProcessID::FromNative(PlatformProcessID _id) {
    return ProcessID();
}
