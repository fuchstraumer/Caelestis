#pragma once
#ifndef APPLICATION_CONTEXT_PROCESS_INFO_HPP
#define APPLICATION_CONTEXT_PROCESS_INFO_HPP
#include <cstdint>
#include "ProcessID.hpp"

class ProcessInfo {
public:

    ProcessInfo(const ProcessID& id);
    ~ProcessInfo();
    ProcessInfo(const ProcessInfo&) = delete;
    ProcessInfo& operator=(const ProcessInfo&) = delete;

    size_t VirtualMemorySize() const;
    size_t ResidentSize() const;
    double GetMemoryPressure() const;
    size_t GetTotalPhysicalMemory() const;
    size_t GetAvailPhysicalMemory() const;
    size_t GetCommittedPhysicalMemory() const;
    size_t GetTotalCommittedMemoryLimit() const;
    size_t GetCurrentCommitLimit() const;

    static size_t GetPageSize();
    /*
    static const char* GetOsFamily();
    static const char* GetOsName();
    static const char* GetOsVersion();
    static size_t GetTotalSystemMemoryMB();
    static size_t GetNumCores();
    // Gets number of cores assigned to current process.
    static size_t GetAssignedNumCores();
    static size_t GetSystemPageSizeMB();
    static void WriteExtraSysInfo(size_t* num_info_strings, char* const* strings);
    */

private:

    ProcessID pid;
    struct SystemInfo* sysInfo;

};

#endif //!APPLICATION_CONTEXT_PROCESS_INFO_HPP