IF(WIN32)
SET(DIALOG_IMPL "src/dialog/DialogWin32.cpp")
SET(SECURE_MEMORY_IMPL "src/memory/impl/SecureMemoryWin32.hpp")
SET(PROCESS_INFO_IMPL "src/process/ProcessInfo_Win32.cpp")
ELSEIF(UNIX)
SET(DIALOG_IMPL "src/dialog/DialogLinux.cpp")
SET(SECURE_MEMORY_IMPL "src/memory/impl/SecureMemoryUnix.hpp")
ELSE()
SET(DIALOG_IMPL "src/dialog/DialogMac.cpp")
SET(SECURE_MEMORY_IMPL "src/memory/impl/SecureMemoryUnix.hpp")
ENDIF()

IF(WIN32)
SET(GAME_MODE_SOURCES "src/platform/GameModeWin32.hpp" "src/platform/GameModeWin32.cpp")
ENDIF()

ADD_SUBDIRECTORY("../../third_party/memory" "${CMAKE_CURRENT_BINARY_DIR}/memory")
SET(build_static_lib ON CACHE BOOL "Build easyloggingpp as a static library")
ADD_SUBDIRECTORY("../../third_party/easyloggingpp" "${CMAKE_CURRENT_BINARY_DIR}/easylogging")

SET(MEMORY_ALLOCATOR_SOURCES
    "src/memory/impl/AllocatorImpl.hpp"
    "src/memory/impl/SecureAllocator.hpp"
    ${SECURE_MEMORY_IMPL}
    "src/memory/impl/SecureAllocator.cpp"
    "src/memory/impl/VirtualMemoryAllocator.hpp"
    "src/memory/impl/VirtualMemoryAllocator.cpp"
    "src/memory/impl/PoolAllocator.hpp"
    "src/memory/impl/PoolAllocator.cpp"
    "src/memory/impl/HeapAllocator.hpp"
    "src/memory/impl/HeapAllocator.cpp"
    "src/memory/impl/TemporaryAllocator.hpp"
    "src/memory/impl/TemporaryAllocator.cpp"
    "src/memory/Allocator.cpp"
)

ADD_PLUGIN(application_context 
    "include/AppContextAPI.hpp"
    "include/filesystem/FileManipulation.hpp"
    "include/ApplicationConfigurationFile.hpp"
    "include/memory/Allocator.hpp"
    "include/process/ProcessID.hpp"
    "include/process/ProcessInfo.hpp"
    "include/dialog/Dialog.hpp"
    "src/AppContextAPI.cpp"
    "src/filesystem/FileManipulationStd.cpp"
    ${DIALOG_IMPL}
    "src/ApplicationConfigurationFile.cpp"
    "src/process/ProcessID.cpp"
    ${PROCESS_INFO_IMPL}
    ${MEMORY_ALLOCATOR_SOURCES}
    ${GAME_MODE_SOURCES}
)

SOURCE_GROUP("process_info" FILES "include/process/ProcessID.hpp" "include/process/ProcessInfo.hpp" 
    "src/process/ProcessID.cpp" ${PROCESS_INFO_IMPL})
SOURCE_GROUP("game_mode" FILES ${GAME_MODE_SOURCES})
SOURCE_GROUP("memory" FILES ${MEMORY_ALLOCATOR_SOURCES})
TARGET_LINK_LIBRARIES(application_context PUBLIC foonathan_memory easyloggingpp)
TARGET_INCLUDE_DIRECTORIES(application_context PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "../../third_party/simplefilewatcher/include" "../../third_party/json/include"
    "../../third_party/easyloggingpp/src")
