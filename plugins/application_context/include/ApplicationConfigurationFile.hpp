#pragma once
#ifndef APPLICATION_CONFIGURATION_FILE_HPP
#define APPLICATION_CONFIGURATION_FILE_HPP
#include <unordered_map>
#include <vector>
#include <string>

class ApplicationConfigurationFile {
public:

    ApplicationConfigurationFile() = default;
    ApplicationConfigurationFile(const char* json_cfg_path);

    const std::vector<std::string>& GetRequiredModules() const noexcept;
    const char* GetModuleConfigFile(const char* module_name);

private:
    std::vector<std::string> requiredModules;
    std::unordered_map<std::string, std::string> moduleConfigFiles;    
};

#endif //!APPLICATION_CONFIGURATION_FILE_HPP
