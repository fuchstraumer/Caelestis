
ADD_PLUGIN_TEST(RendererContextTriangleTest
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanTriangle.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanTriangle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Triangle.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/Triangle.frag"
    "../test_fixtures/VulkanScene.hpp"
    "../test_fixtures/VulkanScene.cpp"
    "../test_fixtures/CommonCreationFunctions.hpp"
    "../test_fixtures/CommonCreationFunctions.cpp"
)

TARGET_INCLUDE_DIRECTORIES(RendererContextTriangleTest PRIVATE 
    "${Vulkan_INCLUDE_DIR}"
    "../../../third_party/glm"
    "../test_fixtures/"
)

TARGET_LINK_LIBRARIES(RendererContextTriangleTest PRIVATE vpr_core vpr_sync ${Vulkan_LIBRARY})
