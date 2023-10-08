#pragma once

#include <interface/ResourceContainer.h>
#include <typeindex>
#include <memory>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class CVkResourceContext
			{
			public:
				template<class _Ty>
				void registerResource()
				{
					const std::type_index type_index(typeid(_Ty));
					containers[type_index] = std::make_shared<CResourceContainer<_Ty>>();
				}

				template<class _Ty>
				resource_handle_t add(const std::string& name, _Ty&& object)
				{
					auto container = get_container<_Ty>();
					return container->add(name, std::forward<_Ty>(object));
				}

				template<class _Ty>
				void remove(resource_handle_t id)
				{
					auto container = get_container<_Ty>();
					return container->remove(id);
				}

				template<class _Ty>
				void remove(const std::string& name)
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
				CResourceContainer<_Ty>* get_container()
				{
					const std::type_index type_index(typeid(_Ty));
					return static_cast<CResourceContainer<_Ty>*>(containers[type_index].get());
				}
			private:
				std::unordered_map<std::type_index, std::shared_ptr<void>> containers{};
			};
		}
	}
}