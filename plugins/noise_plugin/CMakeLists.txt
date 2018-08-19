
SET(NOISE_IMAGE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/image/Image.hpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/image/Image.cpp"
)

SOURCE_GROUP("image" FILES ${NOISE_IMAGE})
SOURCE_GROUP("models" FILES ${NOISE_MODELS})
SOURCE_GROUP("shaders/combiners" FILES ${NOISE_COMBINER_SHADERS})
SOURCE_GROUP("shaders/modifiers" FILES ${NOISE_MODIFIER_SHADERS})
SOURCE_GROUP("shaders/generators" FILES ${NOISE_GENERATOR_SHADERS})

ADD_PLUGIN(noise_plugin
    "include/ModuleTypes.hpp"
    "include/NoiseModule.hpp"
    "include/ComputeDispatcher.hpp"
    "include/NoisePluginAPI.hpp"
    "src/ComputeDispatcher.cpp"
    "src/NoisePluginAPI.cpp"
    ${NOISE_IMAGE}
    ${NOISE_MODELS}
    ${NOISE_COMBINER_SHADERS}
    ${NOISE_MODIFIER_SHADERS}
    ${NOISE_GENERATOR_SHADERS}
)

TARGET_INCLUDE_DIRECTORIES(noise_plugin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models" ${VPR_INCLUDE_DIRS_PLUGIN_RELATIVE} "../../plugins")
TARGET_INCLUDE_DIRECTORIES(noise_plugin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/common" "${CMAKE_CURRENT_SOURCE_DIR}/frontend/modules")
TARGET_INCLUDE_DIRECTORIES(noise_plugin PRIVATE "../resource_context/include" "../renderer_context/include" "${Vulkan_INCLUDE_DIR}")
