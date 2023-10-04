#pragma once

#include <vma/vk_mem_alloc.hpp>
#include <vector>
#include <string>

namespace engine
{
	namespace graphics
	{
		namespace vulkan
		{
			enum class ERenderApiVersion
			{
				eVK10,
				eVK11,
				eVK12,
				eVK13
			};

			enum EnableValidationFeatureFlagBits
			{
				GPU_ASSISTED_FLAG_BIT = 1 << 0,
				GPU_ASSISTED_RESERVE_BINDING_SLOT_FLAG_BIT = 1 << 1,
				BEST_PRACTICE_FLAG_BIT = 1 << 2,
				DEBUG_PRINTF_FLAG_BIT = 1 << 3,
				SYNCHRONIZATION_VALIDATION_FLAG_BIT = 1 << 4
			};

			struct FRenderContextCreateInfo
			{
				std::string app_name{ "dummy_app" };
				ERenderApiVersion api_version{};

#if defined( VK_USE_PLATFORM_WIN32_KHR )
				HINSTANCE                       hinstance;
				HWND                            hwnd;
#endif
				
				bool validation_state{ false };
				uint32_t validation_features{ 0u };
				std::vector<const char*> validation_layers{};
			};

			struct FPhysicalDevice
			{
				vk::PhysicalDevice device{ nullptr };
				vk::PhysicalDeviceProperties device_properties{};
			};

			class CVkRenderContext
			{
			public:
				void initialize(const FRenderContextCreateInfo& context_ci);
			protected:
				vk::Result register_allocation_callbacks();
				vk::Result create_instance(const FRenderContextCreateInfo& context_ci);
				vk::Result create_surface(const FRenderContextCreateInfo& context_ci);
			private:
				vk::AllocationCallbacks* allocator_callbacks{ nullptr };

				vk::Instance instance{ nullptr };
				vk::SurfaceKHR surface{ nullptr };
				std::vector<vk::LayerProperties> instance_layer_props{};

				bool enable_validation{ false };
				uint32_t validation_features{ 0u };
			};
		}
	}
}