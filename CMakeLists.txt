CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(VulpesSceneKit)

FILE(GLOB ECS "${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/core/*.hpp")
FILE(GLOB SIGNALS "${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/signal/*.hpp")
FILE(GLOB STORAGE "${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/storage/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/storage/*.inl" )
FILE(GLOB SCRIPT_BINDINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/bindings/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/ecs/bindings/*.cpp")
FILE(GLOB_RECURSE COMPONENTS "${CMAKE_CURRENT_SOURCE_DIR}/include/components/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/components/*.cpp")
FILE(GLOB SYSTEMS "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/systems/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/systems/*.cpp")
FILE(GLOB RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/resources/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/resources/*.cpp")
FILE(GLOB RENDERER_CORE "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/*.cpp")
FILE(GLOB RENDERER_OBJECTS "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/objects/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/objects/*.cpp")
FILE(GLOB IMGUI "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/*.cpp")
FILE(GLOB UTIL "${CMAKE_CURRENT_SOURCE_DIR}/include/util/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cpp")

ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyobjloader")
ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/third_party/vulpesrender")
ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/third_party/chrysocyon")

SOURCE_GROUP("ecs\\core" FILES ${ECS})
SOURCE_GROUP("ecs\\signals" FILES ${SIGNALS})
SOURCE_GROUP("ecs\\storage" FILES ${STORAGE})
SOURCE_GROUP("ecs\\bindings" FILES ${SCRIPT_BINDINGS})
SOURCE_GROUP("components" FILES ${COMPONENTS})
SOURCE_GROUP("renderer\\core" FILES ${RENDERER_CORE})
SOURCE_GROUP("renderer\\resources" FILES ${RESOURCES})
SOURCE_GROUP("renderer\\systems" FILES ${SYSTEMS})
SOURCE_GROUP("renderer\\objects" FILES ${RENDERER_OBJECTS})
SOURCE_GROUP("imgui" FILES ${IMGUI})
SOURCE_GROUP("util" FILES ${UTIL})

ADD_LIBRARY(VulpesSceneKit STATIC ${ECS} ${SCRIPT_BINDINGS} ${SIGNALS} ${STORAGE} ${SYSTEMS} ${COMPONENTS} ${RESOURCES} ${RENDERER_CORE}
    ${RENDERER_OBJECTS} ${IMGUI})
TARGET_INCLUDE_DIRECTORIES(VulpesSceneKit PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/" "${CMAKE_CURRENT_SOURCE_DIR}/ext/include" 
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/json/include")
TARGET_INCLUDE_DIRECTORIES(VulpesSceneKit PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/vulpesrender/include"
"$ENV{VULKAN_SDK}/include" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/shadertools/include")
TARGET_LINK_LIBRARIES(VulpesSceneKit INTERFACE VulpesRender chrysocyon)

IF(MSVC)
    TARGET_COMPILE_OPTIONS(VulpesSceneKit PRIVATE "/std:c++17")
ELSEIF(UNIX)
    # Enables more symbols to be loaded when debugging with GDB
    TARGET_COMPILE_DEFINITIONS(VulpesSceneKit PUBLIC "-DGLIBCXX_DEBUG")
    # Somehow, GLFW gets made x64 but this and VPR can get made as x86
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
ENDIF()

ADD_EXECUTABLE(VPSK_Tests "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
TARGET_COMPILE_OPTIONS(VPSK_Tests PRIVATE "/std:c++17")
TARGET_INCLUDE_DIRECTORIES(VPSK_Tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/vulpesrender/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/include" "$ENV{VULKAN_SDK}/include")
TARGET_LINK_LIBRARIES(VPSK_Tests PRIVATE tinyobjloader VulpesRender VulpesSceneKit chrysocyon)
