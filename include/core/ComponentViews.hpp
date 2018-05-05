#pragma once
#ifndef VPSK_COMPONENT_VIEWS_HPP
#define VPSK_COMPONENT_VIEWS_HPP
#include "Identifiers.hpp"
#include "storage/SparseSet.hpp"
#include <tuple>

namespace vpsk {

    template<typename>
    class Registry;

    template<typename Entity, typename...Components>
    class PersistentComponentView final {
        static_assert(sizeof...(Components) > 1, "Need to have more than one component in View template argument list.");

        friend class Registry<Entity>;

        // Use specialized SparseSet for objects for component storage 
        template<typename ComponentType>
        using pool_type = SparseSet<Entity, ComponentType>;
        // Use base SparseSet for collating entities, but not the components or the backing data for the same
        using view_type = SparseSet<Entity>;

        // Search/filter "pattern" involves a tuple of component pools we're trying to identify.
        using pattern_type = std::tuple<pool_type<Components>&...>;
        
        PersistentComponentView(view_type& entity_view, pool_type<Components>&... component_pools) noexcept : entityView(entity_view), componentPools(component_pools) {};

    public:

        using iterator = typename view_type::iterator;
        using const_iterator = typename view_type::const_iterator;
        using entity_type = typename view_type::entity_type;
        using size_type = typename view_type::size_type;

        size_type NumEntitiesWithComponents() const noexcept {
            return entityView.size();
        }

        bool NoEntitiesWithComponents() const noexcept {
            return entityView.empty();
        }

        const entity_type* EntitiesPtr() const noexcept {
            return entityView.data();
        }

        const_iterator cbegin() const noexcept {
            return entityView.cbegin();
        }

        const_iterator cend() const noexcept {
            return entityView.cend();
        }

        iterator cbegin() const noexcept {
            return entityView.cbegin();
        }

        iterator cend() const noexcept {
            return entityView.cend();
        }

        iterator begin() noexcept {
            return entityView.begin();
        }

        iterator end() noexcept {
            return entityView.end();
        }

        bool HasEntity(entity_type ent) const noexcept {
            return entityView.has(ent) && (entityView[ent] == ent);
        }

        template<typename ComponentType>
        const ComponentType& GetComponent(entity_type ent) const noexcept {
            return std::get<pool_type<ComponentType>&>(componentPools).get(ent);
        }

        template<typename ComponentType>
        ComponentType& GetComponent(entity_type ent) noexcept {
            return std::get<pool_type<ComponentType>&>(componentPools).get(ent);
        }

        // Does not have to match or be the full list of components given when defining this view
        template<typename...ComponentTypes>
        std::enable_if_t<(sizeof...(ComponentTypes) > 1), std::tuple<const ComponentTypes&...>> GetComponents(entity_type ent) const noexcept {
            return std::tuple<const ComponentTypes&...>{ GetComponent<ComponentTypes>(ent)... };
        }

        template<typename...ComponentTypes>
        std::enable_if_t<(sizeof...(ComponentTypes) > 1), std::tuple<ComponentTypes&...>> GetComponents(entity_type end) noexcept {
            return std::tuple<ComponentTypes&...>{ GetComponent<ComponentTypes>(ent)... };
        }

        template<typename Function>
        void ApplyToEach(Function func) const {
            std::for_each(entityView.cbegin(), entityView.cend(), [&func, this](const auto& ent) {
                func(ent, std::get<pool_type<Components>&>(pools).get(ent)...);
            });
        }

        template<typename Function>
        void ApplyToEach(Function func) {
            std::for_each(entityView.begin(), entityView.end(), [&func, this](const auto& ent) {
                func(ent, std::get<pool_type<Components>&>(pools).get(ent)...);
            });
        }

        // Sort entity pool so that it's ordered by the supplied component first
        template<typename ComponentType>
        void SortByComponentType() {
            entityView.sort_based_on_other(std::get<pool_type<ComponentType>&>(componentPools));
        }

    private:
        view_type & entityView;
        const pattern_type componentPools;
    };

    template<typename EntityType, typename...ComponentTypes>
    class MultiComponentView final {
        //static_assert(sizeof...(ComponentTypes) > 1, "Need more than 1 Component to create a valid view type");

        friend class Registry<EntityType>;

        template<typename Component>
        using pool_type = SparseSet<EntityType, Component>;

        using view_type = SparseSet<EntityType>;
        using view_iterator = typename view_type::const_iterator;
        using unchecked_type = std::array<const view_type*, (sizeof...(ComponentTypes)-1)>;
        using pattern_type = std::tuple<pool_type<ComponentTypes>&...>;
        using traits_type = EntityTraits<EntityType>;

        class mcmv_iterator_t {
            using size_type = typename view_type::size_type;

