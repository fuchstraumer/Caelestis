#include "core/Registry.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <string_view>

struct PositionComponent {
    float x{ 0.0f };
    float y{ 0.0f };
    float z{ 0.0f };
};

struct NameComponent {

    NameComponent(const std::string& name) : Name(createView(name)) {}
    std::string_view Name;

private:

    static std::string_view createView(const std::string& name) {
        auto& storage_set = getStorage();
        auto iter = storage_set.find(name);
        if (iter == storage_set.cend()) {
            auto emplaced = storage_set.emplace(name);
            return std::string_view(*emplaced.first);
        }
        else {
            return std::string_view(*iter);
        }
    }

    static std::unordered_set<std::string>& getStorage() {
        static std::unordered_set<std::string> storage;
        return storage;
    }

};

struct HealthComponent {
    float Hitpoints{ 0.0f };
};

int main(int argc, char* argv[]) {
    using namespace vpsk;
    DefaultRegistryType Reg;

    DefaultRegistryType::entity_type ent = Reg.Create();
    Reg.AssignComponent<NameComponent>(ent, "BaseEntity");

}