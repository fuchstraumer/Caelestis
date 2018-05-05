#include "SparseSet.hpp"

namespace vpsk {

    template<typename EntityType>
    inline  SparseSet<EntityType>::size_type SparseSet<EntityType>::reserve(size_type desired_capacity) {
        return packed_data.reserve(desired_capacity).capacity();
    }

    template<typename EntityType>
    inline  SparseSet<EntityType>::size_type SparseSet<EntityType>::extent() const noexcept {
        return sparse_data.size();
    }

    template<typename EntityType>
    inline  SparseSet<EntityType>::size_type SparseSet<EntityType>::size() const noexcept  {
        return packed_data.size();
    }

    template<typename EntityType>
    inline bool SparseSet<EntityType>::empty() const noexcept {
        return packed_data.empty();
    }

    template<typename EntityType>
    inline const  SparseSet<EntityType>::entity_type * SparseSet<EntityType>::data() const noexcept {
        return packed_data.data();
    }

    template<typename EntityType>
    inline  SparseSet<EntityType>::const_iterator_type SparseSet<EntityType>::cbegin() const noexcept {
        return const_iterator_type(packed_data.data(), packed_data.size());
    }

    template<typename EntityType>
    inline  SparseSet<EntityType>::const_iterator_type SparseSet<EntityType>::cend() const noexcept {
        return const_iterator_type(packed_data.data(), 0);
    }

    template<typename EntityType>
    inline  SparseSet<EntityType>::iterator SparseSet<EntityType>::cbegin() const noexcept {
        return iterator(packed_data.data(), packed_data.size());
    }

    template<typename EntityType>
    inline  SparseSet<EntityType>::iterator SparseSet<EntityType>::cend() const noexcept {
        return iterator(packed_data.data(), 0);
    }

    template<typename EntityType>
    inline  SparseSet<EntityType>::iterator SparseSet<EntityType>::begin() noexcept {
        return iterator(packed_data.data(), packed_data.size());
    }

    template<typename EntityType>
    inline SparseSet<EntityType>::iterator SparseSet<EntityType>::end() noexcept {
        return iterator(packed_data.data(), 0);
    }

    template<typename EntityType>
    inline bool SparseSet<EntityType>::has(entity_type entity) const noexcept {
        const size_type position = static_cast<size_type>(entity & traits_type::entity_mask);
        return (position < sparse_data.size()) && (sparse_data[position] != PENDING_ENTITY);
    }

    template<typename EntityType>
    inline bool SparseSet<EntityType>::check_has_fast(entity_type entity) const {
        const size_type position = static_cast<size_type>(entity & traits_type::entity_mask);
        return sparse_data[position] != PENDING_ENTITY;
    }

    template<typename EntityType>
    inline position_type SparseSet<EntityType>::get(entity_type entity) const noexcept {
        return sparse_data[entity & traits_type::entity_mask];
    }

    template<typename EntityType>
    inline void SparseSet<EntityType>::construct(entity_type entity) {
        const size_type position = static_cast<size_type>(entity & traits_type::entity_mask);
        if (!(position < sparse_data.size())) {
            // resize and assign default values.
            sparse_data.resize(position + 1, PENDING_ENTITY);
        }
        sparse_data[position] = static_cast<position_type>(packed_data.size());
        packed_data.emplace_back(entity);
    }

    template<typename EntityType>
    inline void SparseSet<EntityType>::destroy(entity_type entity) {
        const EntityType& back = packed_data.back();
        position_type& candidate = sparse_data[entity & traits_type::entity_mask];
        sparse_data[back & traits_type::entity_mask] = candidate;
        candidate = PENDING_ENTITY;
        packed_data.pop_back();
    }

    template<typename EntityType>
    inline void SparseSet<EntityType>::swap_entity_positions(position_type lhs, position_type rhs) {
        auto& source = packed_data[lhs];
        auto& destination = packed_data[rhs];
        std::swap(sparse_data[source & traits_type::entity_mask], sparse_data[destination & traits_type::entity_mask]);
        std::swap(source, destination);
    }

    template<typename EntityType>
    inline void SparseSet<EntityType>::sort_based_on_other(const SparseSet<EntityType>& other) {
        SparseSet<EntityType>::const_iterator_type begin_range = other.cbegin();
        SparseSet<EntityType>::const_iterator_type end_range = other.cend();

        position_type position = static_cast<position_type>(packed_data.size() - 1);

        while ((position != 0) && (begin_range != end_range)) {
            if (has(*begin_range)) {
                if (*begin_range != packed_data[position]) {
                    swap(position, get(*begin_range));
                }

                --position;
            }

            ++begin_range;
        }
    }

    template<typename EntityType>
    inline void SparseSet<EntityType>::reset() {
        packed_data.clear(); packed_data.shrink_to_fit();
        sparse_data.clear(); sparse_data.shrink_to_fit();
    }

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>::SparseSet(SparseSet && other) noexcept : underlying_type(std::move(other)), objects(std::move(other.objects)) {}

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>& SparseSet<typename EntityType, typename ObjectType>::operator=(SparseSet&& other) noexcept {
        underlying_type::operator=(other);
        objects = std::move(other.objects);
        return *this;
    }

    template<typename EntityType, typename ObjectType>
    void SparseSet<typename EntityType, typename ObjectType>::reserve(size_type desired_capacity) {
        underlying_type::reserve(desired_capacity);
        objects.reserve(desired_capacity);
    }

