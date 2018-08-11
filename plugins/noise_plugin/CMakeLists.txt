
SET(NOISE_COMBINERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/combiners/Combiners.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/combiners/Add.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/combiners/Add.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/combiners/Blend.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/combiners/Blend.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/combiners/Max.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/combiners/Max.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/combiners/Min.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/combiners/Min.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/combiners/Power.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/combiners/Power.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/combiners/Select.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/combiners/Select.cpp"
)

SET(NOISE_COMBINER_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/combiners/add.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/combiners/blend.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/combiners/divide.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/combiners/max.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/combiners/min.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/combiners/multiply.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/combiners/power.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/combiners/select.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/combiners/subtract.comp"
)

SET(NOISE_MODIFIERS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/modifiers/Modifiers.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/modifiers/Abs.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/modifiers/Abs.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/modifiers/Clamp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/modifiers/Clamp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/modifiers/Curve.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/modifiers/Curve.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/modifiers/ScaleBias.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/modifiers/ScaleBias.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/modifiers/Terrace.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/modifiers/Terrace.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/modifiers/Turbulence.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/modifiers/Turbulence.cpp"
)

SET(NOISE_MODIFIER_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/modifiers/abs.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/modifiers/clamp.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/modifiers/curve.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/modifiers/downsample.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/modifiers/scalebias.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/modifiers/terrace.comp"
)

SET(NOISE_GENERATORS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/generators/Generators.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/generators/Billow.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/generators/Billow.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/generators/DecarpientierSwiss.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/generators/DecarpientierSwiss.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/generators/FBM.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/generators/FBM.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/generators/RidgedMulti.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/generators/RidgedMulti.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/generators/Voronoi.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/generators/Voronoi.cpp"
)

SET(NOISE_GENERATOR_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/generators/Billow3D.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/generators/Decarpientier2D.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/generators/Decarpientier3D.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/generators/FBM3D.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/generators/Jordan2D.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/generators/perlin2d.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/generators/Ridged3D.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/generators/simplex3d_deriv.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/generators/simplex3d.comp"
)

SET(NOISE_UTILITY
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/utility/Utility.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/utility/Cache.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/utility/Cache.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/utility/Checkerboard.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/utility/Checkerboard.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/utility/Constant.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/utility/Constant.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/utility/Normalize.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/utility/Normalize.cpp"
)

SET(NOISE_BASE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/modules/Base.hpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/modules/Base.cpp"
)

SET(NOISE_IMAGE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/image/Image.hpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/image/Image.cpp"
)

SOURCE_GROUP("base" FILES ${NOISE_BASE})
SOURCE_GROUP("combiners" FILES ${NOISE_COMBINERS})
SOURCE_GROUP("modifiers" FILES ${NOISE_MODIFIERS})
SOURCE_GROUP("generators" FILES ${NOISE_GENERATORS})
SOURCE_GROUP("utility" FILES ${NOISE_UTILITY})
SOURCE_GROUP("image" FILES ${NOISE_IMAGE})
SOURCE_GROUP("models" FILES ${NOISE_MODELS})
SOURCE_GROUP("shaders/combiners" FILES ${NOISE_COMBINER_SHADERS})
SOURCE_GROUP("shaders/modifiers" FILES ${NOISE_MODIFIER_SHADERS})
SOURCE_GROUP("shaders/generators" FILES ${NOISE_GENERATOR_SHADERS})

ADD_PLUGIN(noise_plugin
    ${NOISE_BASE}
    ${NOISE_COMBINERS}
    ${NOISE_GENERATORS}
    ${NOISE_MODIFIERS}
    ${NOISE_UTILITY}
    ${NOISE_IMAGE}
    ${NOISE_MODELS}
    ${NOISE_COMBINER_SHADERS}
    ${NOISE_MODIFIER_SHADERS}
    ${NOISE_GENERATOR_SHADERS}
)

TARGET_INCLUDE_DIRECTORIES(noise_plugin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models")
TARGET_INCLUDE_DIRECTORIES(noise_plugin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/common" "${CMAKE_CURRENT_SOURCE_DIR}/frontend/modules")
TARGET_INCLUDE_DIRECTORIES(noise_plugin PRIVATE "../resource_context/include" "../renderer_context/include" "${Vulkan_INCLUDE_DIR}")
