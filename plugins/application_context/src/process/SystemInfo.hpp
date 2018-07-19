#pragma once
#ifndef SYSTEM_INFO_HPP
#define SYSTEM_INFO_HPP
#include <string>
#include <vector>

struct SystemInfo {
    std::string osFamily;
    std::string osName;
    std::string osVersion;
    size_t AddressSize;
    size_t TotalMemorySize;
    size_t NumCores;
    size_t PageSize;
    std::string CpuArchitecture;
    std::vector<std::string> ExtraSysInfo;
};

#endif //!SYSTEM_INFO_HPP
