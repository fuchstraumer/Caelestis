
#include "../../../plugins/renderer_context/include/RendererContextAPI.hpp"
#include "../../../plugins//renderer_context/include/core/RendererContext.hpp"
#include "../../../plugin_manager/include/PluginManager.hpp"
#include "../../..//plugin_manager/include/CoreAPIs.hpp"
#include "ResourceLoader.hpp"
#include "TransferSystem.hpp"
#include "chrysocyon/signal/Delegate.hpp"
#include "GLFW/glfw3.h"

static RendererContext* context = nullptr;

struct ResourceCommandPool_T {

};

using ResourceCommandPool = ResourceCommandPool_T*;

struct ResourceCommandBuffer_T {


    bool waitingForLoad : 1;
    void* hostData;
};

using ResourceCommandBuffer = ResourceCommandBuffer_T*;



int main(int argc, char* argv[]) {
    PluginManager& manager = PluginManager::GetPluginManager();
    manager.LoadPlugin("application_context.dll");
    manager.LoadPlugin("renderer_context.dll");
    manager.LoadPlugin("resource_context.dll");

    RendererContext_API* renderer_api = reinterpret_cast<RendererContext_API*>(manager.RetrieveAPI(RENDERER_CONTEXT_API_ID));
    context = renderer_api->GetContext();

}
