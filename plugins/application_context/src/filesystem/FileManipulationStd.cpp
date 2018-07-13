#include "filesystem/FileManipulation.hpp"
#include <experimental/filesystem>

namespace stdfs = std::experimental::filesystem;
namespace fs {

const char* TempDirPath() {
    return stdfs::temp_directory_path().string().c_str();    
}

int CreateDirectory(const char* name, bool recursive) {
    std::error_code ec;
    if (recursive) {
        stdfs::create_directories(stdfs::path(name), ec);
        return ec.value();
    }
    else {
        stdfs::create_directory(stdfs::path(name), ec);
        return ec.value();
    }
}

int CreateFile(const char* fname, uint32_t file_type, bool recursive) {
    std::error_code ec;
    if (recursive) {
        return 0;
    }
    else {
        return 0;
    }
}

const char* CreateDirectoryForCurrentDateTime() {
    return nullptr;
}

bool PathExists(const char* _path) {
    return stdfs::exists(stdfs::path(_path));
}

const char* GetAbsolutePath(const char* input) { 
    stdfs::path path(input);
    if (stdfs::exists(path)) {
        const std::string result = stdfs::absolute(path).string();
        return result.c_str();
    }
    else {
        return nullptr;
    }
}

const char* GetCanonicalPath(const char* input) {
    stdfs::path path(input);
    if (stdfs::exists(path)) {
        const std::string result = stdfs::canonical(path).string();
        return result.c_str();
    }
    else {
        return nullptr;
    }
}

bool FindFile(const char* fname, const char* starting_dir, uint32_t depth_limit, char** found_file) {
    stdfs::path starting_path(starting_dir);
    if (!stdfs::exists(starting_path)) {
        return false;
    }

    {
        stdfs::path file_name_path(fname);
        file_name_path = file_name_path.filename();

        if (stdfs::exists(file_name_path)) {
            const std::string found_path = file_name_path.string();
            *found_file = _strdup(found_path.c_str());
            return true;
        }

        stdfs::path file_path = starting_path;

        uint32_t current_recursion = 0;

        while (current_recursion < depth_limit) {
            for (auto& dir_entry : stdfs::recursive_directory_iterator(file_path)) {
                if (dir_entry == starting_path) {
                    continue;
                }

                if (stdfs::is_directory(dir_entry)) {
                    continue;
                }

                stdfs::path entry_path(dir_entry);
                if (entry_path.has_filename() && (entry_path.filename() == file_name_path)) {
                    const std::string found_path_str = entry_path.string();
                    *found_file = _strdup(found_path_str.c_str());
                    return true;
                }
            }

            ++current_recursion;
            file_path = file_path.parent_path();
        }
        
    }

    return false;
}

int Copy(const char* from, const char* to, uint32_t options) {
    return 0;
}

int CopyFile(const char* from, const char* to, uint32_t options) {
    std::error_code ec;
    stdfs::copy_file(stdfs::path(from), stdfs::path(to), stdfs::copy_options(options), ec);
    return ec.value();
}

uint32_t GetPermissions(const char* _path) {
    return 0;
}

int SetPermissions(const char* path, uint32_t permissions) {
    return 0;
}

int Remove(const char* _path) {
    std::error_code ec;
    stdfs::remove(stdfs::path(_path), ec);
    return ec.value();
}

int RemoveAll(const char* path, size_t* num_removed) {
    std::error_code ec;
    size_t num = stdfs::remove_all(stdfs::path(path), ec);
    *num_removed = num;
    return ec.value();
}

int ResizeFile(const char* path, const size_t new_size) {
    std::error_code ec;
    stdfs::resize_file(stdfs::path(path), new_size, ec);
    return ec.value();
}

int RenameFile(const char* path, const char* new_name) {
    std::error_code ec;
    stdfs::path new_path(path);
    new_path.replace_filename(new_name);
    stdfs::rename(stdfs::path(path), new_path, ec);
    return ec.value();
}

int MoveFile(const char* path, const char* new_path) {
    std::error_code ec;
    stdfs::rename(stdfs::path(path), stdfs::path(new_path), ec);
    return ec.value();
}

}
