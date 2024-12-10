#include "RenderContext.h"
#include "UniformPool.h"
#include "RendererInterface.h"

#include "GroupExecutionNode.h"

#include "render_graph/RenderPass.h"
#include "render_graph/ComputePass.h"

#include <engine/runtime/Window.h>

#include "gfx/gfx_context.h"
#include "gfx/gfx_util.h"

#include <format>

#include <logger.hpp>

#include <tracy/Tracy.hpp>

namespace engine
{
	namespace render
	{
		class RenderContext::RenderFrame
		{
		public:
			RenderFrame(gfx::Context& context, gfx::Swapchain* swapchain, const gfx::Queue* queue, uint32_t frame_index) :
				context{ context }
			{
				backbuffer = context.CreateSwapchainTexture(swapchain, frame_index);
				command_pool = context.CreateCommandPool(queue);
				descriptor_allocator = context.CreateDescriptorAllocator();
				uniform_pool = std::make_unique<UniformPool>(context, 16384ull);
				image_avaliable = context.CreateSemaphore();
				image_avaliable->SetResourceName(std::format("image_avaliable_{}", frame_index));
				render_finished = context.CreateSemaphore();
				render_finished->SetResourceName(std::format("render_finished_{}", frame_index));
				upload_avaliable = context.CreateSemaphore();
				render_finished->SetResourceName(std::format("upload_avaliable_{}", frame_index));
				fence = context.CreateFence();
			}

			~RenderFrame()
			{
				context.Destroy(backbuffer);
				context.Destroy(command_pool);
				context.Destroy(descriptor_allocator);
				context.Destroy(image_avaliable);
				context.Destroy(render_finished);
				context.Destroy(upload_avaliable);
				context.Destroy(fence);
				uniform_pool.reset();
			}

			void UpdateBackbuffer(gfx::Swapchain* swapchain, uint32_t frame_index)
			{
				context.Destroy(backbuffer);
				context.CreateSwapchainTexture(swapchain, frame_index);
			}

			void Reset()
			{
				descriptor_allocator->Reset();
				command_pool->Reset();
				uniform_pool->Reset();
			}

			gfx::Texture* backbuffer{ nullptr };
			gfx::CommandPool* command_pool{ nullptr };
			gfx::DescriptorAllocator* descriptor_allocator{ nullptr };
			std::unique_ptr<UniformPool> uniform_pool{};
			gfx::Semaphore* image_avaliable{ nullptr };
			gfx::Semaphore* render_finished{ nullptr };
			gfx::Semaphore* upload_avaliable{ nullptr };
			gfx::Fence* fence{ nullptr };

		private:
			gfx::Context& context;
		};

		RenderContext::RenderContext(IRenderer& renderer, Window* window) :
			context{ renderer.GetContext() },
			resource_loader{ renderer.GetResourceLoader() },
			window{ *window }
		{
			// Select queue that can present
			queue = context.RequestQueue(gfx::QueueTypeFlag::ePresent);
			
			gfx::SwapChainCreateInfo swapchain_ci{};
			swapchain_ci.window = window;
			swapchain_ci.image_count = 3u;

			swapchain = context.CreateSwapchain(swapchain_ci);

			auto image_count = swapchain->GetImageCount();

			for (int32_t i = 0; i < image_count; ++i)
			{
				render_frames[i] = std::make_unique<RenderFrame>(context, swapchain, queue, i);
			}

			// Create default image sampler
			gfx::SamplerCreateInfo sampler_ci{};
			sampler_ci.address_u = sampler_ci.address_v = sampler_ci.address_w = gfx::SamplerAddressMode::eClampToEdge;
			sampler_ci.min_filter = gfx::Filter::eLinear;
			sampler_ci.mag_filter = gfx::Filter::eLinear;
			sampler_ci.mip_map_mode = gfx::Filter::eLinear;

			default_sampler = context.CreateSampler(sampler_ci);

			// Create timestamp query for collecting pass info
			gfx::QueryPoolCreateInfo query_pool_ci{};
			query_pool_ci.type = gfx::QueryType::eTimestamp;
			query_pool_ci.query_count = 2u;
			timestamp_query = context.CreateQueryPool(query_pool_ci);
			timestamp_query->Reset(0u);

			context.RequestTimestampFrequency(&timestamp_frequency);

			// TODO: now it creates just default one
			gfx::SharedDescriptorCreateInfo shared_descriptor_ci{};
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eUniformBuffer, 0u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eUniformBuffer, 1u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eUniformBuffer, 2u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eSampledImage, 3u, shlib::MAX_SAMPLED_TEXTURES);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eSampler, 4u, shlib::MAX_SAMPLED_TEXTURES);

