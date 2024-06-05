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
		resource_handle_t Add(const char* name, std::unique_ptr<_Ty>&& object)
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

		void Remove(resource_handle_t id)
		{
			auto it = std::find_if(nameToId.begin(), nameToId.end(), [id](const auto& pair) { return pair.second == id; });
			if (it != nameToId.end())
			{
				freeIds.push(id);
				nameToId.erase(it);
				objects.at(id) = nullptr;
			}
		}

		void Remove(const char* name)
		{
			auto it = nameToId.find(utl::fnv1a_64_hash_cstr(name));
			if (it != nameToId.end())
			{
				freeIds.push(it->second);
				nameToId.erase(it);
				objects.at(it->second) = nullptr;
			}
		}

		const std::unique_ptr<_Ty>& Get(resource_handle_t id)
		{
			return objects.at(id);
		}

		const std::unique_ptr<_Ty>& Get(const char* name)
		{
			auto id = nameToId[utl::fnv1a_64_hash(name)];
			return Get(id);
		}

		utl::vector<std::unique_ptr<_Ty>>& Data()
		{
			return objects;
		}

		resource_handle_t GetId(const char* name)
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

	template<class... _Args>
	class CResourceContainer
	{
	public:
		template<class _Ty>
		resource_handle_t Add(const char* name, std::unique_ptr<_Ty>&& object)
		{ return std::get<CResourceHolder<_Ty>>(holders).Add(name, std::move(object)); }

		template<class _Ty>
		const std::unique_ptr<_Ty>& Get(resource_handle_t id)
		{ return std::get<CResourceHolder<_Ty>>(holders).Get(id); }

		template<class _Ty>
		const std::unique_ptr<_Ty>& Get(const char* name)
		{ return std::get<CResourceHolder<_Ty>>(holders).Get(name); }

		template<class _Ty>
		void Remove(resource_handle_t id)
		{ std::get<CResourceHolder<_Ty>>(holders).Remove(id); }

		template<class _Ty>
		void Remove(const char* name)
		{ std::get<CResourceHolder<_Ty>>(holders).Remove(name); }

		template<class _Ty>
		utl::vector<std::unique_ptr<_Ty>>& Data()
		{ return std::get<CResourceHolder<_Ty>>(holders).Data(); }

		template<class _Ty>
		resource_handle_t GetId(const char* name)
		{ return std::get<CResourceHolder<_Ty>>(holders).GetId(name); }
	private:
		std::tuple<CResourceHolder<_Args>...> holders;
	};
}