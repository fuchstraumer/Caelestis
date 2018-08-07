#pragma once
#ifndef APPLICATION_CONTEXT_API_HPP
#define APPLICATION_CONTEXT_API_HPP
#include <cstdint>
#include <cstddef>

constexpr static uint32_t APPLICATION_CONTEXT_API_ID = 0x920ac193;
constexpr static uint32_t FIBER_JOB_SYSTEM_API_ID = 0x2e37eeb4;

namespace file_type {
    enum e : uint32_t {
        none = 0,
        regular_file,
        character_file,
        directory,
        empty_directory,
        empty_file,
        fifo_pipe,
        socket,
        symlink
    };
}

namespace copy_options {
    enum e : uint32_t {
        none = 0,
        skip_existing,
        overwrite_existing,
        update_existing,
        recursive,
        copy_symlinks,
        skip_symlinks,
        directories_only,
        create_symlinks,
        create_hard_links
    };
}

/*
    Application context is intended to be used a singleton
    for platform-specific operations, state, error handling,
    and more. Think of it as the most basic infrastructure that
    an application is built on: lots of boring stuff, but it's 
    very useful to have.

    Further contexts (like rendering, memory, assets, etc) will
    all sit on top of, and potentially rely on, this system.
*/
struct ApplicationContext_API {
    // Returns last platform error as a string.
    const char* (*LastPlatformError)(void);
    /*
        Message and window boxes
        - Uses platform interfaces and systems on Mac and Windows
        - "type" specifies the type of dialog popup/box (e.g, warning, info, error)
        - "Buttons" specifies options to allow (ok, cancel, y/n)
        - "Selection" is a returned value representing the users choice (ok, exit, yes, etc)

        The enums these values map to can be in "Dialog.hpp"
        https://stackoverflow.com/questions/13500069/simplest-way-to-pop-up-error-message-box-on-windows-nix-and-macos
    */
    uint32_t (*ShowDialog)(const char* message, const char* title, uint32_t dialog_type, uint32_t buttons);

    /*
        File and folder manipulation

        Functions returning "int" parameters are really returning a filesystem_error_code.
        This is the OS API code translated to some common format, making it easier to act
        upon. Include "filesystem/ErrorCodes.hpp" to translate them or view attached messages.

        Functions returning or writing to a char* are using _strdup: make sure to free the 
        returned or written string once you are done with it.

        These map to std::experimental::filesystem calls on platforms that support it
    */
    const char* (*TemporaryDirectoryPath)(void);
    // Recursive specifies is directories leading to desired dir_name should be created as well
    int (*CreateDirectory)(const char* dir_name, bool recursive);
    int (*CreateFile)(const char* fname, uint32_t file_type, bool recursive);
    // Format is HOUR-MINUTE-SECOND-MONTH-DAY-YEAR
    const char* (*CreateDirectoryForCurrentDateTime)(void);
    bool (*PathExists)(const char* path);
    const char* (*GetAbsolutePath)(const char* input);
    // Same as above, but no symlinks or "."/".."
    const char* (*GetCanonicalPath)(const char* input);
    // Found path returned via _strdup. Must be freed by user.
    bool (*FindFile)(const char* file_name, const char* starting_dir, uint32_t recursion_limit, char** found_path);
    // Copies files and/or directories based on copy_options
    int (*Copy)(const char* from, const char* to, uint32_t copy_options);
    // Copies a file "from" to "to", also obeying "copy_options"
    int (*CopyFile)(const char* from, const char* to, uint32_t copy_options);
    uint32_t (*GetPermissions)(const char* path);
    int (*SetPermissions)(const char* path, uint32_t permissions);
    int (*Remove)(const char* path);
    int (*RemoveAll)(const char* path, size_t* num_removed);
    // Can only be applied to regular file types. Truncates or zero-fills as appropriate
    int (*ResizeFile)(const char* path, const size_t new_size);
    int (*RenameFile)(const char* path, const char* new_name);
    int (*MoveFile)(const char* path, const char* new_path);
    /*
        File watching:
        - Requires update method to be connected to logical update of Plugin_API
        - "action" specifies what occured to the file. The potential values are in "FileWatcher.hpp"
        - "recursive" watches subdirectories as well as the base directory monitored
        - Watching operates on directories, but files inside these directories are watched as well
          and file names modified are included in dispatched signals
    */
    using FileListenerFn = void(*)(uint32_t watch_id, const char* dir_name, const char* fname, uint32_t action);
    uint32_t (*AddDirectoryWatch)(const char* dir_name, void* listener, bool recursive);
    void (*RemoveDirectoryWatch)(const char* dir_name);
    void (*RemoveWatchID)(uint32_t watch_id);
    /*
        Utilities for loaded plugins: 
        - Fetch required list of plugins. Done by plugin manager.
        - Fetch config file name given a plugin name. Done by plugin manager as well.
    */
    void (*GetRequiredModules)(size_t* num_modules, const char** modules);
    const char* (*GetPluginConfigFile)(const char* module_name);
    /*
        Use this to publish logging functions through a single interface, guaranteeing they use a singular logging repository when called through
        here. One may also retrieve the logging repository through this interface, and use it to set a shared repository in other modules.
    */
    void*(*GetLoggingStoragePointer)(void);
    void (*InfoLog)(const char* message);
    void (*WarningLog)(const char* message);
    void (*ErrorLog)(const char* message);
    /*
        Gets memory status info for current process. Shouldn't have more than one process attached or running to anything
        using the plugin manager system, however.
    */
    size_t (*VirtualMemorySize)(void);
    size_t (*ResidentSize)(void);
    double (*MemoryPressure)(void);
    size_t (*TotalPhysicalMemory)(void);
    size_t (*AvailPhysicalMemory)(void);
    size_t (*CommittedPhysicalMemory)(void);
    size_t (*PageSize)(void);
};

#endif //!APPLICATION_CONTEXT_API_HPP