    template<typename EntityType, typename ObjectType>
    const SparseSet<typename EntityType, typename ObjectType>::object_type* SparseSet<typename EntityType, typename ObjectType>::objects_ptr() const noexcept {
        return objects.data();
    }

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>::object_type* SparseSet<typename EntityType, typename ObjectType>::objects_ptr() noexcept {
        return objects.data();
    }

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>::const_iterator_type SparseSet<typename EntityType, typename ObjectType>::cbegin() const noexcept {
        return const_iterator_type{ objects.data(), objects.size() };
    }

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>::const_iterator_type SparseSet<typename EntityType, typename ObjectType>::cend() const noexcept {
        return const_iterator_type{ objects.data(), 0 };
    }

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>::iterator SparseSet<typename EntityType, typename ObjectType>::cbegin() const noexcept {
        return iterator{ objects.data(), objects.size() };
    }

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>::iterator SparseSet<typename EntityType, typename ObjectType>::cend() const noexcept {
        return iterator{ objects.data(), 0 };
    }

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>::iterator SparseSet<typename EntityType, typename ObjectType>::begin() noexcept {
        return iterator{ objects.data(), objects.size() };
    }

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>::iterator SparseSet<typename EntityType, typename ObjectType>::end() noexcept {
        return iterator{ objects.data(), 0 };
    }

    template<typename EntityType, typename ObjectType>
    const SparseSet<typename EntityType, typename ObjectType>::object_type& SparseSet<typename EntityType, typename ObjectType>::get(SparseSet::entity_type entity) const noexcept {
        return objects[underlying_type::get(entity)];
    }

    template<typename EntityType, typename ObjectType>
    SparseSet<typename EntityType, typename ObjectType>::object_type& SparseSet<typename EntityType, typename ObjectType>::get(SparseSet::entity_type entity) noexcept {
        return objects[underlying_type::get(entity)];
    }

    template<typename EntityType, typename ObjectType>
    template<typename ...Args>
    inline std::enable_if_t<std::is_constructible<ObjectType, Args...>::value, SparseSet<typename EntityType, typename ObjectType>::object_type&> 
        SparseSet<typename EntityType, typename ObjectType>::construct(entity_type entity, Args && ...args) {
        underlying_type::construct(entity);
        objects.emplace_back(std::forward<Args&&>(args)...);
        return objects.back();
    }

    template<typename EntityType, typename ObjectType>
    template<typename ...Args>
    inline std::enable_if_t<!std::is_constructible<ObjectType, Args...>::value, SparseSet<typename EntityType, typename ObjectType>::object_type&>
        SparseSet<typename EntityType, typename ObjectType>::construct(entity_type entity, Args && ...args) {
        underlying_type::construct(entity);
        objects.emplace_back(ObjectType{ std::forward<Args&&>(args)... });
        return objects.back();
    }

    template<typename EntityType, typename ObjectType>
    inline void SparseSet<typename EntityType, typename ObjectType>::destroy(SparseSet::entity_type entity) {
        ObjectType&& tmp = std::move(objects.back());
        objects[underlying_type::get(entity)] = std::move(tmp);
        objects.pop_back();
        underlying_type::destroy(entity);
    }

    template<typename EntityType, typename ObjectType>
    template<typename CompareFunc, typename Sort>
    inline void SparseSet<typename EntityType, typename ObjectType>::sort(CompareFunc cmp, Sort sorter) {
        std::vector<position_type> copies(objects.size());
        std::iota(copies.begin(), copies.end(), 0);

        sorter(copies.begin(), copies.end(), [this, cmp = std::move(cmp)](auto lhs, auto rhs){
            return cmp(const_cast<const object_type&>(objects[rhs]), const_cast<const object_type&>(objects[lhs]));
        });

        for (position_type position = 0, last = copies.size(); position < last; ++position) {
            position_type current = position;
            position_type next = copies[current];
            while (current != next) {
                const position_type lhs = copies[current];
                const position_type rhs = copies[next];
                std::swap(objects[lhs], objects[rhs]);
                underlying_type::swap(lhs, rhs);
                copies[current] = current;
                current = next;
                next = copies[current];
            }
        }
    }

    template<typename EntityType, typename ObjectType>
    inline void SparseSet<typename EntityType, typename ObjectType>::sort_based_on_other(const SparseSet<SparseSet::entity_type>& other) {
        SparseSet<SparseSet::entity_type>::const_iterator_type begin_range = other.cbegin();
        SparseSet<SparseSet::entity_type>::const_iterator_type end_range = other.cend();

        position_type position = underlying_type::size() - 1;
        const underlying_type::entity_type* local = underlying_type::data();

        while (position && begin_range != end_range) {
            const auto current = *begin_range;

            if (underlying_type::has(current)) {
                if (current != *(local + position)) {
                    auto candidate = underlying_type::get(current);
                    std::swap(objects[position], objects[candidate]);
                    underlying_type::swap(position, candidate);
                }

                --position;
            }

            ++begin_range;
        }
    }

    template<typename EntityType, typename ObjectType>
    inline void SparseSet<typename EntityType, typename ObjectType>::reset() {
        underlying_type::reset();
        objects.clear(); objects.shrink_to_fit();
    }

}