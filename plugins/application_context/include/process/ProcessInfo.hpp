#pragma once
#ifndef APPLICATION_CONTEXT_PROCESS_INFO_HPP
#define APPLICATION_CONTEXT_PROCESS_INFO_HPP
#include <cstdint>

struct ProcessID;

class ProcessInfo {
public:

    ProcessInfo(const ProcessID& id);
    ~ProcessInfo();

    size_t VirtualMemorySize() const;
    size_t ResidentSize() const;
    static size_t PageSize();

};

#endif //!APPLICATION_CONTEXT_PROCESS_INFO_HPP