            inline bool valid() const noexcept {
                const auto entity = *begin;
                const size_type sz = static_cast<size_type>(entity & traits_type::entity_mask);
                size_type position = static_cast<size_type>(unchecked.size());

                if (sz < extent) {
                    for (; position && unchecked[pos - 1]->check_has_fast(entity); --position);
                }

                return !position;
            }

        public:
            using difference_type = typename view_iterator::difference_type;
            using value_type = typename view_iterator::value_type;
            using pointer = typename view_iterator::pointer;
            using reference = typename view_iterator::reference;
            using iterator_category = typename view_iterator::iterator_category;

            mcmv_iterator_t(unchecked_type _unchecked, size_type _extent, view_iterator _begin, view_iterator _end) noexcept : unchecked(_unchecked), extent(_extent), begin(_begin), end(_end) {
                if (begin != end && !valid()) {
                    ++(*this);
                }
            }

            mcmv_iterator_t& operator++() noexcept {
                return (++begin != end && !valid()) ? ++(*this) : *this;
            }

            mcmv_iterator_t operator++(int) noexcept {
                mcmv_iterator_t tmp = *this;
                return ++(*this), tmp;
            }

            mcmv_iterator_t& operator+=(difference_type diff) noexcept {
                return ((begin += value) != end && !valid()) ? ++(*this) : *this;
            }

            mcmv_iterator_t& operator+(difference_type diff) noexcept {
                return mcmv_iterator_t{ unchecked, extent, begin + diff, end };
            }

            bool operator==(const mcmv_iterator_t& other) const noexcept {
                return begin == other.begin;
            }

            bool operator!=(const mcmv_iterator_t& other) const noexcept {
                return begin != other.begin;
            }

            value_type operator*() const noexcept {
                return *begin;
            }

        private:
            const unchecked_type unchecked;
            const size_type extent;
            view_iterator begin;
            view_iterator end;
        };

        MultiComponentView(pool_type<ComponentTypes>&... _pools) noexcept : pools{ _pools... }, view{ nullptr }, unchecked{}, idx{} {

        }

        template<typename Component, typename Other, typename Iter>
        std::enable_if_t<std::is_same_v<Component, Other>, const Other&> getComponent(const Iter& iter, EntityType) const {
            return *iter;
        }

        template<typename Component, typename Other, typename Iter>
        std::enable_if_t<!std::is_same_v<Component, Other>, const Other&> getComponent(const Iter& iter, EntityType entity) const {
            return std::get<pool_type<Other>&>(pools).get(entity);
        }

        template<typename Component, typename Func>
        void forEach(Func func) const {
            const auto extent = std::min({ std::get<pool_type<Component>&>(pools).extent()... });
            auto& pool = std::get<pool_type<Component>&>(pools);

            std::for_each(pool.view_type::cbegin(), pool.view_type::cend(), [func = std::move(func), raw = pool.cbegin(), extent, this](const auto ent) mutable {
                const size_type sz = static_cast<size_type>(ent & traits_type::entity_mask);

                if (sz < extent) {
                    size_type position = unchecked.size();
                    for (; position && unchecked[position - 1]->check_has_fast(ent); --position);

                    if (!position) {
                        func(ent, this->getComponent<Component, ComponentTypes>(raw, ent)...);
                    }
                }

                ++raw;
            });

        }

    public:

        using iterator = mcmv_iterator_t;
        using const_iterator = mcmv_iterator_t;
        using entity_type = typename view_type::entity_type;
        using size_type = typename view_type::size_type;

        size_type NumEntitiesWithComponents() const noexcept {
            return view->size();
        }

        bool NoEntitiesWithComponents() const noexcept {
            return view->empty();
        }

        const_iterator cbegin() const noexcept {
            const auto extent = std::min({ std::get<pool_type<ComponentTypes>&>(pools).extent()... });
            return const_iterator{ unchecked, extent, view->cbegin(), view->cend() };
        }

        const_iterator cend() const noexcept {
            const auto extent = std::min({ std::get<pool_type<ComponentTypes>&>(pools).extent()... });
            return const_iterator{ unchecked, extent, view->cend(), view->cend() };
        }

        iterator cbegin() const noexcept {
            const auto extent = std::min({ std::get<pool_type<ComponentTypes>&>(pools).extent()... });
            return iterator{ unchecked, extent, view->cbegin(), view->cend() };
        }

        iterator cend() const noexcept {
            const auto extent = std::min({ std::get<pool_type<ComponentTypes>&>(pools).extent()... });
            return iterator{ unchecked, extent, view->cend(), view->cend() };
        }

        iterator begin() noexcept {
            const auto extent = std::min({ std::get<pool_type<ComponentTypes>&>(pools).extent()... });
            return iterator{ unchecked, extent, view->begin(), view->end() };
        }

