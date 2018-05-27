#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "RendererCore.hpp"
#include "objects/MeshPrimitives.hpp"
#include "resources/MeshData.hpp"
#include "components/graphics/DrawCommandComponents.hpp"
#include "components/NameComponent.hpp"
#include "chrysocyon/core/Registry.hpp" 
#include "chrysocyon/core/ComponentViews.hpp"

TEST_CASE("TestECS_DrawSystem") {
    using namespace vpsk;
    DefaultRegistryType registry;
    std::unique_ptr<MeshData> mesh = CreateBox();
    auto& renderer = RendererCore::GetRenderer();
    SUBCASE("CreateMeshBuffers") {
        mesh->CreateBuffers(renderer.Device());

        SUBCASE("CreateEntity") {
            uint32_t main_ent = registry.Create();

            SUBCASE("AssignToEntity") {
                registry.AssignComponent<NameComponent>(main_ent, "MainEntity");
                registry.AssignComponent<VertexBufferComponent>(main_ent, (VertexBufferComponent)*mesh);
                registry.AssignComponent<IndexBufferComponent>(main_ent, (IndexBufferComponent)*mesh);
                registry.AssignComponent<IndexedDrawCommandComponent>(main_ent, static_cast<uint32_t>(mesh->Indices.size()), uint32_t(1), uint32_t(0), uint32_t(0), uint32_t(0));
            }

            SUBCASE("CreateView") {
                registry.PreparePersistentViews<VertexBufferComponent, IndexBufferComponent, IndexedDrawCommandComponent>();
                auto& draw_components_view = registry.PersistentView<VertexBufferComponent, IndexBufferComponent, IndexedDrawCommandComponent>();
            }
        }

    }

}