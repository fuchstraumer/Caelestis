#pragma once
#ifndef NOISE_PLUGIN_BASE_MODULE_HPP
#define NOISE_PLUGIN_BASE_MODULE_HPP
#include <vector>
#include <utility>
#include <memory>
/*
    
    Defines a base module class.

    Each module inherits from this, so that we can
    link modules together safely.

    This mainly involves checking for compatible parameters
    between linked modules, and chaining together generate
    commands to create the final object.

*/

struct ResourceContext_API;
struct VulkanResource;

namespace cnoise {

    enum class noise_t {
        Invalid = 0,
        Simplex = 1,
        Perlin = 2
    };

    class Module {
        // Delete copy ctor and operator
        Module(const Module& other) = delete;
        Module& operator=(const Module& other) = delete;
    public:

        Module(const ResourceContext_API* rsrc_api, const size_t& width, const size_t& height);
        Module(Module&& other) noexcept;
        Module& operator=(Module&& other) noexcept;
        virtual ~Module();

        void ConnectModule(const std::shared_ptr<Module>& other);
        virtual void Generate() = 0;

        std::vector<float> GetData() const;

        const Module* GetModule(size_t idx) const;
        virtual size_t GetSourceModuleCount() const = 0;

        std::vector<float> GetDataNormalized(float upper_bound, float lower_bound) const;

        // Save current module to an image with name "name"
        void SaveToPNG(const char* name);
        void SaveToPNG_16(const char* filename);
        void SaveRaw32(const char* filename);
        void SaveToTER(const char* name);

        // Tells us whether or not this module has already Generated data.
        bool Generated;
        float* GetDataPtr();

        const size_t& Width() const noexcept;
        const size_t& Height() const noexcept;

    protected:

        const ResourceContext_API* resourceApi;

        void checkSourceModules();

        VulkanResource* output;

        // Dimensions of textures.
        std::pair<size_t, size_t> dims;
        // Modules that precede this module, with the back 
        // of the vector being the module immediately before 
        // this one, and the front of the vector being the initial
        // module.
        std::vector<std::shared_ptr<Module>> sourceModules;
    };

    namespace generators {

        // Config struct for noise generators.
        struct noiseCfg {
            float Frequency;
            float Lacunarity;
            float Persistence;
            float Warp;
            float Damp;
            float DampScale;
            int Seed;
            int Octaves;

            noiseCfg(int s, float f, float l, int o, float p) : Seed(s), Frequency(f), Lacunarity(l), Octaves(o), Persistence(p) {}

            noiseCfg() = default;
            ~noiseCfg() = default;

        };

    }
}


#endif // !NOISE_PLUGIN_BASE_MODULE_HPP
