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
#include "Entity.hpp"

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
        struct iterator final {
            using difference_type = std::size_t;
            using value_type = EntityType;
            using pointer = const value_type*;
            using reference = value_type;
            using iterator_category = std::input_iterator_tag;

            iterator(pointer ptr, std::size_t _pos) : value_ptr(ptr), pos(std::move(_pos)) {}

            iterator& operator++() noexcept {
                --pos;
                return *this;
                // could've been return "--pos, *this": I just find comma operator a bit confusing so didn't use that
            }

            iterator& operator++(int) noexcept {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            iterator& operator+=(difference_type value) noexcept {
                pos -= value;
                return *this;
            }

            iterator operator+(difference_type value) noexcept {
                return iterator{ value_ptr, pos - value };
            }

            bool operator==(const iterator& other) const noexcept {
                return !(*this == other);
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
        using iterator_type = iterator;
        using const_iterator_type = iterator;

        SparseSet() noexcept = default;
        virtual ~SparseSet() noexcept = default;

        SparseSet(SparseSet&& other) noexcept;
        SparseSet(const SparseSet&) = delete;

        SparseSet& operator=(const SparseSet&) = delete;
        SparseSet& operator=(SparseSet&& other) noexcept;

        size_type reserve(size_type desired_capacity);
        // size of the sparse array attached to this object
        size_type extent() const noexcept;
        // size of the dense array attached to this object
        size_type size() const noexcept;
        bool empty() const noexcept;
        const entity_type* data() const noexcept;

        const_iterator_type cbegin() const noexcept;
        const_iterator_type cend() const noexcept;
        iterator_type cbegin() const noexcept;
        iterator_type cend() const noexcept;
        iterator_type begin() noexcept;
        iterator_type end() noexcept;

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
        struct iterator final {
            using difference_type = std::size_t;
            using value_type = std::conditional_t<constant_variant, const ObjectType, ObjectType>;
            using pointer = value_type*;
            using reference = value_type&;
            using iterator_category = std::input_iterator_tag;

            iterator(pointer _values, std::size_t _pos) noexcept : values(_values) : pos(std::move(_pos)) {}

            iterator& operator++() noexcept {
                --pos;
                return *this;
            }

            iterator& operator++(int) noexcept {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            iterator& operator+=(difference_type value) noexcept {
                pos -= value;
                return *this;
            }

            iterator operator+(difference_type value) noexcept {
                return iterator{ values, pos - value };
            }

            bool operator==(const iterator& other) const noexcept {
                return other.pos == pos;
            }

            inline bool operator!=(const iterator& other) const noexcept {
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
        using size_type = typename underyling_type::size_type;
        using iterator_type = iterator<false>;
        using const_iterator_type = iterator<true>;

        SparseSet() noexcept = default;
        SparseSet(const SparseSet&) = delete;
        SparseSet(SparseSet&& other) noexcept;
        SparseSet& operator=(const SparseSet&) = delete;
        SparseSet& operator=(SparseSet&& other) = delete;

        void reserve(size_type desired_capacity);
        const object_type* objects_ptr() const noexcept;
        object_type* objects_ptr() noexcept;

        const_iterator_type cbegin() const noexcept;
        iterator_type cbegin() const noexcept;
        const_iterator_type cend() const noexcept;
        iterator_type cend() const noexcept;
        iterator_type begin() noexcept;
        iterator_type end() noexcept;

        const object_type& get(entity_type entity) const noexcept;
        object_type& get(entity_type entity) noexcept;

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

}

#endif //!VPSK_SPARSE_SET_HPP
