#include "resources/DescriptorResources.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include <future>

namespace vpsk {

    DescriptorResources::DescriptorResources(const vpr::Device* dvc) : device(dvc) {}
    
    DescriptorResources::~DescriptorResources() {}

    size_t DescriptorResources::AddResources(const std::map<std::string, VkDescriptorSetLayoutBinding>& resources) {

        std::vector<std::future<size_t>> set_indices;
        for (const auto& name : resources) {
            set_indices.emplace_back(std::async(std::launch::async, &DescriptorResources::FindIdxOfSetWithResource, this, name.first));
        }

        std::vector<size_t> search_results;
        for (auto&& fut : set_indices) {
            search_results.emplace_back(fut.get());
        }

        auto valid_idx = [](const size_t& idx) { return idx != std::numeric_limits<size_t>::max(); };
        auto iter  = std::find_if(search_results.cbegin(), search_results.cend(), valid_idx);

        std::lock_guard<std::mutex> insertion_lock(insertionGuard);
        if (iter == search_results.cend()) {
            setResources.emplace_back(std::map<std::string, VkDescriptorSetLayoutBinding>{ resources.cbegin(), resources.cend() });
            return setResources.size() - 1;
        }
        else {
            size_t set_to_add_names_to = *iter;
            setResources[set_to_add_names_to].insert(resources.cbegin(), resources.cend());
            return set_to_add_names_to;
        }
    }

    size_t DescriptorResources::FindIdxOfSetWithResource(const std::string& name) const {

        auto has_resource = [name](const std::map<std::string, VkDescriptorSetLayoutBinding>& set_to_search)->bool {
            return (set_to_search.count(name) != 0);
        };

        std::vector<std::future<bool>> search_futures;
        for (const auto& set : setResources) {
            search_futures.emplace_back(std::async(std::launch::async, has_resource, set));
        }

        std::vector<bool> search_results;
        for (auto&& fut : search_futures) {
            search_results.emplace_back(fut.get());
        }

        auto iter = std::find(search_results.cbegin(), search_results.cend(), true);
        if (iter == search_results.cend()) {
            return std::numeric_limits<size_t>::max();
        }
        else {
            return static_cast<size_t>(std::distance(search_results.cbegin(), iter));
        }

    }

    vpr::DescriptorPool * DescriptorResources::GetDescriptorPool()
    {
        return nullptr;
    }

    vpr::DescriptorSetLayout * DescriptorResources::GetSetLayout(const size_t & idx)
    {
        return nullptr;
    }

    const std::map<std::string, VkDescriptorSetLayoutBinding>& DescriptorResources::GetSetResources(const size_t & idx) const {
        return setResources.at(idx);
    }

    void DescriptorResources::AllocatePool() {
        descriptorPool = std::make_unique<vpr::DescriptorPool>(device, setResources.size());
        std::map<VkDescriptorType, uint32_t> num_per_type;

        for (const auto& entry : setResources) {
            for (auto& rsrc : entry) {
                num_per_type[rsrc.second.descriptorType] += rsrc.second.descriptorCount;
            }
        }

        for (auto& type : num_per_type) {
            descriptorPool->AddResourceType(type.first, type.second);
        }

        descriptorPool->Create();
    }

    void DescriptorResources::CreateSetLayouts() {
        for (const auto& entry : setResources) {
            std::unique_ptr<vpr::DescriptorSetLayout> set_layout(std::make_unique<vpr::DescriptorSetLayout>(device));
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            for (const auto& rsrc : entry) {
                bindings.emplace_back(rsrc.second);
            }
            set_layout->AddDescriptorBindings(static_cast<uint32_t>(bindings.size()), bindings.data());
            setLayouts.emplace_back(std::move(set_layout));
        }
    }
}