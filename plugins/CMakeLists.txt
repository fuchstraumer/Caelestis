SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(VPR_INCLUDE_DIRS_PLUGIN_RELATIVE
    "../../third_party/vulpesrender/alloc/include"
    "../../third_party/vulpesrender/command/include"
    "../../third_party/vulpesrender/common"
    "../../third_party/vulpesrender/core/include"
    "../../third_party/vulpesrender/render/include"
    "../../third_party/vulpesrender/resource/include"
    "../../third_party/vulpesrender/sync/include"
)

FUNCTION(ADD_PLUGIN NAME)
    ADD_LIBRARY(${NAME} SHARED ${ARGN})
    TARGET_INCLUDE_DIRECTORIES(${NAME} PRIVATE "../../plugin_manager/include")
    TARGET_COMPILE_OPTIONS(${NAME} PRIVATE ${MODULE_CXX_FLAGS})
    IF(MSVC)
        # Multithreaded compile, and intrinsic functions
        TARGET_COMPILE_OPTIONS(${NAME} PRIVATE $<$<CONFIG:RELEASE>:/MP> $<$<CONFIG:RELWITHDEBINFO>:/MP>)
        TARGET_COMPILE_OPTIONS(${NAME} PRIVATE $<$<CONFIG:RELEASE>:/Oi>)
    ENDIF()
    TARGET_COMPILE_DEFINITIONS(${NAME} PRIVATE "DOCTEST_CONFIG_DISABLE")
    TARGET_COMPILE_DEFINITIONS(${NAME} PRIVATE "NOMINMAX" "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS"
        "_SCL_SECURE_NO_WARNINGS")
    INSTALL(TARGETS ${NAME} LIBRARY DESTINATION "lib" ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin")
        TARGET_COMPILE_DEFINITIONS(${NAME} PUBLIC 
        $<BUILD_INTERFACE:BUILDING_SHARED_LIBRARY>
        $<INSTALL_INTERFACE:USING_SHARED_LIBRARY>)
    SET_TARGET_PROPERTIES(${NAME} PROPERTIES FOLDER "Plugins")
    SET_TARGET_PROPERTIES(${NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
ENDFUNCTION()

ADD_SUBDIRECTORY(application_context)
ADD_SUBDIRECTORY(renderer_context)
#ADD_SUBDIRECTORY(resource_context)
#ADD_SUBDIRECTORY(lua_plugin)
#ADD_SUBDIRECTORY(rendergraph_plugin)
#ADD_SUBDIRECTORY(noise_plugin)
#ADD_SUBDIRECTORY(content_compiler)
