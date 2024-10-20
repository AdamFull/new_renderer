#include <base/CommandBuffer.h>
#include <base/CommandPool.h>
#include <base/DebugUtils.h>
#include <RenderDevice.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			CommandBuffer::CommandBuffer(CommandPool& command_pool, vk::CommandBufferLevel level) :
				VulkanResource{ nullptr },
				command_pool{ command_pool }
			{
				handle = command_pool.AllocateCommandBuffer(level);

				auto reset_mode = command_pool.GetResetMode();
				Reset(reset_mode);
			}

			CommandBuffer::CommandBuffer(CommandBuffer&& other) :
				VulkanResource{ std::move(other) },
				command_pool{ other.command_pool }
			{

			}

			CommandBuffer::~CommandBuffer()
			{
				command_pool.FreeCommandBuffer(handle);
			}

			vk::Result CommandBuffer::Begin(vk::CommandBufferUsageFlags flags, CommandBuffer* primary)
			{
				if (primary)
				{
					return vk::Result::eErrorDeviceLost;
				}

				return Begin(flags, nullptr, nullptr, 0u);
			}

			vk::Result CommandBuffer::Begin(vk::CommandBufferUsageFlags flags, vk::RenderPass* render_pass, vk::Framebuffer* framebuffer, uint32_t subpass)
			{
				vk::CommandBufferBeginInfo begin_info{ flags };
				vk::CommandBufferInheritanceInfo inheritance{};

				if (render_pass)
				{
					inheritance.renderPass = *render_pass;
					inheritance.framebuffer = *framebuffer;
					inheritance.subpass = subpass;

					begin_info.pInheritanceInfo = &inheritance;
				}

				return handle.begin(&begin_info);
			}

			void CommandBuffer::End()
			{
				handle.end();
			}

			void CommandBuffer::Reset(ResetMode reset_mode)
			{
				if (reset_mode == ResetMode::ResetIndividually)
				{
					handle.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
				}
			}

			void CommandBuffer::BindPipeline(vk::PipelineBindPoint bind_point, vk::Pipeline& pipeline)
			{
				handle.bindPipeline(bind_point, pipeline);
			}

			void CommandBuffer::PushConstants(const vk::PushConstantsInfoKHR& push_info)
			{
				handle.pushConstants(push_info.layout, push_info.stageFlags, push_info.offset, push_info.size, push_info.pValues);
			}

			void CommandBuffer::BindVertexBuffers(uint32_t first_binding, const std::vector<vk::Buffer>& buffers, const std::vector<uint64_t>& offsets)
			{
				handle.bindVertexBuffers(first_binding, buffers, offsets);
			}

			void CommandBuffer::PipelineBarrier(const vk::DependencyInfoKHR& dependency_info)
			{
				handle.pipelineBarrier2KHR(dependency_info);
			}

			void CommandBuffer::ResetTimestampQuery(FVkTimestampQuery& timestamp_query)
			{
				handle.resetQueryPool(timestamp_query.handle, 0u, static_cast<u32>(timestamp_query.values.size()));
				timestamp_query.queryIndex = 0u;
			}

			void CommandBuffer::WriteTimestampQuery(vk::PipelineStageFlagBits pipeline_state, FVkTimestampQuery& timestamp_query)
			{
				handle.writeTimestamp(pipeline_state, timestamp_query.handle, timestamp_query.queryIndex++);
			}

			void CommandBuffer::BeginDebugLabel(const std::string& label, u32 color)
			{
				auto& render_device = command_pool.GetDevice();
				auto& debug_utils = render_device.GetDebugUtils();
				debug_utils.BeginLabel(handle, label.c_str(), color);
			}

			void CommandBuffer::InsertDebugLabel(const std::string& label, u32 color)
			{
				auto& render_device = command_pool.GetDevice();
				auto& debug_utils = render_device.GetDebugUtils();
				debug_utils.InsertLabel(handle, label.c_str(), color);
			}

			void CommandBuffer::EndDebugLabel()
			{
				auto& render_device = command_pool.GetDevice();
				auto& debug_utils = render_device.GetDebugUtils();
				debug_utils.EndLabel(handle);
			}
		}
	}
}