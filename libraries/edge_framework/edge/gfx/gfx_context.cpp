#include "gfx_context.h"

#if D3D12_SUPPORTED
#include <directx12/D3D12Context.h>
#endif

#if VULKAN_SUPPORTED
#include <vulkan/VkContext.h>
#endif

#include "../util/Enum.hpp"

namespace edge::gfx
{
	std::unique_ptr<IContext> create_context(const ContextCreateInfo& create_info) {
		switch (create_info.render_api) {
#if D3D12_SUPPORTED
		case RenderApi::eDirectx12:
			return std::make_unique<d3d12::D3D12Context>(create_info);
			break;
#endif
#if VULKAN_SUPPORTED
		case RenderApi::eVulkan:
			return std::make_unique<vulkan::VulkanContext>(create_info);
			break;
#endif
		default:
			break;
		}

		return nullptr;
	}
}