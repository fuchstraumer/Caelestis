#include "components/NameComponent.hpp"
#include <unordered_set>
namespace vpsk {

    static std::unordered_set<std::string>& GetStorage() noexcept {
        std::unordered_set<std::string> storage{};
        return storage;
    }

    NameComponent::NameComponent(std::string && name) : Name(createView(name)) {}

    NameComponent::NameComponent(const std::string & name) : Name(createView(name)) {}

    NameComponent::NameComponent(const NameComponent & other) noexcept : Name(other.Name) {}

    NameComponent::NameComponent(NameComponent && other) noexcept : Name(std::move(other.Name)) {}

    NameComponent & NameComponent::operator=(const NameComponent & other) noexcept {
        Name = other.Name;
        return *this;
    }

    NameComponent & NameComponent::operator=(NameComponent && other) noexcept {
        Name = std::move(other.Name);
        return *this;
    }

    std::string_view NameComponent::createView(std::string && name) {
        auto& storage = GetStorage();
        auto iter = storage.find(name);
        if (iter == storage.cend()) {
            auto emplaced = storage.emplace(std::move(name));
            return std::string_view(*emplaced.first);
        }
        else {
            return std::string_view(*iter);
        }
    }

    std::string_view NameComponent::createView(const std::string & name) {
        auto& storage = GetStorage();
        auto iter = storage.find(name);
        if (iter == storage.cend()) {
            auto emplaced = storage.emplace(name);
            return std::string_view(*emplaced.first);
        }
        else {
            return std::string_view(*iter);
        }
    }

}