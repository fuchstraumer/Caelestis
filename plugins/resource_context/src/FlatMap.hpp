#pragma once
#ifndef FLAT_MAP_CONTAINER_CLASS_HPP
#define FLAT_MAP_CONTAINER_CLASS_HPP
#include <algorithm>
#include <foonathan/memory/smart_ptr.hpp>
#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>
#include <foonathan/memory/fallback_allocator.hpp>

using set_pool_allocator = foonathan::memory::memory_pool<foonathan::memory::small_node_pool>;
using set_allocator = foonathan::memory::fallback_allocator<set_pool_allocator, foonathan::memory::heap_allocator>;

template<typename Key, typename Compare = std::less<Key>>
struct flat_set : private foonathan::memory::vector<Key, foonathan::memory::heap_allocator> {
private:
    Compare comp;
public:
    using key_type = Key;
    using value_type = Key;
    using container_type = foonathan::memory::vector<key_type, foonathan::memory::heap_allocator>;
    using key_compare = Compare;
    using value_compare = Compare;
    using typename container_type::size_type;
    using typename container_type::difference_type;
    using typename container_type::allocator_type;
    using typename container_type::reference;
    using typename container_type::const_reference;
    using typename container_type::pointer;
    using typename container_type::const_pointer;
    using typename container_type::iterator;
    using typename container_type::const_iterator;
    using typename container_type::reverse_iterator;
    using typename container_type::const_reverse_iterator;
    using container_type::begin;
    using container_type::cbegin;
    using container_type::rbegin;
    using container_type::crbegin;
    using container_type::end;
    using container_type::cend;
    using container_type::rend;
    using container_type::crend;
    using container_type::empty;
    using container_type::size;
    using container_type::max_size;

    flat_set() = default;

    template<typename...Args>
    std::pair<iterator, bool> emplace(Args&&...args) {
        container_type::emplace_back(std::forward<Args>(args)...);
        auto lower = std::lower_bound(begin(), end(), container_type::back(), comp);
        if (*lower == container_type::back() && lower != end() - 1) {
            container_type::pop_back();
            return {lower, false };
        }
        else if (lower == end() - 1) {
            return { end() - 1, false };
        }
        return { std::rotate(rbegin(), rbegin() + 1, reverse_iterator{ lower }).base(), true }
    }

    iterator find(const key_type& key) {
        auto lower = std::lower_bound(begin(), end(), key, comp);
        if (lower != end() && *lower == key) {
            return lower;
        }
        return end();
    }

    const_iterator find(const key_type& key) const {
        auto lower = std::lower_bound(begin(), end(), key, comp);
        if (lower != end() && *lower == key) {
            return lower;
        }
        return end();
    }

    iterator erase(const_iterator pos) {
        return container_type::erase(pos);
    }

    size_type erase(const key_type& key) {
        auto iter = find(key);
        if (iter == end()) {
            return 0;
        }
        erase(iter);
        return 1;
    }

};


template<typename Key, typename ValueType, typename Compare = std::less<Key>>
struct flat_map : private foonathan::memory::vector<std::pair<Key, ValueType>, foonathan::memory::heap_allocator> {


    using key_type = Key;
    using value_type = std::pair<Key, ValueType>;
    using container_type = foonathan::memory::vector<value_type, foonathan::memory::heap_allocator>;
    using key_compare = Compare;

    struct value_compare {
    protected:
        key_compare keyCompare;
    public:
        value_compare(key_compare kc) : keyCompare(kc) {}
        bool operator()(const value_type& lhs, const value_type& rhs) const noexcept {
            return keyCompare(lhs.first, rhs.first);
        }
    };

    flat_map() = default;
    ~flat_map() = default;

private:
    flat_map(const flat_map&) = delete;
    flat_map& operator=(const flat_map&) = delete;
    key_compare keyCompare;
    value_compare valueCompare{keyCompare};
public:

    using typename container_type::size_type;
    using typename container_type::difference_type;
    using typename container_type::allocator_type;
    using typename container_type::reference;
    using typename container_type::const_reference;
    using typename container_type::pointer;
    using typename container_type::const_pointer;
    using typename container_type::iterator;
    using typename container_type::const_iterator;
    using typename container_type::reverse_iterator;
    using typename container_type::const_reverse_iterator;
    using container_type::begin;
    using container_type::cbegin;
    using container_type::rbegin;
    using container_type::crbegin;
    using container_type::end;
    using container_type::cend;
    using container_type::rend;
    using container_type::crend;
    using container_type::empty;
    using container_type::size;
    using container_type::max_size;

    template<typename...Args>
    std::pair<iterator, bool> emplace(Args&&...args) {
        container_type::emplace_back(std::forward<Args>(args)...);
        auto lower = std::lower_bound(begin(), end(), container_type::back(), value_compare);
		if (lower->first == container_type::back().first && lower != end() - 1) {
			container_type::pop_back();
			return { lower, false };
		}
		if (lower == end() - 1) {
			return { end() - 1, true };
		}
		return { std::rotate(rbegin(), rbegin() + 1, reverse_iterator{ lower }).base(), true }
    }

    iterator find(const key_type& key) {
        auto lower = std::lower_bound(begin(), end(), key,
            [keyCompare](const value_type& val, const key_type& key) { return keyCompare(val.first, key); });
        if (lower != end() && lower->first == key) {
            return lower;
        }
        return end();
    }

    const_iterator find(const key_type& key) const {
        auto lower = std::lower_bound(cbegin(), cend(), key,
            [keyCompare](const value_type& val, const key_type& key) { return keyCompare(val.first, key); });
        if (lower != cend() && lower->first == key) {
            return lower;
        }
        return end();
    }

    iterator erase(const_iterator pos) {
        return container_type::erase(pos);
    }

    size_type erase(const key_type& key) {
        auto iter = find(key);
        if (iter == end()) {
            return 0;
        }
        erase(iter);
        return 1;
    }
    
};

#endif //!FLAT_MAP_CONTAINER_CLASS_HPP
