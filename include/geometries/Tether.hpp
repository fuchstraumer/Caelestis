#pragma once
#ifndef VPSK_TETHER_OBJECT_HPP
#define VPSK_TETHER_OBJECT_HPP
#include "ForwardDecl.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include <memory>
#include <vector>
#include <array>
#include <algorithm>

namespace vpsk {

    class Tether {
    public:

        Tether(const char* data_file);

    private:

        uint16_t addVertex(glm::vec3 p, glm::vec3 n);
        void createMeshData();
        void createMeshBuffers();
        void createUBOs();
        void setVertexPipelineData();
        void createDescriptorSet();
        void drawGui();
        void drawGuiPlots();
        void loadTetherData(const char* file);

        const vpr::Device* device;
        std::unique_ptr<vpr::Buffer> vbo, ebo, offsetUBO;
        std::unique_ptr<vpr::DescriptorSet> descriptorSet;
        std::vector<uint16_t> indices;
        std::vector<glm::vec3> vertices;
        std::vector<float> instanceData;

        struct tether_data_t {
            std::vector<float> RadialDistance;
            float MinDist, MaxDist;
            std::vector<float> InPlaneLibration;
            float MinIPL, MaxIPL;
            std::vector<float> OutOfPlaneLibration;
            float MinOOPL, MaxOOPL;
            std::vector<float> Tension;
            float MinTension, MaxTension;
        } tetherData;

        struct tether_data_buffer_t {
            std::array<float*, 2000> Dist;
            std::array<float*, 2000> IPL;
            std::array<float*, 2000> OOPL;
            std::array<float*, 2000> T;
            void rotate() {
                std::rotate(Dist.begin(), Dist.begin() + 1, Dist.end());
                std::rotate(IPL.begin(), IPL.begin() + 1, IPL.end());
                std::rotate(OOPL.begin(), OOPL.begin() + 1, OOPL.end());
                std::rotate(T.begin(), T.begin() + 1, T.end());
            }
        } tetherBuffer;

        struct timestep_offsets_t {
            timestep_offsets_t(const size_t& num_elements) : NumElements(num_elements) {}

            void SetData(const fs::path& file);

            void Step() {
                curr += NumElements;
            }

            const void* CurrentAddress() const noexcept {
                return &(*curr);
            }


            std::vector<glm::vec3> offsets;
            std::vector<glm::vec3>::const_iterator curr;
            size_t NumElements = 0;
            size_t stepCount = 0;
        } offsetData;

        struct instance_data_t {
            std::vector<glm::vec4> Offsets;
            instance_data_t(size_t num_elems) {
                Offsets.resize(num_elems);
            }
            void setData(const timestep_offsets_t& vals);
        } uboData;

        size_t currStep = 0;
        size_t stepsPerFrame = 1;
        bool paused = false;
    };

}

#endif //!VPSK_TETHER_OBJECT_HPP