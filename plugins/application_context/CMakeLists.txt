IF(WIN32)
SET(DIALOG_IMPL "src/dialog/DialogWin32.cpp")
SET(PROCESS_INFO_IMPL "src/process/ProcessInfo_Win32.cpp")
ELSEIF(APPLE)
SET(DIALOG_IMPL "src/dialog/DialogMac.mm")
SET(PROCESS_INFO_IMPL "src/process/ProcessInfo_Mac.cpp")
ELSEIF(UNIX)
SET(DIALOG_IMPL "src/dialog/DialogLinux.cpp")
ENDIF()

IF(WIN32)
SET(GAME_MODE_SOURCES "src/platform/GameModeWin32.hpp" "src/platform/GameModeWin32.cpp")
ENDIF()

SET(build_static_lib ON CACHE BOOL "Build easyloggingpp as a static library")
ADD_SUBDIRECTORY("../../third_party/easyloggingpp" "${CMAKE_CURRENT_BINARY_DIR}/easylogging")

IF(APPLE)
# Needed to support Boost filesystem
SET(boost_min_ver 1.67.0)
SET(boost_libs system filesystem)
FIND_PACKAGE(Boost ${boost_min_ver})
IF(Boost_FOUND)
    FIND_PACKAGE(Boost ${boost_min_ver} COMPONENTS ${boost_libs})
ENDIF()
ENDIF()

ADD_PLUGIN(application_context 
    "include/AppContextAPI.hpp"
    "include/filesystem/FileManipulation.hpp"
    "include/ApplicationConfigurationFile.hpp"
    "include/process/ProcessID.hpp"
    "include/process/ProcessInfo.hpp"
    "include/dialog/Dialog.hpp"
    "src/AppContextAPI.cpp"
    "src/filesystem/FileManipulationStd.cpp"
    ${DIALOG_IMPL}
    "src/ApplicationConfigurationFile.cpp"
    "src/process/ProcessID.cpp"
    ${PROCESS_INFO_IMPL}
    ${GAME_MODE_SOURCES}
)

# Libraries and external frameworks required for dialog box stuff on Mac/Linux
IF(APPLE)
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   TARGET_LINK_LIBRARIES(application_context PRIVATE ${COCOA_LIBRARY})
ELSEIF(LINUX)
   FIND_PACKAGE(PkgConfig REQUIRED)
   pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
   TARGET_LINK_LIBRARIES(application_context PRIVATE ${GTK3_LIBRARIES})
   TARGET_INCLUDE_DIRECTORIES(application_context PRIVATE ${GTK3_INCLUDE_DIRS})
endif ()

SOURCE_GROUP("process_info" FILES "include/process/ProcessID.hpp" "include/process/ProcessInfo.hpp" 
    "src/process/ProcessID.cpp" ${PROCESS_INFO_IMPL})
SOURCE_GROUP("game_mode" FILES ${GAME_MODE_SOURCES})
SOURCE_GROUP("memory" FILES ${MEMORY_ALLOCATOR_SOURCES})
TARGET_LINK_LIBRARIES(application_context PUBLIC easyloggingpp)
TARGET_INCLUDE_DIRECTORIES(application_context PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "../../third_party/simplefilewatcher/include" "../../third_party/json/include"
    "../../third_party/easyloggingpp/src")

IF(APPLE)
TARGET_LINK_LIBRARIES(application_context PRIVATE ${Boost_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(application_context PRIVATE ${Boost_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
ENDIF()
