#pragma once
#ifndef FOUNDATION_PARTITIONED_MAP_HPP
#define FOUNDATION_PARTITIONED_MAP_HPP
#include "safe_ptr.hpp"
#include "contention_free_mutex.hpp"
#include <unordered_map>
#include <vector>

namespace foundation
{

    namespace detail
    {

        template<typename MutexType>
        struct shared_lock_guard
        {
            MutexType& mutexRef;

            shared_lock_guard(MutexType& mutex) : mutexRef(mutex)
            {
                mutex.lock_shared();
            }

            ~shared_lock_guard()
            {
                mutex.unlock_shared();
            }

        };

        template<typename T>
        using default_safe_ptr = safe_ptr<T, contention_free_mutex, std::unique_lock<contention_free_mutex>, shared_lock_guard<contention_free_mutex>>;
    }

    template<typename KeyType, typename ValueType, typename ContainerType = std::unordered_map<KeyType, ValueType>>
    class partitioned_threadsafe_map
    {

        template<typename T>
        using SafePointerType = detail::default_safe_ptr<T>;
		using SafeContainerType = SafePointerType<ContainerType>;
        using DataPageType = typename std::unordered_map<KeyType, SafeContainerType>;

        std::shared_ptr<DataPageType> rootPartition;

    public:
		
		using partition_iterator = typename DataPageType::iterator;
		using const_partition_iterator = typename DataPageType::const_iterator;

        static_assert(std::is_default_constructible_v<KeyType>, "Key type must be default-constructible!");
        static_assert(std::is_default_constructible_v<ContainerType>, "Container type must be default-constructible!");

        partitioned_threadsafe_map() noexcept : rootPartition(std::make_shared<DataPageType>())
        {
            rootPartition->emplace(KeyType(), ContainerType());
        }

        partitioned_threadsafe_map(const KeyType& begin_range, const KeyType& end_range, const KeyType& step_size) noexcept : rootPartition(std::make_shared<DataPageType>())
        {
			size_t reserve_size = static_cast<size_t>((end_range - begin_range) / step_size);
			rootPartition->reserve(reserve_size);

			for (KeyType i = begin_range; i <= end_range; ++i)
			{
				rootPartition->emplace(i, ContainerType());
			}

        }

		partition_iterator find_partition(const KeyType& k) noexcept
		{
			assert(rootPartition->count(k) != 0u);
			return rootPartition->lower_bound(k);
		}

		const_partition_iterator find_partition(const KeyType& k) const noexcept
		{
			assert(rootPartition->count(k) != 0u);
			return rootPartition->lower_bound(k);
		}

		share_locked_safe_ptr<SafeContainerType> find_partition_readonly(const KeyType& k) const noexcept
		{
			auto iter = find_partition(k);
			assert(iter != std::cend(*rootPartition));
			return share_lock_safe_ptr(iter->second);
		}

		template<typename T, typename...Args>
		std::pair<typename SafeContainerType::value_type::iterator, bool> emplace(T&& key, Args&& ...args) noexcept
		{
			auto iter = find_partition(key);
			auto result = iter->second->emplace(std::forward<T>(key), std::forward<Args>(args)...);
			return result;
		}

		typename SafeContainerType::value_type::iterator erase(const SafeContainerType::value_type::const_iterator pos)
		{
			auto partition = find_partition(pos->first);
			return partition->second->erase(pos);
		}

		size_t erase(const KeyType& k)
		{
			auto partition = find_partition(k);
			return partition->second->erase(k);
		}

		size_t size() const noexcept
		{
			size_t result{ 0u };
			for (auto iter = rootPartition->begin(); iter != rootPartition->end(); ++iter)
			{
				result += iter->second->size();
			}
			return result;
		}

		void clear() noexcept
		{
			for (auto iter = rootPartition->begin(); iter != rootPartition->end(); ++iter)
			{
				iter->second->clear();
			}
		}

    };

}

#endif // !FOUNDATION_PARTITIONED_MAP_HPP
