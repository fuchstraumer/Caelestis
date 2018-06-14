#ifndef VPSK_GENERATED_BINDINGS_PIPELINERESOURCE_HPP
#define VPSK_GENERATED_BINDINGS_PIPELINERESOURCE_HPP
#include "graph/PipelineResource.hpp"
#include "sol.hpp"

template<typename T>
inline void BindTypeToLua(lua_State* state);

template<>
inline void BindTypeToLua<vpsk::image_info_t>(lua_State* state) {
    sol::state_view solState(state);
    solState.new_usertype<vpsk::image_info_t>("image_info_t",
        "SizeX", &vpsk::image_info_t::SizeX,
        "SizeY", &vpsk::image_info_t::SizeY,
        "MipLevels", &vpsk::image_info_t::MipLevels,
        "ArrayLayers", &vpsk::image_info_t::ArrayLayers
    );
}

template<>
inline void BindTypeToLua<vpsk::buffer_info_t>(lua_State* state) {
    sol::state_view solState(state);
    solState.new_usertype<vpsk::buffer_info_t>("buffer_info_t",
        "Size", &vpsk::buffer_info_t::Size
    );
}

template<>
inline void BindTypeToLua<vpsk::resource_dimensions_t>(lua_State* state) {
    sol::state_view solState(state);
    solState.new_usertype<vpsk::resource_dimensions_t>("resource_dimensions_t",
        "BufferInfo", &vpsk::resource_dimensions_t::BufferInfo,
        "Width", &vpsk::resource_dimensions_t::Width,
        "Height", &vpsk::resource_dimensions_t::Height,
        "Depth", &vpsk::resource_dimensions_t::Depth,
        "Layers", &vpsk::resource_dimensions_t::Layers,
        "Levels", &vpsk::resource_dimensions_t::Levels,
        "Samples", &vpsk::resource_dimensions_t::Samples,
        "Transient", &vpsk::resource_dimensions_t::Transient,
        "Persistent", &vpsk::resource_dimensions_t::Persistent,
        "Storage", &vpsk::resource_dimensions_t::Storage
    );
}

template<>
inline void BindTypeToLua<vpsk::PipelineResource>(lua_State* state) {
    sol::state_view solState(state);
    solState.new_usertype<vpsk::PipelineResource>("PipelineResource",
        sol::constructors<vpsk::PipelineResource(std::string, size_t)>(),
        "IsBuffer", &vpsk::PipelineResource::IsBuffer,
        "IsImage", &vpsk::PipelineResource::IsImage,
        "IsStorage", &vpsk::PipelineResource::IsStorage,
        "IsTransient", &vpsk::PipelineResource::IsTransient,
        "WrittenBySubmission", &vpsk::PipelineResource::WrittenBySubmission,
        "ReadBySubmission", &vpsk::PipelineResource::ReadBySubmission,
        "SetIdx", &vpsk::PipelineResource::SetIdx,
        "SetParentSetName", &vpsk::PipelineResource::SetParentSetName,
        "SetName", &vpsk::PipelineResource::SetName,
        "SetDescriptorType", &vpsk::PipelineResource::SetDescriptorType,
        "SetUsedPipelineStages", &vpsk::PipelineResource::SetUsedPipelineStages,
        "AddUsedPipelineStages", &vpsk::PipelineResource::AddUsedPipelineStages,
        "SetInfo", &vpsk::PipelineResource::SetInfo,
        "SetStorage", &vpsk::PipelineResource::SetStorage,
        "SetTransient", &vpsk::PipelineResource::SetTransient,
        "GetIdx", &vpsk::PipelineResource::GetIdx,
        "ParentSetName", &vpsk::PipelineResource::ParentSetName,
        "DescriptorType", &vpsk::PipelineResource::DescriptorType,
        "Name", &vpsk::PipelineResource::Name,
        "PipelineStages", &vpsk::PipelineResource::PipelineStages,
        "SubmissionsReadIn", &vpsk::PipelineResource::SubmissionsReadIn,
        "SubmissionsWrittenIn", &vpsk::PipelineResource::SubmissionsWrittenIn,
        "GetInfo", &vpsk::PipelineResource::GetInfo,
        "GetImageInfo", &vpsk::PipelineResource::GetImageInfo,
        "GetBufferInfo", &vpsk::PipelineResource::GetBufferInfo,
        "AllStagesUsedIn", &vpsk::PipelineResource::AllStagesUsedIn
    );
}

#endif //!VPSK_GENERATED_BINDINGS_PIPELINERESOURCE_HPP
