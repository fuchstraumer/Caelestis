#include "filesystem/FileManipulation.hpp"
#ifdef __APPLE_CC__
#include <boost/filesystem.hpp>
#else
#include <experimental/filesystem>
#endif
#include <unordered_map>

#ifndef __APPLE_CC__
namespace stdfs = std::experimental::filesystem;
#else
namespace stdfs = boost::filesystem;
#endif
static std::unordered_map<std::string, std::string> pathStrings;

namespace fs {

const char* TempDirPath() {
    return stdfs::temp_directory_path().string().c_str();    
}

int CreateDirectory(const char* name, bool recursive) {
#ifndef __APPLE_CC__
    std::error_code ec;
#else
    boost::system::error_code ec;
#endif
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
#ifndef __APPLE_CC__
    std::error_code ec;
#else
    boost::system::error_code ec;
#endif
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
    if (pathStrings.count(stdfs::absolute(path).string()) != 0) {
        return pathStrings.at(stdfs::absolute(path).string()).c_str();
    }
    else if (stdfs::exists(path)) {
        auto iter = pathStrings.emplace(stdfs::absolute(path).string(), stdfs::absolute(path).string());
        return iter.first->second.c_str();
    }
    else {
        return nullptr;
    }
}

const char* GetCanonicalPath(const char* input) {
    stdfs::path path(input);
    if (pathStrings.count(stdfs::canonical(path).string()) != 0) {
        return pathStrings.at(stdfs::canonical(path).string()).c_str();
    }
    else if (stdfs::exists(path)) {
        auto iter = pathStrings.emplace(stdfs::canonical(path).string(), stdfs::canonical(path).string());
        return iter.first->second.c_str();
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
            const std::string found_path_str = file_name_path.string();
#ifndef __APPLE_CC__
            *found_file = _strdup(found_path_str.c_str());
#else
            *found_file = strdup(found_path_str.c_str());
#endif
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
#ifndef __APPLE_CC__
                    *found_file = _strdup(found_path_str.c_str());
#else
                    *found_file = strdup(found_path_str.c_str());
#endif
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
#ifndef __APPLE_CC__
    std::error_code ec;
#else
    boost::system::error_code ec;
    boost::filesystem::copy_option option(static_cast<boost::filesystem::copy_option>(options));
    stdfs::copy_file(stdfs::path(from), stdfs::path(to), option, ec);
#endif
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
#ifndef __APPLE_CC__
    std::error_code ec;
#else
    boost::system::error_code ec;
#endif
    stdfs::remove(stdfs::path(_path), ec);
    return ec.value();
}

int RemoveAll(const char* path, size_t* num_removed) {
#ifndef __APPLE_CC__
    std::error_code ec;
#else
    boost::system::error_code ec;
#endif
    size_t num = stdfs::remove_all(stdfs::path(path), ec);
    *num_removed = num;
    return ec.value();
}

int ResizeFile(const char* path, const size_t new_size) {
#ifndef __APPLE_CC__
    std::error_code ec;
#else
    boost::system::error_code ec;
#endif
    stdfs::resize_file(stdfs::path(path), new_size, ec);
    return ec.value();
}

int RenameFile(const char* path, const char* new_name) {
#ifndef __APPLE_CC__
    std::error_code ec;
#else
    boost::system::error_code ec;
#endif
    stdfs::path new_path(path);
    stdfs::rename(stdfs::path(path), new_path, ec);
    return ec.value();
}

int MoveFile(const char* path, const char* new_path) {
#ifndef __APPLE_CC__
    std::error_code ec;
#else
    boost::system::error_code ec;
#endif
    stdfs::rename(stdfs::path(path), stdfs::path(new_path), ec);
    return ec.value();
}

}
