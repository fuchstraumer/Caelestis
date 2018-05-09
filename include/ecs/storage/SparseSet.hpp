#pragma once
#ifndef VPSK_SPARSE_SET_HPP
#define VPSK_SPARSE_SET_HPP
#include <iterator>
#include <algorithm>
#include <numeric>
#include <vector>
#include <type_traits>
#include <functional>
#include <utility>
#include "../EntityTraits.hpp"

namespace vpsk {

    struct StdSort {
        template<typename Iter, typename CompareFunc = std::less<>>
        void operator()(Iter first, Iter last, CompareFunc cmp = CompareFunc{}) {
            std::sort(std::move(first), std::move(last), std::move(cmp));
        }
    };

    struct InsertionSort {
        template<typename Iter, typename CompareFunc = std::less<>>
        void operator()(Iter first, Iter last, CompareFunc cmp = CompareFunc{}) {
            auto iter = first + 1;
            while (iter != last) {
                auto&& value = *iter;
                auto&& pre = iter;

                while (pre ! = first && compare(value, *(pre - 1))) {
                    *pre = *(pre - 1);
                    --pre;
                }

                *pre = value;
                ++iter;
            }
        }
    };

    template<typename...>
    class SparseSet;

    template<typename EntityType>
    class SparseSet<EntityType> {
        using traits_type = EntityTraits<EntityType>;
        // Specialize iterator type for this container
        struct sparse_set_iterator_t final {
            using difference_type = std::size_t;
            using value_type = EntityType;
            using pointer = const value_type*;
            using reference = value_type;
            using iterator_category = std::input_iterator_tag;

            sparse_set_iterator_t(pointer ptr, std::size_t _pos) : value_ptr(ptr), pos(std::move(_pos)) {}

            sparse_set_iterator_t& operator++() noexcept {
                --pos;
                return *this;
                // could've been return "--pos, *this": I just find comma operator a bit confusing so didn't use that
            }

            sparse_set_iterator_t& operator++(int) noexcept {
                sparse_set_iterator_t tmp = *this;
                ++(*this);
                return tmp;
            }

            sparse_set_iterator_t& operator+=(difference_type value) noexcept {
                pos -= value;
                return *this;
            }

            sparse_set_iterator_t operator+(difference_type value) noexcept {
                return sparse_set_iterator_t{ value_ptr, pos - value };
            }

            bool operator!=(const sparse_set_iterator_t& other) const noexcept {
                return pos != other.pos;
            }

            bool operator==(const sparse_set_iterator_t& other) const noexcept {
                return pos == other.pos;
            }

            reference operator*() const noexcept {
                return value_ptr[pos-1];
            }

        private:
            pointer value_ptr;
            std::size_t pos;
        };

    public:

        using entity_type = EntityType;
        using position_type = entity_type;
        static constexpr auto PENDING_ENTITY = std::numeric_limits<traits_type::entity_type>::max();
        using size_type = std::size_t;
        using iterator = sparse_set_iterator_t;
        using const_iterator = sparse_set_iterator_t;

        SparseSet() noexcept = default;
        virtual ~SparseSet() noexcept = default;

        SparseSet(SparseSet&& other) noexcept;
        SparseSet(const SparseSet&) = delete;

        SparseSet& operator=(const SparseSet&) = delete;
        SparseSet& operator=(SparseSet&& other) noexcept;

        void reserve(size_type desired_capacity);
        // size of the sparse array attached to this object
        size_type extent() const noexcept;
        // size of the dense array attached to this object
        size_type size() const noexcept;
        bool empty() const noexcept;
        const entity_type* data() const noexcept;

        iterator cbegin() const noexcept;
        iterator cend() const noexcept;
        iterator begin() noexcept;
        iterator end() noexcept;

        bool has(entity_type entity) const noexcept;
        // Will throw if invoked outside of bounds
        bool check_has_fast(entity_type entity) const;
        position_type get(entity_type) const noexcept;
        void construct(entity_type entity);
        virtual void destroy(entity_type entity);
        void swap_entity_positions(position_type lhs, position_type rhs);
        void sort_based_on_other(const SparseSet<EntityType>& other);

        virtual void reset();
    
    private:
        std::vector<position_type> sparse_data;
        std::vector<EntityType> packed_data;
    };

    template<typename EntityType>
    inline SparseSet<EntityType>::SparseSet(SparseSet && other) noexcept : sparse_data(std::move(other.sparse_data)), packed_data(std::move(other.packed_data)) {}

