#include "ApplicationConfigurationFile.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

ApplicationConfigurationFile::ApplicationConfigurationFile(const char* json_cfg_path) {
    std::ifstream input_file(json_cfg_path);
    if (!input_file.is_open()) {
        throw std::runtime_error("Failed to open input json configuration file!");
    }
    nlohmann::json json_file;
    input_file >> json_file;

    auto json_iter = json_file.find("RequiredModules");
    if (json_iter != json_file.end()) {
        nlohmann::json required_json = *json_iter;
        for (auto& str : required_json) {
            if (str.is_string()) {
                requiredModules.emplace_back(str);
            }
            else {
                throw std::runtime_error("Non-string value specified in JSON configuration file.");
            }
        }
    }
    else {
        std::cerr << "Warning: No required modules specified in ApplicationConfig.json!";
    }

    json_iter = json_file.find("ModuleConfigs");
    if (json_iter != json_file.end()) {
        nlohmann::json module_cfgs = *json_iter;
        for (auto iter = module_cfgs.begin(); iter != module_cfgs.end(); ++iter) {
            if (!iter->is_string()) {
                throw std::runtime_error("Non-string value for module config!");
            }
            moduleConfigFiles.emplace(iter.key(), *iter);
        }
    }

}

const std::vector<std::string>& ApplicationConfigurationFile::GetRequiredModules() const noexcept {
    return requiredModules;
}

const char* ApplicationConfigurationFile::GetModuleConfigFile(const char* module_name) {
    auto iter = moduleConfigFiles.find(module_name);
    if (iter == std::end(moduleConfigFiles)) {
        return nullptr;
    }
    else {
        return iter->second.c_str();
    }
}
