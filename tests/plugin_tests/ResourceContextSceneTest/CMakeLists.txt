
ADD_PLUGIN_TEST(ResourceContextSceneTest
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanComplexScene.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanComplexScene.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "../test_fixtures/VulkanScene.hpp"
    "../test_fixtures/VulkanScene.cpp"
    "../test_fixtures/CommonCreationFunctions.hpp"
    "../test_fixtures/CommonCreationFunctions.cpp"
    "ObjModel.hpp"
    "ObjModel.cpp"
    "House.vert"
    "House.frag"
    "Skybox.vert"
    "Skybox.frag"
)

ADD_SUBDIRECTORY("../../../third_party/tinyobjloader" "${CMAKE_CURRENT_BINARY_DIR}/tinyobjloader")

TARGET_INCLUDE_DIRECTORIES(ResourceContextSceneTest PRIVATE 
    "${Vulkan_INCLUDE_DIR}"
    "../../../third_party/glm"
    "../test_fixtures/"
    "../../../third_party/memory/include"
    "../../../third_party"
    "../../../ext/include"
    "../../../third_party/gli"
    "../../../third_party/tinyobjloader/experimental"
)

TARGET_LINK_LIBRARIES(ResourceContextSceneTest PRIVATE foonathan_memory vpr_core vpr_resource
    vpr_sync vpr_alloc vpr_command ${Vulkan_LIBRARY} easyloggingpp)