    template<typename EntityType>
    inline SparseSet<EntityType>& SparseSet<EntityType>::operator=(SparseSet&& other) noexcept {
        sparse_data = std::move(other.sparse_data);
        packed_data = std::move(other.packed_data);
        return *this;
    }

    // Specialiation of sparse set for associating an object explicitly to an entity.
    template<typename Entity, typename ObjectType>
    class SparseSet<Entity, ObjectType> : public SparseSet<Entity> {
        using underlying_type = SparseSet<Entity>;

        template<bool constant_variant>
        struct sparse_object_set_iterator_t final {
            using difference_type = std::size_t;
            using value_type = std::conditional_t<constant_variant, const ObjectType, ObjectType>;
            using pointer = value_type*;
            using reference = value_type&;
            using iterator_category = std::input_iterator_tag;

            sparse_object_set_iterator_t(pointer _values, std::size_t _pos) noexcept : values(_values) : pos(std::move(_pos)) {}

            sparse_object_set_iterator_t& operator++() noexcept {
                --pos;
                return *this;
            }

            sparse_object_set_iterator_t& operator++(int) noexcept {
                sparse_object_set_iterator_t tmp = *this;
                ++(*this);
                return tmp;
            }

            sparse_object_set_iterator_t& operator+=(difference_type value) noexcept {
                pos -= value;
                return *this;
            }

            sparse_object_set_iterator_t operator+(difference_type value) noexcept {
                return sparse_object_set_iterator_t{ values, pos - value };
            }

            bool operator==(const sparse_object_set_iterator_t& other) const noexcept {
                return other.pos == pos;
            }

            inline bool operator!=(const sparse_object_set_iterator_t& other) const noexcept {
                return other.pos != pos;
            }

            reference operator*() const noexcept {
                return values[pos - 1];
            }

            pointer operator->() const noexcept {
                return (values + pos - 1);
            }

        private:
            pointer values;
            std::size_t pos;
        };

    public:

        using object_type = ObjectType;
        using entity_type = typename underlying_type::entity_type;
        using position_type = typename underlying_type::position_type;
        using size_type = typename underlying_type::size_type;
        using iterator = sparse_object_set_iterator_t<false>;
        using const_iterator_type = sparse_object_set_iterator_t<true>;

        SparseSet() noexcept = default;
        SparseSet(const SparseSet&) = delete;
        SparseSet(SparseSet&& other) noexcept;
        SparseSet& operator=(const SparseSet&) = delete;
        SparseSet& operator=(SparseSet&& other) noexcept;

        void reserve(size_type desired_capacity);
        const object_type* objects_ptr() const noexcept;
        object_type* objects_ptr() noexcept;

        const_iterator_type cbegin() const noexcept;
        const_iterator_type cend() const noexcept;
        iterator begin() noexcept;
        iterator end() noexcept;

       const object_type& get(SparseSet::entity_type entity) const noexcept {
            return objects[underlying_type::get(entity)];
        }

        object_type& get(SparseSet::entity_type entity) noexcept {
            return objects[underlying_type::get(entity)];
        }

        template<typename...Args>
        std::enable_if_t<std::is_constructible<ObjectType, Args...>::value, object_type&> construct(entity_type entity, Args&&...args);
        template<typename...Args>
        std::enable_if_t<!std::is_constructible<ObjectType, Args...>::value, object_type&> construct(entity_type entity, Args&&...args);

        void destroy(entity_type entity) final;

        template<typename CompareFunc, typename Sort = StdSort>
        void sort(CompareFunc cmp, Sort sorter = Sort{});

        void sort_based_on_other(const SparseSet<entity_type>& other) noexcept;

        void reset() final;

    private:
        std::vector<object_type> objects;
    };

    template<typename EntityType>
    inline void SparseSet<EntityType>::reserve(size_type desired_capacity) {
        packed_data.reserve(desired_capacity);
    }

    template<typename EntityType>
    inline typename SparseSet<EntityType>::size_type SparseSet<EntityType>::extent() const noexcept {
        return sparse_data.size();
    }

    template<typename EntityType>
    inline typename SparseSet<EntityType>::size_type SparseSet<EntityType>::size() const noexcept {
        return packed_data.size();
    }

    template<typename EntityType>
    inline bool SparseSet<EntityType>::empty() const noexcept {
        return packed_data.empty();
    }

