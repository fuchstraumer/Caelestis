#pragma once
#ifndef SYSTEM_INFO_HPP
#define SYSTEM_INFO_HPP
#include <string>
#include <vector>

struct SystemInfo {
    std::string osFamily{ "" };
    std::string osName{ "" };
    std::string osVersion{ "" };
    size_t AddressSize{ 64 };
    size_t TotalMemorySize{ 0 };
    size_t NumCores{ 0 };
    size_t PageSize{ 0 };
    std::string CpuArchitecture{ "" };
    std::vector<std::string> ExtraSysInfo;
};

#endif //!SYSTEM_INFO_HPP
