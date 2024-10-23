#pragma once

#include <ResourceStructures.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			inline vk::DeviceSize CalculateAlignedSize(vk::DeviceSize allocation_size, vk::DeviceSize minimal_alignment)
			{
				if (minimal_alignment > 0ull)
					return (allocation_size + minimal_alignment - 1ull) & ~(minimal_alignment - 1ull);
				return allocation_size;
			}
		}
	}
}

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			inline bool is_depth_only_format(vk::Format format)
			{
				return format == vk::Format::eD16Unorm ||
					format == vk::Format::eD32Sfloat;
			}

			inline bool is_depth_stencil_format(vk::Format format)
			{
				return format == vk::Format::eD16UnormS8Uint ||
					format == vk::Format::eD24UnormS8Uint ||
					format == vk::Format::eD32SfloatS8Uint;
			}

			inline bool is_depth_format(vk::Format format)
			{
				return is_depth_only_format(format) || is_depth_stencil_format(format);
			}

			inline bool is_stencil_format(vk::Format format)
			{
				return !is_depth_only_format(format) || !is_depth_stencil_format(format);
			}

			// Helper functions
			inline bool check_contains_single(const std::string& s1, const utl::vector<std::string>& rhs)
			{
				return std::find_if(rhs.begin(), rhs.end(),
					[s1](const std::string& s2) { return s1 == s2; }) != rhs.end();
			}

			// Check that second vector contains all items from first one
			inline bool check_contains_all(const utl::vector<std::string>& lhs, const utl::vector<std::string>& rhs)
			{
				return std::all_of(lhs.begin(), lhs.end(),
					[&rhs](const std::string& s1) { return check_contains_single(s1, rhs); });
			}

			inline utl::vector<const char*> get_instance_extension_names()
			{
#if defined( VK_USE_PLATFORM_WIN32_KHR )
				return utl::vector<const char*>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
#else
				return utl::vector<const char*>{};
#endif
			}

			inline vk::PrimitiveTopology get_primitive_topology(EVertexPrimitiveTopology mask)
			{
				switch (mask)
				{
				case EVertexPrimitiveTopology::ePointList: return vk::PrimitiveTopology::ePointList; break;
				case EVertexPrimitiveTopology::eLineList: return vk::PrimitiveTopology::eLineList; break;
				case EVertexPrimitiveTopology::eLineStrip: return vk::PrimitiveTopology::eLineStrip; break;
				case EVertexPrimitiveTopology::eTriangleList: return vk::PrimitiveTopology::eTriangleList; break;
				case EVertexPrimitiveTopology::eTriangleStrip: return vk::PrimitiveTopology::eTriangleStrip; break;
				case EVertexPrimitiveTopology::eTriangleFan: return vk::PrimitiveTopology::eTriangleFan; break;
				}

				return vk::PrimitiveTopology::ePointList;
			}

			inline vk::ShaderStageFlagBits get_shader_stage(u32 mask)
			{
				switch (mask)
				{
				case 1: return vk::ShaderStageFlagBits::eVertex; break;
				case 2: return vk::ShaderStageFlagBits::eTessellationControl; break;
				case 3: return vk::ShaderStageFlagBits::eTessellationEvaluation; break;
				case 4: return vk::ShaderStageFlagBits::eGeometry; break;
				case 5: return vk::ShaderStageFlagBits::eFragment; break;
				case 6: return vk::ShaderStageFlagBits::eCompute; break;
				case 7: return vk::ShaderStageFlagBits::eRaygenKHR; break;
				case 8: return vk::ShaderStageFlagBits::eIntersectionKHR; break;
				case 9: return vk::ShaderStageFlagBits::eAnyHitKHR; break;
				case 10: return vk::ShaderStageFlagBits::eClosestHitKHR; break;
				case 11: return vk::ShaderStageFlagBits::eMissKHR; break;
				case 12: return vk::ShaderStageFlagBits::eCallableKHR; break;
				case 13: return vk::ShaderStageFlagBits::eTaskEXT; break;
				case 14: return vk::ShaderStageFlagBits::eMeshEXT; break;
				}

				return vk::ShaderStageFlagBits::eAll;
			}

			inline vk::CullModeFlags get_cull_mode_flags(EShaderCullModeFlags mask)
			{
				vk::CullModeFlags flags{};

				if (mask & EShaderCullModeFlagBits::eNone)
					flags |= vk::CullModeFlagBits::eNone;
				if (mask & EShaderCullModeFlagBits::eFront)
					flags |= vk::CullModeFlagBits::eFront;
				if (mask & EShaderCullModeFlagBits::eBack)
					flags |= vk::CullModeFlagBits::eBack;

				return flags;
			}

			inline vk::FrontFace get_front_face(EShaderFrontFace mask)
			{
				if (mask == EShaderFrontFace::eCCW)
					return vk::FrontFace::eCounterClockwise;

				return vk::FrontFace::eClockwise;
			}

