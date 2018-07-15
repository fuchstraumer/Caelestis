#include "process/ProcessID.hpp"

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
    return ProcessID();
}

ProcessID ProcessID::FromNative(PlatformProcessID _id) {
    return ProcessID();
}
