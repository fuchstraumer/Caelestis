#pragma once
#ifndef APPLICATION_CONTEXT_FILE_MANIPULATION_HPP
#define APPLICATION_CONTEXT_FILE_MANIPULATION_HPP
#include <cstdint>

namespace fs {

namespace file_type {
    enum e : uint32_t {
        None = 0,
        NotFound,
        Regular,
        Directory,
        Symlink,
        Block,
        Character,
        Fifo,
        Socket,
        Unknown
    };
}

namespace copy_options {
    enum e : uint32_t {
        None = 0,
        SkipExisting,
        OverwriteExisting,
        UpdateExisting,
        Recursive,
        CopySymlinks,
        SkipSymlinks,
        DirectoriesOnly,
        CreateSymlinks,
        CreateHardLinks
    };
}

const char* TempDirPath();
int CreateDirectory(const char* name, bool recursive);
int CreateFile(const char* fname, uint32_t file_type, bool recursive);
const char* CreateDirectoryForCurrentDateTime();
bool PathExists(const char* path);
const char* GetAbsolutePath(const char* input);
const char* GetCanonicalPath(const char* input);
bool FindFile(const char* fname, const char* starting_dir, uint32_t depth_limit, char** found_file);
int Copy(const char* from, const char* to, uint32_t options);
int CopyFile(const char* from, const char* to, uint32_t options);
uint32_t GetPermissions(const char* path);
int SetPermissions(const char* path, uint32_t permissions);
int Remove(const char* path);
int RemoveAll(const char* path, size_t* num_removed);
int ResizeFile(const char* path, const size_t new_size);
int RenameFile(const char* path, const char* new_name);
int MoveFile(const char* path, const char* new_path);

} // end namespace fs

#endif //!APPLICATION_CONTEXT_FILE_MANIPULATION_HPP
