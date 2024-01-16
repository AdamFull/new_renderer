#pragma once

#include <atomic>
#include <queue>
#include <typeindex>
#include <memory>
#include <vector>
#include <unordered_map>
#include <utility.hpp>

namespace engine
{
	using resource_handle_t = size_t;
	constexpr const resource_handle_t invalid_handle{ ~0ull };

	template<class _Ty>
	class CResourceHolder
	{
	public:
		resource_handle_t add(const char* name, std::unique_ptr<_Ty>&& object)
		{
			static std::atomic<resource_handle_t> uid{ 0ull };

			auto name_hash = utl::fnv1a_64_hash(name);
			auto found_association = nameToId.find(name_hash);
			if (found_association != nameToId.end())
			{
				auto& current_id = found_association->second;
				objects.at(current_id).emplace_back(std::move(object));
				return current_id;
			}
			
			if (!freeIds.empty())
			{
				resource_handle_t current_id = freeIds.front();
				freeIds.pop();

				objects.at(current_id).emplace_back(std::move(object));

				return current_id;
			}

			resource_handle_t current_id = uid.load(std::memory_order_relaxed);
			uid.fetch_add(1ull, std::memory_order_relaxed);

			nameToId.emplace(utl::fnv1a_64_hash(name), current_id);
			auto& instances = objects.emplace_back();
			instances.emplace_back(std::move(object));

			return current_id;
		}

		void remove(resource_handle_t id)
		{
			auto it = std::find_if(nameToId.begin(), nameToId.end(), [id](const auto& pair) { return pair.second == id; });
			if (it != nameToId.end())
			{
				freeIds.push(id);
				nameToId.erase(it);
				objects.at(id).clear();
			}
		}

		void remove(const char* name)
		{
			auto it = nameToId.find(utl::fnv1a_64_hash(name));
			if (it != nameToId.end())
			{
				freeIds.push(it->second);
				nameToId.erase(it);
				objects.at(it->second).clear();
			}
		}

		const std::unique_ptr<_Ty>& get(resource_handle_t id, size_t index = 0ull)
		{
			return objects.at(id).at(index);
		}

		const std::unique_ptr<_Ty>& get(const char* name, size_t index = 0ull)
		{
			auto id = nameToId[utl::fnv1a_64_hash(name)];
			return get(id, index);
		}

		std::vector<std::vector<std::unique_ptr<_Ty>>>& data()
		{
			return objects;
		}

		resource_handle_t getId(const char* name)
		{
			auto it = nameToId.find(utl::fnv1a_64_hash(name));
			if (it != nameToId.end())
				return it->second;

			return ~0ull;
		}
	private:
		std::queue<resource_handle_t> freeIds{};
		std::unordered_map<size_t, resource_handle_t> nameToId{};
		std::vector<std::vector<std::unique_ptr<_Ty>>> objects{};
	};

	class CResourceContainer
	{
	public:
		template<class _Ty>
		void registerResource()
		{
			const std::type_index type_index(typeid(_Ty));
			holders[type_index] = std::make_shared<CResourceHolder<_Ty>>();
		}

		template<class _Ty>
		resource_handle_t add(const char* name, std::unique_ptr<_Ty>&& object)
		{
			auto container = get_container<_Ty>();
			return container->add(name, std::move(object));
		}

		template<class _Ty>
		const std::unique_ptr<_Ty>& get(resource_handle_t id)
		{
			auto container = get_container<_Ty>();
			return container->get(id);
		}

		template<class _Ty>
		const std::unique_ptr<_Ty>& get(const char* name)
		{
			auto container = get_container<_Ty>();
			return container->get(name);
		}

		template<class _Ty>
		void remove(resource_handle_t id)
		{
			auto container = get_container<_Ty>();
			return container->remove(id);
		}

		template<class _Ty>
		void remove(const char* name)
		{
			auto container = get_container<_Ty>();
			return container->remove(name);
		}

		template<class _Ty>
		std::vector<std::vector<std::unique_ptr<_Ty>>>& data()
		{
			auto container = get_container<_Ty>();
			return container->data();
		}

		template<class _Ty>
		resource_handle_t getId(const char* name)
		{
			auto container = get_container<_Ty>();
			return container->getId(name);
		}
	protected:
		template<class _Ty>
		CResourceHolder<_Ty>* get_container()
		{
			const std::type_index type_index(typeid(_Ty));
			return static_cast<CResourceHolder<_Ty>*>(holders[type_index].get());
		}
	private:
		std::unordered_map<std::type_index, std::shared_ptr<void>> holders{};
	};
}