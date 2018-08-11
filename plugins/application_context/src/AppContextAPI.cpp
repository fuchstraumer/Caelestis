#include "AppContextAPI.hpp"
#include "PluginAPI.hpp"
#include "CoreAPIs.hpp"
#include "filesystem/FileManipulation.hpp"
#include "dialog/Dialog.hpp"
#include "ApplicationConfigurationFile.hpp"
#ifdef _WIN32
#include "platform/GameModeWin32.hpp"
#endif
#include "process/ProcessID.hpp"
#include "process/ProcessInfo.hpp"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP
#undef CreateDirectory
#undef MoveFile
#undef CopyFile
#undef CreateFile
#include <unordered_map>
#include <string>
#include <vector>
#ifndef __APPLE_CC__
#include <experimental/filesystem>
#else
#include <boost/filesystem.hpp>
#endif
#include <unordered_map>

constexpr const char* const APPLICATION_CONTEXT_CFG_FILE_NAME = "ApplicationConfig.json";
static ApplicationConfigurationFile CfgFile;
static ProcessInfo* processInfo = nullptr;

static void FindApplicationConfigFile() {
#ifndef __APPLE_CC__
    namespace stdfs = std::experimental::filesystem;
#else
    namespace stdfs = boost::filesystem;
#endif
    
    if (!stdfs::exists(APPLICATION_CONTEXT_CFG_FILE_NAME)) {
        char* found_file = nullptr;
        stdfs::path starting_path = stdfs::current_path();
        const std::string starting_path_str = starting_path.string();

        fs::FindFile(APPLICATION_CONTEXT_CFG_FILE_NAME, starting_path_str.c_str(), 5, &found_file);

        if (!stdfs::exists(found_file)) {
            if (found_file) {
                free(found_file);
            }
            throw std::runtime_error("Could not find master config file!");
        }
        else {
            CfgFile = ApplicationConfigurationFile(found_file);
        }

        if (found_file) {
            free(found_file);
        }
    }
    else {
        CfgFile = ApplicationConfigurationFile(APPLICATION_CONTEXT_CFG_FILE_NAME);
    }
}

static const char* ContextName() {
    return "ApplicationContextAPI";
}

static uint32_t PluginID() {
    return APPLICATION_CONTEXT_API_ID;
}

static void WriteLoadedLog() {
#ifndef __APPLE_CC__
    namespace fs = std::experimental::filesystem;
#else
    namespace fs = boost::filesystem;
#endif
    LOG(INFO) << "Application context loaded.";
    LOG(INFO) << "Temporary data path: " << fs::temp_directory_path().string();
    LOG(INFO) << "Current canonical working directory: " << fs::canonical(fs::current_path()).string();
    LOG(INFO) << "Registered modules and config files: ";
    const auto& module_names = CfgFile.GetRequiredModules();
    for (auto& plugin : module_names) {
        LOG(INFO) << "    " << plugin.c_str() << " : " << CfgFile.GetModuleConfigFile(plugin.c_str());
    }
    LOG(INFO) << "System Memory available: " << processInfo->GetAvailPhysicalMemory();
    LOG(INFO) << "System Virtual memory available: " << processInfo->VirtualMemorySize();
    LOG(INFO) << "Current memory committed: " << processInfo->GetCommittedPhysicalMemory();
#ifdef _WIN32
    LOG(INFO) << "Game mode supported: " << ExclusiveCapable();
    LOG_IF(ExclusiveCapable(), INFO) << "    ExclusiveModeCoreCount: " << ExclusiveCoreCount();
#endif // _WIN32
}

static void Load(GetEngineAPI_Fn fn) {
    processInfo = new ProcessInfo(ProcessID::GetCurrent());
    el::Configurations conf;
    conf.setToDefault();
    conf.setGlobally(el::ConfigurationType::Format, "%level :: %datetime :: %msg");
    conf.setGlobally(el::ConfigurationType::Filename, "application_log.log");
    conf.setGlobally(el::ConfigurationType::SubsecondPrecision, "4");
    conf.set(el::Level::Warning, el::ConfigurationType::Format, "%level :: %datetime :: %func :: %msg");
    conf.set(el::Level::Error, el::ConfigurationType::Format, "%level :: %datetime :: %file :: %func :: %line :: %msg");
    conf.set(el::Level::Debug, el::ConfigurationType::Format, "%level :: %datetime :: %file :: %func :: %line :: %msg");
    el::Loggers::reconfigureAllLoggers(conf);
    // Ignoring both input arguments. We're the core API: we are what ends up helping supply these arguments to everyone else!
    FindApplicationConfigFile();
#ifdef _WIN32
    QueryGameModeCapabilities();
#endif // _WIN32
    WriteLoadedLog();
}

static void Unload() {
    // Clear filewatcher here.
}

static void Update() {
#ifdef _WIN32
    GameModeFrameFn();
#endif
}

static uint32_t ShowDialog(const char* message, const char* title, uint32_t dialog_type, uint32_t buttons) {
    return ShowDialogImpl(message, title, dialog_type, buttons);
}

static const char* TemporaryDirectoryPath() {
    return fs::TempDirPath();
}

static int CreateDir(const char* name, bool recursive) {
    return fs::CreateDirectory(name, recursive);
}

static int CreateFile(const char* f, uint32_t ftype, bool r) {
    return fs::CreateFile(f, ftype, r);
}

static const char* TimeStampedDir() {
    return fs::CreateDirectoryForCurrentDateTime();
}

