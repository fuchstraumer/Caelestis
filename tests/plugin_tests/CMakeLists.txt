
SET(VPR_INCLUDE_DIRS_TEST_RELATIVE
    "../../../third_party/vulpesrender/alloc/include"
    "../../../third_party/vulpesrender/command/include"
    "../../../third_party/vulpesrender/common"
    "../../../third_party/vulpesrender/core/include"
    "../../../third_party/vulpesrender/render/include"
    "../../../third_party/vulpesrender/resource/include"
    "../../../third_party/vulpesrender/sync/include"
)

FUNCTION(ADD_PLUGIN_TEST NAME)
    ADD_EXECUTABLE(${NAME} ${ARGN})
    TARGET_INCLUDE_DIRECTORIES(${NAME} PRIVATE
        "../../../plugin_manager/include"
        "../../../ext/include"
        ${VPR_INCLUDE_DIRS_TEST_RELATIVE}
    )
    TARGET_COMPILE_DEFINITIONS(${NAME} PUBLIC 
        $<BUILD_INTERFACE:BUILDING_SHARED_LIBRARY> $<INSTALL_INTERFACE:USING_SHARED_LIBRARY>
        "NOMINMAX" "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS")
    IF(MSVC)
        # Multithreaded compile, and intrinsic functions
        TARGET_COMPILE_OPTIONS(${NAME} PRIVATE $<$<CONFIG:RELEASE>:/MP> $<$<CONFIG:RELWITHDEBINFO>:/MP>)
        TARGET_COMPILE_OPTIONS(${NAME} PRIVATE $<$<CONFIG:RELEASE>:/Oi>)
    ENDIF()
    TARGET_LINK_LIBRARIES(${NAME} PRIVATE plugin_manager)
    SET_TARGET_PROPERTIES(${NAME} PROPERTIES FOLDER "Plugin Tests")
    SET_TARGET_PROPERTIES(${NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
    ADD_CUSTOM_COMMAND(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND}
        -E copy_directory "${CMAKE_INSTALL_PREFIX}/bin/" "${CMAKE_CURRENT_BINARY_DIR}/")
    ADD_CUSTOM_COMMAND(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND}
        -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/cfg/" "${CMAKE_CURRENT_BINARY_DIR}/")
ENDFUNCTION()

ADD_SUBDIRECTORY(RendererContextTriangleTest)
ADD_SUBDIRECTORY(ResourceContextSceneTest)