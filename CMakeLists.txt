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


ADD_EXECUTABLE(VulpesSceneKit "src/main.cpp")
ADD_DEPENDENCIES(VulpesSceneKit VulpesRender)