static bool PathExists(const char* p) {
    return fs::PathExists(p);
}

const char* AbsolutePath(const char* p) {
    return fs::GetAbsolutePath(p);
}

const char* CanonicalPath(const char* p) {
    return fs::GetCanonicalPath(p);
}

static bool FindFile(const char* f, const char* starting_dir, uint32_t limit, char** ff) {
    if (starting_dir == nullptr) {
#ifndef __APPLE_CC__
        namespace stdfs = std::experimental::filesystem;
#else
        namespace stdfs = boost::filesystem;
#endif
        const std::string current_path = stdfs::current_path().string();
        return fs::FindFile(f, current_path.c_str(), limit, ff);
    }
    else {
        return fs::FindFile(f, starting_dir, limit, ff);
    }
}

static int Copy(const char* from, const char* to, uint32_t opts) {
    return fs::Copy(from, to, opts);
}

static int CopyFile(const char* f, const char* t, uint32_t opts) {
    return fs::CopyFile(f, t, opts);
}

static uint32_t GetPermissions(const char* p) {
    return fs::GetPermissions(p);
}

static int SetPermissions(const char* p, uint32_t perms) {
    return fs::SetPermissions(p, perms);
}

static int Remove(const char* p) {
    return fs::Remove(p);
}

static int RemoveAll(const char* p, size_t* num_removed) {
    return fs::RemoveAll(p, num_removed);
}

static int ResizeFile(const char* p, const size_t size) {
    return fs::ResizeFile(p, size);
}

static int RenameFile(const char* p, const char* name) {
    return fs::RenameFile(p, name);
}

static int MoveFile(const char* p, const char* new_p) {
    return fs::MoveFile(p, new_p);
}

static void GetRequiredModules(size_t* num, const char** names) {
    *num = CfgFile.GetRequiredModules().size();
    if (names != nullptr) {
        const auto& modules = CfgFile.GetRequiredModules();
        for (size_t i = 0; i < *num; ++i) {
            names[i] = modules[i].c_str();
        }
    }
}

static const char* GetModuleCfgFile(const char* module_name) {
    return CfgFile.GetModuleConfigFile(module_name);
}

static void* GetLoggingStoragePointer() {
    static auto storage = el::Helpers::storage();
    return &storage;
}

static void InfoLog(const char* msg) {
    LOG(INFO) << msg;
}

static void WarningLog(const char* msg) {
    LOG(WARNING) << msg;
}

static void ErrorLog(const char* msg) {
    LOG(ERROR) << msg;
}

static size_t VirtualMemorySize() {
    return processInfo->VirtualMemorySize();
}

static size_t ResidentSize() {
    return processInfo->ResidentSize();
}

static double GetMemoryPressure() {
    return processInfo->GetMemoryPressure();
}

static size_t GetTotalPhysicalMemory() {
    return processInfo->GetTotalPhysicalMemory();
}

static size_t GetAvailPhysicalMemory() {
    return processInfo->GetAvailPhysicalMemory();
}

static size_t GetCommittedPhysicalMemory() {
    return processInfo->GetCommittedPhysicalMemory();
}

static size_t GetPageSize() {
    return ProcessInfo::GetPageSize();
}

static void* CorePluginAPI() {
    static Plugin_API api{ nullptr };
    api.PluginName = ContextName;
    api.PluginID = PluginID;
    api.Load = Load;
    api.Unload = Unload;
    api.LogicalUpdate = Update;
    return &api;
}

static void* ApplicationContextAPI() {
    static ApplicationContext_API api{ nullptr };
    api.ShowDialog = ShowDialog;
    api.TemporaryDirectoryPath = TemporaryDirectoryPath;
    api.CreateDirectory = CreateDir;
    api.CreateFile = CreateFile;
    api.CreateDirectoryForCurrentDateTime = TimeStampedDir;
    api.PathExists = PathExists;
    api.GetAbsolutePath = AbsolutePath;
    api.GetCanonicalPath = CanonicalPath;
    api.FindFile = FindFile;
    api.Copy = Copy;
    api.CopyFile = CopyFile;
    api.GetPermissions = GetPermissions;
    api.SetPermissions = SetPermissions;
    api.Remove = Remove;
    api.RemoveAll = RemoveAll;
    api.ResizeFile = ResizeFile;
    api.RenameFile = RenameFile;
    api.MoveFile = MoveFile;
    api.GetRequiredModules = GetRequiredModules;
    api.GetPluginConfigFile = GetModuleCfgFile;
    api.GetLoggingStoragePointer = GetLoggingStoragePointer;
    api.InfoLog = InfoLog;
    api.WarningLog = WarningLog;
    api.ErrorLog = ErrorLog;
    api.VirtualMemorySize = VirtualMemorySize;
    api.ResidentSize = ResidentSize;
    api.MemoryPressure = GetMemoryPressure;
    api.TotalPhysicalMemory = GetTotalPhysicalMemory;
    api.AvailPhysicalMemory = GetAvailPhysicalMemory;
    api.CommittedPhysicalMemory = GetCommittedPhysicalMemory;
    api.PageSize = GetPageSize;
    return &api;
}

PLUGIN_API void* GetPluginAPI(uint32_t api_id) {
    switch (api_id) {
    case 0:
        return CorePluginAPI();
    case APPLICATION_CONTEXT_API_ID:
        return ApplicationContextAPI();
    default:
        return nullptr;
    };
}
