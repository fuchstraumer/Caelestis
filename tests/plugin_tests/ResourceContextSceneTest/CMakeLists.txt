
ADD_PLUGIN_TEST(ResourceContextSceneTest
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanComplexScene.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanComplexScene.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "../test_fixtures/VulkanScene.hpp"
    "../test_fixtures/VulkanScene.cpp"
    "../test_fixtures/CommonCreationFunctions.hpp"
    "../test_fixtures/CommonCreationFunctions.cpp"
)


TARGET_INCLUDE_DIRECTORIES(ResourceContextSceneTest PRIVATE 
    "${Vulkan_INCLUDE_DIR}"
    "../../../third_party/glm"
    "../test_fixtures/"
    "../../../third_party/memory/include"
)

TARGET_LINK_LIBRARIES(ResourceContextSceneTest PRIVATE foonathan_memory vpr_core vpr_sync vpr_alloc vpr_command ${Vulkan_LIBRARY} glfw)
