CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(Caelestis)

SET(BUILD_SHARED_LIBS ON CACHE BOOL "Build package with shared libraries")

ADD_SUBDIRECTORY(foundation)
ADD_SUBDIRECTORY(plugins)
ADD_SUBDIRECTORY(tests/plugin_tests)
