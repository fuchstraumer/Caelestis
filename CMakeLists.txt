CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(VulpesSceneKit)

FILE(GLOB_RECURSE COMPONENTS "${CMAKE_CURRENT_SOURCE_DIR}/include/components/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/components/*.cpp")
FILE(GLOB SYSTEMS "${CMAKE_CURRENT_SOURCE_DIR}/include/systems/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/systems/*.cpp")
FILE(GLOB RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/include/resources/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/*.cpp")
FILE(GLOB RENDERER_CORE "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
FILE(GLOB RENDERER_OBJECTS "${CMAKE_CURRENT_SOURCE_DIR}/include/objects/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/objects/*.cpp")
FILE(GLOB IMGUI "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/*.cpp")
FILE(GLOB UTIL "${CMAKE_CURRENT_SOURCE_DIR}/include/util/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cpp")
FILE(GLOB GRAPH "${CMAKE_CURRENT_SOURCE_DIR}/include/graph/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/graph/*.cpp")

ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyobjloader")
ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/third_party/vulpesrender")
ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/third_party/chrysocyon")
SET(DOCTEST_WITH_TESTS OFF CACHE BOOL "Build tests/examples")
ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/third_party/shadertools")

SOURCE_GROUP("graph" FILES ${GRAPH})
SOURCE_GROUP("components" FILES ${COMPONENTS})
SOURCE_GROUP("core" FILES ${RENDERER_CORE})
SOURCE_GROUP("resources" FILES ${RESOURCES})
SOURCE_GROUP("systems" FILES ${SYSTEMS})
SOURCE_GROUP("objects" FILES ${RENDERER_OBJECTS})
SOURCE_GROUP("imgui" FILES ${IMGUI})
SOURCE_GROUP("util" FILES ${UTIL})

ADD_LIBRARY(VulpesSceneKit STATIC ${GRAPH} ${SYSTEMS} ${COMPONENTS} ${RESOURCES} ${RENDERER_CORE} ${RENDERER_OBJECTS} ${IMGUI} ${UTIL})
TARGET_INCLUDE_DIRECTORIES(VulpesSceneKit PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/" "${CMAKE_CURRENT_SOURCE_DIR}/ext/include" 
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/json/include" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/shadertools/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/chrysocyon/include")
TARGET_INCLUDE_DIRECTORIES(VulpesSceneKit PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "ext/include/vpr"
"$ENV{VULKAN_SDK}/include" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/shadertools/include" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest"
"${CMAKE_CURRENT_SOURCE_DIR}/third_party/shadertools/third_party/easyloggingpp/src" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sol2"
"${CMAKE_CURRENT_SOURCE_DIR}/third_party/shadertools/third_party/luajit/src")
TARGET_LINK_LIBRARIES(VulpesSceneKit INTERFACE vpr_core vpr_alloc vpr_resource chrysocyon liblua ShaderTools easyloggingpp "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
SET_TARGET_PROPERTIES(vpr_core vpr_alloc vpr_resource vpr_command vpr_sync vpr_render ShaderTools easyloggingpp tinyobjloader PROPERTIES FOLDER "VulpesSceneKit Dependencies")
TARGET_COMPILE_DEFINITIONS(VulpesSceneKit PRIVATE "DOCTEST_CONFIG_DISABLE")
IF(MSVC)
    TARGET_COMPILE_OPTIONS(VulpesSceneKit PRIVATE "/std:c++17")
ELSEIF(UNIX)
    # Enables more symbols to be loaded when debugging with GDB
    TARGET_COMPILE_DEFINITIONS(VulpesSceneKit PUBLIC "-DGLIBCXX_DEBUG")
    # Somehow, GLFW gets made x64 but this and VPR can get made as x86
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
ENDIF()

ADD_EXECUTABLE(VPSK_UNIT_TESTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/DoctestMain.cpp" ${GRAPH} ${SYSTEMS} ${COMPONENTS} ${RESOURCES} ${RENDERER_CORE} ${RENDERER_OBJECTS} ${IMGUI} ${UTIL})
TARGET_COMPILE_DEFINITIONS(VPSK_UNIT_TESTS PRIVATE "-DVPSK_TESTING_ENABLED")
TARGET_COMPILE_OPTIONS(VPSK_UNIT_TESTS PRIVATE "/std:c++17")
TARGET_INCLUDE_DIRECTORIES(VPSK_UNIT_TESTS PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest" "${CMAKE_CURRENT_SOURCE_DIR}/include" 
    "ext/include/vpr" "${CMAKE_CURRENT_SOURCE_DIR}/ext/include" "$ENV{VULKAN_SDK}/include")
TARGET_INCLUDE_DIRECTORIES(VPSK_UNIT_TESTS PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest")
TARGET_LINK_LIBRARIES(VPSK_UNIT_TESTS PRIVATE tinyobjloader vpr_core vpr_alloc vpr_resource chrysocyon liblua ShaderTools easyloggingpp)

ADD_EXECUTABLE(VPSK_Tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp")
TARGET_COMPILE_OPTIONS(VPSK_Tests PRIVATE "/std:c++17")
TARGET_INCLUDE_DIRECTORIES(VPSK_Tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "ext/include/vpr"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/include" "$ENV{VULKAN_SDK}/include")
TARGET_LINK_LIBRARIES(VPSK_Tests PRIVATE tinyobjloader vpr_core vpr_alloc vpr_resource VulpesSceneKit chrysocyon liblua ShaderTools)

SET_TARGET_PROPERTIES(VPSK_Tests VPSK_UNIT_TESTS PROPERTIES FOLDER "VPSK Test Binaries")
