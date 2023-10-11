#pragma once

#include <atomic>
#include <queue>
#include <typeindex>
#include <memory>
#include <vector>
#include <unordered_map>
#include <udetail.hpp>

namespace engine
{
	using resource_handle_t = size_t;
	constexpr const resource_handle_t invalid_handle{ ~0ull };

	template<class _Ty>
	class CResourceHolder
	{
	public:
		resource_handle_t add(const char* name, _Ty&& object)
		{
			static std::atomic<resource_handle_t> uid{ 0ull };
			
			if (!freeIds.empty())
			{
				resource_handle_t current_id = freeIds.front();
				freeIds.pop();

				auto insertion_point = objects.begin() + current_id;
				objects.insert(insertion_point, object);

				return current_id;
			}

			resource_handle_t current_id = uid.load(std::memory_order_relaxed);
			uid.fetch_add(1ull, std::memory_order_relaxed);

			nameToId.emplace(utl::fnv1a_64_hash(name), current_id);
			objects.emplace_back(object);

			return current_id;
		}

		void remove(resource_handle_t id)
		{
			auto it = std::find_if(nameToId.begin(), nameToId.end(), [id](const auto& pair) { return pair.second == id; });
			if (it != nameToId.end())
			{
				freeIds.push(id);
				nameToId.erase(it);
			}
		}

		void remove(const char* name)
		{
			auto it = nameToId.find(utl::fnv1a_64_hash(name));
			if (it != nameToId.end())
			{
				freeIds.push(it->second);
				nameToId.erase(it);
			}
		}

		_Ty& get(resource_handle_t id)
		{
			return objects.at(id);
		}

		_Ty& get(const char* name)
		{
			auto id = nameToId[utl::fnv1a_64_hash(name)];
			return get(id);
		}

		std::vector<_Ty>& data()
		{
			return objects;
		}
	private:
		std::queue<resource_handle_t> freeIds{};
		std::unordered_map<size_t, resource_handle_t> nameToId{};
		std::vector<_Ty> objects{};
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
		resource_handle_t add(const char* name, _Ty&& object)
		{
			auto container = get_container<_Ty>();
			return container->add(name, std::forward<_Ty>(object));
		}

		template<class _Ty>
		_Ty& get(resource_handle_t id)
		{
			auto container = get_container<_Ty>();
			return container->get(id);
		}

		template<class _Ty>
		_Ty& get(const char* name)
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
		std::vector<_Ty>& data()
		{
			auto container = get_container<_Ty>();
			return container->data();
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