#include "util/EnumStringConverters.hpp"
#include <map>
#include <cassert>
namespace vpsk {

    VkImageLayout shortformConvertLayout(const std::string& str) {
        if (str == "PRESENT_SRC") {
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        else if (str == "COLOR_ATTACHMENT") {
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        else if (str == "DEPTH_STENCIL_ATTACHMENT") {
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else if (str == "DEPTH_STENCIL_READ_ONLY") {
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }
        else if (str == "SHADER_READ") {
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        else if (str == "TRANFER_SRC") {
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        }
        else if (str == "TRANSFER_DST") {
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }
        else {
            throw std::domain_error("Invalid shortform string passed to image layout enum getter.");
        }
    }
    

    VkImageLayout LayoutFromStr(const std::string & str, bool shortform) {
        if (shortform) {
            return shortformConvertLayout(str);
        }
        else {
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }

    const static std::map<const std::string, const VkFormat> shortform_formats_map {
        std::make_pair<const std::string, const VkFormat>( "UNDEFINED", VK_FORMAT_UNDEFINED),
        std::make_pair<const std::string, const VkFormat>( "R4G4_UNORM_PACK8", VK_FORMAT_R4G4_UNORM_PACK8),
        std::make_pair<const std::string, const VkFormat>( "R4G4B4A4_UNORM_PACK16", VK_FORMAT_R4G4B4A4_UNORM_PACK16),
        std::make_pair<const std::string, const VkFormat>( "B4G4R4A4_UNORM_PACK16", VK_FORMAT_B4G4R4A4_UNORM_PACK16),
        std::make_pair<const std::string, const VkFormat>( "R5G6B5_UNORM_PACK16", VK_FORMAT_R5G6B5_UNORM_PACK16),
        std::make_pair<const std::string, const VkFormat>( "B5G6R5_UNORM_PACK16", VK_FORMAT_B5G6R5_UNORM_PACK16),
        std::make_pair<const std::string, const VkFormat>( "R5G5B5A1_UNORM_PACK16", VK_FORMAT_R5G5B5A1_UNORM_PACK16),
        std::make_pair<const std::string, const VkFormat>( "B5G5R5A1_UNORM_PACK16", VK_FORMAT_B5G5R5A1_UNORM_PACK16),
        std::make_pair<const std::string, const VkFormat>( "A1R5G5B5_UNORM_PACK16", VK_FORMAT_A1R5G5B5_UNORM_PACK16),
        std::make_pair<const std::string, const VkFormat>( "R8_UNORM", VK_FORMAT_R8_UNORM),
        std::make_pair<const std::string, const VkFormat>( "R8_SNORM", VK_FORMAT_R8_SNORM),
        std::make_pair<const std::string, const VkFormat>( "R8_USCALED", VK_FORMAT_R8_USCALED),
        std::make_pair<const std::string, const VkFormat>( "R8_SSCALED", VK_FORMAT_R8_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "R8_UINT", VK_FORMAT_R8_UINT),
        std::make_pair<const std::string, const VkFormat>( "R8_SINT", VK_FORMAT_R8_SINT),
        std::make_pair<const std::string, const VkFormat>( "R8_SRGB", VK_FORMAT_R8_SRGB),
        std::make_pair<const std::string, const VkFormat>( "R8G8_UNORM", VK_FORMAT_R8G8_UNORM),
        std::make_pair<const std::string, const VkFormat>( "R8G8_SNORM", VK_FORMAT_R8G8_SNORM),
        std::make_pair<const std::string, const VkFormat>( "R8G8_USCALED", VK_FORMAT_R8G8_USCALED),
        std::make_pair<const std::string, const VkFormat>( "R8G8_SSCALED", VK_FORMAT_R8G8_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "R8G8_UINT", VK_FORMAT_R8G8_UINT),
        std::make_pair<const std::string, const VkFormat>( "R8G8_SINT", VK_FORMAT_R8G8_SINT),
        std::make_pair<const std::string, const VkFormat>( "R8G8_SRGB", VK_FORMAT_R8G8_SRGB),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8_UNORM", VK_FORMAT_R8G8B8_UNORM),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8_SNORM", VK_FORMAT_R8G8B8_SNORM),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8_USCALED", VK_FORMAT_R8G8B8_USCALED),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8_SSCALED", VK_FORMAT_R8G8B8_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8_UINT", VK_FORMAT_R8G8B8_UINT),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8_SINT", VK_FORMAT_R8G8B8_SINT),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8_SRGB", VK_FORMAT_R8G8B8_SRGB),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8_UNORM", VK_FORMAT_B8G8R8_UNORM),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8_SNORM", VK_FORMAT_B8G8R8_SNORM),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8_USCALED", VK_FORMAT_B8G8R8_USCALED),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8_SSCALED", VK_FORMAT_B8G8R8_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8_UINT", VK_FORMAT_B8G8R8_UINT),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8_SINT", VK_FORMAT_B8G8R8_SINT),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8_SRGB", VK_FORMAT_B8G8R8_SRGB),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8A8_UNORM", VK_FORMAT_R8G8B8A8_UNORM),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8A8_SNORM", VK_FORMAT_R8G8B8A8_SNORM),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8A8_USCALED", VK_FORMAT_R8G8B8A8_USCALED),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8A8_SSCALED", VK_FORMAT_R8G8B8A8_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8A8_UINT", VK_FORMAT_R8G8B8A8_UINT),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8A8_SINT", VK_FORMAT_R8G8B8A8_SINT),
        std::make_pair<const std::string, const VkFormat>( "R8G8B8A8_SRGB", VK_FORMAT_R8G8B8A8_SRGB),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8A8_UNORM", VK_FORMAT_B8G8R8A8_UNORM),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8A8_SNORM", VK_FORMAT_B8G8R8A8_SNORM),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8A8_USCALED", VK_FORMAT_B8G8R8A8_USCALED),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8A8_SSCALED", VK_FORMAT_B8G8R8A8_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8A8_UINT", VK_FORMAT_B8G8R8A8_UINT),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8A8_SINT", VK_FORMAT_B8G8R8A8_SINT),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8A8_SRGB", VK_FORMAT_B8G8R8A8_SRGB),
        std::make_pair<const std::string, const VkFormat>( "A8B8G8R8_UNORM_PACK32", VK_FORMAT_A8B8G8R8_UNORM_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A8B8G8R8_SNORM_PACK32", VK_FORMAT_A8B8G8R8_SNORM_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A8B8G8R8_USCALED_PACK32", VK_FORMAT_A8B8G8R8_USCALED_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A8B8G8R8_SSCALED_PACK32", VK_FORMAT_A8B8G8R8_SSCALED_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A8B8G8R8_UINT_PACK32", VK_FORMAT_A8B8G8R8_UINT_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A8B8G8R8_SINT_PACK32", VK_FORMAT_A8B8G8R8_SINT_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A8B8G8R8_SRGB_PACK32", VK_FORMAT_A8B8G8R8_SRGB_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2R10G10B10_UNORM_PACK32", VK_FORMAT_A2R10G10B10_UNORM_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2R10G10B10_SNORM_PACK32", VK_FORMAT_A2R10G10B10_SNORM_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2R10G10B10_USCALED_PACK32", VK_FORMAT_A2R10G10B10_USCALED_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2R10G10B10_SSCALED_PACK32", VK_FORMAT_A2R10G10B10_SSCALED_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2R10G10B10_UINT_PACK32", VK_FORMAT_A2R10G10B10_UINT_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2R10G10B10_SINT_PACK32", VK_FORMAT_A2R10G10B10_SINT_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2B10G10R10_UNORM_PACK32", VK_FORMAT_A2B10G10R10_UNORM_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2B10G10R10_SNORM_PACK32", VK_FORMAT_A2B10G10R10_SNORM_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2B10G10R10_USCALED_PACK32", VK_FORMAT_A2B10G10R10_USCALED_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2B10G10R10_SSCALED_PACK32", VK_FORMAT_A2B10G10R10_SSCALED_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2B10G10R10_UINT_PACK32", VK_FORMAT_A2B10G10R10_UINT_PACK32),
        std::make_pair<const std::string, const VkFormat>( "A2B10G10R10_SINT_PACK32", VK_FORMAT_A2B10G10R10_SINT_PACK32),
        std::make_pair<const std::string, const VkFormat>( "R16_UNORM", VK_FORMAT_R16_UNORM),
        std::make_pair<const std::string, const VkFormat>( "R16_SNORM", VK_FORMAT_R16_SNORM),
        std::make_pair<const std::string, const VkFormat>( "R16_USCALED", VK_FORMAT_R16_USCALED),
        std::make_pair<const std::string, const VkFormat>( "R16_SSCALED", VK_FORMAT_R16_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "R16_UINT", VK_FORMAT_R16_UINT),
        std::make_pair<const std::string, const VkFormat>( "R16_SINT", VK_FORMAT_R16_SINT),
        std::make_pair<const std::string, const VkFormat>( "R16_SFLOAT", VK_FORMAT_R16_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R16G16_UNORM", VK_FORMAT_R16G16_UNORM),
        std::make_pair<const std::string, const VkFormat>( "R16G16_SNORM", VK_FORMAT_R16G16_SNORM),
        std::make_pair<const std::string, const VkFormat>( "R16G16_USCALED", VK_FORMAT_R16G16_USCALED),
        std::make_pair<const std::string, const VkFormat>( "R16G16_SSCALED", VK_FORMAT_R16G16_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "R16G16_UINT", VK_FORMAT_R16G16_UINT),
        std::make_pair<const std::string, const VkFormat>( "R16G16_SINT", VK_FORMAT_R16G16_SINT),
        std::make_pair<const std::string, const VkFormat>( "R16G16_SFLOAT", VK_FORMAT_R16G16_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16_UNORM", VK_FORMAT_R16G16B16_UNORM),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16_SNORM", VK_FORMAT_R16G16B16_SNORM),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16_USCALED", VK_FORMAT_R16G16B16_USCALED),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16_SSCALED", VK_FORMAT_R16G16B16_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16_UINT", VK_FORMAT_R16G16B16_UINT),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16_SINT", VK_FORMAT_R16G16B16_SINT),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16_SFLOAT", VK_FORMAT_R16G16B16_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16A16_UNORM", VK_FORMAT_R16G16B16A16_UNORM),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16A16_SNORM", VK_FORMAT_R16G16B16A16_SNORM),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16A16_USCALED", VK_FORMAT_R16G16B16A16_USCALED),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16A16_SSCALED", VK_FORMAT_R16G16B16A16_SSCALED),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16A16_UINT", VK_FORMAT_R16G16B16A16_UINT),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16A16_SINT", VK_FORMAT_R16G16B16A16_SINT),
        std::make_pair<const std::string, const VkFormat>( "R16G16B16A16_SFLOAT", VK_FORMAT_R16G16B16A16_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R32_UINT", VK_FORMAT_R32_UINT),
        std::make_pair<const std::string, const VkFormat>( "R32_SINT", VK_FORMAT_R32_SINT),
        std::make_pair<const std::string, const VkFormat>( "R32_SFLOAT", VK_FORMAT_R32_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R32G32_UINT", VK_FORMAT_R32G32_UINT),
        std::make_pair<const std::string, const VkFormat>( "R32G32_SINT", VK_FORMAT_R32G32_SINT),
        std::make_pair<const std::string, const VkFormat>( "R32G32_SFLOAT", VK_FORMAT_R32G32_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R32G32B32_UINT", VK_FORMAT_R32G32B32_UINT),
        std::make_pair<const std::string, const VkFormat>( "R32G32B32_SINT", VK_FORMAT_R32G32B32_SINT),
        std::make_pair<const std::string, const VkFormat>( "R32G32B32_SFLOAT", VK_FORMAT_R32G32B32_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R32G32B32A32_UINT", VK_FORMAT_R32G32B32A32_UINT),
        std::make_pair<const std::string, const VkFormat>( "R32G32B32A32_SINT", VK_FORMAT_R32G32B32A32_SINT),
        std::make_pair<const std::string, const VkFormat>( "R32G32B32A32_SFLOAT", VK_FORMAT_R32G32B32A32_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R64_UINT", VK_FORMAT_R64_UINT),
        std::make_pair<const std::string, const VkFormat>( "R64_SINT", VK_FORMAT_R64_SINT),
        std::make_pair<const std::string, const VkFormat>( "R64_SFLOAT", VK_FORMAT_R64_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R64G64_UINT", VK_FORMAT_R64G64_UINT),
        std::make_pair<const std::string, const VkFormat>( "R64G64_SINT", VK_FORMAT_R64G64_SINT),
        std::make_pair<const std::string, const VkFormat>( "R64G64_SFLOAT", VK_FORMAT_R64G64_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R64G64B64_UINT", VK_FORMAT_R64G64B64_UINT),
        std::make_pair<const std::string, const VkFormat>( "R64G64B64_SINT", VK_FORMAT_R64G64B64_SINT),
        std::make_pair<const std::string, const VkFormat>( "R64G64B64_SFLOAT", VK_FORMAT_R64G64B64_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "R64G64B64A64_UINT", VK_FORMAT_R64G64B64A64_UINT),
        std::make_pair<const std::string, const VkFormat>( "R64G64B64A64_SINT", VK_FORMAT_R64G64B64A64_SINT),
        std::make_pair<const std::string, const VkFormat>( "R64G64B64A64_SFLOAT", VK_FORMAT_R64G64B64A64_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "B10G11R11_UFLOAT_PACK32", VK_FORMAT_B10G11R11_UFLOAT_PACK32),
        std::make_pair<const std::string, const VkFormat>( "E5B9G9R9_UFLOAT_PACK32", VK_FORMAT_E5B9G9R9_UFLOAT_PACK32),
        std::make_pair<const std::string, const VkFormat>( "D16_UNORM", VK_FORMAT_D16_UNORM),
        std::make_pair<const std::string, const VkFormat>( "X8_D24_UNORM_PACK32", VK_FORMAT_X8_D24_UNORM_PACK32),
        std::make_pair<const std::string, const VkFormat>( "D32_SFLOAT", VK_FORMAT_D32_SFLOAT),
        std::make_pair<const std::string, const VkFormat>( "S8_UINT", VK_FORMAT_S8_UINT),
        std::make_pair<const std::string, const VkFormat>( "D16_UNORM_S8_UINT", VK_FORMAT_D16_UNORM_S8_UINT),
        std::make_pair<const std::string, const VkFormat>( "D24_UNORM_S8_UINT", VK_FORMAT_D24_UNORM_S8_UINT),
        std::make_pair<const std::string, const VkFormat>( "D32_SFLOAT_S8_UINT", VK_FORMAT_D32_SFLOAT_S8_UINT),
        std::make_pair<const std::string, const VkFormat>( "BC1_RGB_UNORM_BLOCK", VK_FORMAT_BC1_RGB_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC1_RGB_SRGB_BLOCK", VK_FORMAT_BC1_RGB_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC1_RGBA_UNORM_BLOCK", VK_FORMAT_BC1_RGBA_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC1_RGBA_SRGB_BLOCK", VK_FORMAT_BC1_RGBA_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC2_UNORM_BLOCK", VK_FORMAT_BC2_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC2_SRGB_BLOCK", VK_FORMAT_BC2_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC3_UNORM_BLOCK", VK_FORMAT_BC3_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC3_SRGB_BLOCK", VK_FORMAT_BC3_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC4_UNORM_BLOCK", VK_FORMAT_BC4_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC4_SNORM_BLOCK", VK_FORMAT_BC4_SNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC5_UNORM_BLOCK", VK_FORMAT_BC5_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC5_SNORM_BLOCK", VK_FORMAT_BC5_SNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC6H_UFLOAT_BLOCK", VK_FORMAT_BC6H_UFLOAT_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC6H_SFLOAT_BLOCK", VK_FORMAT_BC6H_SFLOAT_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC7_UNORM_BLOCK", VK_FORMAT_BC7_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "BC7_SRGB_BLOCK", VK_FORMAT_BC7_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ETC2_R8G8B8_UNORM_BLOCK", VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ETC2_R8G8B8_SRGB_BLOCK", VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ETC2_R8G8B8A1_UNORM_BLOCK", VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ETC2_R8G8B8A1_SRGB_BLOCK", VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ETC2_R8G8B8A8_UNORM_BLOCK", VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ETC2_R8G8B8A8_SRGB_BLOCK", VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "EAC_R11_UNORM_BLOCK", VK_FORMAT_EAC_R11_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "EAC_R11_SNORM_BLOCK", VK_FORMAT_EAC_R11_SNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "EAC_R11G11_UNORM_BLOCK", VK_FORMAT_EAC_R11G11_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "EAC_R11G11_SNORM_BLOCK", VK_FORMAT_EAC_R11G11_SNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_4x4_UNORM_BLOCK", VK_FORMAT_ASTC_4x4_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_4x4_SRGB_BLOCK", VK_FORMAT_ASTC_4x4_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_5x4_UNORM_BLOCK", VK_FORMAT_ASTC_5x4_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_5x4_SRGB_BLOCK", VK_FORMAT_ASTC_5x4_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_5x5_UNORM_BLOCK", VK_FORMAT_ASTC_5x5_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_5x5_SRGB_BLOCK", VK_FORMAT_ASTC_5x5_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_6x5_UNORM_BLOCK", VK_FORMAT_ASTC_6x5_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_6x5_SRGB_BLOCK", VK_FORMAT_ASTC_6x5_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_6x6_UNORM_BLOCK", VK_FORMAT_ASTC_6x6_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_6x6_SRGB_BLOCK", VK_FORMAT_ASTC_6x6_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_8x5_UNORM_BLOCK", VK_FORMAT_ASTC_8x5_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_8x5_SRGB_BLOCK", VK_FORMAT_ASTC_8x5_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_8x6_UNORM_BLOCK", VK_FORMAT_ASTC_8x6_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_8x6_SRGB_BLOCK", VK_FORMAT_ASTC_8x6_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_8x8_UNORM_BLOCK", VK_FORMAT_ASTC_8x8_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_8x8_SRGB_BLOCK", VK_FORMAT_ASTC_8x8_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_10x5_UNORM_BLOCK", VK_FORMAT_ASTC_10x5_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_10x5_SRGB_BLOCK", VK_FORMAT_ASTC_10x5_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_10x6_UNORM_BLOCK", VK_FORMAT_ASTC_10x6_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_10x6_SRGB_BLOCK", VK_FORMAT_ASTC_10x6_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_10x8_UNORM_BLOCK", VK_FORMAT_ASTC_10x8_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_10x8_SRGB_BLOCK", VK_FORMAT_ASTC_10x8_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_10x10_UNORM_BLOCK", VK_FORMAT_ASTC_10x10_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_10x10_SRGB_BLOCK", VK_FORMAT_ASTC_10x10_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_12x10_UNORM_BLOCK", VK_FORMAT_ASTC_12x10_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_12x10_SRGB_BLOCK", VK_FORMAT_ASTC_12x10_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_12x12_UNORM_BLOCK", VK_FORMAT_ASTC_12x12_UNORM_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "ASTC_12x12_SRGB_BLOCK", VK_FORMAT_ASTC_12x12_SRGB_BLOCK),
        std::make_pair<const std::string, const VkFormat>( "PVRTC1_2BPP_UNORM_BLOCK_IMG", VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG),
        std::make_pair<const std::string, const VkFormat>( "PVRTC1_4BPP_UNORM_BLOCK_IMG", VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG),
        std::make_pair<const std::string, const VkFormat>( "PVRTC2_2BPP_UNORM_BLOCK_IMG", VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG),
        std::make_pair<const std::string, const VkFormat>( "PVRTC2_4BPP_UNORM_BLOCK_IMG", VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG),
        std::make_pair<const std::string, const VkFormat>( "PVRTC1_2BPP_SRGB_BLOCK_IMG", VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG),
        std::make_pair<const std::string, const VkFormat>( "PVRTC1_4BPP_SRGB_BLOCK_IMG", VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG),
        std::make_pair<const std::string, const VkFormat>( "PVRTC2_2BPP_SRGB_BLOCK_IMG", VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG),
        std::make_pair<const std::string, const VkFormat>( "PVRTC2_4BPP_SRGB_BLOCK_IMG", VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG),
        std::make_pair<const std::string, const VkFormat>( "G8B8G8R8_422_UNORM_KHR", VK_FORMAT_G8B8G8R8_422_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "B8G8R8G8_422_UNORM_KHR", VK_FORMAT_B8G8R8G8_422_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G8_B8_R8_3PLANE_420_UNORM_KHR", VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G8_B8R8_2PLANE_420_UNORM_KHR", VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G8_B8_R8_3PLANE_422_UNORM_KHR", VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G8_B8R8_2PLANE_422_UNORM_KHR", VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G8_B8_R8_3PLANE_444_UNORM_KHR", VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "R10X6_UNORM_PACK16_KHR", VK_FORMAT_R10X6_UNORM_PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "R10X6G10X6_UNORM_2PACK16_KHR", VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR", VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR", VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR", VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR", VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR", VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR", VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR", VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR", VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "R12X4_UNORM_PACK16_KHR", VK_FORMAT_R12X4_UNORM_PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "R12X4G12X4_UNORM_2PACK16_KHR", VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR", VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR", VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR", VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR", VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR", VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR", VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR", VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR", VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR),
        std::make_pair<const std::string, const VkFormat>( "G16B16G16R16_422_UNORM_KHR", VK_FORMAT_G16B16G16R16_422_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "B16G16R16G16_422_UNORM_KHR", VK_FORMAT_B16G16R16G16_422_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G16_B16_R16_3PLANE_420_UNORM_KHR", VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G16_B16R16_2PLANE_420_UNORM_KHR", VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G16_B16_R16_3PLANE_422_UNORM_KHR", VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G16_B16R16_2PLANE_422_UNORM_KHR", VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR),
        std::make_pair<const std::string, const VkFormat>( "G16_B16_R16_3PLANE_444_UNORM_KHR", VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR),
    };

    VkFormat FormatFromStr(const std::string & str, bool shortform) {
        if (shortform) {
            assert(shortform_formats_map.count(str) != 0);
            return shortform_formats_map.at(str);
        }
        else {
            return VK_FORMAT_UNDEFINED;
        }
    }

    VkAttachmentLoadOp shortformConvertLoadOp(const std::string& str) {
        if (str == "DONT_CARE") {
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        else if (str == "LOAD") {
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        }
        else if (str == "CLEAR") {
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        }
        else {
            throw std::domain_error("Invalid shortform string passed to load op getter.");
        }
    }

    VkAttachmentLoadOp LoadOpFromStr(const std::string & str, bool shortform) {
        if (shortform) {
            return shortformConvertLoadOp(str);
        }
        else {
            return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
        }
    }

    VkAttachmentStoreOp shortformConvertStoreOp(const std::string& str) {
        if (str == "DONT_CARE") {
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
        else if (str == "STORE") {
            return VK_ATTACHMENT_STORE_OP_STORE;
        }
        else {
            throw std::domain_error("Invalid shortform string passed to store op getter.");
        }
    }

    VkAttachmentStoreOp StoreOpFromStr(const std::string & str, bool shortform) {
        if (shortform) {
            return shortformConvertStoreOp(str);
        }
        else {
            return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
        }
    }

}