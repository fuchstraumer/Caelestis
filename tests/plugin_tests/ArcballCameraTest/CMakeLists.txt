
SET(IMGUI_SOURCES 
    "../../../third_party/imgui/imgui.h"
    "../../../third_party/imgui/imconfig.h"
    "../../../third_party/imgui/imgui_draw.cpp"
    "../../../third_party/imgui/imgui.cpp"
    "../../../third_party/imgui/imgui_demo.cpp"
    "../test_fixtures/ImGuiWrapper.hpp"
    "../test_fixtures/ImGuiWrapper.cpp"
)

SET(TEST_FIXTURES
    "../test_fixtures/VulkanScene.hpp"
    "../test_fixtures/VulkanScene.cpp"
    "../test_fixtures/CommonCreationFunctions.hpp"
    "../test_fixtures/CommonCreationFunctions.cpp"
    "../test_fixtures/ObjModel.hpp"
    "../test_fixtures/ObjModel.cpp"
    "../test_fixtures/House.vert"
    "../test_fixtures/House.frag"
)

SET(ARCBALL_CAMERA_SOURCES
    "../test_fixtures/Camera.hpp"
    "../test_fixtures/Camera.cpp"
    "../test_fixtures/ArcballCameraController.hpp"
    "../test_fixtures/ArcballCameraController.cpp"
    "../test_fixtures/ArcballHelper.hpp"
    "../test_fixtures/ArcballHelper.cpp"
)

ADD_PLUGIN_TEST(ArcballCameraTest
    "main.cpp"
    "ArcballScene.hpp"
    "ArcballScene.cpp"
    ${TEST_FIXTURES}
    ${IMGUI_SOURCES}
    ${ARCBALL_CAMERA_SOURCES}
)

SOURCE_GROUP("imgui" FILES ${IMGUI_SOURCES})
SOURCE_GROUP("fixtures" FILES ${TEST_FIXTURES})
SOURCE_GROUP("arcball" FILES ${ARCBALL_CAMERA_SOURCES})

TARGET_COMPILE_DEFINITIONS(ArcballCameraTest PRIVATE "GLM_ENABLE_EXPERIMENTAL" "GLM_FORCE_RADIANS" "GLM_FORCE_DEPTH_ZERO_TO_ONE")

TARGET_INCLUDE_DIRECTORIES(ArcballCameraTest PRIVATE 
    "../test_fixtures/"
    "../../../plugins"
    "../../../third_party"
    "../../../ext/include"
    "../../../third_party/glm"
    "../../../third_party/gli"
    "../../../third_party/tinyobjloader/experimental"
    "../../../third_party/imgui"
    "${Vulkan_INCLUDE_DIR}"
)

TARGET_LINK_LIBRARIES(ArcballCameraTest PRIVATE vpr_core vpr_resource vpr_sync vpr_alloc 
    vpr_command vpr_render ${Vulkan_LIBRARY} easyloggingpp)
