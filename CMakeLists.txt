CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(VulpesSceneKit)

FILE(GLOB CORE "${CMAKE_CURRENT_SOURCE_DIR}/include/core/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp")
FILE(GLOB SCRIPT_BINDINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/bindings/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/bindings/*.cpp")

SOURCE_GROUP("core" FILES ${CORE})
SOURCE_GROUP("bindings" FILES ${SCRIPT_BINDINGS})

ADD_LIBRARY(VulpesSceneKit STATIC ${CORE} ${SCRIPT_BINDINGS})
TARGET_INCLUDE_DIRECTORIES(VulpesSceneKit PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
IF(MSVC)
    TARGET_COMPILE_OPTIONS(VulpesSceneKit PRIVATE "/std:c++17")
ELSEIF(UNIX)
    # Enables more symbols to be loaded when debugging with GDB
    TARGET_COMPILE_DEFINITIONS(VulpesSceneKit PUBLIC "-DGLIBCXX_DEBUG")
    # Somehow, GLFW gets made x64 but this and VPR can get made as x86
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
ENDIF()
