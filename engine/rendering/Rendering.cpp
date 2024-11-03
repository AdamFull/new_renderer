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

#include <imgui.h>

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

namespace engine
{
	namespace render
	{
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
				return *geometry_buffers[found->second];
			}

			auto& new_geometry_buffer = geometry_buffers.emplace_back(std::make_unique<InterleavedGeometryBuffer>(*resource_context, create_info));
			interleaved_geometry_buffers[hash] = geometry_buffers.size() - 1ull;
			return *new_geometry_buffer;
		}

		GeometryBuffer& Renderer::RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info)
		{
			uint32_t hash = Hash(create_info);

			if (auto found = deinterleaved_geometry_buffers.find(hash); found != deinterleaved_geometry_buffers.end())
			{
				return *geometry_buffers[found->second];
			}

			auto& new_geometry_buffer = geometry_buffers.emplace_back(std::make_unique<DeinterleavedGeometryBuffer>(*resource_context, create_info));
			deinterleaved_geometry_buffers[hash] = geometry_buffers.size() - 1ull;
			return *new_geometry_buffer;
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


		void Renderer::BuildGraph()
		{
			using namespace utl;

			// Some binding points 
			constexpr const uint32_t sampler_color_bnd = "sampler_color"_hash32;
			constexpr const uint32_t out_color_bnd = "out_color"_hash32;
			constexpr const uint32_t roughness_bnd = "roughness"_hash32;

			// Register backbuffer resource

			backbuffer_resource = render_graph->AddResource();

			auto& backbuffer_resource_binding = render_graph->GetResource(backbuffer_resource);
			backbuffer_resource_binding.resource_type = RenderResourceTypeFlag::eBackbuffer;
			backbuffer_resource_binding.intermediate_state = gfx::ResourceStateFlag::eAttachmentWrite;
			backbuffer_resource_binding.final_state_comp = gfx::ResourceStateFlag::eShaderResource;
			backbuffer_resource_binding.final_state_other = gfx::ResourceStateFlag::ePresent;

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

			gfx::Sampler* shadow_sampler{ nullptr };
			{
				gfx::SamplerCreateInfo create_info{};
				create_info.min_filter = gfx::Filter::eLinear;
				create_info.mag_filter = gfx::Filter::eLinear;
				create_info.mip_map_mode = gfx::Filter::eLinear;
				create_info.depth_compare = gfx::CompareOp::eLess;

				shadow_sampler = resource_context->GetSampler(create_info);
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
			{
				gfx::TextureCreateInfo create_info{};
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

			// Shadow targets
			uint32_t cascaded_shadows_target{ ~0u };
			{
				gfx::TextureCreateInfo create_info{};
				create_info.width = create_info.height = cascade_shadow_map_resolution;
				create_info.layers = cascade_shadow_map_cascade_count * max_directional_light_shadow_count;
				create_info.format = gfx::Format::eD32Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = false;
				
				cascaded_shadows_target = render_graph->AddResource(create_info, shadow_sampler);
			}

			uint32_t directional_shadows_target{ ~0u };
			{
				gfx::TextureCreateInfo create_info{};
				create_info.width = create_info.height = spot_light_shadow_map_resolution;
				create_info.layers = max_spot_light_shadow_count;
				create_info.format = gfx::Format::eD16Unorm;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = false;

				directional_shadows_target = render_graph->AddResource(create_info, shadow_sampler);
			}

			uint32_t omni_shadows_target{ ~0u };
			{
				gfx::TextureCreateInfo create_info{};
				create_info.width = create_info.height = point_light_shadow_map_resolution;
				create_info.layers = max_point_light_shadow_count;
				create_info.format = gfx::Format::eD16Unorm;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget | gfx::TextureUsageFlag::eCubemap;
				create_info.resizable = false;

				omni_shadows_target = render_graph->AddResource(create_info, shadow_sampler);
			}

			auto* backbuffer_texture = render_graph->GetTexture(backbuffer_resource);

			// Visibility buffer targets
			uint32_t visbuffer_geometry_target{ ~0u };
			{
				gfx::TextureCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth();
				create_info.height = backbuffer_texture->GetHeight();
				create_info.format = gfx::Format::eR8G8B8A8Unorm;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = true;

				visbuffer_geometry_target = render_graph->AddResource(create_info, nearest_sampler);
			}

			uint32_t visbuffer_vboinst_target{ ~0u };
			{
				gfx::TextureCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth();
				create_info.height = backbuffer_texture->GetHeight();
				create_info.format = gfx::Format::eR8G8B8A8Unorm;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = true;

				visbuffer_vboinst_target = render_graph->AddResource(create_info, nearest_sampler);
			}

			uint32_t zbuffer_depth_target{ ~0u };
			{
				gfx::TextureCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth();
				create_info.height = backbuffer_texture->GetHeight();
				create_info.format = gfx::Format::eD32Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = true;

				zbuffer_depth_target = render_graph->AddResource(create_info, nearest_sampler);
			}

			// Passes
			// Generate brdflut
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:brgflut_generator";
				create_info.one_time = true;

				create_info.draw_callback = [this, 
					brdf_lut_target,
					out_color_bnd
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						self.WriteResource(brdf_lut_target);
						self.ResolveDependencies(command_list);

						self.BindResource(out_color_bnd, brdf_lut_target);

						self.BindResources(command_list);
				
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

				create_info.draw_callback = [this, 
					irradiance_cube_target, 
					casual_day_4k_cube,
					sampler_color_bnd,
					out_color_bnd
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						self.WriteResource(irradiance_cube_target);
						self.ResolveDependencies(command_list);

						self.BindResource(sampler_color_bnd, casual_day_4k_cube);
						self.BindResource(out_color_bnd, irradiance_cube_target);

						self.BindResources(command_list);
				
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

				create_info.draw_callback = [this,
					prefiltered_cube_target, 
					casual_day_4k_cube,
					sampler_color_bnd,
					out_color_bnd,
					roughness_bnd
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						auto target = render_graph->GetTexture(prefiltered_cube_target);
						for (int32_t mip = 0; mip < target->GetMipLevelCount(); ++mip)
						{
							self.WriteResource(prefiltered_cube_target, mip);

							if (mip == 0)
							{
								self.BindResource(sampler_color_bnd, casual_day_4k_cube);
							}
							else
							{
								self.ReadResource(prefiltered_cube_target, mip - 1);
								self.BindResource(sampler_color_bnd, prefiltered_cube_target, mip - 1);
							}

							self.BindResource(out_color_bnd, prefiltered_cube_target, mip);

							self.ResolveDependencies(command_list);
							self.BindResources(command_list);

							uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

							float layer_roughness = static_cast<float>(mip) / static_cast<float>(target->GetMipLevelCount());

							auto& constants = self.GetConstantBlock();
							constants.Set(roughness_bnd, layer_roughness);
							self.PushConstantRanges(command_list);

							group_x = target->GetWidth() >> mip;
							group_y = target->GetHeight() >> mip;
							
							self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
							command_list.Dispatch(group_x, group_y, group_z);
						}
					};

				static_cast<GroupNode*>(group_node.get())->AddNode(std::move(render_graph->CreateNode(create_info)));

				render_graph->AddNode(std::move(group_node));
			}

			// Geometry draw
			{
				RenderShaderPassCreateInfo create_info{};
				create_info.shader_name = "visibilitybuffer:write";
				create_info.AddWriteAttachment(visbuffer_geometry_target, ResourceAttachmentTypeFlag::eColor, gfx::ClearColor{ 0.f, 0.f, 0.f, 1.f });
				create_info.AddWriteAttachment(visbuffer_vboinst_target, ResourceAttachmentTypeFlag::eColor, gfx::ClearColor{ 0.f, 0.f, 0.f, 1.f });
				create_info.AddWriteAttachment(zbuffer_depth_target, ResourceAttachmentTypeFlag::eDepth, gfx::ClearColor{ 1.f });

				create_info.pipeline_state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
				create_info.pipeline_state.color_blending_state.attachment_count = 1;

				create_info.draw_callback = [this](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{

					};

				//render_graph->AddNode(render_graph->CreateNode(create_info));
			}

			// Test output shader
			{
				RenderShaderPassCreateInfo create_info{};
				create_info.shader_name = "testing:screenspace";
				create_info.AddWriteAttachment(backbuffer_resource, ResourceAttachmentTypeFlag::eColor, gfx::ClearColor{ 1.f, 0.5f, 1.f, 1.f });

				create_info.pipeline_state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
				create_info.pipeline_state.color_blending_state.attachment_count = 1;
			
				create_info.draw_callback = [this, 
					irradiance_cube_target, 
					brdf_lut_target,
					input_image_bnd = "input_image"_hash32
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						auto* backbuffer = render_graph->GetTexture(backbuffer_resource);
			
						auto viewport_width = backbuffer->GetWidth();
						auto viewport_height = backbuffer->GetHeight();
			
						command_list.SetViewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
						command_list.SerScissor(0u, 0u, viewport_width, viewport_height);

						self.ReadResource(brdf_lut_target);
						self.ResolveDependencies(command_list);
			
						self.BindResource(input_image_bnd, brdf_lut_target);			
						self.BindResources(command_list);
						
						self.Begin(command_list);
						command_list.Draw(3u, 0u, 0u, 1u);
						self.End(command_list);
					};
			
				render_graph->AddNode(render_graph->CreateNode(create_info));
			}

			{
				if (ImGui::GetCurrentContext())
				{
					// Create font texture
					unsigned char* pixels;
					int width, height;

					ImGuiIO& io = ImGui::GetIO();
					io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
					uint64_t upload_size = width * height * 4ull * sizeof(char);

					gfx::TextureCreateInfo font_texture_ci{};
					font_texture_ci.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eTransferable;
					font_texture_ci.width = width;
					font_texture_ci.height = height;
					font_texture_ci.format = gfx::Format::eR8G8B8A8Unorm;

					uint32_t imgui_font_texture = render_graph->AddResource(font_texture_ci, {pixels, upload_size});

					GeometryBufferCreateInfo geometry_buffer_ci{};
					geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::ePosition, gfx::Format::eR32G32Sfloat);
					geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::eTexCoord, gfx::Format::eR32G32Sfloat);
					geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::eColor, gfx::Format::eR8G8B8A8Unorm);
					geometry_buffer_ci.index_description.type = gfx::IndexType::eUint16;

					auto& geometry_buffer = RequestInterleavedGeometryBuffer(geometry_buffer_ci);

					RenderShaderPassCreateInfo create_info{};
					create_info.shader_name = "testing:imgui";
					create_info.pipeline_state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
					create_info.pipeline_state.vertex_layout = geometry_buffer.GetVertexLayout();
					create_info.AddWriteAttachment(backbuffer_resource, ResourceAttachmentTypeFlag::eColor, gfx::ClearColor{ 0.f, 0.f, 0.f, 1.f });

					create_info.pipeline_state.color_blending_state.attachment_count = 1;
					auto& blending = create_info.pipeline_state.color_blending_state.attachments[0];
					blending.enable = true;
					blending.src_color = gfx::BlendFactor::eSrcAlpha;
					blending.dst_color = gfx::BlendFactor::eOneMinusSrcAlpha;
					blending.src_alpha = gfx::BlendFactor::eOneMinusSrcAlpha;
					blending.dst_alpha = gfx::BlendFactor::eZero;

					create_info.draw_callback = [this, 
						&geometry_buffer,
						imgui_font_texture
					](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
						{
							ImGui::Render();

							// Update geometry
							ImDrawData* draw_data = ImGui::GetDrawData();
							if (draw_data && (draw_data->TotalVtxCount != 0) && (draw_data->TotalIdxCount != 0))
							{
								uint32_t total_vertices{ 0u };
								uint32_t total_indices{ 0u };

								for (int32_t i = 0; i < draw_data->CmdListsCount; i++)
								{
									const ImDrawList* cmd_list = draw_data->CmdLists[i];
									geometry_buffer.AddVertexStreamData(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, sizeof(ImDrawVert));
									geometry_buffer.AddIndexStreamData(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, sizeof(ImDrawIdx));

									total_vertices += cmd_list->VtxBuffer.Size;
									total_indices += cmd_list->IdxBuffer.Size;
								}

								// Upload geometry to gpu
								geometry_buffer.Flush();

								// Bind vertex and index buffers
								geometry_buffer.Bind(command_list);

								auto* backbuffer = render_graph->GetTexture(backbuffer_resource);

								auto viewport_width = backbuffer->GetWidth();
								auto viewport_height = backbuffer->GetHeight();

								command_list.SetViewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);

								// Setup scale and translation:
								// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
								{
									auto& constants = self.GetConstantBlock();

									glm::vec2 scale;
									scale.x = 2.f / draw_data->DisplaySize.x;
									scale.y = 2.f / draw_data->DisplaySize.y;

									constants.Set("scale"_hash32, scale);

									glm::vec2 translate;
									translate.x = -1.0f - draw_data->DisplayPos.x * scale.x;
									translate.y = -1.0f - draw_data->DisplayPos.y * scale.y;

									constants.Set("translate"_hash32, translate);

									self.PushConstantRanges(command_list);
								}

								// Will project scissor/clipping rectangles into framebuffer space
								ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
								ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

								// Collect resource dependencies
								for (i32 n = 0; n < draw_data->CmdListsCount; n++)
								{
									const ImDrawList* cmd_list = draw_data->CmdLists[n];
									for (i32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
									{
										const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

										uint32_t resource_binding{ (uint32_t)pcmd->TextureId };
										if (sizeof(ImTextureID) < sizeof(ImU64))
										{
											// We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
											IM_ASSERT(pcmd->TextureId == (ImTextureID)imgui_font_texture);
											resource_binding = imgui_font_texture;
										}

										if (!pcmd->TextureId)
										{
											resource_binding = imgui_font_texture;
										}

										self.ReadResource(resource_binding);
									}
								}

								self.ResolveDependencies(command_list);

								// Render gui
								uint32_t last_bound_resource{ ~0u };

								self.Begin(command_list);

								// Render command lists
								// (Because we merged all buffers into a single one, we maintain our own offset into them)
								i32 global_vtx_offset = 0;
								i32 global_idx_offset = 0;
								for (i32 n = 0; n < draw_data->CmdListsCount; n++)
								{
									const ImDrawList* cmd_list = draw_data->CmdLists[n];
									for (i32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
									{
										const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
										if (pcmd->UserCallback != nullptr)
										{
											// User callback, registered via ImDrawList::AddCallback()
											// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
											//if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
											//	ImGui_ImplVulkan_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width, fb_height);
											//else
											pcmd->UserCallback(cmd_list, pcmd);
										}
										else
										{
											// Project scissor/clipping rectangles into framebuffer space
											ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
											ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

											// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
											if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
											if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
											if (clip_max.x > viewport_width) { clip_max.x = viewport_width; }
											if (clip_max.y > viewport_height) { clip_max.y = viewport_height; }
											if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
												continue;

											// Apply scissor/clipping rectangle
											command_list.SerScissor((int32_t)(clip_min.x), (int32_t)(clip_min.y), (uint32_t)(clip_max.x - clip_min.x), (uint32_t)(clip_max.y - clip_min.y));

											// Bind DescriptorSet with font or user texture
											uint32_t resource_binding{ (uint32_t)pcmd->TextureId };
											if (sizeof(ImTextureID) < sizeof(ImU64))
											{
												// We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
												IM_ASSERT(pcmd->TextureId == (ImTextureID)imgui_font_texture);
												resource_binding = imgui_font_texture;
											}
											
											if (!pcmd->TextureId)
												resource_binding = imgui_font_texture;
											
											if (resource_binding != last_bound_resource)
											{
												self.BindResource("font_sampler"_hash32, resource_binding);
												self.BindResources(command_list);
												last_bound_resource = resource_binding;
											}

											command_list.DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, 1u, pcmd->VtxOffset + global_vtx_offset, 0u);
										}
									}
									global_idx_offset += cmd_list->IdxBuffer.Size;
									global_vtx_offset += cmd_list->VtxBuffer.Size;
								}

								self.End(command_list);
							}
						};

					render_graph->AddNode(render_graph->CreateNode(create_info));
				}
			}
		}
	}
}