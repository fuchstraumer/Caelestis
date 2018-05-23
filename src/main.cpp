#include <string>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <random>
#include <iostream>
#include "renderer/RendererCore.hpp"
#include "renderer/objects/MeshPrimitives.hpp"
#include "renderer/resources/MeshData.hpp"
#include "components/graphics/DrawCommandComponents.hpp"
#include "components/NameComponent.hpp"
#include "chrysocyon/core/Registry.hpp" 
#include "chrysocyon/core/ComponentViews.hpp"

#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP

void DrawEntity(uint32_t ent, vpsk::VertexBufferComponent& vbo, vpsk::IndexBufferComponent& ibo, vpsk::IndexedDrawCommandComponent& draw_cmd);

int main(int argc, char* argv[]) {
    using namespace vpsk;
    DefaultRegistryType registry;
    std::unique_ptr<MeshData> mesh = CreateBox();
    auto& renderer = RendererCore::GetRenderer();
    mesh->CreateBuffers(renderer.Device());
    uint32_t main_ent = registry.Create();
    registry.AssignComponent<NameComponent>(main_ent, "MainEntity");
    registry.AssignComponent<VertexBufferComponent>(main_ent, (VertexBufferComponent)*mesh);
    registry.AssignComponent<IndexBufferComponent>(main_ent, (IndexBufferComponent)*mesh);
    registry.AssignComponent<IndexedDrawCommandComponent>(main_ent, static_cast<uint32_t>(mesh->Indices.size()), uint32_t(1), uint32_t(0), uint32_t(0), uint32_t(0));   
    registry.PreparePersistentViews<VertexBufferComponent, IndexBufferComponent, IndexedDrawCommandComponent>();
    auto& draw_components_view = registry.PersistentView<VertexBufferComponent, IndexBufferComponent, IndexedDrawCommandComponent>();
    draw_components_view.ApplyToEach(&DrawEntity);
}

void DrawEntity(uint32_t ent, vpsk::VertexBufferComponent& vbo, vpsk::IndexBufferComponent& ibo, vpsk::IndexedDrawCommandComponent& draw_cmd) {
    using namespace vpsk;
    VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
    // bind our vertex and index buffers
    vbo(cmd_buffer);
    ibo(cmd_buffer);
    // issue an indexed draw command
    draw_cmd(cmd_buffer);
}