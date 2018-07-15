#include "process/ProcessInfo.hpp"

ProcessInfo::ProcessInfo(const ProcessID & id)
{
}

ProcessInfo::~ProcessInfo()
{
}

size_t ProcessInfo::VirtualMemorySize() const {
    return 0;
}

size_t ProcessInfo::ResidentSize() const {
    return 0;
}

size_t ProcessInfo::PageSize() {
    return 0;
}
