#pragma once

#include <typeindex>
#include <container/hash_map.h>

namespace engine
{
	using resource_handle_t = uint64_t;
	constexpr const resource_handle_t invalid_handle{ ~0ull };

	template<class _Ty>
	class CResourceHolder
	{
	public:
		resource_handle_t add(const char* name, std::unique_ptr<_Ty>&& object)
		{
			static std::atomic<resource_handle_t> uid{ 0ull };

			auto name_hash = utl::murmur_hash_64(name);
			auto found_association = nameToId.find(name_hash);
			if (found_association != nameToId.end())
				return found_association->second;

			if (!freeIds.empty())
			{
				resource_handle_t current_id = freeIds.front();
				freeIds.pop();

				objects[current_id] = std::move(object);

				return current_id;
			}

			resource_handle_t current_id = uid.load(std::memory_order_relaxed);
			uid.fetch_add(1ull, std::memory_order_relaxed);

			nameToId.emplace(utl::murmur_hash_64(name), current_id);
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
				objects[id] = nullptr;
			}
		}

		void remove(const char* name)
		{
			auto it = nameToId.find(utl::murmur_hash_64(name));
			if (it != nameToId.end())
			{
				freeIds.push(it->second);
				nameToId.erase(it);
				objects[it->second] = nullptr;
			}
		}

		const std::unique_ptr<_Ty>& get(resource_handle_t id) const
		{
			return objects[id];
		}

		std::unique_ptr<_Ty>& get(resource_handle_t id)
		{
			return objects[id];
		}

		const std::unique_ptr<_Ty>& get(const char* name) const
		{
			auto id = nameToId[utl::murmur_hash_64(name)];
			return get(id);
		}

		std::unique_ptr<_Ty>& get(const char* name)
		{
			auto id = nameToId[utl::murmur_hash_64(name)];
			return get(id);
		}

		std::vector<std::unique_ptr<_Ty>>& data()
		{
			return objects;
		}

		resource_handle_t getId(const char* name)
		{
			auto it = nameToId.find(utl::murmur_hash_64(name));
			if (it != nameToId.end())
				return it->second;

			return invalid_handle;
		}
	private:
		std::queue<resource_handle_t> freeIds{};
		std::unordered_map<uint64_t, resource_handle_t> nameToId{};
		std::vector<std::unique_ptr<_Ty>> objects{};
	};

	template<class... _Args>
	class CResourceContainer
	{
	public:
		template<class _Ty>
		resource_handle_t add(const char* name, std::unique_ptr<_Ty>&& object)
		{
			return std::get<CResourceHolder<_Ty>>(holders).add(name, std::move(object));
		}

		template<class _Ty>
		const std::unique_ptr<_Ty>& get(resource_handle_t id) const
		{
			return std::get<CResourceHolder<_Ty>>(holders).get(id);
		}

		template<class _Ty>
		std::unique_ptr<_Ty>& get(resource_handle_t id)
		{
			return std::get<CResourceHolder<_Ty>>(holders).get(id);
		}

		template<class _Ty>
		const std::unique_ptr<_Ty>& get(const char* name) const
		{
			return std::get<CResourceHolder<_Ty>>(holders).get(name);
		}

		template<class _Ty>
		std::unique_ptr<_Ty>& get(const char* name)
		{
			return std::get<CResourceHolder<_Ty>>(holders).get(name);
		}

		template<class _Ty>
		void remove(resource_handle_t id)
		{
			std::get<CResourceHolder<_Ty>>(holders).remove(id);
		}

		template<class _Ty>
		void remove(const char* name)
		{
			std::get<CResourceHolder<_Ty>>(holders).remove(name);
		}

		template<class _Ty>
		std::vector<std::unique_ptr<_Ty>>& data()
		{
			return std::get<CResourceHolder<_Ty>>(holders).data();
		}

		template<class _Ty>
		resource_handle_t getId(const char* name)
		{
			return std::get<CResourceHolder<_Ty>>(holders).getId(name);
		}
	private:
		std::tuple<CResourceHolder<_Args>...> holders;
	};
}