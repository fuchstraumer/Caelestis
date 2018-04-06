#pragma once
#ifndef VPSK_DESCRIPTOR_RESOURCES_HPP
#define VPSK_DESCRIPTOR_RESOURCES_HPP
#include "ForwardDecl.hpp"
#include "vulkan/vulkan.h"
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace vpsk {

    class DescriptorResources {
        DescriptorResources(const DescriptorResources&) = delete;
        DescriptorResources& operator=(const DescriptorResources&) = delete;
    public:

        DescriptorResources(const vpr::Device* dvc);
        ~DescriptorResources();

        size_t AddResources(const std::map<std::string, VkDescriptorSetLayoutBinding>& resources);
        size_t FindIdxOfSetWithResource(const std::string& name) const;

        void AllocatePool();
        void CreateSetLayouts();

    private:
        const vpr::Device* device;
        std::unique_ptr<vpr::DescriptorPool> descriptorPool;
        std::vector<std::unique_ptr<vpr::DescriptorSetLayout>> setLayouts;
        std::vector<std::unique_ptr<vpr::DescriptorSet>> descriptorSets;
        std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>> setResources;
        std::mutex insertionGuard;
    };

}

#endif //!VPSK_DESCRIPTOR_RESOURCES_HPP