        iterator end() noexcept {
            const auto extent = std::min({ std::get<pool_type<ComponentTypes>&>(pools).extent()... });
            return iterator{ unchecked, extent, view->end(), view->end() }:
        }

        bool HasEntity(entity_type ent) const noexcept {
            const auto extent = std::min({ std::get<pool_type<ComponentTypes>&>(pools).extent()... });
            const size_type sz = static_cast<size_type>(ent & traits_type::entity_mask);
            size_type position = unchecked.size();

            if (sz < extent && view->has(ent) && (view[view->get(ent)] == ent)) {
                for (; position && unchecked[position - 1]->check_has_fast(ent); --position);
            }

            return !position;
        }

        template<typename ComponentType>
        const ComponentType& GetComponent(entity_type ent) const noexcept {
            return std::get<pool_type<ComponentType>&>(pools).get(ent);
        }

        template<typename ComponentType>
        ComponentType& GetComponent(entity_type ent) noexcept {
            return std::get<pool_type<ComponentType>&>(pools).get(ent);
        }

        template<typename...RetrievedComponentTypes>
        std::enable_if_t<(sizeof...RetrievedComponentTypes > 1), std::tuple<const RetrievedComponentTypes&... >> GetComponents(entity_type ent) const noexcept {
            return std::tuple<const RetrievedComponentTypes&...>{ GetComponent<RetrievedComponentTypes>(ent)... };
        }
        
        template<typename...RetrievedComponentTypes>
        std::enable_if_t<(sizeof...RetrievedComponentTypes > 1), std::tuple<RetrievedComponentTypes&...>> GetComponents(entity_type ent) noexcept {
            return std::tuple<RetrievedComponentTypes&..>{ GetComponent<RetrievedComponentTypes>(ent)... };
        }

        template<typename Func>
        inline void ForEach(Func fn) const {
            constexpr auto indices = Identifier<ComponentTypes...>;
            using accum_type = size_t[];
            accum_type accum = { (indices.template get<ComponentTypes>() == idx ? (forEach<ComponentTypes>(std::move(func)), 0) : 0)... };
            (void)accum;
        }

        template<typename Func>
        inline void ForEach(Func fn) {
            constexpr auto indices = Identifier<ComponentTypes...>;
            using accum_type = size_t[];
            accum_type accum = { (indices.template get<ComponentTypes>() == idx ? (forEach<ComponentTypes>(std::move(func)), 0) : 0)... };
            (void)accum;
        }

        void Reset() {
            using accum_type = size_t[];
            size_type sz = std::max({ std::get<pool_type<ComponentTypes>&>(pools).size()... }) + std::size_t(1);
            size_type next;

            auto probe = [this](size_type sz, const auto& pool) {
                return pool.size() < sz ? (view = &pool, pool.size()) : sz;
            };

            auto filter = [this](size_type next, const auto& pool) {
                return (view == &pool) ? (idx = next) : (unchecked[next++] = &pool, next);
            };

            accum_type probing = { (sz = probe(sz, std::get<pool_type<ComponentTypes>&>(pools)))... };
            accum_type filtering = { (next = filter(next, std::get<pool_type<ComponentTypes>&>(pools)))... };

            (void)probing;
            (void)filtering;

        }

    private:
        const pattern_type pools;
        const view_type* view;
        unchecked_type unchecked;
        size_type idx;
    };

    template<typename EntityType, typename ComponentType>
    class RawView final {
        friend class Registry<EntityType>;
        using pool_type = SparseSet<EntityType, ComponentType>;
        pool_type& pool;
        RawView(pool_type& _pool) noexcept : pool(_pool) {}
    public:
        using iterator = typename pool_type::iterator;
        using const_iterator = typename pool_type::const_iterator;
        using entity_type = typename pool_type::entity_type;
        using size_type = typename pool_type::size_type;

        size_type NumOfComponent() const noexcept {
            return pool.size();
        }

        bool NoComponenets() const noexcept {
            return pool.empty();
        }

        const ComponentType* raw() const noexcept {
            return pool.objects_ptr();
        }

        ComponentType* raw() noexcept {
            return pool.objects_ptr();
        }

        const_iterator cbegin() const noexcept {
            return pool.cbegin():
        }

        const_iterator cend() const noexcept {
            return pool.cend():
        }

        iterator cbegin() const noexcept {
            return pool.cbegin();
        }

        iterator cend() const noexcept {
            return pool.cend();
        }

        iterator begin() noexcept {
            return pool.begin();
        }

        iterator end() noexcept {
            return pool.end();
        }

        template<typename FunctionType>
        void ForEach(FunctionType fn) const {
            std::for_each(cbegin(), cend(), fn);
        }

        template<typename FunctionType>
        void ForEach(FunctionType fn) {
            std::for_each(begin(), end(), fn);
        }

    };

}

#endif //!VPSK_COMPONENT_VIEWS_HPP