#if 0
			inline utl::vector<vk::SampleCountFlagBits> get_avaliable_sample_count(const FVkPhysicalDevice& device)
			{
				utl::vector<vk::SampleCountFlagBits> avaliable_samples{ vk::SampleCountFlagBits::e1 };

				auto sample_count_flags = device.properties.limits.framebufferColorSampleCounts & device.properties.limits.framebufferDepthSampleCounts;
				if (sample_count_flags & vk::SampleCountFlagBits::e64)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e64);
				if (sample_count_flags & vk::SampleCountFlagBits::e32)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e32);
				if (sample_count_flags & vk::SampleCountFlagBits::e16)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e16);
				if (sample_count_flags & vk::SampleCountFlagBits::e8)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e8);
				if (sample_count_flags & vk::SampleCountFlagBits::e4)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e4);
				if (sample_count_flags & vk::SampleCountFlagBits::e2)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e2);

				return avaliable_samples;
			}

			inline bool is_sample_count_supported(const FVkPhysicalDevice& device, vk::SampleCountFlagBits sample_count)
			{
				auto avaliable_samples = get_avaliable_sample_count(device);
				auto found = std::find(avaliable_samples.begin(), avaliable_samples.end(), sample_count);
				return found != avaliable_samples.end();
			}
#endif
			inline EQueueTypeFlags convert_vk_queue_flags_to_impl(vk::QueueFlags flags)
			{
				EQueueTypeFlags mask{ 0u };
				if (flags & vk::QueueFlagBits::eGraphics)
					mask |= EQueueTypeFlagBits::eGraphics;
				if (flags & vk::QueueFlagBits::eCompute)
					mask |= EQueueTypeFlagBits::eCompute;
				if (flags & vk::QueueFlagBits::eTransfer)
					mask |= EQueueTypeFlagBits::eTransfer;

				return mask;
			}

			inline vk::DeviceSize get_alligned_instance_size(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment)
			{
				if (minOffsetAlignment > 0ull)
					return (instanceSize + minOffsetAlignment - 1ull) & ~(minOffsetAlignment - 1ull);
				return instanceSize;
			}

			inline vk::PipelineStageFlags2KHR get_translation_stage_flags(vk::ImageLayout layout, EPassType pass_type = EPassType::eNone)
			{
				auto get_stages_by_pass_type = [](EPassType type) -> vk::PipelineStageFlags2KHR
					{
						//if (type == EPassType::eCompute)
						//	return vk::PipelineStageFlagBits2KHR::eComputeShader;
						//else if (type == EPassType::eRender)
						//	return vk::PipelineStageFlagBits2KHR::eAllGraphics;
						//else
							return vk::PipelineStageFlagBits2KHR::eAllGraphics | vk::PipelineStageFlagBits2KHR::eComputeShader;
					};

				switch (layout)
				{
				case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits2KHR::eAllCommands;
				case vk::ImageLayout::ePreinitialized: return vk::PipelineStageFlagBits2KHR::eHost;
				case vk::ImageLayout::eTransferDstOptimal:
				case vk::ImageLayout::eTransferSrcOptimal: return vk::PipelineStageFlagBits2KHR::eTransfer;
				case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits2KHR::eColorAttachmentOutput;
				case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				case vk::ImageLayout::eDepthAttachmentOptimal: return vk::PipelineStageFlagBits2KHR::eEarlyFragmentTests | vk::PipelineStageFlagBits2KHR::eLateFragmentTests;
				case vk::ImageLayout::eDepthReadOnlyOptimal:
				case vk::ImageLayout::eDepthStencilReadOnlyOptimal: return get_stages_by_pass_type(pass_type);
				case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR: return vk::PipelineStageFlagBits2KHR::eFragmentShadingRateAttachmentKHR;
				case vk::ImageLayout::eShaderReadOnlyOptimal: return get_stages_by_pass_type(pass_type);
				case vk::ImageLayout::ePresentSrcKHR: return vk::PipelineStageFlagBits2KHR::eAllCommands;
				case vk::ImageLayout::eGeneral: return get_stages_by_pass_type(pass_type);
				default: return vk::PipelineStageFlagBits2KHR::eNone;
				}
			}

			inline vk::AccessFlags2KHR get_translation_access_flags(vk::ImageLayout layout, EPassType pass_type = EPassType::eNone, ERenderResourceAccessFlagBits access_type = ERenderResourceAccessFlagBits::eNone)
			{
				switch (layout)
				{
				case vk::ImageLayout::eUndefined:
				case vk::ImageLayout::ePresentSrcKHR: return vk::AccessFlagBits2KHR::eNone;
				case vk::ImageLayout::ePreinitialized: return vk::AccessFlagBits2KHR::eHostWrite;
				case vk::ImageLayout::eColorAttachmentOptimal: return vk::AccessFlagBits2KHR::eColorAttachmentRead | vk::AccessFlagBits2KHR::eColorAttachmentWrite;
				case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				case vk::ImageLayout::eDepthAttachmentOptimal: return vk::AccessFlagBits2KHR::eDepthStencilAttachmentRead | vk::AccessFlagBits2KHR::eDepthStencilAttachmentWrite;
				case vk::ImageLayout::eDepthReadOnlyOptimal:
				case vk::ImageLayout::eDepthStencilReadOnlyOptimal: {
					if (pass_type == EPassType::eCompute)
						return vk::AccessFlagBits2KHR::eShaderRead; 

					return vk::AccessFlagBits2KHR::eShaderRead | vk::AccessFlagBits2KHR::eDepthStencilAttachmentRead;
				} 
				case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR: return vk::AccessFlagBits2KHR::eFragmentShadingRateAttachmentReadKHR;
				case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits2KHR::eShaderRead | vk::AccessFlagBits2KHR::eInputAttachmentRead;
				case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits2KHR::eTransferRead;
				case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits2KHR::eTransferWrite;
				case vk::ImageLayout::eGeneral: {
					constexpr const vk::AccessFlags2KHR read_flags = vk::AccessFlagBits2KHR::eShaderRead | vk::AccessFlagBits2KHR::eShaderStorageRead | vk::AccessFlagBits2KHR::eShaderSampledRead;
					constexpr const vk::AccessFlags2KHR write_flags = vk::AccessFlagBits2KHR::eShaderWrite | vk::AccessFlagBits2KHR::eShaderStorageWrite;

					if (access_type == ERenderResourceAccessFlagBits::eRead)
						return read_flags;
					else if (access_type == ERenderResourceAccessFlagBits::eWrite)
						return write_flags;

					return read_flags | write_flags;

				} break; 
				default: return vk::AccessFlagBits2KHR::eNone;
				}
			}

			inline vk::PipelineStageFlags2KHR get_translation_stage_flags(EPassType pass_type)
			{
				switch (pass_type)
				{
				case engine::render::EPassType::eNone: return vk::PipelineStageFlagBits2KHR::eAllCommands; break;
				case engine::render::EPassType::eCompute: return vk::PipelineStageFlagBits2KHR::eComputeShader; break;
				case engine::render::EPassType::eRender: return vk::PipelineStageFlagBits2KHR::eAllGraphics; break;
				}

				return vk::PipelineStageFlagBits2KHR::eComputeShader;
			}

			inline vk::AccessFlags2KHR get_translation_access_flags(EPassType pass_type, ERenderResourceAccessFlagBits access_flag)
			{
				constexpr const vk::AccessFlags2KHR read_flags = vk::AccessFlagBits2KHR::eShaderRead | vk::AccessFlagBits2KHR::eShaderStorageRead | vk::AccessFlagBits2KHR::eShaderSampledRead;
				constexpr const vk::AccessFlags2KHR write_flags = vk::AccessFlagBits2KHR::eShaderWrite | vk::AccessFlagBits2KHR::eShaderStorageWrite;

				switch (access_flag)
				{
				case engine::render::ERenderResourceAccessFlagBits::eNone: return vk::AccessFlagBits2KHR::eNone; break;
				case engine::render::ERenderResourceAccessFlagBits::eRead: return read_flags | (pass_type != EPassType::eCompute ? vk::AccessFlagBits2KHR::eIndirectCommandRead : vk::AccessFlagBits2KHR::eNone); break;
				case engine::render::ERenderResourceAccessFlagBits::eWrite: return write_flags; break;
				}

				return vk::AccessFlagBits2KHR::eNone;
			}

			inline vk::AttachmentLoadOp get_attachment_load_op(const ERenderResourceLoadOp load_op)
			{
				switch (load_op)
				{
				case ERenderResourceLoadOp::eDontCare: return vk::AttachmentLoadOp::eDontCare;
				case ERenderResourceLoadOp::eClear: return vk::AttachmentLoadOp::eClear;
				case ERenderResourceLoadOp::eLoad: return vk::AttachmentLoadOp::eLoad;
				}

				return vk::AttachmentLoadOp::eNoneEXT;
			}

			inline vk::AttachmentStoreOp get_attachment_store_op(const ERenderResourceStoreOp store_op)
			{
				switch (store_op)
				{
				case ERenderResourceStoreOp::eDontCare: return vk::AttachmentStoreOp::eDontCare;
				case ERenderResourceStoreOp::eStore: return vk::AttachmentStoreOp::eStore;
				}

				return vk::AttachmentStoreOp::eNone;
			}

			inline vk::CompareOp get_compare_op(EDepthCompareOp op)
			{
				switch (op)
				{
				case EDepthCompareOp::eNever: return vk::CompareOp::eNever; break;
				case EDepthCompareOp::eLess: return vk::CompareOp::eLess; break;
				case EDepthCompareOp::eEqual: return vk::CompareOp::eEqual; break;
				case EDepthCompareOp::eLessOrEqual: return vk::CompareOp::eLessOrEqual; break;
				case EDepthCompareOp::eGreater: return vk::CompareOp::eGreater; break;
				case EDepthCompareOp::eNotEqual: return vk::CompareOp::eNotEqual; break;
				case EDepthCompareOp::eGreaterOrEqual: return vk::CompareOp::eGreaterOrEqual; break;
				case EDepthCompareOp::eAlways: return vk::CompareOp::eAlways; break;
				}

				return vk::CompareOp::eNever;
			}

			inline vk::DeviceSize CalculateAlignedSize(vk::DeviceSize allocation_size, vk::DeviceSize minimal_alignment)
			{
				if (minimal_alignment > 0ull)
					return (allocation_size + minimal_alignment - 1ull) & ~(minimal_alignment - 1ull);
				return allocation_size;
			}
		}
	}
}