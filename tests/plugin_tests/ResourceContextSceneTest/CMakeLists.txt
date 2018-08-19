
ADD_PLUGIN_TEST(ResourceContextSceneTest
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanComplexScene.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanComplexScene.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "../test_fixtures/VulkanScene.hpp"
    "../test_fixtures/VulkanScene.cpp"
    "../test_fixtures/CommonCreationFunctions.hpp"
    "../test_fixtures/CommonCreationFunctions.cpp"
    "../test_fixtures/ObjModel.hpp"
    "../test_fixtures/ObjModel.cpp"
)

ADD_SUBDIRECTORY("../../../third_party/tinyobjloader" "${CMAKE_CURRENT_BINARY_DIR}/tinyobjloader")

TARGET_INCLUDE_DIRECTORIES(ResourceContextSceneTest PRIVATE 
    "${Vulkan_INCLUDE_DIR}"
    "../../../third_party/glm"
    "../test_fixtures/"
    "../../../third_party"
    "../../../ext/include"
    "../../../third_party/gli"
    "../../../third_party/tinyobjloader/experimental"
)

TARGET_LINK_LIBRARIES(ResourceContextSceneTest PRIVATE vpr_core vpr_resource
    vpr_sync vpr_alloc vpr_command ${Vulkan_LIBRARY} easyloggingpp)

IF(APPLE)# Needed to support Boost filesystem
SET(boost_min_ver 1.67.0)
SET(boost_libs system filesystem)
FIND_PACKAGE(Boost ${boost_min_ver})
IF(Boost_FOUND)
    FIND_PACKAGE(Boost ${boost_min_ver} COMPONENTS ${boost_libs})
ENDIF()
TARGET_LINK_LIBRARIES(ResourceContextSceneTest PRIVATE ${Boost_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(ResourceContextSceneTest PRIVATE ${Boost_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
ENDIF()

ADD_CUSTOM_COMMAND(TARGET ResourceContextSceneTest POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_file "../../../assets/textures/Starbox.dds" "${CMAKE_CURRENT_BINARY_DIR}/")
ADD_CUSTOM_COMMAND(TARGET ResourceContextSceneTest POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_file "../../../assets/textures/House.png" "${CMAKE_CURRENT_BINARY_DIR}/")
ADD_CUSTOM_COMMAND(TARGET ResourceContextSceneTest POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_file "../../../assets/models/House.obj"  "${CMAKE_CURRENT_BINARY_DIR}/")    
