#pragma once
#ifndef VPSK_IMAGE_RESOURCE_CACHE_HPP
#define VPSK_IMAGE_RESOURCE_CACHE_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace vpsk {

    class ImageResourceCache {
        ImageResourceCache(const ImageResourceCache&) = delete;
        ImageResourceCache& operator=(const ImageResourceCache&) = delete;
    public:
    };

}

#endif //!VPSK_IMAGE_RESOURCE_CACHE_HPP
