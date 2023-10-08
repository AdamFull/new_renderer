#pragma once

#include <atomic>
#include <queue>
#include <string>
#include <vector>
#include <unordered_map>

namespace engine
{
	using resource_handle_t = size_t;
	constexpr const resource_handle_t invalid_handle{ ~0ull };

	template<class _Ty>
	class CResourceContainer
	{
	public:
		resource_handle_t add(const std::string& name, _Ty&& object)
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

			nameToId.emplace(name, current_id);
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

		void remove(const std::string& name)
		{
			auto it = nameToId.find(name);
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

		_Ty& get(const std::string& name)
		{
			auto id = nameToId[name];
			return get(id);
		}

		std::vector<_Ty>& data()
		{
			return objects;
		}
	private:
		std::queue<resource_handle_t> freeIds{};
		std::unordered_map<std::string, resource_handle_t> nameToId{};
		std::vector<_Ty> objects{};
	};
}