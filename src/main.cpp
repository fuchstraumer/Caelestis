#include "core/Registry.hpp"
#include "core/signal/Emitter.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <random>
#include <iostream>

struct PositionComponent {
    float x{ 0.0f };
    float y{ 0.0f };
    float z{ 0.0f };
};

struct NameComponent {

    explicit NameComponent(const char* name) : Name(createView(name)) {}
    explicit NameComponent(const std::string& name) : Name(createView(name)) {}
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

struct CentralBody {

};

int main(int argc, char* argv[]) {
    using namespace vpsk;
    DefaultRegistryType Reg;
    


    std::vector<std::uint32_t> entities;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> real_distr(-100.0f, 100.0f);

    Reg.ReserveForComponentType<NameComponent>(30);
    Reg.ReserveForComponentType<PositionComponent>(30);

    for (size_t i = 0; i < 30; ++i) {
        entities.emplace_back(Reg.Create());
        std::string entity_name = std::string("Entity") + std::to_string(i);
        Reg.AssignComponent<NameComponent>(entities.back(), entity_name);
        Reg.AssignComponent<PositionComponent>(entities.back(), real_distr(gen), real_distr(gen), real_distr(gen));
        if ((i % 2 == 0) && i != 0) {
            Reg.AssignComponent<HealthComponent>(entities.back(), std::fabsf(real_distr(gen)));
        }
        if (i == 0) {
            Reg.AssignTag<CentralBody>(entities[0]);
        }
    }

    auto[ name, pos ] = Reg.GetComponents<NameComponent, PositionComponent>(entities[0]);
    pos.x += 10.0f;
    auto& pos1 = Reg.GetComponent<PositionComponent>(entities[0]);
    Reg.RemoveComponent<NameComponent>(entities[3]);
    Reg.RemoveComponent<PositionComponent>(entities[3]);
    Reg.Destroy(entities[4]);
    
    auto& positions_view = Reg.PersistentView<NameComponent, PositionComponent>();

    for (auto&& pair : positions_view) {
        
    }

    Reg.DiscardPersistentViews<NameComponent, PositionComponent>();

}