#pragma once

#include <vulkan/vulkan.hpp>
#include <hash.hpp>

namespace gfx
{
	namespace vulkan
	{
		template<typename _Ty>
		uint32_t HandleHash(const _Ty& handle, uint32_t seed = 0u)
		{
			auto handle_address = reinterpret_cast<uint64_t>(static_cast<_Ty::CType>(handle));
			return utl::murmur_hash(handle_address, seed);
		}

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

			explicit VulkanResource(_Ty resource, vk::AllocationCallbacks* allocation_callbacks) :
				handle{ resource },
				allocation_callbacks{ allocation_callbacks }
			{
			}

			VulkanResource(const VulkanResource&) = delete;
			VulkanResource& operator=(const VulkanResource&) = delete;

			VulkanResource(VulkanResource&& other) :
				handle{ std::exchange(other.handle, {}) },
				allocation_callbacks{ std::exchange(other.allocation_callbacks, {}) }
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

			uint32_t Hash() const
			{
				return HandleHash(handle);
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
			vk::AllocationCallbacks* allocation_callbacks{ nullptr };
		};
	}
}

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			template<typename _Ty>
			uint32_t HandleHash(const _Ty& handle, uint32_t seed = 0u)
			{
				auto handle_address = reinterpret_cast<uint64_t>(static_cast<_Ty::CType>(handle));
				return utl::murmur_hash(handle_address, seed);
			}

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

				uint32_t Hash() const
				{
					return HandleHash(handle);
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