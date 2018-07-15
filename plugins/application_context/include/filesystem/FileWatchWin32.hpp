#pragma once
#ifndef FILE_WATCH_WIN32_HPP
#define FILE_WATCH_WIN32_HPP
#include "AppContextAPI.hpp"

class FileWatcher {
public:
    FileWatcher(ApplicationContext_API::FileListenerFn callback_func);
    ~FileWatcher();

    void Init(const char* full_file_path);
    bool CheckForChanges(uint32_t wait_time = 0);
    void StartThread();
    bool ThreadRunning();
    void StopThread();

private:
    
};

#endif //!FILE_WATCH_WIN32_HPP
