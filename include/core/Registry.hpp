#pragma once
#ifndef VPSK_REGISTRY_HPP
#define VPSK_REGISTRY_HPP
#include "CommonDef.hpp"
#include "Exceptions.hpp"
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
        const SparseSet<EntityType, Component>& getComponentStorage() const noexcept {
            const component_family::family_type component_type = component_family::Type<Component>();
            return static_cast<const SparseSet<EntityType, Component>&>(*std::get<0>(pools[component_type]));
        }

        template<typename Component>
        SparseSet<EntityType, Component>& getComponentStorage() noexcept {
            const component_family::family_type component_type = component_family::Type<Component>();
            return static_cast<SparseSet<EntityType, Component>&>(*std::get<0>(pools[component_type]));
        }

        // Ensures that a getComponentStorage for the Component type given exists.
        template<typename Component>
        void assureComponentStorage() {
            const component_family::family_type component_type = component_family::Type<Component>();

            if (!(component_type < pools.size())) {
                pools.resize(component_type + 1);
            }
            
            auto& component_pool = std::get<0>(pools[component_type]);

            if (!component_pool) {
                component_pool = std::make_unique<SparseSet<EntityType, Component>>();
            }
        }

        // Ensures that a tag group/getComponentStorage for the given tag type exists
        template<typename Tag>
        void assureTagStorage() {
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
        typename traits_type::entity_type next{};

    public:

        using entity_type = typename traits_type::entity_type;
        using version_type = typename traits_type::version_type;
        using size_type = std::size_t;
        using tag_type = typename tag_family::family_type;
        using component_type = typename component_family::family_type;
        using sink_type = typename signal_type::sink_type;

        Registry() noexcept = default;
        Registry(const Registry&) = delete;
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
            return managed<ComponentType>() ? getComponentStorage<ComponentType>().size() ? size_type{ 0 };
        }

        size_type NumEntitiesInUse() const noexcept {
            return entities.size() - available;
        }

        template<typename ComponentType>
        void ReserveForComponentType(size_type capacity) {
            assureComponentStorage<ComponentType>();
            getComponentStorage<ComponentType>().reserve(capacity);
        }

        void ReserveForEntities(size_type capacity) {
            entities.reserve(capacity);
        }

        size_type EntitiesCreated() const noexcept {
            return entities.size();
        }

        template<typename ComponentType>
        bool ComponentPoolEmpty() const noexcept {
            return !managed<ComponentType>() || getComponentStorage<ComponentType>().empty();
        }

        bool EntitiesEmpty() const noexcept {
            return entities.empty();
        }

        template<typename ComponentType>
        const ComponentType* ComponentsPtr() const noexcept {
            return managed<ComponentType>() ? getComponentStorage<ComponentType>().objects_ptr() : nullptr;
        }

        template<typename ComponentType>
        ComponentType* ComponentsPtr() noexcept {
            return managed<ComponentType>() ? getComponentStorage<ComponentType>().objects_ptr() : nullptr;
        }

        template<typename ComponentType>
        const entity_type* EntitiesPtr() const noexcept {
            return managed<ComponentType>() ? getComponentStorage<ComponentType>().data() : nullptr;
        }

        bool Valid(entity_type ent) const noexcept {
            const size_type position = static_cast<size_type>(ent & traits_type::entity_mask);
            return (entities[position] == ent);
        }

        void ValidException(entity_type ent) const {
            if !(Valid(ent)) {
                throw bad_entity("Invalid entity handle!");
           }
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
                const auto version = entities[entity] & (~traits_type::entity_mask);

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

            std::for_each(tags.begin(), tags.end(), [ent, this](auto&& tag_tuple) {
                auto& tag = std::get<0>(tag_tuple);

                if (tag && tag->entity == ent) {
                    std::get<2>(tag_tuple).TriggerSignal(*this, ent);
                    tag.reset();
                }
            });

            const auto entity = ent & traits_type::entity_mask;
            const auto version = (((ent >> traits_type::entity_shift) + 1) & traits_type::version_mask) << traits_type::entity_shift;
            const auto node = (available ? next : ((entity + 1) & traits_type::entity_mask)) | version;

            entities[entity] = node;
            next = entity;
            ++available;
        }

        template<typename TagType, typename...Args>
        TagType& AssignTag(tag_t, entity_type ent, Args&&...args) {
            assureTagStorage<TagType>();
            auto& tag_tuple = tags[tag_family::Type<TagType>()];
            std::get<0>(tag_tuple).reset(std::make_unique<Attaching<TagType>>(ent, TagType{ std::forward<Args&&>(args)... }));
            std::get<1>(tag_tuple).TriggerSignal(*this, ent);
            return get<TagType>();
        }

        template<typename ComponentType, typename...Args>
        ComponentType& AssignComponent(entity_type ent, Args&&...args) {
            assureComponentStorage<ComponentType>();
            getComponentStorage<ComponentType>().construct(ent, std::forward<Args&&>(args)...);
            std::get<1>(pools[component_family::Type<ComponentType>()]).TriggerSignal(*this, ent);
            return getComponentStorage<ComponentType>().get(ent);
        }

        template<typename TagType>
        void RemoveTag() {
            if (HasTag<TagType>()) {
                auto& tuple = tags[tag_family::Type<TagType>()];
                auto& tag = std::get<0>(tuple);
                std::get<2>(tuple).TriggerSignal(*this, tag->entity);
                tag.reset();
            }
        }

        template<typename ComponentType>
        void RemoveComponent(entity_type ent) {
            const component_family::family_type component_type = component_family::Type<ComponentType>();
            std::get<2>(pools[component_type]).TriggerSignal(*this, ent);
            getComponentStorage<ComponentType>().destroy(ent);
        }

        template<typename TagType>
        bool HasTag() const noexcept {
            const tag_family::family_type tag_type = tag_family::Type<TagType>();
            if (tag_type < tags.size()) {
                auto& found_tag = std::get<0>(tags[tag_type]);
                return found_tag && (tag->entity == (entities[tag->entity & traits_type::entity_mask]));
            }
            else {
                return false;
            }
        }

        template<typename TagType>
        bool HasTag(tag_t, entity_type ent) const noexcept {
            return HasTag<TagType>() && Attachee<TagType>() == ent;
        }

        template<typename TagType>
        void HasTagException() const {
            if (!HasTag<TagType>()) {
                throw invalid_tag("Specified tag type does not exist!");
            }
        }

        template<typename...ComponentTypes>
        bool HasComponents(entity_type ent) const noexcept {
            using accum_type = bool[];
            bool all = true;
            accum_type accumulator = { all, (all = all && managed<ComponentTypes>() && getComponentStorage<ComponentTypes>().has(ent))... };
            (void)accum; // evaluates the sequence
            return all;
        }

        template<typename TagType>
        const TagType& GetTag() const noexcept {
            return static_cast<Attaching<TagType>*>(std::get<0>(tags[tag_family::Type<TagType>()]).get())->tag;
        }

        template<typename TagType>
        TagType& GetTag() const noexcept {
            return static_cast<Attaching<TagType>*>(std::get<0>(tags[tag_family::Type<TagType>()]).get())->tag;
        }

        template<typename ComponentType>
        const ComponentType& GetComponent(entity_type ent) const noexcept {
            return getComponentStorage<ComponentType>().get(ent);
        }

        template<typename ComponentType>
        ComponentType& GetComponent() noexcept {
            return getComponentStorage<ComponentType>().get(ent);
        }

        template<typename...ComponentTypes>
        std::enable_if_t<(sizeof...(ComponentTypes) > 1), std::tuple<const ComponentTypes&...>> GetComponents(entity_type ent) const noexcept {
            return std::tuple<const ComponentTypes&...>{ Get<ComponentTypes>(ent)... };
        }

        template<typename...ComponentTypes>
        std::enable_if_t<(sizeof...(ComponentTypes) > 1), std::tuple<ComponentTypes&...>> GetComponents(entity_type ent) noexcept {
            return std::tuple<ComponentTypes&...>{ Get<ComponentTypes>(ent)... };
        }

        template<typename TagType, typename...Args>
        TagType& Replace(tag_t, Args&&...args) {
            return (get<TagType>() = TagType{ std::forward<Args&&>(args)... });
        }

        template<typename ComponentType, typename...Args>
        ComponentType& Replace(entity_type ent, Args&&...args) {
            return (get<ComponentType>() = ComponentType{ std::forward<Args&&>(args)... });
        }

        // Returns original owner of the tag, but replaces the tag's current owner with "ent"
        template<typename TagType>
        entity_type ChangeTagOwner(entity_type ent) {
            if (ENTITY_SYSTEM_CHECKS) {
                ValidException(ent);
                HasTagException<TagType>();
            }
            auto& tag = std::get<0>(tags[tag_family::Type<TagType>()]);
            const entity_type owner = tag->entity;
            tag->entity = ent;
            return owner;
        }

        template<typename TagType>
        entity_type TagOwner() const noexcept {
            if (ENTITY_SYSTEM_CHECKS) {
                HasTagException<TagType>():
            }
            return std::get<0>(tags[tag_family::Type<TagType>()]);
        }

        template<typename ComponentType, typename...Args>
        ComponentType& Accomodate(entity_type ent, Args&&...args) {
            assureComponentStorage<ComponentType>();
            auto& component_pool = getComponentStorage<ComponentType>();
            if (component_pool.has(ent)) {
                return (component_pool.get(ent) = ComponentType{ std::forward<Args&&>(args)... });
            }
            else {
                return component_pool.construct(ent, std::forward<Args&&>(args)...);
            }
        }

        template<typename TagType>
        sink_type GetTagCreationAndAssignmentSink() noexcept {
            assureTagStorage<TagType>();
            return std::get<1>(tags[tag_family::Type<TagType>()]).sink();
        }

        // Listeners are invoked BEFORE removal or destruction, but are invoked when this process begins
        template<typename TagType>
        sink_type GetTagDestructionAndRemovalSink() noexcept {
            assureTagStorage<TagType>();
            return std::get<2>(tags[tag_family::Type<TagType>()]).sink();
        }

        template<typename ComponentType>
        sink_type GetComponentCreationAndAssignmentSink() noexcept {
            assureComponentStorage<ComponentType>();
            return std::get<1>(pools[component_family::Type<ComponentType>()]).sink();
        }

        template<typename ComponentType>
        sink_type GetComponentDestructionAndRemovalSink() noexcept {
            assureComponentStorage<ComponentType>();
            return std::get<2>(pools[component_family::Type<ComponentType>()]).sink();
        }

        // imposes an order on the componentstorage/pool for the given component type.
        // (e.g, use it to sort positions by distance from a point)
        template<typename ComponentType, typename CompareFunc, typename SortType = StdSort>
        void Sort(CompareFunc comparator, SortType sorter = Sort{}) {
            assureComponentStorage<ComponentType>();
            getComponentStorage<ComponentType>().sort(std::move(comparator), std::move(sorter));
        }

        template<typename UnsortedComponentType, typename ComponentTypeToSortFrom>
        void Sort() {
            assureComponentStorage<UnsortedComponentType>();
            assureComponentStorage<ComponentTypeToSortFrom>();
            getComponentStorage<UnsortedComponentType>().sort_based_on_other(getComponentStorage<ComponentTypeToSortFrom>());
        }

        template<typename ComponentType>
        void RemoveComponentType(entity_type ent) {
            if (ENTITY_SYSTEM_CHECKS) {
                ValidException(ent);
            }

            assureComponentStorage<ComponentType>();
            const component_family::family_type component_type = component_family::Type<ComponentType>();
            auto& component_pool = *std::get<0>(pools[component_type]);

            if (component_pool.has(ent)) {
                // trigger destruction/removal signals
                std::get<2>(pools[component_type]).TriggerSignal(*this, ent);
                component_pool.destroy(ent);
            }
        }

        template<typename ComponentType>
        void RemoveAllComponentsOfType() {
            assureComponentStorage<ComponentType>();
            const component_family::family_type component_type = component_family::Type<ComponentType>();
            auto& pool = *std::get<0>(pools[component_type]);
            // destruction signal handler
            auto& signal_handler = std::get<2>(pools[component_type]);

            for (const auto& ent : pool) {
                signal_handler.TriggerSignal(*this, ent);
                pool.destroy(ent);
            }

        }

        void Reset() {
            
        }

        template<typename Functor>
        void ForEach(Functor fn) const {
            if (available) {
                for (size_type pos = entities.size(); pos; --pos) {
                    const entity_type current = static_cast<entity_type>(pos - 1);
                    const entity_type entity = entities[current];
                    const entity_type entt = entity & traits_type::entity_mask;

                    if (current == entt) {
                        fn(entity);
                    }
                }
            }
            else {
                for (auto pos = entities.size(); pos; --pos) {
                    fn(entities[pos - 1]);
                }
            }
        }

        bool EmptyEntity(entity_type ent) const {
            if (ENTITY_SYSTEM_CHECKS) {
                ValidException(ent);
            }

            bool is_empty = true;

            for (size_type i = 0; (i < pools.size()) && is_empty; ++i) {
                const auto& pool = std::get<0>(pools[i]);
                is_empty = !(pool && pool->has(ent));
            }

            for (size_type i = 0; (i < tags.size()) && is_empty; ++i) {
                const auto& tag = std::get<0>(tags[i]);
                is_empty = !(tag && (tag->entity == ent));
            }

            return is_empty;
        }

        template<typename Functor>
        void ApplyToEmptyEntities(Functor func) {
            ForEach([func = std::move(func), this](auto entity) {
                if (EmptyEntity(entity)) {
                    func(entity);
                }
            });
        }

        template<typename...ComponentTypes>
        MultiComponentView<EntityType, ComponentTypes...> View() {
            return MultiComponentView<EntityType, ComponentTypes...>{ (assureComponentStorage<ComponentTypes>(), getComponentStorage<ComponentTypes>())... };
        }

        template<typename...ComponentTypes>
        void PreparePersistentViews() {
            static_assert((sizeof...(ComponentTypes) > 1), "Need more than one component type to prepare corresponding persistent views");

            const handler_family::family_type htype = handler_family::Type<ComponentTypes...>();
            if (htype >= handlers.size()) {
                handlers.resize(htype + 1);
            }

            if (!handlers[htype]) {
                using accum_type = size_type[];
                handlers[htype] = std::make_unique<SparseSet<entity_type>>();
                auto& handler = handlers[htype];

                for (auto entity : View<ComponentTypes...>()) {
                    handler->construct(entity);
                }

                auto connect = [this](const auto& ctype) {
                    auto& cpool = pools[ctype];
                    std::get<1>(cpool).sink().template connect<&Registry::GetComponentCreationAndAssignmentSink<ComponentTypes...>>();
                    std::get<2>(cpool).sink().template connect<&Registry::GetComponentDestructionAndRemovalSink<ComponentTypes...>>();
                };

                accum_type accumulator = { (assureComponentStorage<ComponentTypes>(), connect(component_family::Type<ComponentTypes>()), 0)... };
                (void)accumulator;
            }
        }

        template<typename...ComponentTypes>
        void DiscardPersistentViews() {
            if (HasPersistentView<ComponentTypes...>()) {
                using accum_type = size_type[];

                const handler_family::family_type htype = handler_family::Type<ComponentTypes...>();

                auto disconnect = [this](const auto& ctype) {
                    auto& cpool = pools[ctype];
                    std::get<1>(cpool).sink().template disconnect<&Registry::GetComponentCreationAndAssignmentSink<ComponentTypes...>>();
                    std::get<2>(cpool).sink().template disconnect<&Registry::GetComponentDestructionAndRemovalSink<ComponentTypes...>>();
                };

                accum_type accumulator = { (disconnect(component_family::Type<ComponentTypes>()), 0)... };
                (void)accumulator;
            }
        }

        template<typename...ComponentTypes>
        bool HasPersistentView() const noexcept {
            static_assert(sizeof...(ComponentTypes) > 1, "Need more than one component type for any persistent-view related functions");
            const handler_family::family_type htype = handler_family::Type<ComponentTypes...>();
            return (htype < handlers.size() && handlers[htype]);
        }

        template<typename...ComponentTypes>
        PersistentComponentView<EntityType, ComponentTypes...> PersistentView() {
            PreparePersistentViews<ComponentTypes...>();
            const handler_family::family_type htype = handler_family::Type<ComponentTypes...>();
            return PersistentComponentView<EntityType, ComponentTypes...>{ *handlers[htype], (assureComponentStorage<ComponentTypes>(), getComponentStorage<ComponentTypes>())... };
        }

        template<typename ComponentType>
        RawComponentView<EntityType, ComponentType> RawComponentView() {
            assureComponentStorage<ComponentType>();
            return RawComponentView<EntityType, ComponentType>{ getComponentStorage<ComponentType>() };
        }

    };

    using DefaultRegistryType = Registry<std::uint32_t>;

}

#endif //!VPSK_REGISTRY_HPP