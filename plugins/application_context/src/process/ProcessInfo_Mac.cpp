#include "process/ProcessInfo.hpp"
#include "SystemInfo.hpp"
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/mach_traps.h>
#include <mach/task.h>
#include <mach/task_info.h>
#include <mach/vm_map.h>
#include <mach/vm_statistics.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/types.h>

ProcessInfo::ProcessInfo(const ProcessID& id) : pid(id), sysInfo(new SystemInfo()) {

}

ProcessInfo::~ProcessInfo() {
    if (sysInfo) {
        delete sysInfo;
    }
}

size_t ProcessInfo::VirtualMemorySize() const {
    return 0;
}

size_t ProcessInfo::ResidentSize() const {
    return 0;
}

double ProcessInfo::GetMemoryPressure() const {
    return 0.0;
}

size_t ProcessInfo::GetTotalPhysicalMemory() const {
    return 0;
}

size_t ProcessInfo::GetAvailPhysicalMemory() const {
    return 0;
}

size_t ProcessInfo::GetCommittedPhysicalMemory() const {
    return 0;
}

size_t ProcessInfo::GetTotalCommittedMemoryLimit() const {
    return 0;
}

size_t ProcessInfo::GetCurrentCommitLimit() const {
    return 0;
}

size_t ProcessInfo::GetPageSize() {
    return 0;
}
