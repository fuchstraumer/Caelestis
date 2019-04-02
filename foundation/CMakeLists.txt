
IF(WIN32)

    SET(PLUGIN_MANAGER_IMPL 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Win32/PluginManagerImpl.hpp" 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Win32/PluginManagerImpl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Win32/PDB_Helpers.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Win32/PDB_Helpers.cpp"
    )
    SET(PLUGIN_MANAGER_IMPL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/Win32")

ELSEIF(UNIX)

    SET(PLUGIN_MANAGER_IMPL
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Unix/PluginManagerImpl.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Unix/PluginManagerImpl.cpp"
    )
    SET(PLUGIN_MANAGER_IMPL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/Unix")

ENDIF()

ADD_LIBRARY(foundation STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/threading/contention_free_mutex.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/threading/safe_ptr.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/threading/spinlock.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/threading/partitioned_map.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/CoreAPIs.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/PluginAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/PluginManager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PluginManager.cpp"
    ${PLUGIN_MANAGER_IMPL}
)

SET_TARGET_PROPERTIES(foundation PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
TARGET_COMPILE_OPTIONS(foundation PRIVATE ${CAELESTIS_CXX_FLAGS})

TARGET_INCLUDE_DIRECTORIES(foundation PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" ${PLUGIN_MANAGER_IMPL_INCLUDE_DIR})

IF(WIN32)
    TARGET_COMPILE_DEFINITIONS(foundation PRIVATE "_SCL_SECURE_NO_WARNINGS" "NOMINMAX")
ENDIF()
IF(UNIX)
    TARGET_LINK_LIBRARIES(foundation PUBLIC "c++fs" "dl" "pthread")
ENDIF()
