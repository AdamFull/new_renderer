#include <engine/rendering/Rendering.h>

#include <engine/rendering/gfx/Context.h>

#include <engine/rendering/RenderContext.h>
#include <engine/rendering/UploadContext.h>

#include <engine/rendering/render_graph/ComputePass.h>
#include <engine/rendering/render_graph/RenderPass.h>

#include <engine/rendering/GeometryBuffer.h>
#include <engine/rendering/RenderCamera.h>
#include <engine/rendering/DynamicBuffer.h>

#include <engine/rendering/render_graph/RenderGraph.h>

#include <util/Logger.h>

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

namespace engine
{
	namespace render
	{
		bool IsXOnlyAccess(const RenderGraphResource& resource, gfx::AccessTypeFlags access_flags)
		{
			//for (const auto& access : resource.usages)
			//{
			//	if (!(access.access_flags & access_flags))
			//		return false;
			//}

			return true;
		}

		bool IsROAccessOnlyRemain(const RenderGraphResource& resource, u64 current_pass)
		{
			//for (const auto& access : resource.usages)
			//{
			//	if (access.pass_index > current_pass)
			//		return false;
			//}

			return true;
		}


		Renderer::Renderer(Window* window, filesystem::IVirtualFileSystemInterface* vfs) :
			vfs{ vfs }
		{
			gfx::ContextCreateInfo gfx_create_info{};
			gfx_create_info.app_name = "MyVulkanEngine";
			gfx_create_info.render_api = gfx::RenderApi::eVulkan;
			gfx_create_info.window = window;

			gfx_context = gfx::CreateContext(gfx_create_info);

			resource_context = std::make_unique<UploadContext>(*gfx_context, vfs);
			render_graph = std::make_unique<RenderGraph>(*this, window);

			// Create buffers
			node_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FSceneNode), 64ull, gfx::BufferUsageFlag::eStorageBuffer);
			matrices_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(glm::mat4), 64ull, gfx::BufferUsageFlag::eStorageBuffer);
			mesh_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FMesh), 64ull, gfx::BufferUsageFlag::eStorageBuffer);

			material_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FMaterial), 64ull, gfx::BufferUsageFlag::eStorageBuffer);

			light_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FLightCommit), 64ull, gfx::BufferUsageFlag::eStorageBuffer);

			cascaded_shadow_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FCascadedShadowCommit), max_directional_light_shadow_count, gfx::BufferUsageFlag::eStorageBuffer);
			omni_shadow_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FOmniShadowCommit), max_point_light_shadow_count, gfx::BufferUsageFlag::eStorageBuffer);
			spot_shadow_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FSpotShadowCommit), max_spot_light_shadow_count, gfx::BufferUsageFlag::eStorageBuffer);

			geometry_batches_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FGeometryBatch), 64ull, gfx::BufferUsageFlag::eStorageBuffer);
			raw_geometry_batches_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FGeometryBatch), 64ull, gfx::BufferUsageFlag::eStorageBuffer);

			frustum_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(glm::vec4), 64ull, gfx::BufferUsageFlag::eStorageBuffer);

			//indexed_draw_indirect_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(), 64ull, gfx::BufferUsageFlag::eIndirectBuffer);
			indirect_count_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(uint32_t), 64ull, gfx::BufferUsageFlag::eIndirectBuffer);
			mesh_command_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FDrawMeshIndirectData), 64ull, gfx::BufferUsageFlag::eStorageBuffer);

			camera_buffer = std::make_unique<DynamicBuffer>(*resource_context, sizeof(FCamera), 64ull, gfx::BufferUsageFlag::eStorageBuffer);

			// Temporary func to create test render graph
			BuildGraph();
		}

		Renderer::~Renderer()
		{
			
		}

		uint32_t Renderer::AddRenderResource(const std::string& path, gfx::Sampler* sampler)
		{
			return render_graph->AddResource(path, sampler);
		}

		uint32_t Renderer::AddRenderResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler)
		{
			return render_graph->AddResource(create_info, sampler);
		}

		uint32_t Renderer::AddRenderResource(const gfx::BufferCreateInfo& create_info)
		{
			return render_graph->AddResource(create_info);
		}

		void Renderer::Render(float delta_time)
		{
			render_graph->BeginFrame();
			render_graph->Evaluate(delta_time);
			render_graph->EndFrame();
		}

		gfx::ResourceContext& Renderer::GetResourceContext()
		{
			return *resource_context;
		}

		gfx::ResourceContext const& Renderer::GetResourceContext() const
		{
			return *resource_context;
		}

		gfx::Context& Renderer::GetContext()
		{
			return *gfx_context;
		}

		GeometryBuffer& Renderer::RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info)
		{
			uint32_t hash = Hash(create_info);

			if (auto found = interleaved_geometry_buffers.find(hash); found != interleaved_geometry_buffers.end())
			{
				return *found->second;
			}

			interleaved_geometry_buffers[hash] = std::make_unique<InterleavedGeometryBuffer>(*resource_context, create_info);
			return *interleaved_geometry_buffers[hash];
		}

		GeometryBuffer& Renderer::RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info)
		{
			uint32_t hash = Hash(create_info);

			if (auto found = deinterleaved_geometry_buffers.find(hash); found != deinterleaved_geometry_buffers.end())
			{
				return *found->second;
			}

			deinterleaved_geometry_buffers[hash] = std::make_unique<DeinterleavedGeometryBuffer>(*resource_context, create_info);
			return *deinterleaved_geometry_buffers[hash];
		}

		uint32_t Renderer::AddRenderCamera()
		{
			auto camera_id = render_cameras.size();

			render_cameras.emplace_back(std::make_unique<RenderCamera>());

			return camera_id;
		}

		IRenderCamera& Renderer::GetRenderCamera(uint32_t camera_id)
		{
			return *render_cameras[camera_id];
		}

		IDynamicBuffer& Renderer::GetNodeBuffer()
		{
			return *node_buffer;
		}

		IDynamicBuffer& Renderer::GetMatrixBuffer()
		{
			return *matrices_buffer;
		}

		IDynamicBuffer& Renderer::GetMeshBuffer()
		{
			return *mesh_buffer;
		}

		IDynamicBuffer& Renderer::GetMaterialBuffer()
		{
			return *material_buffer;
		}

		IDynamicBuffer& Renderer::GetLightBuffer()
		{
			return *light_buffer;
		}

		IDynamicBuffer& Renderer::GetCascadedShadowBuffer()
		{
			return *cascaded_shadow_buffer;
		}

		IDynamicBuffer& Renderer::GetOmniShadowBuffer()
		{
			return *omni_shadow_buffer;
		}

		IDynamicBuffer& Renderer::GetSpotShadowBuffer()
		{
			return *spot_shadow_buffer;
		}

		IDynamicBuffer& Renderer::GetGeobetryBatchBuffer()
		{
			return *geometry_batches_buffer;
		}

		IDynamicBuffer& Renderer::GetRawGeobetryBatchBuffer()
		{
			return *raw_geometry_batches_buffer;
		}

		IDynamicBuffer& Renderer::GetFrustumBuffer()
		{
			return *frustum_buffer;
		}

		IDynamicBuffer& Renderer::GetIndirectDrawBuffer()
		{
			return *indexed_draw_indirect_buffer;
		}

		IDynamicBuffer& Renderer::GetIndirectCountDrawBuffer()
		{
			return *indirect_count_buffer;
		}

		IDynamicBuffer& Renderer::GetMeshCommandBuffer()
		{
			return *mesh_command_buffer;
		}

		IDynamicBuffer& Renderer::GetCameraBuffer()
		{
			return *camera_buffer;
		}

		//void Renderer::EvaluateShaderPass(uint32_t pass_index)
		//{
		//	static gfx::AccessTypeFlag access_map[] = { gfx::AccessTypeFlag::eRead, gfx::AccessTypeFlag::eWrite, gfx::AccessTypeFlag::eRW };
		//
		//	std::array<gfx::BufferBarrier, 64ull> buffer_barriers{};
		//	std::array<gfx::TextureBarrier, 64ull> texture_barriers{};
		//	std::array<gfx::MemoryBarrier, 64ull> memory_barriers{};
		//
		//	gfx::ResourceBarrierInfo resource_barriers{};
		//	resource_barriers.buffer_barriers = buffer_barriers.data();
		//	resource_barriers.texture_barriers = texture_barriers.data();
		//	resource_barriers.memory_barriers = memory_barriers.data();
		//
		//	auto& shader_pass = shader_passes[pass_index];
		//	auto& pass_bindings = per_pass_resource_bindings[pass_index];
		//
		//	// Start from resource bindings
		//	for (const auto& resource_binding : pass_bindings.bindings)
		//	{
		//		auto& render_resource = graph_resources[resource_binding.render_resource_id];
		//
		//		// Do not need to sync static images
		//		if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
		//		{
		//			continue;
		//		}
		//
		//		// Ignore read only resource
		//		//if (IsXOnlyAccess(render_resource, gfx::AccessTypeFlag::eRead))
		//		//	continue;
		//
		//		// Graph also should support work with subresources
		//		uint32_t subresource_key = resource_binding.subresource_index == ~0u ? 0u : resource_binding.subresource_index;
		//		auto resource_key = std::make_pair(resource_binding.render_resource_id, subresource_key);
		//
		//		auto& last_resource_access = resource_access_map[resource_key];
		//		auto last_access = render_resource.GetAccess(resource_binding.subresource_index);
		//
		//		auto access_flag = resource_binding.access;
		//
		//		// Ignoring read after read barriers
		//		if ((last_access & access_flag) && access_flag == gfx::AccessTypeFlag::eRead)
		//			continue;
		//
		//		bool is_write_after_write = ((last_access & access_flag) && access_flag == gfx::AccessTypeFlag::eWrite);
		//		bool is_read_after_write = (last_access & gfx::AccessTypeFlag::eWrite) && access_flag == gfx::AccessTypeFlag::eRead;
		//		bool is_change_pipeline_ownership = last_resource_access.pass_type == utl::enum_cast(shader_pass->GetPassType()) && is_write_after_write;
		//
		//		if (render_resource.resource_type & RenderResourceTypeFlag::eBuffer)
		//		{
		//			auto& resource = buffers.Get(render_resource.resource_id);
		//			auto& barrier = buffer_barriers[resource_barriers.buffer_barrier_count++];
		//			barrier.buffer = resource.buffer;
		//			barrier.src_state = render_resource.GetState(resource_binding.subresource_index);
		//
		//			if (access_flag == gfx::AccessTypeFlag::eRead)
		//				barrier.dst_state = render_resource.final_state;
		//			else
		//				barrier.dst_state = render_resource.intermediate_state;
		//
		//				render_resource.SetState(barrier.dst_state, resource_binding.subresource_index);
		//		}
		//		else
		//		{
		//			auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
		//			barrier.src_state = render_resource.GetState(resource_binding.subresource_index);
		//			barrier.texture = GetTextureResource(render_resource);
		//			barrier.subresource_index = resource_binding.subresource_index;
		//
		//			if (last_access == 0)
		//				barrier.dst_state = access_flag == gfx::AccessTypeFlag::eRead ? render_resource.final_state : render_resource.intermediate_state;
		//			//else if (access_flag != gfx::AccessTypeFlag::eWrite && IsROAccessOnlyRemain(render_resource, pass_index))
		//			//	barrier.dst_state = render_resource.final_state;
		//			else if (is_change_pipeline_ownership)
		//				barrier.dst_state = render_resource.intermediate_state;
		//			else if(is_read_after_write)
		//				barrier.dst_state = render_resource.final_state;
		//
		//			render_resource.SetState(barrier.dst_state, resource_binding.subresource_index);
		//		}
		//
		//		last_resource_access.pass_type = utl::enum_cast(shader_pass->GetPassType());
		//	}
		//
		//	// For attachments (write only)
		//	if (!pass_bindings.attachments.empty())
		//	{
		//		auto* render_pass = static_cast<RenderPass*>(shader_pass.get());
		//		for (const auto& resource_attachment : pass_bindings.attachments)
		//		{
		//			auto& render_resource = graph_resources[resource_attachment.render_resource_id];
		//
		//			// Wtf? Trying write to static resource????
		//			if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
		//			{
		//				log_error("Trying bind static texture as color attachment!");
		//				assert(false && "INCORRECT OUTPUT RESOURCE BINDING!");
		//				continue;
		//			}
		//
		//			// Graph also should support work with subresources
		//			auto resource_key = std::make_pair(resource_attachment.render_resource_id, 0u);
		//
		//			auto& last_resource_access = resource_access_map[resource_key];
		//			auto last_access = render_resource.GetAccess();
		//
		//			auto access_flag = resource_attachment.access;
		//
		//			bool is_backbuffer_resource = resource_attachment.render_resource_id == backbuffer_resource;
		//			bool is_write_after_write = ((last_access & access_flag) && access_flag == gfx::AccessTypeFlag::eWrite);
		//			bool is_change_pipeline_ownership = last_resource_access.pass_type == utl::enum_cast(shader_pass->GetPassType()) && is_write_after_write;
		//
		//			gfx::Attachment attachment{};
		//			attachment.load_op = is_write_after_write ? gfx::LoadOp::eLoad : gfx::LoadOp::eClear;
		//			//attachment.store_op = !IsXOnlyAccess(render_resource, gfx::AccessTypeFlag::eRead) || is_backbuffer_resource ? gfx::StoreOp::eStore : gfx::StoreOp::eDontCare;
		//			attachment.store_op = gfx::StoreOp::eStore;
		//			attachment.clear_color = resource_attachment.clear_color;
		//
		//			auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
		//			barrier.src_state = render_resource.GetState();
		//			barrier.texture = GetTextureResource(render_resource);
		//
		//			if (last_access == 0)
		//				barrier.dst_state = access_flag == gfx::AccessTypeFlag::eRead ? render_resource.final_state : render_resource.intermediate_state;
		//			//else if (access_flag != gfx::AccessTypeFlag::eWrite && IsROAccessOnlyRemain(render_resource, pass_index))
		//			//	barrier.dst_state = is_backbuffer_resource ? render_resource.intermediate_state : render_resource.final_state;
		//			else if (is_change_pipeline_ownership)
		//				barrier.dst_state = render_resource.intermediate_state;
		//
		//			render_resource.SetState(barrier.dst_state);
		//
		//			attachment.texture = barrier.texture;
		//
		//			if (resource_attachment.flags & ResourceAttachmentTypeFlag::eDepthStencil)
		//				render_pass->SetDepthStencilAttachment(attachment);
		//			else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eDepth)
		//				render_pass->SetDepthAttachment(attachment);
		//			else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eStencil)
		//				render_pass->SetStencilAttachment(attachment);
		//			else
		//				render_pass->AddColorAttachment(attachment);
		//
		//			last_resource_access.pass_type = utl::enum_cast(shader_pass->GetPassType());
		//		}
		//	}
		//
		//	// Translate textures
		//	auto& command_list = render_context->GetCurrentCommandList();
		//	command_list.ResourceBarrier(resource_barriers);
		//
		//	// Bind pass resources
		//	for (const auto& resource_binding : pass_bindings.bindings)
		//	{
		//		auto& render_resource = graph_resources[resource_binding.render_resource_id];
		//
		//		// Bind resource
		//		ShaderResourceBinding shader_resource_binding{};
		//		shader_resource_binding.binding_point = resource_binding.bind_point;
		//		shader_resource_binding.subresource = resource_binding.subresource_index;
		//
		//		// Do not need to sync static images
		//		if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
		//		{
		//			auto& resource = textures.Get(render_resource.resource_id);
		//			shader_resource_binding.texture = resource.texture;
		//			shader_resource_binding.sampler = resource.sampler;
		//		}
		//		else if ((render_resource.resource_type & RenderResourceTypeFlag::eRenderTarget) || (render_resource.resource_type & RenderResourceTypeFlag::eStorageTarget))
		//		{
		//			auto& resource = render_targets.Get(render_resource.resource_id);
		//			shader_resource_binding.texture = resource.texture;
		//			shader_resource_binding.sampler = resource.sampler;
		//		}
		//		else if (render_resource.resource_type & RenderResourceTypeFlag::eBuffer)
		//		{
		//			auto& resource = buffers.Get(render_resource.resource_id);
		//			shader_resource_binding.buffer = resource.buffer;
		//		}
		//		else
		//		{
		//			assert(false && "THIS RESOURCE TYPE IS NOT SUPPORTED FOR BINDING");
		//		}
		//
		//		shader_pass->BindResource(shader_resource_binding);
		//	}
		//}


		void Renderer::BuildGraph()
		{
			using namespace utl;

			// Register backbuffer resource

			backbuffer_resource = render_graph->AddResource();

			auto& backbuffer_resource_binding = render_graph->GetResource(backbuffer_resource);
			backbuffer_resource_binding.resource_type = RenderResourceTypeFlag::eBackbuffer;
			backbuffer_resource_binding.intermediate_state = gfx::ResourceStateFlag::eRenderTarget;
			backbuffer_resource_binding.final_state = gfx::ResourceStateFlag::ePresent;

			// Create default samplers

			gfx::Sampler* nearest_sampler{ nullptr };
			{
				gfx::SamplerCreateInfo create_info{};
				create_info.min_filter = gfx::Filter::eNearest;
				create_info.mag_filter = gfx::Filter::eNearest;
				create_info.mip_map_mode = gfx::Filter::eNearest;

				nearest_sampler = resource_context->GetSampler(create_info);
			}

			gfx::Sampler* linear_sampler{ nullptr };
			{
				gfx::SamplerCreateInfo create_info{};
				create_info.min_filter = gfx::Filter::eLinear;
				create_info.mag_filter = gfx::Filter::eLinear;
				create_info.mip_map_mode = gfx::Filter::eLinear;

				linear_sampler = resource_context->GetSampler(create_info);
			}

			// Load some static textures
			auto casual_day_4k_cube = render_graph->AddResource("/root/embed/rendering/textures/CasualDay4K.ktx2");
			auto dirt_mask_texture = render_graph->AddResource("/root/embed/rendering/textures/DirtMaskTextureExample.png");

			// GI targets
			uint32_t brdf_lut_target{ ~0u };
			{
				// TODO: make it fixed size
				gfx::TextureCreateInfo create_info{};
				create_info.width = create_info.height = 1024u;
				create_info.format = gfx::Format::eR16G16Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage;
				create_info.resizable = false;

				brdf_lut_target = render_graph->AddResource(create_info, nearest_sampler);
			}

			uint32_t irradiance_cube_target{ ~0u };
			{
				gfx::TextureCreateInfo create_info{};
				create_info.width = create_info.height = 64u;
				create_info.format = gfx::Format::eR32G32B32A32Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage | gfx::TextureUsageFlag::eCubemap;
				create_info.resizable = false;

				irradiance_cube_target = render_graph->AddResource(create_info, linear_sampler);
			}

			uint32_t prefiltered_cube_target{ ~0u };
			{gfx::TextureCreateInfo create_info{};
				create_info.width = create_info.height = 512u;
				create_info.format = gfx::Format::eR16G16B16A16Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage | gfx::TextureUsageFlag::eCubemap;
				create_info.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(create_info.width, create_info.height)))) + 1u;
				create_info.resizable = false;

				gfx::Sampler* special_sampler{ nullptr };
				{
					gfx::SamplerCreateInfo create_info_s{};
					create_info_s.min_filter = gfx::Filter::eLinear;
					create_info_s.mag_filter = gfx::Filter::eLinear;
					create_info_s.mip_map_mode = gfx::Filter::eLinear;
					create_info_s.min_lod = 0.f;
					create_info_s.max_lod = static_cast<float>(create_info.levels);

					special_sampler = resource_context->GetSampler(create_info_s);
				}

				prefiltered_cube_target = render_graph->AddResource(create_info, special_sampler);
			}
			// Passes
			// Generate brdflut
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:brgflut_generator";
				create_info.one_time = true;

				create_info.draw_callback = [this, brdf_lut_target](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						self.BindWriteResource("out_color"_hash32, brdf_lut_target);

						self.FlushResources(command_list);
				
						uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };
				
						auto texture = render_graph->GetTexture(brdf_lut_target);
				
						group_x = texture->GetWidth();
						group_y = texture->GetHeight();
				
						self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
						command_list.Dispatch(group_x, group_y, group_z);
					};

				render_graph->AddNode(render_graph->CreateNode(create_info));
			}

			// Generate irradiance cubemap
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:irradiancecube_generator";
				create_info.one_time = true;

				create_info.draw_callback = [this, irradiance_cube_target, casual_day_4k_cube](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						self.BindReadResource("sampler_color"_hash32, casual_day_4k_cube);
						self.BindWriteResource("out_color"_hash32, irradiance_cube_target);

						self.FlushResources(command_list);
				
						uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };
				
						auto texture = render_graph->GetTexture(irradiance_cube_target);
				
						group_x = texture->GetWidth();
						group_y = texture->GetHeight();
				
						self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
						command_list.Dispatch(group_x, group_y, group_z);
					};

				render_graph->AddNode(render_graph->CreateNode(create_info));
			}

			// Generate prefiltered cubemap
			{
				auto group_node = render_graph->CreateNode("prefiltered map generator", 0x99F9F1FF);

				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:prefilteredmap_generator";
				create_info.one_time = true;

				create_info.draw_callback = [this, prefiltered_cube_target, casual_day_4k_cube](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						auto target = render_graph->GetTexture(prefiltered_cube_target);
						for (int32_t mip = 0; mip < target->GetMipLevelCount(); ++mip)
						{
							self.BindReadResource("sampler_color"_hash32, mip == 0 ? casual_day_4k_cube : prefiltered_cube_target, mip == 0 ? ~0u : mip - 1);
							self.BindWriteResource("out_color"_hash32, prefiltered_cube_target, mip);

							self.FlushResources(command_list);

							uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

							float layer_roughness = static_cast<float>(mip) / static_cast<float>(target->GetMipLevelCount());

							auto& constants = self.GetConstantBlock();
							constants.Set("roughness"_hash32, layer_roughness);
							self.PushConstantRanges(command_list);

							group_x = target->GetWidth() >> mip;
							group_y = target->GetHeight() >> mip;
							
							self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
							command_list.Dispatch(group_x, group_y, group_z);
						}
					};

				static_cast<GroupNode*>(group_node.get())->AddNode(std::move(render_graph->CreateNode(create_info)));

			//
			//	auto& resource = graph_resources[prefiltered_cube_target];
			//	auto* texture = GetTextureResource(resource);
			//
			//	for (int32_t mip = 0; mip < texture->GetMipLevelCount(); ++mip)
			//	{
			//		ComputeShaderPassCreateInfo create_info{};
			//		create_info.shader_name = "lighting:prefilteredmap_generator";
			//		create_info.one_time = true;
			//		create_info.BindWriteResource("out_color"_hash32, prefiltered_cube_target, mip);
			//		create_info.BindReadResource("sampler_color"_hash32, mip == 0 ? casual_day_4k_cube : prefiltered_cube_target, mip == 0 ? ~0u : mip - 1);
			//		//create_info.BindReadResource("sampler_color"_hash32, casual_day_4k_cube);
			//
			//		create_info.draw_callback = [this, mip, texture](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
			//			{
			//				self.FlushResources(command_list);
			//
			//				uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };
			//
			//				float layer_roughness = static_cast<float>(mip) / static_cast<float>(texture->GetMipLevelCount());
			//				
			//				auto& constants = self.GetConstantBlock();
			//				constants.Set("roughness"_hash32, layer_roughness);
			//				self.PushConstantRanges(command_list);
			//
			//				group_x = texture->GetWidth() >> mip;
			//				group_y = texture->GetHeight() >> mip;
			//
			//				self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
			//				command_list.Dispatch(group_x, group_y, group_z);
			//			};
			//
			//		render_graph->AddNode(render_graph->CreateNode(create_info));
			//		//group->nodes.emplace_back(CreateShaderPassNode(create_info));
			//	}
				render_graph->AddNode(std::move(group_node));
			}

			// Test output shader
			{
				RenderShaderPassCreateInfo create_info{};
				create_info.shader_name = "testing:screenspace";
				create_info.AddWriteAttachment(backbuffer_resource, ResourceAttachmentTypeFlag::eColor, gfx::ClearColor{ 1.f, 0.5f, 1.f, 1.f });

				create_info.draw_callback = [this, irradiance_cube_target, brdf_lut_target](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						auto* backbuffer = render_graph->GetTexture(backbuffer_resource);

						auto viewport_width = backbuffer->GetWidth();
						auto viewport_height = backbuffer->GetHeight();

						command_list.SetViewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
						command_list.SerScissor(0u, 0u, viewport_width, viewport_height);

						self.BindReadResource("input_image"_hash32, brdf_lut_target);
						
						self.FlushResources(command_list);
						
						self.Begin(command_list);
						command_list.Draw(3u, 0u, 0u, 1u);
						self.End(command_list);
					};

				render_graph->AddNode(render_graph->CreateNode(create_info));
			}
		}
	}
}