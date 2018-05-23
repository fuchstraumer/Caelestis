#include "components/graphics/RenderableComponent.hpp"

namespace vpsk {

    RenderableComponent::~RenderableComponent() {
        Unload();
    }

    void RenderableComponent::Initialize() {}

    void RenderableComponent::Load() {}

    void RenderableComponent::Unload() {}

    void RenderableComponent::Start() {}

    void RenderableComponent::Stop() {}

    void RenderableComponent::UpdateLogic() {}

    void RenderableComponent::UpdatePhysics() {}

    void RenderableComponent::Render() {}

    void RenderableComponent::Compose() {}
}