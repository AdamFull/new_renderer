#pragma once

#include <engine/rendering/Common.h>

namespace engine
{
	namespace render
	{
		template<class _Ty>
		class ResourceContainer final
		{
		public:
			Handle Add(std::unique_ptr<_Ty>&& object)
			{
				static std::atomic<uint32_t> uid{ 0u };

				if (!free_ids.empty())
				{
					uint32_t current_id = free_ids.front();
					free_ids.pop();

					objects[current_id] = std::move(object);

					return Handle{ current_id };
				}

				uint32_t current_id = uid.load(std::memory_order_relaxed);
				uid.fetch_add(1ull, std::memory_order_relaxed);

				objects.emplace_back(std::move(object));

				return Handle{ current_id };
			}

			void Remove(Handle id)
			{
				free_ids.push(id);
				objects[id] = nullptr;
			}

			const std::unique_ptr<_Ty>& Get(Handle id) const
			{
				return objects[id.Get()];
			}

			std::unique_ptr<_Ty>& Get(Handle id)
			{
				return objects[id.Get()];
			}

			std::vector<std::unique_ptr<_Ty>>& data()
			{
				return objects;
			}
		private:
			std::queue<Handle> free_ids{};
			std::vector<std::unique_ptr<_Ty>> objects{};
		};
	}
}