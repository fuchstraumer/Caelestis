#pragma once
#ifndef VPSK_RENDERABLE_COMPONENT_BASE_HPP
#define VPSK_RENDERABLE_COMPONENT_BASE_HPP
#include <string>
#include <unordered_map>
#include <variant>
#include <functional>
#include <memory>

namespace vpsk {
    
    using cvar_t = std::variant<float, double, int64_t, uint64_t, bool, std::string>;
    using cvar_map_t = std::unordered_map<std::string, cvar_t>;

    class RenderTarget;

    class RenderableComponent {
    public:
        cvar_map_t Parameters;
        std::function<void()> OnStart;
        std::function<void()> OnStop;

        RenderableComponent() = default;
        virtual ~RenderableComponent();

        virtual void Initialize();
        virtual void Load();
        virtual void Unload();
        virtual void Start();
        virtual void Stop();
        virtual void UpdateLogic();
        virtual void UpdatePhysics();
        virtual void Render();
        virtual void Compose();
        
    protected:

        virtual void ResizeCallback();
    };

}

#endif //!VPSK_RENDERABLE_COMPONENT_BASE_HPP
