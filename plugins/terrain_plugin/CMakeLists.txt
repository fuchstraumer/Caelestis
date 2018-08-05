
ADD_PLUGIN(terrain_plugin
    "../../shared/primitives/include/AABB.hpp"
    "../../shared/primitives/include/ViewFrustum.hpp"
    "include/HeightNode.hpp"
    "include/HeightSample.hpp"
    "include/Noise.hpp"
    "include/PlanarMesh.hpp"
    "include/TerrainNode.hpp"
    "include/TerrainPluginAPI.hpp"
    "include/TerrainQuadtree.hpp"
    "../../shared/primitives/src/AABB.cpp"
    "../../shared/primitives/src/ViewFrustum.cpp"
    "src/HeightNode.cpp"
    "src/HeightSample.cpp"
    "src/Noise.cpp"
    "src/PlanarMesh.cpp"
    "src/sdnoise1234.cpp"
    "src/TerrainNode.cpp"
    "src/TerrainPluginAPI.cpp"
    "src/TerrainQuadtree.cpp"
)

TARGET_INCLUDE_DIRECTORIES(terrain_plugin PRIVATE 
    "${Vulkan_INCLUDE_DIR}"
    "../../ext/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "../../plugins"
    "../../third_party/"
    "../../third_party/glm"
    "../../third_party/easyloggingpp/src"
)

TARGET_LINK_LIBRARIES(terrain_plugin PRIVATE vpr_resource vpr_core)
