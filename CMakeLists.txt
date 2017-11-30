INCLUDE(ExternalProject)
CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(VulpesSceneKit)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

ExternalProject_Add(VulpesRender 
    PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/download/vulpesrender"
    GIT_REPOSITORY "https://github.com/fuchstraumer/vulpesrender.git"
    CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/ext/"
        "-DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_CURRENT_SOURCE_DIR}/ext/include/")

FILE(GLOB GEOMETRIES "${CMAKE_CURRENT_SOURCE_DIR}/include/geometries/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/geometries/*.cpp")
FILE(GLOB CAMERAS "${CMAKE_CURRENT_SOURCE_DIR}/include/camera/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/camera/*.cpp")
FILE(GLOB LIGHTS "${CMAKE_CURRENT_SOURCE_DIR}/include/lights/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/lights/*.cpp")
FILE(GLOB MATH "${CMAKE_CURRENT_SOURCE_DIR}/include/math/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/math/*.cpp")
FILE(GLOB SCENE "${CMAKE_CURRENT_SOURCE_DIR}/include/scene/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/scene/*.cpp")
FILE(GLOB UTIL "${CMAKE_CURRENT_SOURCE_DIR}/include/util/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/util/*.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cc")

ADD_EXECUTABLE(VulpesSceneKit "src/main.cpp" ${GEOMETRIES} ${CAMERAS} ${LIGHTS} ${MATH} ${SCENE} ${UTIL})
ADD_DEPENDENCIES(VulpesSceneKit VulpesRender)
IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
ENDIF()
TARGET_INCLUDE_DIRECTORIES(VulpesSceneKit PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ext/include")
