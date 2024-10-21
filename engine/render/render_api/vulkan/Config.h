#pragma once

#include <vulkan/vulkan.hpp>

namespace gfx
{
	namespace vulkan
	{
		const char* required_instance_extensions[]
		{
			""
		};

		const char* required_device_extensions[] =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
			VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
			VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME,
			VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
			VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
			VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
			VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
			VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
			VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
			VK_KHR_SPIRV_1_4_EXTENSION_NAME,
			//VK_EXT_MESH_SHADER_EXTENSION_NAME,
			VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
			//VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
			VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
			//VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME,
			VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
		};

		const char* optional_device_extensions[] =
		{
			VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
		};

		static void EnablePhysicalDeviceFeatures(vk::PhysicalDeviceFeatures& features)
		{
			features.samplerAnisotropy = true;
			features.sampleRateShading = true;
			features.fillModeNonSolid = true;
			features.multiViewport = true;
			features.geometryShader = true;
			features.tessellationShader = true;
			features.fragmentStoresAndAtomics = true;
			features.logicOp = true;
			features.imageCubeArray = true;
			features.depthClamp = true;
			features.independentBlend = true;
			features.multiDrawIndirect = true;
			features.shaderInt16 = true;
			features.shaderStorageImageReadWithoutFormat = true;
			features.shaderStorageImageWriteWithoutFormat = true;
			features.shaderInt64 = true;
		}
	}
}

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			const char* required_instance_extensions[]
			{
				""
			};

			const char* required_device_extensions[] =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
				VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
				VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
				VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
				VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME,
				VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
				VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
				VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
				VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
				VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
				VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
				VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
				VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
				VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
				VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
				VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
				VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
				VK_KHR_SPIRV_1_4_EXTENSION_NAME,
				//VK_EXT_MESH_SHADER_EXTENSION_NAME,
				VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
				//VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
				VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
				//VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME,
				VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
			};

			const char* optional_device_extensions[] =
			{
				VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
			};

			static void EnablePhysicalDeviceFeatures(vk::PhysicalDeviceFeatures& features)
			{
				features.samplerAnisotropy = true;
				features.sampleRateShading = true;
				features.fillModeNonSolid = true;
				features.multiViewport = true;
				features.geometryShader = true;
				features.tessellationShader = true;
				features.fragmentStoresAndAtomics = true;
				features.logicOp = true;
				features.imageCubeArray = true;
				features.depthClamp = true;
				features.independentBlend = true;
				features.multiDrawIndirect = true;
				features.shaderInt16 = true;
				features.shaderStorageImageReadWithoutFormat = true;
				features.shaderStorageImageWriteWithoutFormat = true;
				features.shaderInt64 = true;
			}
		}
	}
}