#pragma once
#ifndef BASE_H
#define BASE_H
#include <vector>
#include <utility>
#include <memory>
#include <variant>
/*
    
    Defines a base module class.

    Each module inherits from this, so that we can
    link modules together safely.

    This mainly involves checking for compatible parameters
    between linked modules, and chaining together generate
    commands to create the final object.

*/
#include "CommonStructs.hpp"
#include "CommonDef.hpp"

namespace cnoise {

    struct cpu_module_data {
        std::vector<float> data;
        cpu_module_data() = default;
        cpu_module_data(const size_t& w, const size_t& h);
        std::vector<float> GetData() const noexcept;
        size_t width, height;
    };

    struct cuda_module_data {
        cuda_module_data(const cuda_module_data&) = delete;
        cuda_module_data& operator=(const cuda_module_data&) = delete;
        cuda_module_data(const size_t& w, const size_t& h);
        cuda_module_data(cuda_module_data&& other) noexcept;
        cuda_module_data& operator=(cuda_module_data&& other) noexcept;
        ~cuda_module_data();
        std::vector<float> GetData() const noexcept;
        size_t width, height;
        float* data;
    };

    class Module {
        // Delete copy ctor and operator
        Module(const Module& other) = delete;
        Module& operator=(const Module& other) = delete;
        Module(Module&& other) = delete;
        Module& operator=(Module&& other) = delete;
    public:

        static bool CUDA_LOADED;
        static bool VULKAN_LOADED;

        Module(const size_t& width, const size_t& height);
        virtual ~Module() {};

        virtual void ConnectModule(const std::shared_ptr<Module>& other);
        virtual void Generate() = 0;


        virtual std::vector<float> GetData() const;

        virtual Module& GetModule(size_t idx) const;
        virtual size_t GetSourceModuleCount() const = 0;

        virtual std::vector<float> GetDataNormalized(float upper_bound, float lower_bound) const;

        // Save current module to an image with name "name"
        virtual void SaveToPNG(const char* name);

        void SaveToPNG_16(const char * filename);

        void SaveRaw32(const char * filename);

        void SaveToTER(const char * name);

        // Tells us whether or not this module has already Generated data.
        bool Generated;
        float* GetDataPtr();

        const size_t& Width() const noexcept;
        const size_t& Height() const noexcept;

        const auto& GetVariant() const noexcept {
            return data;
        }

    protected:

        void checkSourceModules();

        // Dimensions of textures.
        std::pair<size_t, size_t> dims;
        std::variant<cpu_module_data, cuda_module_data> data;
        // Modules that precede this module, with the back 
        // of the vector being the module immediately before 
        // this one, and the front of the vector being the initial
        // module.
        std::vector<std::shared_ptr<Module>> sourceModules;
    };

        namespace generators {

            // Config struct for noise generators.
            struct noiseCfg {

                // Seed for the noise generator
                int Seed;
                // Frequency of the noise
                float Frequency;
                // Lacunarity controls amplitude of the noise, effectively
                float Lacunarity;
                // Controls how many octaves to use during octaved noise generation
                int Octaves;
                // Persistence controls how the amplitude of successive octaves decreases.
                float Persistence;

                noiseCfg(int s, float f, float l, int o, float p) : Seed(s), Frequency(f), Lacunarity(l), Octaves(o), Persistence(p) {}

                noiseCfg() = default;
                ~noiseCfg() = default;

            };

        }
}


#endif // !BASE_H
