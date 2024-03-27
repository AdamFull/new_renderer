#pragma once

#include <typeindex>
#include <utils/hash.h>

namespace engine
{
	using resource_handle_t = u64;
	constexpr const resource_handle_t invalid_handle{ ~0ull };

	template<class _Ty>
	class CResourceHolder
	{
	public:
		resource_handle_t add(const char* name, stl::unique_ptr<_Ty>&& object)
		{
			static stl::atomic<resource_handle_t> uid{ 0ull };

			auto name_hash = utl::utils::fnv1a_64_hash(name);
			auto found_association = nameToId.find(name_hash);
			if (found_association != nameToId.end())
				return found_association->second;
			
			if (!freeIds.empty())
			{
				resource_handle_t current_id = freeIds.front();
				freeIds.pop();

				objects.at(current_id) = stl::move(object);

				return current_id;
			}

			resource_handle_t current_id = uid.load(stl::memory_order_relaxed);
			uid.fetch_add(1ull, stl::memory_order_relaxed);

			nameToId.emplace(utl::utils::fnv1a_64_hash(name), current_id);
			objects.emplace_back(stl::move(object));

			return current_id;
		}

		void remove(resource_handle_t id)
		{
			auto it = stl::find_if(nameToId.begin(), nameToId.end(), [id](const auto& pair) { return pair.second == id; });
			if (it != nameToId.end())
			{
				freeIds.push(id);
				nameToId.erase(it);
				objects.at(id) = nullptr;
			}
		}

		void remove(const char* name)
		{
			auto it = nameToId.find(utl::utils::fnv1a_64_hash(name));
			if (it != nameToId.end())
			{
				freeIds.push(it->second);
				nameToId.erase(it);
				objects.at(it->second) = nullptr;
			}
		}

		const stl::unique_ptr<_Ty>& get(resource_handle_t id)
		{
			return objects.at(id);
		}

		const stl::unique_ptr<_Ty>& get(const char* name)
		{
			auto id = nameToId[utl::utils::fnv1a_64_hash(name)];
			return get(id);
		}

		stl::vector<stl::unique_ptr<_Ty>>& data()
		{
			return objects;
		}

		resource_handle_t getId(const char* name)
		{
			auto it = nameToId.find(utl::utils::fnv1a_64_hash(name));
			if (it != nameToId.end())
				return it->second;

			return ~0ull;
		}
	private:
		stl::queue<resource_handle_t> freeIds{};
		stl::unordered_map<u64, resource_handle_t> nameToId{};
		stl::vector<stl::unique_ptr<_Ty>> objects{};
	};

	class CResourceContainer
	{
	public:
		template<class _Ty>
		void registerResource()
		{
			const stl::type_index type_index(typeid(_Ty));
			holders[type_index] = stl::make_shared<CResourceHolder<_Ty>>();
		}

		template<class _Ty>
		resource_handle_t add(const char* name, stl::unique_ptr<_Ty>&& object)
		{
			auto container = get_container<_Ty>();
			return container->add(name, stl::move(object));
		}

		template<class _Ty>
		const stl::unique_ptr<_Ty>& get(resource_handle_t id)
		{
			auto container = get_container<_Ty>();
			return container->get(id);
		}

		template<class _Ty>
		const stl::unique_ptr<_Ty>& get(const char* name)
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
		stl::vector<stl::unique_ptr<_Ty>>& data()
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
			const stl::type_index type_index(typeid(_Ty));
			return utl::cast<CResourceHolder<_Ty>*>(holders[type_index].get());
		}
	private:
		stl::unordered_map<stl::type_index, stl::shared_ptr<void>> holders{};
	};
}