#include "process/ProcessInfo.hpp"
#include "SystemInfo.hpp"
#include "easylogging++.h"
#define WIN32_MEAN_AND_LEAN
#include <Psapi.h>
#include <windows.h>
#pragma comment(lib, "Windowsapp.lib")

struct PsAPI_INIT {

    bool supported;
    typedef BOOL(WINAPI* pQueryWorkingSetEx)(HANDLE hProcess, PVOID pv, DWORD cb);
    pQueryWorkingSetEx QueryWorkingSetExFn = nullptr;

    static PsAPI_INIT& GetPsApi() {
        static PsAPI_INIT ps_api;
        return ps_api;
    }

private:

    PsAPI_INIT() {
        HINSTANCE psapi_lib = LoadLibrary(TEXT("psapi.dll"));
        if (psapi_lib) {
            QueryWorkingSetExFn = reinterpret_cast<pQueryWorkingSetEx>(GetProcAddress(psapi_lib, "QueryWorkingSetEx"));
            if (QueryWorkingSetExFn) {
                supported = true;
            }
        }
        else {
            supported = false;
        }
    }

};

MEMORYSTATUSEX GetMemoryStatus() {
    MEMORYSTATUSEX mse;
    mse.dwLength = sizeof(mse);
    BOOL status = GlobalMemoryStatusEx(&mse);
    if (!status) {
        static bool failure_logged = false;
        LOG_IF(!failure_logged, ERROR) << "Failed to get MEMORYSTATUSEX!";
        failure_logged = true;
    }
    return mse;
}

PROCESS_MEMORY_COUNTERS GetProcessMemoryCounters() {
    PROCESS_MEMORY_COUNTERS pmc;
    BOOL status = GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    if (!status) {
        static bool failure_logged = false;
        LOG_IF(!failure_logged, ERROR) << "Failed to get process memory counters!";
        failure_logged = true;
    }
    return pmc;
}

int _wconvertmtos(SIZE_T sz) {
    return static_cast<int>(sz / (1024 * 1024));
}

ProcessInfo::ProcessInfo(const ProcessID& id) : sysInfo(new SystemInfo()) {
    auto& pse_api = PsAPI_INIT::GetPsApi();
}

ProcessInfo::~ProcessInfo() {
    if (sysInfo) {
        delete sysInfo;
    }
}

size_t ProcessInfo::VirtualMemorySize() const {
    auto mse = GetMemoryStatus();
    DWORDLONG x = mse.ullTotalVirtual - mse.ullAvailVirtual;
    return static_cast<size_t>(x);
}

size_t ProcessInfo::ResidentSize() const {
    auto pmc = GetProcessMemoryCounters();
    return static_cast<size_t>(_wconvertmtos(pmc.WorkingSetSize));
}

double ProcessInfo::GetMemoryPressure() const {
    auto mse = GetMemoryStatus();
    DWORDLONG total_page_file = mse.ullTotalPageFile;
    if (total_page_file == 0) {
        return 0.0;
    }

    DWORDLONG high_watermark = total_page_file / DWORDLONG(2);
    DWORDLONG very_high_watermark = DWORDLONG(3) * (total_page_file / DWORDLONG(4));
    DWORDLONG used_page_file = mse.ullTotalPageFile - mse.ullAvailPageFile;

    if (used_page_file < high_watermark || very_high_watermark <= high_watermark) {
        return 0.0;
    }

    return static_cast<double>(used_page_file - high_watermark) / static_cast<double>(very_high_watermark - high_watermark);
}

size_t ProcessInfo::GetTotalPhysicalMemory() const {
    auto mse = GetMemoryStatus();
    return static_cast<size_t>(mse.ullTotalPhys);
}

size_t ProcessInfo::GetAvailPhysicalMemory() const {
    auto mse = GetMemoryStatus();
    return static_cast<size_t>(mse.ullAvailPhys);
}

size_t ProcessInfo::GetCommittedPhysicalMemory() const {
    auto mse = GetMemoryStatus();
    return static_cast<size_t>(mse.ullTotalPhys - mse.ullAvailPhys);
}

size_t ProcessInfo::GetTotalCommittedMemoryLimit() const {
    auto mse = GetMemoryStatus();
    return static_cast<size_t>(mse.ullTotalPageFile);
}

size_t ProcessInfo::GetCurrentCommitLimit() const {
    auto mse = GetMemoryStatus();
    return static_cast<size_t>(mse.ullAvailPageFile);
}

size_t ProcessInfo::GetPageSize() {
    SYSTEM_INFO sys_info;
    GetNativeSystemInfo(&sys_info);
    return static_cast<size_t>(sys_info.dwPageSize);
}
