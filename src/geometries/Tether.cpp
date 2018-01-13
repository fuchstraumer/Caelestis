#define _HAS_CXX17 1
#include "geometries/Tether.hpp"
#include "core/LogicalDevice.hpp"
#include "resource/Buffer.hpp"
#include "scene/BaseScene.hpp"
#include <fstream>
#include <string>
#include <string_view>
#include <experimental/filesystem>

#include "imgui/imgui.h"
namespace vpsk {

    
    // front 0, right 1, top 2, left 3, bottom 4, back 5

    constexpr static std::array<const glm::vec3, 6> face_normals{
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f,-1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f,-1.0f)
    };

    constexpr static std::array<const glm::vec3, 8> base_vertices{
        glm::vec3(-0.50f,-0.50f, 0.50f),
        glm::vec3(0.50f,-0.50f, 0.50f),
        glm::vec3(0.50f, 0.50f, 0.50f),
        glm::vec3(-0.50f, 0.50f, 0.50f),
        glm::vec3(0.50f,-0.50f,-0.50f),
        glm::vec3(-0.50f,-0.50f,-0.50f),
        glm::vec3(-0.50f, 0.50f,-0.50f),
        glm::vec3(0.50f, 0.50f,-0.50f),
    };
    
    const static std::array<const std::initializer_list<uint16_t>, 6> indices_ilist {
        std::initializer_list<uint16_t>{ 0, 1, 2, 2, 3, 0 },
        std::initializer_list<uint16_t>{ 3, 2, 6, 6, 7, 3 },
        std::initializer_list<uint16_t>{ 1, 5, 6, 6, 2, 1 },
        std::initializer_list<uint16_t>{ 4, 5, 1, 1, 0, 4 },
        std::initializer_list<uint16_t>{ 4, 0, 3, 3, 7, 4 },
        std::initializer_list<uint16_t>{ 7, 6, 5, 5, 4, 7 },
    };

    void getFaceVertices(const size_t i, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) {
        switch (i) {
        case 0:
            v0 = base_vertices[0];
            v1 = base_vertices[1];
            v2 = base_vertices[2];
            v3 = base_vertices[3];
            break;
        case 1:
            v0 = base_vertices[1];
            v1 = base_vertices[4];
            v2 = base_vertices[7];
            v3 = base_vertices[2];
            break;
        case 2:
            v0 = base_vertices[3];
            v1 = base_vertices[2];
            v2 = base_vertices[7];
            v3 = base_vertices[6];
            break;
        case 3:
            v0 = base_vertices[5];
            v1 = base_vertices[0];
            v2 = base_vertices[3];
            v3 = base_vertices[6];
            break;
        case 4:
            v0 = base_vertices[5];
            v1 = base_vertices[4];
            v2 = base_vertices[1];
            v3 = base_vertices[0];
            break;
        case 5:
            v0 = base_vertices[4];
            v1 = base_vertices[5];
            v2 = base_vertices[6];
            v3 = base_vertices[7];
            break;
        }
    }

    uint16_t Tether::addVertex(glm::vec3 p, glm::vec3 n) {
        static uint16_t count = 0;
        vertices.insert(vertices.end(), { std::move(p), std::move(n) });
        return count++;
    }

    void Tether::createMeshData() {
        for (size_t i = 0; i < 6; ++i) {
            glm::vec3 v0, v1, v2, v3;
            glm::vec3 n = face_normals[i];
            getFaceVertices(i, v0, v1, v2, v3);
            uint16_t i0 = addVertex(v0, n);
            uint16_t i1 = addVertex(v1, n);
            uint16_t i2 = addVertex(v2, n);
            uint16_t i3 = addVertex(v3, n);
            indices.insert(indices.end(), { i0, i1, i2 });
            indices.insert(indices.end(), { i2, i3, i0 });
        }
    }

    void Tether::createMeshBuffers() {
        vbo = std::make_unique<vpr::Buffer>(device);
        vbo->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::vec3) * vertices.size());
        ebo = std::make_unique<vpr::Buffer>(device);
        ebo->CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint16_t) * indices.size());
    }

    void Tether::createUBOs() {
        offsetUBO = std::make_unique<vpr::Buffer>(device);
        offsetUBO->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(float) * offsetData.NumElements);
    }

    void Tether::drawGui() {
        ImGui::Begin("Static Visualization");
        ImGui::ProgressBar(static_cast<float>(currStep) / static_cast<float>(offsetData.stepCount), ImVec2(ImGui::GetContentRegionAvailWidth(), 30), "Simulation Progess");
        bool pressed = ImGui::Button("Pause");
        if (paused && pressed) {
            paused = false;
        }
        else if (pressed) {
            paused = true;
        }
        if (paused) {
            ImGui::SameLine();
            ImGui::Text("Visualization paused...");
        }
        ImGui::Separator();
        ImGui::Checkbox("Lock mouse to screen", &BaseScene::SceneConfiguration.EnableMouseLocking);
        ImGui::SameLine(); ImGui::InputInt("Data steps per frame", &stepsPerFrame); ImGui::Separator();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(450.0f);
            ImGui::TextUnformatted("This lets you adjust the speed of the simulation, by changing the number of steps (by line) through the input data file per frame rendered. Large values will appear to stutter, potentially.");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
        if (stepsPerFrame < 1) {
            stepsPerFrame = 1;
        }
        else if (stepsPerFrame > 4) {
            stepsPerFrame = 4;
        }
    }

    void Tether::drawGuiPlots() {
        if (currStep < 2000) {
            ImGui::ProgressBar(static_cast<float>(currStep) / 2000.0f, ImVec2(ImGui::GetContentRegionAvailWidth(), 30), "Preloading plot data...");
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(450.0f);
                ImGui::TextUnformatted("In order to avoid copies, the plots read from a pointer array of the Tether's data. This array spans from currStep - 2000 to the current step: thus, it's impossible to view until we reach timestep 2000.");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }
        else {
            static float plot_height = 120.0f;
            static bool plotT = true, plotIPL = true;
            static bool plotOOPL = true, plotDist = true;
            ImGui::InputFloat("Plot Window Height", &plot_height, 1.0f, 3.0f);
            if (plot_height > 150.0f) {
                plot_height = 150.0f;
            }
            if (plot_height <= 10.0f) {
                plot_height = 10.0f;
            }
            float avail_width = ImGui::GetContentRegionAvailWidth();
            ImGui::Separator();
            ImGui::Checkbox("Plot Tension", &plotT); ImGui::SameLine();
            ImGui::Checkbox("Plot IPL", &plotIPL); ImGui::SameLine();
            ImGui::Checkbox("Plot OOPL", &plotOOPL); ImGui::SameLine();
            ImGui::Checkbox("Plot R. Dist.", &plotDist);
            if (plotT) {
                ImGui::Separator();
                ImGui::Text("Tension in Tether");
                ImGui::PlotLines("Tension", tetherBuffer.T[0], static_cast<int>(2000), 0, "", tetherData.MinTension, tetherData.MaxTension, ImVec2(ImGui::GetContentRegionAvailWidth(), plot_height));
            }
            if (plotIPL) {
                ImGui::Separator();
                ImGui::Text("In Plane Libration");
                ImGui::PlotLines("In Plane Libration", tetherBuffer.IPL[0], static_cast<int>(2000), 0, "", tetherData.MinIPL, tetherData.MaxIPL, ImVec2(ImGui::GetContentRegionAvailWidth(), plot_height));
            }
            if (plotOOPL) {
                ImGui::Separator();
                ImGui::Text("Out of Plane Libration");
                ImGui::PlotLines("Out of Plane Libration", tetherBuffer.OOPL[0], static_cast<int>(2000), 0, "", tetherData.MinOOPL, tetherData.MaxOOPL, ImVec2(ImGui::GetContentRegionAvailWidth(), plot_height));
            }
            if (plotDist) {
                ImGui::Separator();
                ImGui::Text("Radial Distance");
                ImGui::PlotLines("Radial Distance", tetherBuffer.Dist[0], static_cast<int>(2000), 0, "", tetherData.MinDist, tetherData.MaxDist, ImVec2(ImGui::GetContentRegionAvailWidth(), plot_height));
            }
        }
    }

    void Tether::timestep_offsets_t::SetData(const fs::path& path) {
        std::string data;

        std::ifstream _file(path, std::ios::in);
        data = std::string{ std::istreambuf_iterator<char>(_file), std::istreambuf_iterator<char>() };
        std::string line;
        _file.seekg(0);
        while (std::getline(_file, line)) {
            ++stepCount;
        }
        
        std::string_view data_view(data);

        offsets.reserve(stepCount * 12);

        while(!data_view.empty()) {
            glm::vec3 p;
            size_t idx = data_view.find_first_of(' ');
            p.x = strtof(data_view.substr(0,idx).data(),nullptr);
            if (abs(p.x) <= 1.0e-5f) {
                p.x = 0.0f;
            }
            data_view.remove_prefix(idx+1);
            idx = data_view.find_first_of(' ');
            p.y = strtof(data_view.substr(0,idx).data(),nullptr);
            if (abs(p.y) <= 1.0e-5f) {
                p.y = 0.0f;
            }
            data_view.remove_prefix(idx+1);
            idx = data_view.find_first_of(' ');
            size_t n_idx = data_view.find_first_of('\n');
            if (n_idx < idx) {
                p.z = strtof(data_view.substr(0, n_idx).data(), nullptr);
                data_view.remove_prefix(n_idx + 1);
            }
            else {
                p.z = strtof(data_view.substr(0, idx).data(), nullptr);
                data_view.remove_prefix(idx + 1);
            }
            if (abs(p.z) <= 1.0e-5f) {
                p.z = 0.0f;
            }
            offsets.push_back(p);   
        }

        offsets.shrink_to_fit();
    }

    void Tether::loadTetherData(const char* file_path) {
        std::ifstream _file(file_path, std::ios::in);
        std::string data{ std::istreambuf_iterator<char>(_file), std::istreambuf_iterator<char>() };
        std::string_view data_view(data);

        while (!data_view.empty()) {
            // Data is: Radial Distance, In Plane Libration, Out of Plane Libration, Tension
            glm::dvec4 p;
            size_t idx = data_view.find_first_of(' ');
            float dist = strtof(data_view.substr(0, idx).data(), nullptr);
            data_view.remove_prefix(idx + 1);
            idx = data_view.find_first_of(' ');
            float ipl = strtof(data_view.substr(0, idx).data(), nullptr);
            data_view.remove_prefix(idx + 1);
            idx = data_view.find_first_of(' ');
            float oopl = strtof(data_view.substr(0, idx).data(), nullptr);
            data_view.remove_prefix(idx + 1);
            idx = data_view.find_first_of(' ');
            size_t n_idx = data_view.find_first_of('\n');
            float tension = 0.0f;
            if (n_idx < idx) {
                tension = strtof(data_view.substr(0, n_idx).data(), nullptr);
                data_view.remove_prefix(n_idx + 1);
            }
            else {
                tension = strtof(data_view.substr(0, idx).data(), nullptr);
                data_view.remove_prefix(idx + 1);
            }

            tetherData.RadialDistance.push_back(dist);
            tetherData.InPlaneLibration.push_back(ipl);
            tetherData.OutOfPlaneLibration.push_back(oopl);
            tetherData.Tension.push_back(tension);
        }

        auto relems = std::minmax_element(tetherData.RadialDistance.cbegin(), tetherData.RadialDistance.cend());
        tetherData.MinDist = *relems.first;
        tetherData.MaxDist = *relems.second;
        auto iip_min_max = std::minmax_element(tetherData.InPlaneLibration.cbegin(), tetherData.InPlaneLibration.cend());
        tetherData.MinIPL = *iip_min_max.first;
        tetherData.MaxIPL = *iip_min_max.second;
        auto oop_min_max = std::minmax_element(tetherData.OutOfPlaneLibration.cbegin(), tetherData.OutOfPlaneLibration.cend());
        tetherData.MinOOPL = *oop_min_max.first;
        tetherData.MaxOOPL = *oop_min_max.second;
        auto t_min_max = std::minmax_element(tetherData.Tension.cbegin(), tetherData.Tension.cend());
        tetherData.MinTension = *t_min_max.first;
        tetherData.MaxTension = *t_min_max.second;
    }

    void Tether::instance_data_t::setData(const timestep_offsets_t & vals) {

        auto vec_iter = Offsets.begin();
        for (auto iter = vals.curr; iter != vals.curr + vals.NumElements; ++iter) {
            vec_iter->x = iter->x;
            vec_iter->y = iter->y;
            vec_iter->z = iter->z;
            vec_iter->w = 0.0f;
            ++vec_iter;
        }

    }

}