			shared_descriptor = context.CreateSharedDescriptor(shared_descriptor_ci);

			// Create uniform pool
			global_uniform_pool = std::make_unique<UniformPool>(context);
		}

		RenderContext::~RenderContext()
		{
			queue->WaitIdle();

			auto image_count = swapchain->GetImageCount();
			for (int32_t i = 0; i < image_count; ++i)
			{
				render_frames[i] = nullptr;
			}

			context.Destroy(swapchain);

			context.Destroy(default_sampler);
			context.Destroy(shared_descriptor);
			context.Destroy(timestamp_query);

			for (auto& texture_object : static_textures.Data())
			{
				context.Destroy(texture_object.texture);
			}

			for (auto& texture_object : render_targets.Data())
			{
				context.Destroy(texture_object.texture);
			}

			for (auto& buffer_object : buffers.Data())
			{
				context.Destroy(buffer_object.buffer);
			}
		}

		uint32_t RenderContext::AddResource()
		{
			// Just creates empty resource)
			auto& new_resource = render_resources.emplace_back();
			return render_resources.size() - 1ull;
		}

		uint32_t RenderContext::AddResource(std::string_view path, gfx::Sampler* sampler)
		{
			auto& new_resource = render_resources.emplace_back();
			new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			gfx::Texture* texture = resource_loader.LoadTexture(path);
			new_resource.resource_id = static_textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });

			new_resource.current_state = gfx::ResourceStateFlag::eShaderReadOnlyResource;

			return render_resources.size() - 1ull;
		}

		uint32_t RenderContext::AddResource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data)
		{
			auto& new_resource = render_resources.emplace_back();
			new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			gfx::TextureCreateInfo texture_create_info{};
			texture_create_info.usage = create_info.usage;
			texture_create_info.width = create_info.width;
			texture_create_info.height = create_info.height;
			texture_create_info.depth = create_info.depth;
			texture_create_info.layers = create_info.layers;
			texture_create_info.levels = create_info.levels;
			texture_create_info.format = create_info.format;

			gfx::Texture* texture = resource_loader.LoadTexture(texture_create_info, texture_data);
			new_resource.resource_id = static_textures.Add(TextureObject{ texture, default_sampler });

			new_resource.current_state = gfx::ResourceStateFlag::eShaderReadOnlyResource;

			return render_resources.size() - 1ull;
		}

		uint32_t RenderContext::AddResource(const TextureResourceCreateInfo& create_info, gfx::Sampler* sampler)
		{
			auto& new_resource = render_resources.emplace_back();

			// TODO: What if it's storage target and render target at the same time?
			if (create_info.usage & gfx::TextureUsageFlag::eStorage)
				new_resource.resource_type = RenderResourceTypeFlag::eStorageTarget;
			else if (create_info.usage & gfx::TextureUsageFlag::eRenderTarget)
				new_resource.resource_type = RenderResourceTypeFlag::eRenderTarget;
			else
				new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			gfx::TextureCreateInfo texture_create_info{};
			texture_create_info.usage = create_info.usage;
			texture_create_info.width = create_info.width;
			texture_create_info.height = create_info.height;
			texture_create_info.depth = create_info.depth;
			texture_create_info.layers = create_info.layers;
			texture_create_info.levels = create_info.levels;
			texture_create_info.format = create_info.format;

			TextureObject texture_object{};
			texture_object.texture = context.CreateTexture(texture_create_info);
			texture_object.sampler = sampler ? sampler : default_sampler;
			texture_object.usage = create_info.usage;
			texture_object.resizable = create_info.resizable;
			texture_object.scale = create_info.scale;
			texture_object.scale_parent_resource = create_info.scale_parent_resource;

			if ((create_info.usage & gfx::TextureUsageFlag::eRenderTarget) || (create_info.usage & gfx::TextureUsageFlag::eStorage))
				new_resource.resource_id = render_targets.Add(std::move(texture_object));
			else
				new_resource.resource_id = static_textures.Add(std::move(texture_object));

			return render_resources.size() - 1ull;
		}

		uint32_t RenderContext::AddResource(const gfx::BufferCreateInfo& create_info)
		{
			gfx::Buffer* buffer = context.CreateBuffer(create_info);
			auto handle = buffers.Add(BufferObject{ buffer });
			if (create_info.usage != gfx::BufferUsageFlag::eStagingBuffer && create_info.usage != gfx::BufferUsageFlag::eUniformBuffer)
			{
				auto& new_resource = render_resources.emplace_back();
				if (create_info.usage & gfx::BufferUsageFlag::eIndirect)
					new_resource.resource_type = RenderResourceTypeFlag::eIndirectBuffer;
				else if (create_info.usage & gfx::BufferUsageFlag::eStorage)
					new_resource.resource_type = RenderResourceTypeFlag::eStorageBuffer;

				new_resource.resource_id = handle;

				return render_resources.size() - 1ull;
			}

			return ~0u;
		}

		void RenderContext::SetResourceName(uint32_t resource_index, std::string_view name)
		{
			auto& render_resource = render_resources[resource_index];

			gfx::GFXResource* resource{};
			if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				auto& current_frame = GetCurrentFrame();
				resource = current_frame.backbuffer;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
			{
				auto& texture_object = static_textures.Get(render_resource.resource_id);
				resource = texture_object.texture;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				auto& texture_object = render_targets.Get(render_resource.resource_id);
				resource = texture_object.texture;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer)
			{
				auto& buffer_object = buffers.Get(render_resource.resource_id);
				resource = buffer_object.buffer;
			}

			resource->SetResourceName(name);
		}

		RenderResource& RenderContext::GetResource(uint32_t resource_index)
		{
			return render_resources[resource_index];
		}

		RenderResource const& RenderContext::GetResource(uint32_t resource_index) const
		{
			return render_resources[resource_index];
		}

		std::unique_ptr<ExecutionNode> RenderContext::CreateNode(std::string_view group_name, uint32_t group_color)
		{
			return std::make_unique<GroupNode>(group_name, group_color);
		}

		std::unique_ptr<ExecutionNode> RenderContext::CreateNode(const RenderShaderPassCreateInfo& create_info)
		{
			RenderPassCreateInfo pass_ci{};
			pass_ci.debug_name = create_info.shader_name;
			pass_ci.shader = resource_loader.LoadShader(create_info.shader_name);
			pass_ci.shared_descriptor = shared_descriptor;
			pass_ci.pipeline_state = create_info.pipeline_state;
			pass_ci.draw_callback = std::move(create_info.draw_callback);
			pass_ci.enabled_callback = std::move(create_info.enabled_callback);

			pass_ci.resource_attachments = create_info.resource_attachments;

			auto new_pass = std::make_unique<RenderPass>(*this, pass_ci);
			return new_pass;
		}

		std::unique_ptr<ExecutionNode> RenderContext::CreateNode(const ComputeShaderPassCreateInfo& create_info)
		{
			ComputePassCreateInfo pass_ci{};
			pass_ci.debug_name = create_info.shader_name;
			pass_ci.shader = resource_loader.LoadShader(create_info.shader_name);
			pass_ci.one_time = create_info.one_time;
			pass_ci.shared_descriptor = shared_descriptor;
			pass_ci.draw_callback = std::move(create_info.draw_callback);
			pass_ci.enabled_callback = std::move(create_info.enabled_callback);

			auto new_pass = std::make_unique<ComputePass>(*this, pass_ci);
			return new_pass;
		}

		void RenderContext::AddNode(std::unique_ptr<ExecutionNode>&& node)
		{
			graph_nodes.emplace_back(std::move(node));
		}

		void RenderContext::Execute(float delta_time)
		{
			ZoneScoped;

			auto& current_frame = GetCurrentFrame();
			auto& command_list = *current_command_list;

			for (auto& node : graph_nodes)
				node->Execute(command_list, delta_time);

			auto& backbuffer_resource = GetResource(0);

			gfx::TextureBarrier texture_barrier{};
			gfx::ResourceBarrierInfo resource_barriers{};
			resource_barriers.texture_barriers = &texture_barrier;
			resource_barriers.texture_barrier_count = 1;

			texture_barrier.texture = current_frame.backbuffer;
			// TODO: get current resource state from resource itself
			texture_barrier.src_state = backbuffer_resource.GetState();
			texture_barrier.dst_state = gfx::ResourceStateFlag::ePresentSrc;

			command_list.ResourceBarrier(resource_barriers);
		}

		void RenderContext::UpdateGlobalBinding(uint32_t binding, std::span<const uint8_t> data)
		{
			ZoneScoped;

			if (!global_resource_descriptor)
			{
				global_resource_descriptor = shared_descriptor->RequestDescriptors();
			}

			auto buffer_block = global_uniform_pool->RequestBlock(data.size());
			buffer_block.Update(data);

			gfx::BufferDescriptor buffer_descriptor{};
			buffer_descriptor.type = gfx::DescriptorType::eUniformBuffer;
			buffer_descriptor.binding = binding;
			buffer_descriptor.buffer = &buffer_block.GetBuffer();
			buffer_descriptor.offset = buffer_block.GetOffset();
			buffer_descriptor.size = buffer_block.GetSize();

			global_resource_descriptor->BindBuffer(buffer_descriptor);
		}

		void RenderContext::Begin()
		{
			ZoneScoped;

			global_uniform_pool->Reset();

			shared_descriptor->Reset();
			if (!global_resource_descriptor)
			{
				global_resource_descriptor = shared_descriptor->RequestDescriptors();
			}

			bool surface_updated{ false };
			auto& command_list = BeginCommands(surface_updated);
			if (surface_updated)
			{
				UpdateRenderTargets();
			}

			uint64_t timestamps[2] = {};
			timestamp_query->GetData(0u, &timestamps[0]);

			uint64_t elapsed_time = timestamps[1] - timestamps[0];
			if (timestamps[1] <= timestamps[0])
			{
				elapsed_time = 0ull;
			}

			gpu_delta_time = static_cast<float>(elapsed_time) * timestamp_frequency / 1000000000.f;

			command_list.ResetQuery(timestamp_query, 0u, 2u);
			command_list.BeginQuery(timestamp_query, 0u);

			// Update bindless textures
			uint32_t array_index{ 0u };
			for (auto& resource : static_textures.Data())
			{
				gfx::TextureDescriptor texture_descriptor{};
				texture_descriptor.type = gfx::DescriptorType::eSampledImage;
				texture_descriptor.set = 0;
				texture_descriptor.binding = 3;

				texture_descriptor.array_index = array_index;
				texture_descriptor.texture = resource.texture;
				texture_descriptor.resource_state = gfx::ResourceStateFlag::eShaderReadOnlyResource;

				global_resource_descriptor->BindTexture(texture_descriptor);

				gfx::TextureDescriptor sampler_descriptor{};
				sampler_descriptor.type = gfx::DescriptorType::eSampler;
				sampler_descriptor.set = 0;
				sampler_descriptor.binding = 4;

				sampler_descriptor.array_index = array_index;
				sampler_descriptor.sampler = resource.sampler;

				global_resource_descriptor->BindTexture(sampler_descriptor);

				++array_index;
			}
		}

		void RenderContext::End()
		{
			ZoneScoped;

			global_resource_descriptor->Flush();

			gfx::SemaphoreSubmitInfo upload_wait_semaphore{};
			//auto upload_avaliable = GetUploadAvaliableSemaphore();
			//upload_wait_semaphore = Submit(&upload_avaliable, upload_avaliable.semaphore ? 1 : 0);
			upload_wait_semaphore = resource_loader.Submit(nullptr, 0);

			auto& command_list = *current_command_list;
			command_list.EndQuery(timestamp_query, 1u);

			EndFrame(upload_wait_semaphore);

			for (auto& render_resource : render_resources)
			{
				if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage || render_resource.finalized)
					continue;

				render_resource.current_state = gfx::ResourceStateFlag::eUndefined;
			}

			global_resource_descriptor = nullptr;
		}

		void RenderContext::UpdateRenderTargets()
		{
			ZoneScoped;

			uint32_t width, height;
			swapchain->GetSurfaceExtent(width, height);

			// Update/resize render targets here if it's needed
			for (auto& target : render_targets.Data())
			{
				// TODO: resize only if texture is able to resize 
				if (!target.resizable)
					continue;

				gfx::TextureCreateInfo create_info{};
				create_info.usage = target.usage;
				create_info.layers = (target.usage & gfx::TextureUsageFlag::eCubemap) ? target.texture->GetArrayLayerCount() / 6u : target.texture->GetArrayLayerCount();
				// TODO: need to recalculate number of mips?
				create_info.levels = target.texture->GetMipLevelCount();
				create_info.format = target.texture->GetFormat();

				if (target.scale_parent_resource != ~0u)
				{
					gfx::Texture* parent = GetTexture(target.scale_parent_resource);

					create_info.width = parent->GetWidth() * target.scale;
					create_info.height = parent->GetHeight() * target.scale;
				}
				else
				{
					create_info.width = width * target.scale;
					create_info.height = height * target.scale;
				}

				context.Destroy(target.texture);
				target.texture = context.CreateTexture(create_info);

			}
		}

		gfx::Context& RenderContext::GetContext()
		{
			return context;
		}

		gfx::Context const& RenderContext::GetContext() const
		{
			return context;
		}

		gfx::Texture* RenderContext::GetTexture(uint32_t resource_index)
		{
			auto& render_resource = render_resources[resource_index];
			return GetTexture(render_resource);
		}

		gfx::Texture* RenderContext::GetTexture(const RenderResource& render_resource)
		{
			if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				auto& current_frame = GetCurrentFrame();
				return current_frame.backbuffer;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
			{
				auto& texture_object = static_textures.Get(render_resource.resource_id);
				return texture_object.texture;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				auto& texture_object = render_targets.Get(render_resource.resource_id);
				return texture_object.texture;
			}

			return nullptr;
		}

		gfx::Sampler* RenderContext::GetSampler(uint32_t resource_index)
		{
			auto& render_resource = render_resources[resource_index];
			return GetSampler(render_resource);
		}

		gfx::Sampler* RenderContext::GetSampler(const RenderResource& render_resource)
		{
			if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				return default_sampler;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
			{
				auto& texture_object = static_textures.Get(render_resource.resource_id);
				return texture_object.sampler;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				auto& texture_object = render_targets.Get(render_resource.resource_id);
				return texture_object.sampler;
			}

			return default_sampler;
		}

		gfx::Buffer* RenderContext::GetBuffer(uint32_t resource_index)
		{
			auto& render_resource = render_resources[resource_index];
			return GetBuffer(render_resource);
		}

		gfx::Buffer* RenderContext::GetBuffer(const RenderResource& render_resource)
		{
			if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer)
			{
				auto& buffer_object = buffers.Get(render_resource.resource_id);
				return buffer_object.buffer;
			}

			return nullptr;
		}

		void RenderContext::UpdateSwapchain(uint32_t width, uint32_t height)
		{
			gfx::SwapChainCreateInfo new_swapchain_ci{};
			new_swapchain_ci.window = &window;
			new_swapchain_ci.old_swapchain = swapchain;
			new_swapchain_ci.width = width;
			new_swapchain_ci.height = height;

			swapchain = context.CreateSwapchain(new_swapchain_ci);
			context.Destroy(new_swapchain_ci.old_swapchain);
			UpdateResources();
		}

		void RenderContext::UpdateSwapchain(uint32_t image_count)
		{
			gfx::SwapChainCreateInfo new_swapchain_ci{};
			new_swapchain_ci.window = &window;
			new_swapchain_ci.old_swapchain = swapchain;
			new_swapchain_ci.image_count = image_count;

			swapchain = context.CreateSwapchain(new_swapchain_ci);
			context.Destroy(new_swapchain_ci.old_swapchain);
			UpdateResources();
		}

		bool RenderContext::HandleSurfaceChange(bool force_update)
		{
			ZoneScoped;

			if (!swapchain)
			{
				log_warning("Can't handle surface changes in headless mode, skipping.");
				return false;
			}

			uint32_t surface_width, surface_height;
			swapchain->GetSurfaceExtent(surface_width, surface_height);
			if (surface_width == 0xFFFFFFFFu)
			{
				return false;
			}

			auto& current_width = swapchain->GetWidth();
			auto& current_height = swapchain->GetHeight();

			// Only recreate the swapchain if the dimensions have changed;
			// handle_surface_changes() is called on VK_SUBOPTIMAL_KHR,
			// which might not be due to a surface resize
			if (surface_width != current_width ||
				surface_height != current_height ||
				force_update)
			{
				ZoneScopedN("Swapchain update");
				// Recreate swapchain
				queue->WaitIdle();
				UpdateSwapchain(surface_width, surface_height);

				active_frame_index = 0u;
				current_frame_index = 0u;

				return true;
			}

			return false;
		}

		gfx::CommandList& RenderContext::BeginCommands(bool& surface_updated)
		{
			ZoneScoped;

			if (!frame_active)
			{
				BeginFrame(surface_updated);
			}

			if (!acquired_semaphore)
			{
				throw std::runtime_error("Couldn't begin frame");
			}

			auto& current_frame = GetCurrentFrame();
			current_command_list = current_frame.command_pool->Allocate();
			current_command_list->Begin();
			return *current_command_list;
		}

		void RenderContext::BeginFrame(bool& surface_updated)
		{
			ZoneScoped;

			if (swapchain)
			{
				surface_updated = HandleSurfaceChange();
			}

			assert(!frame_active && "Frame is still active, please call end_frame");

			auto& current_frame = GetCurrentFrame();

			// We will use the acquired semaphore in a different frame context,
			// so we need to hold ownership.
			acquired_semaphore = current_frame.image_avaliable;

			if (swapchain)
			{
				if (!swapchain->AcquireNextImage(acquired_semaphore, nullptr, &active_frame_index))
				{
					surface_updated = HandleSurfaceChange(true);

					bool acquire_result{ false };
					if (surface_updated)
					{
						acquire_result = swapchain->AcquireNextImage(acquired_semaphore, nullptr, &active_frame_index);
					}

					if (!acquire_result)
					{
						current_frame.Reset();
						return;
					}
				}
			}

			// Now the frame is active again
			frame_active = true;
			current_frame.Reset();
		}

		void RenderContext::EndFrame(gfx::SemaphoreSubmitInfo wait_semaphore)
		{
			ZoneScoped;

			auto& current_frame = GetCurrentFrame();

			current_command_list->End();
			frame_active = false;

			current_frame.fence->Reset();

			try
			{
				ZoneScopedN("Submit");

				// TODO: COMMAND BUFFERS
				gfx::SubmitInfo submit_info{};

				gfx::SemaphoreSubmitInfo wait_semaphores[2] = {
					gfx::SemaphoreSubmitInfo{ acquired_semaphore, 0ull, gfx::StageFlag::eColorAttachmentOutput },
					wait_semaphore
				};

				submit_info.wait_semaphore_infos = &wait_semaphores[0];
				submit_info.wait_semaphore_count = wait_semaphore.semaphore ? 2 : 1;


				gfx::SemaphoreSubmitInfo signal_semaphores[2] = {
					gfx::SemaphoreSubmitInfo{ current_frame.render_finished, 0ull, gfx::StageFlag::eColorAttachmentOutput },
					gfx::SemaphoreSubmitInfo{ current_frame.upload_avaliable, 0ull, gfx::StageFlag::eAllCommands }
				};

				submit_info.signal_semaphore_infos = &signal_semaphores[0];
				submit_info.signal_semaphore_count = 1u;

				submit_info.command_lists = &current_command_list;
				submit_info.command_buffer_count = 1u;

				queue->Submit(submit_info, current_frame.fence);
			}
			catch (std::exception error)
			{
				log_error("failed to submit Draw command buffer!");
			}

			if (swapchain)
			{
				ZoneScopedN("Present");

				auto* wait_semaphore = current_frame.render_finished;

				gfx::PresentInfo present_info = {};
				present_info.wait_semaphore_count = 1;
				present_info.wait_semaphores = &wait_semaphore;

				present_info.swapchain_count = 1;
				present_info.swapchains = &swapchain;
				present_info.image_indices = &GetCurrentFrameIndex();

				queue->Present(present_info);
				current_frame.fence->Wait();

				current_frame_index = (current_frame_index + 1) % swapchain->GetImageCount();
			}

			previous_frame_semaphore = current_frame.upload_avaliable;
			if (acquired_semaphore)
			{
				acquired_semaphore = nullptr;
			}
		}

		gfx::SemaphoreSubmitInfo RenderContext::GetUploadAvaliableSemaphore()
		{
			return gfx::SemaphoreSubmitInfo{ previous_frame_semaphore, 0ull, gfx::StageFlag::eAllCommands };
		}

		gfx::BufferSuballocation RenderContext::RequestUniformBlock(uint64_t block_size)
		{
			auto& current_frame = GetCurrentFrame();
			return current_frame.uniform_pool->RequestBlock(block_size);
		}

		gfx::CommandList& RenderContext::GetCurrentCommandList()
		{
			return *current_command_list;
		}

		gfx::DescriptorAllocator& RenderContext::GetCurrentDescriptorAllocator()
		{
			auto& current_frame = GetCurrentFrame();
			return *current_frame.descriptor_allocator;
		}

		const uint32_t& RenderContext::GetCurrentFrameIndex() const
		{
			return current_frame_index;
		}

		RenderContext::RenderFrame& RenderContext::GetCurrentFrame()
		{
			return *render_frames[GetCurrentFrameIndex()];
		}
	
		void RenderContext::UpdateResources()
		{
			ZoneScoped;

			auto image_count = swapchain->GetImageCount();

			auto frame_it = render_frames.begin();
			for (int32_t i = 0; i < image_count; ++i)
			{
				if (frame_it != render_frames.end())
				{
					(*frame_it)->UpdateBackbuffer(swapchain, i);
				}
				else
				{
					// Create a new frame if the new swapchain has more images than current frames
					render_frames[i] = std::make_unique<RenderFrame>(context, swapchain, queue, i);
				}

				++frame_it;
			}
		}
	}
}