    template<typename EntityType>
    inline const typename SparseSet<EntityType>::entity_type * SparseSet<EntityType>::data() const noexcept {
        return packed_data.data();
    }

    template<typename EntityType>
    inline typename SparseSet<EntityType>::iterator SparseSet<EntityType>::cbegin() const noexcept {
        return iterator(packed_data.data(), packed_data.size());
    }

    template<typename EntityType>
    inline typename SparseSet<EntityType>::iterator SparseSet<EntityType>::cend() const noexcept {
        return iterator(packed_data.data(), 0);
    }

    template<typename EntityType>
    inline typename SparseSet<EntityType>::iterator SparseSet<EntityType>::begin() noexcept {
        return iterator(packed_data.data(), packed_data.size());
    }

    template<typename EntityType>
    inline typename SparseSet<EntityType>::iterator SparseSet<EntityType>::end() noexcept {
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
    inline typename SparseSet<EntityType>::position_type SparseSet<EntityType>::get(entity_type entity) const noexcept {
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
    inline SparseSet<typename EntityType, typename ObjectType>& SparseSet<typename EntityType, typename ObjectType>::operator=(SparseSet&& other) noexcept {
        underlying_type::operator=(other);
        objects = std::move(other.objects);
        return *this;
    }

    template<typename EntityType, typename ObjectType>
    inline void SparseSet<typename EntityType, typename ObjectType>::reserve(size_type desired_capacity) {
        underlying_type::reserve(desired_capacity);
        objects.reserve(desired_capacity);
    }

    template<typename EntityType, typename ObjectType>
    inline const typename SparseSet<typename EntityType, typename ObjectType>::object_type* SparseSet<typename EntityType, typename ObjectType>::objects_ptr() const noexcept {
        return objects.data();
    }

    template<typename EntityType, typename ObjectType>
    inline typename SparseSet<typename EntityType, typename ObjectType>::object_type* SparseSet<typename EntityType, typename ObjectType>::objects_ptr() noexcept {
        return const_cast<ObjectType*>(const_cast<const SparseSet*>(this)->objects_ptr());
    }

    template<typename EntityType, typename ObjectType>
    inline typename SparseSet<typename EntityType, typename ObjectType>::const_iterator_type SparseSet<typename EntityType, typename ObjectType>::cbegin() const noexcept {
        return const_iterator_type{ objects.data(), objects.size() };
    }

    template<typename EntityType, typename ObjectType>
    inline typename SparseSet<typename EntityType, typename ObjectType>::const_iterator_type SparseSet<typename EntityType, typename ObjectType>::cend() const noexcept {
        return const_iterator_type{ objects.data(), 0 };
    }

    template<typename EntityType, typename ObjectType>
    inline typename SparseSet<typename EntityType, typename ObjectType>::iterator SparseSet<typename EntityType, typename ObjectType>::begin() noexcept {
        return iterator{ objects.data(), objects.size() };
    }

    template<typename EntityType, typename ObjectType>
    inline typename SparseSet<typename EntityType, typename ObjectType>::iterator SparseSet<typename EntityType, typename ObjectType>::end() noexcept {
        return iterator{ objects.data(), 0 };
    }

    template<typename EntityType, typename ObjectType>
    template<typename ...Args>
    inline std::enable_if_t<std::is_constructible<ObjectType, Args...>::value, ObjectType&> SparseSet<EntityType, ObjectType>::construct(entity_type entity, Args && ...args) {
        underlying_type::construct(entity);
        objects.emplace_back(std::forward<Args>(args)...);
        return objects.back();
    }

    template<typename EntityType, typename ObjectType>
    template<typename ...Args>
    inline typename std::enable_if_t<!std::is_constructible<ObjectType, Args...>::value, ObjectType&> SparseSet<EntityType, ObjectType>::construct(entity_type entity, Args && ...args) {
        underlying_type::construct(entity);
        objects.emplace_back(ObjectType{ std::forward<Args>(args)... });
        return objects.back();
    }

    template<typename EntityType, typename ObjectType>
    inline void SparseSet<EntityType, ObjectType>::destroy(SparseSet::entity_type entity) {
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
    inline void SparseSet<typename EntityType, typename ObjectType>::sort_based_on_other(const SparseSet<SparseSet::entity_type>& other) noexcept {
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

#endif //!VPSK_SPARSE_SET_HPP
