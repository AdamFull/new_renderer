#pragma once

#include <vulkan/vulkan.hpp>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			// Cast vulkan resource for get it's handle
			template<typename _Ty>
			auto& Handle(_Ty& object)
			{
				return object.GetHandle();
			}

			template<typename _Ty>
			class VulkanResource
			{
			public:
				using HandleType = _Ty;

				explicit VulkanResource(_Ty resource) :
					handle{ resource }
				{
				}

				VulkanResource(const VulkanResource&) = delete;
				VulkanResource& operator=(const VulkanResource&) = delete;

				VulkanResource(VulkanResource&& other) :
					handle{ std::exchange(other.handle, {}) }
				{
				}

				VulkanResource& operator=(VulkanResource&& other)
				{
					handle = std::exchange(other.handle, {});
					return *this;
				}

				virtual ~VulkanResource() = default;

				_Ty& GetHandle()
				{
					return handle;
				}

				const _Ty& GetHandle() const
				{
					return handle;
				}

				explicit operator bool() const noexcept
				{
					return handle;
				}

				bool operator!() const noexcept
				{
					return !handle;
				}
			protected:
				_Ty handle;
			};
		}
	}
}