#pragma once
#ifndef VPSK_PHONG_RENDERPASS_HPP
#define VPSK_PHONG_RENDERPASS_HPP
#include "BasePass.hpp"

namespace vpsk {

    /**The PhongPass assumes lighting will be performed using primitive phong methods,
     * and thus makes no fancy accomodations for rendering or lighting techniques. This
     * bass is well-used for testing geometry or textures for errors, since it's easy to 
     * setup and only uses simple shaders.
     * \ingroup Renderpasses
    */
    class PhongPass {
    public:

        PhongPass(const vpr::Device* dvc, const VkFormat& format, const VkSampleCountFlagBits& msaa_samples);

    private:
        
        VkFormat colorFormat;
        VkSampleCountFlagBits msaaSamples;
        const vpr::Device* device;
        void createRenderTargetAttachment();
        void createResolveAttachment();
        void createMultisampleDepthAttachment();
        void createResolveDepthAttachment();
        VkSubpassDescription createSubpassDescription();
        void createSubpassDependencies();
        void createRenderpass();
        void setCommonStateInfo();
        std::array<VkAttachmentDescription, 4> attachmentDescriptions;
        std::array<VkAttachmentReference, 4> attachmentReferences;
        std::array<VkSubpassDependency, 2> subpassDependencies;
        std::unique_ptr<vpr::Multisampling> msaa;
    };

}

#endif //!VPSK_PHONG_RENDERPASS_HPP