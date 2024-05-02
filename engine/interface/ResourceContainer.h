#pragma once

#include <typeindex>
#include <utils/hash.h>
#include <container/hash_map.h>

namespace engine
{
	using resource_handle_t = u64;
	constexpr const resource_handle_t invalid_handle{ ~0ull };

	template<class _Ty>
	class CResourceHolder
	{
	public:
		resource_handle_t add(const char* name, std::unique_ptr<_Ty>&& object)
		{
			static std::atomic<resource_handle_t> uid{ 0ull };

			auto name_hash = utl::fnv1a_64_hash_cstr(name);
			auto found_association = nameToId.find(name_hash);
			if (found_association != nameToId.end())
				return found_association->second;
			
			if (!freeIds.empty())
			{
				resource_handle_t current_id = freeIds.front();
				freeIds.pop();

				objects.at(current_id) = std::move(object);

				return current_id;
			}

			resource_handle_t current_id = uid.load(std::memory_order_relaxed);
			uid.fetch_add(1ull, std::memory_order_relaxed);

			nameToId.emplace(utl::fnv1a_64_hash_cstr(name), current_id);
			objects.emplace_back(std::move(object));

			return current_id;
		}

		void remove(resource_handle_t id)
		{
			auto it = std::find_if(nameToId.begin(), nameToId.end(), [id](const auto& pair) { return pair.second == id; });
			if (it != nameToId.end())
			{
				freeIds.push(id);
				nameToId.erase(it);
				objects.at(id) = nullptr;
			}
		}

		void remove(const char* name)
		{
			auto it = nameToId.find(utl::fnv1a_64_hash_cstr(name));
			if (it != nameToId.end())
			{
				freeIds.push(it->second);
				nameToId.erase(it);
				objects.at(it->second) = nullptr;
			}
		}

		const std::unique_ptr<_Ty>& get(resource_handle_t id)
		{
			return objects.at(id);
		}

		const std::unique_ptr<_Ty>& get(const char* name)
		{
			auto id = nameToId[utl::fnv1a_64_hash(name)];
			return get(id);
		}

		utl::vector<std::unique_ptr<_Ty>>& data()
		{
			return objects;
		}

		resource_handle_t getId(const char* name)
		{
			auto it = nameToId.find(utl::fnv1a_64_hash_cstr(name));
			if (it != nameToId.end())
				return it->second;

			return invalid_handle;
		}
	private:
		std::queue<resource_handle_t> freeIds{};
		utl::hash_map<u64, resource_handle_t> nameToId{};
		utl::vector<std::unique_ptr<_Ty>> objects{};
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
		utl::vector<std::unique_ptr<_Ty>>& data()
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
			return utl::cast<CResourceHolder<_Ty>*>(holders[type_index].get());
		}
	private:
		utl::hash_map<std::type_index, std::shared_ptr<void>> holders{};
	};
}