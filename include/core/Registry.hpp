#pragma once
#ifndef VPSK_REGISTRY_HPP
#define VPSK_REGISTRY_HPP
#include "EntityTraits.hpp"
#include "Identifiers.hpp"
#include "ComponentViews.hpp"
#include "Family.hpp"
#include "Identifiers.hpp"
#include "storage/SparseSet.hpp"
#include "signal/Signal.hpp"

namespace vpsk {

    struct tag_t final {};
    struct persistent_t final {};
    struct raw_t final {};
    struct break_t final {};

    template<typename EntityType>
    class Registry {

        using tag_family = Family<struct InternalRegistryTagFamily>;
        using component_family = Family<struct InternalRegistryComponentFamily>;
        using handler_family = Family<struct InternalRegistryHandlerFamily>;
        using signal_type = SignalHandler<void(Registry&, EntityType)>;
        using traits_type = EntityTraits<EntityType>;

        template<typename...Components>
        static void creating(Registry& registry, EntityType ent) {
            if (registry.has<Components...>(ent)) {
                registry.handlers[handler_family::Type<Components...>()]->construct(ent);
            }
        }

        template<typename...Components>
        static void destroying(Registry& registry, EntityType ent) {
            auto& handler = *registry.handlers[handler_family::Type<Components...>()];
            return handler.has(ent) ? handler.destroy(ent) : void();
        }

        struct Attachee {
            Attachee(EntityType ent) : entity(ent) {};
            virtual ~Attachee() noexcept = default;
            EntityType entity;
        };

        template<typename TagType>
        struct Attaching : Attachee {
            template<typename...Args>
            Attaching(EntityType ent, TagType _tag) : Attachee{ ent }, tag(std::move(_tag)) {}
            TagType tag;
        };

        template<typename Component>
        bool managed() const noexcept {
            const component_family::family_type component_type = component_family::Type<Component>();
            return component_type < pools.size() && std::get<0>(pools[component_type]);
        }

        template<typename Component>
        const SparseSet<EntityType, Component>& pool() const noexcept {
            const component_family::family_type component_type = component_family::Type<Component>();
            return static_cast<const SparseSet<EntityType, Component>&>(*std::get<0>(pools[component_type]));
        }

        template<typename Component>
        SparseSet<EntityType, Component>& pool() noexcept {
            const component_family::family_type component_type = component_family::Type<Component>();
            return static_cast<SparseSet<EntityType, Component>&>(*std::get<0>(pools[component_type]));
        }

        // Ensures that a pool for the Component type given exists.
        template<typename Component>
        void assure() {
            const component_family::family_type component_type = component_family::Type<Component>();

            if (!(component_type < pools.size())) {
                pools.resize(component_type + 1);
            }
            
            auto& component_pool = std::get<0>(pools[component_type]);

            if (!component_pool) {
                component_pool = std::make_unique<SparseSet<EntityType, Component>>();
            }
        }

        // Ensures that a tag group/pool for the given tag type exists
        template<typename Tag>
        void assure(tag_t) {
            const tag_family::family_type tag_type = tag_family::Type<Tag>();
            if (!(tag_type < tags.size())) {
                tags.resize(tag_type + 1);
            }
        }

        std::vector<std::unique_ptr<SparseSet<EntityType>>> handlers;
        std::vector<std::tuple<std::unique_ptr<SparseSet<EntityType>>, signal_type, signal_type>> pools;
        std::vector<std::tuple<std::unique_ptr<Attachee>, signal_type, signal_type>> tags;
        std::vector<typename traits_type::entity_type> entities;
        std::size_t available{ 0 };
        traits_type::entity_type next{};

    public:

        using entity_type = typename traits_type::entity_type;
        using version_type = typename traits_type::version_type;
        using size_type = std::size_t;
        using tag_type = typename tag_family::family_type;
        using component_type = typename component_family::family_type;
        using sink_type = typename signal_type::sink_type;

        Registry() noexcept = default;
        Registry(const Register&) = delete;
        Registry(Registry&& other) noexcept;
        Registry& operator=(const Registry&) = delete;
        Registry& operator=(Registry&& other) noexcept;

        template<typename TagType>
        static tag_type Type(tag_t) noexcept {
            return tag_family::Type<TagType>();
        }

        template<typename ComponentType>
        static component_type Type() noexcept {
            return component_family::Type<ComponentType>();
        }

        template<typename ComponentType>
        size_type NumComponentsOfType() const noexcept {
            return managed<ComponentType>() ? pool<ComponentType>().size() ? size_type{ 0 };
        }

        size_type NumEntitiesInUse() const noexcept {
            return entities.size() - available;
        }

        template<typename ComponentType>
        void ReserveForComponentType(size_type capacity) {
            assure<ComponentType>();
            pool<ComponentType>().reserve(capacity);
        }

        void ReserveForEntities(size_type capacity) {
            entities.reserve(capacity);
        }

        size_type EntitiesCreated() const noexcept {
            return entities.size();
        }

        template<typename ComponentType>
        bool ComponentPoolEmpty() const noexcept {
            return !managed<ComponentType>() || pool<ComponentType>().empty();
        }

        bool EntitiesEmpty() const noexcept {
            return entities.empty();
        }

        template<typename ComponentType>
        const ComponentType* ComponentsPtr() const noexcept {
            return managed<ComponentType>() ? pool<ComponentType>().objects_ptr() : nullptr;
        }

        template<typename ComponentType>
        ComponentType* ComponentsPtr() noexcept {
            return managed<ComponentType>() ? pool<ComponentType>().objects_ptr() : nullptr;
        }

        template<typename ComponentType>
        const entity_type* EntitiesPtr() const noexcept {
            return managed<ComponentType>() ? pool<ComponentType>().data() : nullptr;
        }

        bool Valid(entity_type ent) const noexcept {
            const size_type position = static_cast<size_type>(ent & traits_type::entity_mask);
            return (entities[position] == ent);
        }

        version_type Version(entity_type ent) const noexcept {
            return static_cast<version_type>((ent >> traits_type::entity_shift) & traits_type::version_mask);
        }

        version_type CurrentVersion(entity_type ent) const noexcept {
            const size_type position = static_cast<size_type>(ent & traits_type::entity_mask);
            return static_cast<version_type>((entities[position] >> traits_type::entity_shift) & traits_type::version_mask);
        }

        entity_type& Create() noexcept {
            entity_type result;

            if (available != 0) {
                const auto entity = next;
                const auto version = entitites[entity] & (~traits_type::entity_mask);

                result = entity | version;
                next = entities[entity] & traits_type::entity_mask;
                entities[entity] = result;
                --available;
                return entities[entity];
            }
            else {
                result = static_cast<entity_type>(entities.size());
                entities.emplace_back(result);
                return entities.back();
            }
        }

        void Destroy(entity_type ent) {
            std::for_each(pools.begin(), pools.end(), [ent, this](auto&& pool_tuple) {
                auto& component_pool = std::get<0>(pool_tuple);
                if ((component_pool != nullptr) && component_pool->has(ent)) {
                    std::get<2>(pool_tuple).TriggerSignal(*this, ent);
                    component_pool->destroy(ent);
                }
            });
        }
    };

}

#endif //!VPSK_REGISTRY_HPP