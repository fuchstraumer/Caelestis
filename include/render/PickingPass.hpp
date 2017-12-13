#pragma once
#ifndef VPSK_PICKING_RENDERPASS_HPP
#define VPSK_PICKING_RENDERPASS_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <array>
namespace vpsk {

    class PickingPass {
        PickingPass(const PickingPass&) = delete;
        PickingPass& operator=(const PickingPass&) = delete;
    public:

    private:

        VkSubpassDescription subpassDescr;
        VkSubpassDependency subpassDependency;
        std::array<VkAttachmentDescription, 3> attachmentDescriptions;
        std::array<VkAttachmentReference, 3> attachmentReferences;
        std::unique_ptr<vpr::Image> outputAttachment;
        std::unique_ptr<vpr::Renderpass> renderpass;
        const vpr::Device* dvc;
    };

}

#endif //!VPSK_PICKING_RENDERPASS_HPP