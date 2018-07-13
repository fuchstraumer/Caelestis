#include "AppContextAPI.hpp"
#include "PluginAPI.hpp"
#include "CoreAPIs.hpp"
#include "filesystem/FileManipulation.hpp"
#include "dialog/Dialog.hpp"
#include "ApplicationConfigurationFile.hpp"
#include "memory/Allocator.hpp"
#include <unordered_map>
#include <string>
#include <experimental/filesystem>

constexpr const char* const APPLICATION_CONTEXT_CFG_FILE_NAME = "ApplicationConfig.json";
static ApplicationConfigurationFile CfgFile;

static void FindApplicationConfigFile() {
    namespace stdfs = std::experimental::filesystem;
    
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

static void Load(GetEngineAPI_Fn fn) {
    // Ignoring both input arguments. We're the core API: we are what ends u p helping supply these arguments to everyone else!
    FindApplicationConfigFile();
}

static void Unload() {
    // Clear filewatcher here.
}

static void Update() {
    
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
        namespace stdfs = std::experimental::filesystem;
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

constexpr static AllocatorCreateInfo heap_alloc_info {
    allocator_trait_flags::HeapAllocator,
    0,
    nullptr
};

static Allocator* GetHeapAllocator() {
    static Allocator alloc(&heap_alloc_info);
    return &alloc;
}

static Allocator* CreateNewAllocator(const AllocatorCreateInfo* create_info) {
    return new Allocator(create_info);
}

static const char* GetModuleCfgFile(const char* module_name) {
    return CfgFile.GetModuleConfigFile(module_name);
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
    api.GetModuleConfigFile = GetModuleCfgFile;
    api.GetHeapAllocator = GetHeapAllocator;
    api.CreateNewAllocator = CreateNewAllocator;
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