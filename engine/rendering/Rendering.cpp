#include <engine/rendering/Rendering.h>

#include <engine/rendering/gfx/Context.h>

#include <engine/rendering/RenderContext.h>
#include <engine/rendering/UploadContext.h>

#include <engine/rendering/render_graph/ComputePass.h>
#include <engine/rendering/render_graph/RenderPass.h>

#include <engine/rendering/GeometryBuffer.h>

#include <engine/rendering/render_graph/RenderGraph.h>
#include <engine/rendering/render_world/RenderWorld.h>

#include <embed/shaders/includes/shlib.h>

#include <util/Logger.h>

#include "Globals.h"

#include <embed/shaders/includes/culling.h>

#include <engine/world/EntityManager.h>

#include <imgui.h>
#include <tracy/Tracy.hpp>

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

#define SWAP_COUNT(in, out) std::swap(in, out); ++pass_count;

namespace render_label
{
	using namespace utl;

	constexpr const uint32_t write_image0 = "write_image0"_hash32;
	constexpr const uint32_t write_image1 = "write_image1"_hash32;
	constexpr const uint32_t write_image2 = "write_image2"_hash32;
	constexpr const uint32_t write_image3 = "write_image3"_hash32;
	constexpr const uint32_t write_image4 = "write_image4"_hash32;
	constexpr const uint32_t write_image5 = "write_image5"_hash32;
	constexpr const uint32_t write_image6 = "write_image6"_hash32;
	constexpr const uint32_t write_image7 = "write_image7"_hash32;

	constexpr const uint32_t sample_image0 = "sample_image0"_hash32;
	constexpr const uint32_t sample_image1 = "sample_image1"_hash32;
	constexpr const uint32_t sample_image2 = "sample_image2"_hash32;
	constexpr const uint32_t sample_image3 = "sample_image3"_hash32;
	constexpr const uint32_t sample_image4 = "sample_image4"_hash32;
	constexpr const uint32_t sample_image5 = "sample_image5"_hash32;
	constexpr const uint32_t sample_image6 = "sample_image6"_hash32;
	constexpr const uint32_t sample_image7 = "sample_image7"_hash32;
	constexpr const uint32_t sample_image8 = "sample_image8"_hash32;
	constexpr const uint32_t sample_image9 = "sample_image9"_hash32;
	constexpr const uint32_t sample_image10 = "sample_image10"_hash32;
	constexpr const uint32_t sample_image11 = "sample_image11"_hash32;

	constexpr const uint32_t roughness = "roughness"_hash32;

	constexpr const uint32_t position = "position"_hash32;
	constexpr const uint32_t color = "color"_hash32;
	constexpr const uint32_t texcoord = "texcoord"_hash32;
	constexpr const uint32_t normal = "normal"_hash32;
	constexpr const uint32_t tangent = "tangent"_hash32;
	constexpr const uint32_t joints = "joints"_hash32;
	constexpr const uint32_t weights = "weights"_hash32;

	constexpr const uint32_t indices = "indices"_hash32;

	constexpr const uint32_t call_id = "call_id"_hash32;
	constexpr const uint32_t shadow_index = "shadow_index"_hash32;
	constexpr const uint32_t layer = "layer"_hash32;

	constexpr const uint32_t camera_id = "camera_id"_hash32;

	constexpr const uint32_t geometry_buffers = "GeometryBuffers"_hash32;


	constexpr const uint32_t histogram_buffer = "Histogram"_hash32;
	constexpr const uint32_t histogram_compute_params = "FHistogramComputeParams"_hash32;
	constexpr const uint32_t min_log_lum = "minLogLum"_hash32;
	constexpr const uint32_t log_lum_range = "logLumRange"_hash32;
	constexpr const uint32_t time_coeff = "logLumRange"_hash32;
	constexpr const uint32_t num_pixels_x = "numPixelsX"_hash32;
	constexpr const uint32_t num_pixels_y = "numPixelsY"_hash32;
}

namespace engine
{
	namespace render
	{
		PostProcessData postprocess{};

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
			render_world = std::make_unique<RenderWorld>(*this);

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

		uint32_t Renderer::AddRenderResource(const TextureResourceCreateInfo& create_info, gfx::Sampler* sampler)
		{
			return render_graph->AddResource(create_info, sampler);
		}

		uint32_t Renderer::AddRenderResource(const gfx::BufferCreateInfo& create_info)
		{
			return render_graph->AddResource(create_info);
		}

		void Renderer::AddMaterial(const shlib::Material& material)
		{
			// "Patch" material before add

			shlib::Material new_material{ material };

			if (new_material.baseColorTexture != ~0u)
			{
				auto& resource = render_graph->GetResource(new_material.baseColorTexture);
				new_material.baseColorTexture = resource.resource_id;
			}

			if (new_material.metallicRoughnessTexture != ~0u)
			{
				auto& resource = render_graph->GetResource(new_material.metallicRoughnessTexture);
				new_material.metallicRoughnessTexture = resource.resource_id;
			}

			if (new_material.normalTexture != ~0u)
			{
				auto& resource = render_graph->GetResource(new_material.normalTexture);
				new_material.normalTexture = resource.resource_id;
			}

			if (new_material.occlusionTexture != ~0u)
			{
				auto& resource = render_graph->GetResource(new_material.occlusionTexture);
				new_material.occlusionTexture = resource.resource_id;
			}

			if (new_material.emissiveTexture != ~0u)
			{
				auto& resource = render_graph->GetResource(new_material.emissiveTexture);
				new_material.emissiveTexture = resource.resource_id;
			}

			render_world->AddMaterial(new_material);
		}

		void Renderer::Render(float delta_time)
		{
			ZoneScoped;

			// Update geometry buffers
			for (auto& geometry_buffer : geometry_buffers)
			{
				geometry_buffer->Flush();
			}

			render_graph->BeginFrame();
			render_world->BeginFrame(delta_time);

			render_graph->Evaluate(delta_time);

			render_world->EndFrame();
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


		IRenderGraph& Renderer::GetRenderGraph()
		{
			return *render_graph;
		}

		IRenderGraph const& Renderer::GetRenderGraph() const
		{
			return *render_graph;
		}

		IRenderWorld& Renderer::GetRenderWorld()
		{
			return *render_world;
		}

		IRenderWorld const& Renderer::GetRenderWorld() const
		{
			return *render_world;
		}


		void Renderer::BuildGraph()
		{
			using namespace utl;

			static bool use_low_precision_velocity{ false };
			static bool bloom_use_brightness{ true };
			static uint32_t bloom_blur_depth = 5;

			// Register backbuffer resource

			backbuffer_resource = render_graph->AddResource();

			auto& backbuffer_resource_binding = render_graph->GetResource(backbuffer_resource);
			backbuffer_resource_binding.resource_type = RenderResourceTypeFlag::eBackbuffer;

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
				TextureResourceCreateInfo create_info{};
				create_info.width = create_info.height = 1024u;
				create_info.format = gfx::Format::eR16G16Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage;
				create_info.resizable = false;

				brdf_lut_target = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(brdf_lut_target, "brdf_lut_target");
			}

			uint32_t irradiance_cube_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = create_info.height = 64u;
				create_info.format = gfx::Format::eR32G32B32A32Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage | gfx::TextureUsageFlag::eCubemap;
				create_info.resizable = false;

				irradiance_cube_target = render_graph->AddResource(create_info, linear_sampler);
				render_graph->SetResourceName(irradiance_cube_target, "irradiance_cube_target");
			}

			uint32_t prefiltered_cube_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
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
				render_graph->SetResourceName(prefiltered_cube_target, "prefiltered_cube_target");
			}

			// Shadow targets
			uint32_t cascaded_shadows_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = create_info.height = shlib::cascade_shadow_map_resolution;
				create_info.layers = shlib::cascade_shadow_map_cascade_count * shlib::max_directional_light_shadow_count;
				create_info.format = gfx::Format::eD32Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = false;
				
				cascaded_shadows_target = render_graph->AddResource(create_info, shadow_sampler);
				render_graph->SetResourceName(cascaded_shadows_target, "cascaded_shadows_target");
			}

			uint32_t directional_shadows_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = create_info.height = shlib::spot_light_shadow_map_resolution;
				create_info.layers = shlib::max_spot_light_shadow_count;
				create_info.format = gfx::Format::eD16Unorm;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = false;

				directional_shadows_target = render_graph->AddResource(create_info, shadow_sampler);
				render_graph->SetResourceName(directional_shadows_target, "directional_shadows_target");
			}

			uint32_t omni_shadows_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = create_info.height = shlib::point_light_shadow_map_resolution;
				create_info.layers = shlib::max_point_light_shadow_count;
				create_info.format = gfx::Format::eD16Unorm;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget | gfx::TextureUsageFlag::eCubemap;
				create_info.resizable = false;

				omni_shadows_target = render_graph->AddResource(create_info, shadow_sampler);
				render_graph->SetResourceName(omni_shadows_target, "omni_shadows_target");
			}

			auto* backbuffer_texture = render_graph->GetTexture(backbuffer_resource);

			// Visibility buffer targets
			uint32_t visbuffer_geometry_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth();
				create_info.height = backbuffer_texture->GetHeight();
				create_info.format = gfx::Format::eR8G8B8A8Unorm;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = true;

				visbuffer_geometry_target = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(visbuffer_geometry_target, "visbuffer_geometry_target");
			}

			uint32_t visbuffer_vboinst_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth();
				create_info.height = backbuffer_texture->GetHeight();
				create_info.format = gfx::Format::eR8G8B8A8Unorm;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = true;

				visbuffer_vboinst_target = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(visbuffer_vboinst_target, "visbuffer_vboinst_target");
			}

			uint32_t zbuffer_depth_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth();
				create_info.height = backbuffer_texture->GetHeight();
				create_info.format = gfx::Format::eD32Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eRenderTarget;
				create_info.resizable = true;

				zbuffer_depth_target = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(zbuffer_depth_target, "zbuffer_depth_target");
			}

			uint32_t motion_vectors_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth();
				create_info.height = backbuffer_texture->GetHeight();
				create_info.format = use_low_precision_velocity ? gfx::Format::eR8G8Unorm : gfx::Format::eR16G16Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage;
				create_info.resizable = true;

				motion_vectors_target = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(motion_vectors_target, "motion_vectors_target");
			}

			uint32_t hdr0{ ~0u };
			uint32_t hdr1{ ~0u };
			uint32_t hdr0_history{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth();
				create_info.height = backbuffer_texture->GetHeight();
				create_info.format = gfx::Format::eB10G11R11UfloatPack32;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage;
				create_info.resizable = true;

				hdr0 = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(hdr0, "hdr0");

				hdr1 = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(hdr1, "hdr1");

				hdr0_history = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(hdr0_history, "hdr0_history");
			}

			uint32_t hdr0_div2{ ~0u };
			{
				// TODO: add support for half size targets
				TextureResourceCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth() / 2;
				create_info.height = backbuffer_texture->GetHeight() / 2;
				create_info.format = gfx::Format::eB10G11R11UfloatPack32;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage;
				create_info.resizable = true;
				create_info.scale = 0.5f;

				hdr0_div2 = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(hdr0_div2, "hdr0_div2");
			}

			uint32_t bloom_target{ ~0u };
			{
				// TODO: add support for half size targets
				TextureResourceCreateInfo create_info{};
				create_info.width = backbuffer_texture->GetWidth() / 2;
				create_info.height = backbuffer_texture->GetHeight() / 2;
				create_info.levels = bloom_blur_depth;
				create_info.format = gfx::Format::eB10G11R11UfloatPack32;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage;
				create_info.resizable = true;
				create_info.scale = 0.5f;

				bloom_target = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(bloom_target, "bloom_target");
			}

			// Eye adaptation targets
			uint32_t avg_brightness_target{ ~0u };
			{
				TextureResourceCreateInfo create_info{};
				create_info.width = 1u;
				create_info.height = 1u;
				create_info.format = gfx::Format::eR32Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage;
				create_info.resizable = false;

				avg_brightness_target = render_graph->AddResource(create_info, nearest_sampler);
				render_graph->SetResourceName(avg_brightness_target, "avg_brightness_target");
			}

			uint32_t histogram_buffer{ ~0u };
			{
				gfx::BufferCreateInfo create_info{};
				create_info.size = sizeof(uint32_t) * 256u;
				create_info.usage = gfx::BufferUsageFlag::eStorageBuffer;

				histogram_buffer = render_graph->AddResource(create_info);
				render_graph->SetResourceName(histogram_buffer, "histogram_buffer");
			}

			// Buffer that contains alive comands for draw
			uint32_t alive_commands_buffer{ ~0u };
			{
				gfx::BufferCreateInfo create_info{};
				create_info.size = sizeof(shlib::DrawDescription) * shlib::MAX_DRAW_INSTANCE_COUNT;
				create_info.usage = gfx::BufferUsageFlag::eStorageBuffer;

				alive_commands_buffer = render_graph->AddResource(create_info);
				render_graph->SetResourceName(alive_commands_buffer, "alive_commands_buffer");
			}

			// Buffer contains draw indirect commands
			uint32_t indirect_draw_commands{ ~0u };
			{
				gfx::BufferCreateInfo create_info{};
				create_info.size = sizeof(shlib::DrawIndexedIndirectData) * shlib::MAX_DRAW_INSTANCE_COUNT;
				create_info.usage = gfx::BufferUsageFlag::eIndirectBuffer;

				indirect_draw_commands = render_graph->AddResource(create_info);
				render_graph->SetResourceName(indirect_draw_commands, "indirect_draw_commands");
			}

			uint32_t indirect_count_buffer{ ~0u };
			{
				gfx::BufferCreateInfo create_info{};
				create_info.size = 1024u;
				create_info.usage = gfx::BufferUsageFlag::eIndirectBuffer;

				indirect_count_buffer = render_graph->AddResource(create_info);
				render_graph->SetResourceName(indirect_count_buffer, "indirect_count_buffer");
			}

			// Passes
			// Generate brdflut
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:brgflut_generator";
				create_info.one_time = true;

				create_info.draw_callback = [this, 
					brdf_lut_target
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;

						self.BindWriteResource(render_label::write_image0, brdf_lut_target, ~0u);

						self.ResolveDependencies(command_list);
						self.BindResources(command_list);

						self.DispatchForTarget(command_list, brdf_lut_target, true);
						self.FinalizeTarget(command_list, render_label::write_image0, brdf_lut_target);
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
					casual_day_4k_cube
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;

						self.BindWriteResource(render_label::write_image0, irradiance_cube_target, ~0u);

						self.BindResource(render_label::sample_image0, casual_day_4k_cube, ~0u);
						
						self.ResolveDependencies(command_list);
						self.BindResources(command_list);

						self.DispatchForTarget(command_list, irradiance_cube_target, true);

						self.FinalizeTarget(command_list, render_label::write_image0, irradiance_cube_target);
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
					casual_day_4k_cube
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;

						self.WriteResource(render_label::write_image0, prefiltered_cube_target, ~0u);

						auto target = render_graph->GetTexture(prefiltered_cube_target);
						for (int32_t mip = 0; mip < target->GetMipLevelCount(); ++mip)
						{
							if (mip == 0)
							{
								self.BindResource(render_label::sample_image0, casual_day_4k_cube, ~0u);
							}
							else
							{
								self.BindReadResource(render_label::sample_image0, prefiltered_cube_target, mip - 1);
							}

							self.BindWriteResource(render_label::write_image0, prefiltered_cube_target, mip);

							self.ResolveDependencies(command_list);
							self.BindResources(command_list);

							uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

							float layer_roughness = static_cast<float>(mip) / static_cast<float>(target->GetMipLevelCount());

							auto& constants = self.GetConstantBlock();
							constants.Set(render_label::roughness, layer_roughness);
							self.PushConstantRanges(command_list);

							group_x = target->GetWidth() >> mip;
							group_y = target->GetHeight() >> mip;
							
							self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
							command_list.Dispatch(group_x, group_y, group_z);
						}

						self.FinalizeTarget(command_list, render_label::sample_image0, prefiltered_cube_target);
					};

				static_cast<GroupNode*>(group_node.get())->AddNode(std::move(render_graph->CreateNode(create_info)));

				render_graph->AddNode(std::move(group_node));
			}

			// Crear cull counters
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "culling:clear_counters";

				create_info.draw_callback = [this,
					indirect_count_buffer
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;

						auto* count_buffer = render_graph->GetBuffer(indirect_count_buffer);

						self.WriteResource(indirect_count_buffer, ~0u);

						self.ResolveDependencies(command_list);

						auto& draw_data_block = self.GetUniformBlock(shlib::CullingInfo_label);
						auto& draw_desc_buffers = draw_data_block.As<shlib::CullingInfo>();
						draw_desc_buffers.draw_counts = count_buffer->GetAddress();

						self.BindResources(command_list);

						uint32_t group_x{ 512u }, group_y{ 1u }, group_z{ 1u };

						self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
						command_list.Dispatch(group_x, group_y, group_z);
					};

				render_graph->AddNode(render_graph->CreateNode(create_info));
			}

			// Cull meshes
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "culling:cull_and_batch";

				create_info.draw_callback = [this,
					alive_commands_buffer,
					indirect_draw_commands,
					indirect_count_buffer
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;

						auto* draw_commands = render_world->GetDrawBuffer();
						auto* alive_meshes = render_graph->GetBuffer(alive_commands_buffer);
						auto* argument_buffer = render_graph->GetBuffer(indirect_draw_commands);
						auto* count_buffer = render_graph->GetBuffer(indirect_count_buffer);

						self.WriteResource(alive_commands_buffer, ~0u);
						self.WriteResource(indirect_draw_commands, ~0u);
						self.WriteResource(indirect_count_buffer, ~0u);

						self.ResolveDependencies(command_list);

						auto& draw_data_block = self.GetUniformBlock(shlib::CullingInfo_label);
						auto& draw_desc_buffers = draw_data_block.As<shlib::CullingInfo>();
						draw_desc_buffers.source_descriptions = draw_commands->GetAddress();
						draw_desc_buffers.alive_descriptions = alive_meshes->GetAddress();
						draw_desc_buffers.alive_draw_commands = argument_buffer->GetAddress();
						draw_desc_buffers.draw_counts = count_buffer->GetAddress();

						self.BindResources(command_list);

						uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };
						group_x = render_world->GetDrawCount() * render_world->GetCameraCount();

						self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
						command_list.Dispatch(group_x, group_y, group_z);
					};

				render_graph->AddNode(render_graph->CreateNode(create_info));
			}

			// Draw cascaded shadows
			{
				RenderShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:unidirectional_shadow:CASCADED_SHADOW:HAS_TEXCOORD:HAS_NORMAL:TANGENT_VEC4:INDEX_16BIT";
				create_info.pipeline_state.rasterization_state.cull_mode = gfx::CullModeFlag::eFront;
			
				// Depth state
				create_info.pipeline_state.depth_stencil_state.depth_test = true;
				create_info.pipeline_state.depth_stencil_state.depth_write = true;
				create_info.pipeline_state.depth_stencil_state.depth_compare_op = gfx::CompareOp::eLess;
			
				create_info.AddWriteAttachment(cascaded_shadows_target, ResourceAttachmentTypeFlag::eDepth, gfx::ClearColor{ 1.f });
			
				create_info.pipeline_state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
			
				create_info.draw_callback = [this,
					cascaded_shadows_target,
					alive_commands_buffer,
					indirect_draw_commands,
					indirect_count_buffer
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;
			
						auto* shadow_map = render_graph->GetTexture(cascaded_shadows_target);
						auto* alive_meshes = render_graph->GetBuffer(alive_commands_buffer);
						auto* argument_buffer = render_graph->GetBuffer(indirect_draw_commands);
						auto* count_buffer = render_graph->GetBuffer(indirect_count_buffer);
			
						auto viewport_width = shadow_map->GetWidth();
						auto viewport_height = shadow_map->GetHeight();
			
						command_list.SetViewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
						command_list.SerScissor(0u, 0u, viewport_width, viewport_height);
			
						self.ReadResource(alive_commands_buffer, ~0u);
						self.ReadResource(indirect_draw_commands, ~0u);
						self.ReadResource(indirect_count_buffer, ~0u);
			
						self.ResolveDependencies(command_list);
			
						auto& geometry_buffer = *geometry_buffers.back();
						geometry_buffer.BindIndexBuffer(command_list);
			
						auto& buffers = self.GetUniformBlock(render_label::geometry_buffers);
						buffers.Set(render_label::position, geometry_buffer.GetBufferAddress(VertexAttributeFlag::ePosition));
						buffers.Set(render_label::texcoord, geometry_buffer.GetBufferAddress(VertexAttributeFlag::eTexCoord));
			
						// Bind buffers that uses in draw
						auto& draw_data_block = self.GetUniformBlock(shlib::DrawDescriptionBuffers_label);
						auto& draw_desc_buffers = draw_data_block.As<shlib::DrawDescriptionBuffers>();
						draw_desc_buffers.draw_descriptions_ptr = alive_meshes->GetAddress();
						draw_desc_buffers.draw_commands_ptr = argument_buffer->GetAddress();
			
						auto& constants = self.GetConstantBlock();
			
						self.BindResources(command_list);
			
						gfx::IndirectDrawCommand draw_command{};
						draw_command.indexed_draw = true;
						draw_command.draw_count = render_world->GetDrawCount();
						draw_command.argument_buffer = argument_buffer;
						draw_command.command_stride = sizeof(shlib::DrawIndexedIndirectData);
						draw_command.count_buffer = count_buffer;
			
						uint32_t draw_call_id{ 0u };
						self.Begin(command_list);
			
						int32_t shadow_index{ 0 };
			
						auto view = world::EntityManager::View<world::ecs::FDirectionalLightComponent>();
						for (auto [entity, light] : view.each())
						{
							if (!light.castShadows)
								continue;
			
							for (int32_t layer = 0; layer < shlib::cascade_shadow_map_cascade_count; ++layer)
							{
								draw_call_id = light.shadow_casters[layer] * render_world->GetDrawCount();

								constants.Set(render_label::call_id, draw_call_id);
								constants.Set(render_label::shadow_index, shadow_index);
								constants.Set(render_label::layer, layer);
			
								self.PushConstantRanges(command_list);
			
								draw_command.argument_offset = light.shadow_casters[layer] * draw_command.command_stride;
								draw_command.count_offset = light.shadow_casters[layer] * sizeof(uint32_t);
			
								command_list.DrawIndirect(draw_command);
							}
			
							shadow_index++;
						}
			
						self.End(command_list);
					};
			
				render_graph->AddNode(render_graph->CreateNode(create_info));
			}
			
			// Draw omni shadows
			{
				RenderShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:unidirectional_shadow:OMNI_SHADOW:HAS_TEXCOORD:HAS_NORMAL:TANGENT_VEC4:INDEX_16BIT";
				create_info.pipeline_state.rasterization_state.cull_mode = gfx::CullModeFlag::eFront;
			
				// Depth state
				create_info.pipeline_state.depth_stencil_state.depth_test = true;
				create_info.pipeline_state.depth_stencil_state.depth_write = true;
				create_info.pipeline_state.depth_stencil_state.depth_compare_op = gfx::CompareOp::eLess;
			
				create_info.AddWriteAttachment(omni_shadows_target, ResourceAttachmentTypeFlag::eDepth, gfx::ClearColor{ 1.f });
			
				create_info.pipeline_state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
			
				create_info.draw_callback = [this,
					omni_shadows_target,
					alive_commands_buffer,
					indirect_draw_commands,
					indirect_count_buffer
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;
			
						auto* shadow_map = render_graph->GetTexture(omni_shadows_target);
						auto* alive_meshes = render_graph->GetBuffer(alive_commands_buffer);
						auto* argument_buffer = render_graph->GetBuffer(indirect_draw_commands);
						auto* count_buffer = render_graph->GetBuffer(indirect_count_buffer);
			
						auto viewport_width = shadow_map->GetWidth();
						auto viewport_height = shadow_map->GetHeight();
			
						command_list.SetViewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
						command_list.SerScissor(0u, 0u, viewport_width, viewport_height);
			
						self.ReadResource(alive_commands_buffer, ~0u);
						self.ReadResource(indirect_draw_commands, ~0u);
						self.ReadResource(indirect_count_buffer, ~0u);
			
						self.ResolveDependencies(command_list);
			
						auto& geometry_buffer = *geometry_buffers.back();
						geometry_buffer.BindIndexBuffer(command_list);
			
						auto& buffers = self.GetUniformBlock(render_label::geometry_buffers);
						buffers.Set(render_label::position, geometry_buffer.GetBufferAddress(VertexAttributeFlag::ePosition));
						buffers.Set(render_label::texcoord, geometry_buffer.GetBufferAddress(VertexAttributeFlag::eTexCoord));
			
						// Bind buffers that uses in draw
						auto& draw_data_block = self.GetUniformBlock(shlib::DrawDescriptionBuffers_label);
						auto& draw_desc_buffers = draw_data_block.As<shlib::DrawDescriptionBuffers>();
						draw_desc_buffers.draw_descriptions_ptr = alive_meshes->GetAddress();
						draw_desc_buffers.draw_commands_ptr = argument_buffer->GetAddress();
			
						auto& constants = self.GetConstantBlock();
			
						self.BindResources(command_list);
			
						gfx::IndirectDrawCommand draw_command{};
						draw_command.indexed_draw = true;
						draw_command.draw_count = render_world->GetDrawCount();
						draw_command.argument_buffer = render_graph->GetBuffer(indirect_draw_commands);
						draw_command.command_stride = sizeof(shlib::DrawIndexedIndirectData);
						draw_command.count_buffer = render_graph->GetBuffer(indirect_count_buffer);
			
						uint32_t draw_call_id{ 0u };
						self.Begin(command_list);
			
						int32_t shadow_index{ 0 };
			
						auto view = world::EntityManager::View<world::ecs::FPointLightComponent>();
						for (auto [entity, light] : view.each())
						{
							if (!light.castShadows)
								continue;
			
							for (int32_t layer = 0; layer < 6; ++layer)
							{
								draw_call_id = light.shadow_casters[layer] * render_world->GetDrawCount();

								constants.Set(render_label::call_id, draw_call_id);
								constants.Set(render_label::shadow_index, shadow_index);
								constants.Set(render_label::layer, layer);
			
								self.PushConstantRanges(command_list);
			
								draw_command.argument_offset = light.shadow_casters[layer] * draw_command.command_stride;
								draw_command.count_offset = light.shadow_casters[layer] * sizeof(uint32_t);
			
								command_list.DrawIndirect(draw_command);
							}
			
							shadow_index++;
						}
			
						self.End(command_list);
					};
			
				render_graph->AddNode(render_graph->CreateNode(create_info));
			}
			
			// Draw spot shadows
			{
				RenderShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:unidirectional_shadow:HAS_TEXCOORD:HAS_NORMAL:TANGENT_VEC4:INDEX_16BIT";
				create_info.pipeline_state.rasterization_state.cull_mode = gfx::CullModeFlag::eFront;
			
				// Depth state
				create_info.pipeline_state.depth_stencil_state.depth_test = true;
				create_info.pipeline_state.depth_stencil_state.depth_write = true;
				create_info.pipeline_state.depth_stencil_state.depth_compare_op = gfx::CompareOp::eLess;
			
				create_info.AddWriteAttachment(omni_shadows_target, ResourceAttachmentTypeFlag::eDepth, gfx::ClearColor{ 1.f });
			
				create_info.pipeline_state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
			
				create_info.draw_callback = [this,
					directional_shadows_target,
					alive_commands_buffer,
					indirect_draw_commands,
					indirect_count_buffer
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;
			
						auto* shadow_map = render_graph->GetTexture(directional_shadows_target);
						auto* alive_meshes = render_graph->GetBuffer(alive_commands_buffer);
						auto* argument_buffer = render_graph->GetBuffer(indirect_draw_commands);
						auto* count_buffer = render_graph->GetBuffer(indirect_count_buffer);
			
						auto viewport_width = shadow_map->GetWidth();
						auto viewport_height = shadow_map->GetHeight();
			
						command_list.SetViewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
						command_list.SerScissor(0u, 0u, viewport_width, viewport_height);
			
						self.ReadResource(alive_commands_buffer, ~0u);
						self.ReadResource(indirect_draw_commands, ~0u);
						self.ReadResource(indirect_count_buffer, ~0u);
			
						self.ResolveDependencies(command_list);
			
						auto& geometry_buffer = *geometry_buffers.back();
						geometry_buffer.BindIndexBuffer(command_list);
			
						auto& buffers = self.GetUniformBlock(render_label::geometry_buffers);
						buffers.Set(render_label::position, geometry_buffer.GetBufferAddress(VertexAttributeFlag::ePosition));
						buffers.Set(render_label::texcoord, geometry_buffer.GetBufferAddress(VertexAttributeFlag::eTexCoord));
			
						// Bind buffers that uses in draw
						auto& draw_data_block = self.GetUniformBlock(shlib::DrawDescriptionBuffers_label);
						auto& draw_desc_buffers = draw_data_block.As<shlib::DrawDescriptionBuffers>();
						draw_desc_buffers.draw_descriptions_ptr = alive_meshes->GetAddress();
						draw_desc_buffers.draw_commands_ptr = argument_buffer->GetAddress();
			
						auto& constants = self.GetConstantBlock();
			
						self.BindResources(command_list);
			
						gfx::IndirectDrawCommand draw_command{};
						draw_command.indexed_draw = true;
						draw_command.draw_count = render_world->GetDrawCount();
						draw_command.argument_buffer = render_graph->GetBuffer(indirect_draw_commands);
						draw_command.command_stride = sizeof(shlib::DrawIndexedIndirectData);
						draw_command.count_buffer = render_graph->GetBuffer(indirect_count_buffer);
			
						uint32_t draw_call_id{ 0u };
						self.Begin(command_list);
			
						int32_t shadow_index{ 0 };
			
						auto view = world::EntityManager::View<world::ecs::FSpotLightComponent>();
						for (auto [entity, light] : view.each())
						{
							if (!light.castShadows)
								continue;

							draw_call_id = light.shadow_caster * render_world->GetDrawCount();
			
							constants.Set(render_label::call_id, draw_call_id);
							constants.Set(render_label::shadow_index, shadow_index);
							constants.Set(render_label::layer, shadow_index);
			
							self.PushConstantRanges(command_list);
			
							draw_command.argument_offset = light.shadow_caster * draw_command.command_stride;
							draw_command.count_offset = light.shadow_caster * sizeof(uint32_t);
			
							command_list.DrawIndirect(draw_command);
			
							shadow_index++;
						}
			
						self.End(command_list);
					};
			
				render_graph->AddNode(render_graph->CreateNode(create_info));
			}
			
			// Geometry draw
			{
				RenderShaderPassCreateInfo create_info{};
				create_info.shader_name = "visibilitybuffer:write:HAS_TEXCOORD:HAS_NORMAL:TANGENT_VEC4:INDEX_16BIT";
				create_info.pipeline_state.rasterization_state.cull_mode = gfx::CullModeFlag::eFront;
			
				// Depth state
				create_info.pipeline_state.depth_stencil_state.depth_test = true;
				create_info.pipeline_state.depth_stencil_state.depth_write = true;
				create_info.pipeline_state.depth_stencil_state.depth_compare_op = gfx::CompareOp::eLess;
			
			
				create_info.AddWriteAttachment(visbuffer_geometry_target, ResourceAttachmentTypeFlag::eColor, gfx::ClearColor{ 0.f, 0.f, 0.f, 0.f });
				create_info.AddWriteAttachment(visbuffer_vboinst_target, ResourceAttachmentTypeFlag::eColor, gfx::ClearColor{ 0.f, 0.f, 0.f, 0.f });
				create_info.AddWriteAttachment(zbuffer_depth_target, ResourceAttachmentTypeFlag::eDepth, gfx::ClearColor{ 1.f });
			
				create_info.pipeline_state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
				create_info.pipeline_state.color_blending_state.attachment_count = 2;
			
				create_info.draw_callback = [this,
					alive_commands_buffer,
					indirect_draw_commands,
					indirect_count_buffer
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;
			
						auto* backbuffer = render_graph->GetTexture(backbuffer_resource);
						auto* alive_meshes = render_graph->GetBuffer(alive_commands_buffer);
						auto* argument_buffer = render_graph->GetBuffer(indirect_draw_commands);
						auto* count_buffer = render_graph->GetBuffer(indirect_count_buffer);
			
						auto viewport_width = backbuffer->GetWidth();
						auto viewport_height = backbuffer->GetHeight();
			
						command_list.SetViewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
						command_list.SerScissor(0u, 0u, viewport_width, viewport_height);
			
						self.ReadResource(alive_commands_buffer, ~0u);
						self.ReadResource(indirect_draw_commands, ~0u);
						self.ReadResource(indirect_count_buffer, ~0u);
			
						self.ResolveDependencies(command_list);
			
						auto& geometry_buffer = *geometry_buffers.back();
						geometry_buffer.BindIndexBuffer(command_list);
			
						auto& buffers = self.GetUniformBlock(render_label::geometry_buffers);
						buffers.Set(render_label::position, geometry_buffer.GetBufferAddress(VertexAttributeFlag::ePosition));
						buffers.Set(render_label::texcoord, geometry_buffer.GetBufferAddress(VertexAttributeFlag::eTexCoord));
			
						// Bind buffers that uses in draw
						auto& draw_data_block = self.GetUniformBlock(shlib::DrawDescriptionBuffers_label);
						auto& draw_desc_buffers = draw_data_block.As<shlib::DrawDescriptionBuffers>();
						draw_desc_buffers.draw_descriptions_ptr = alive_meshes->GetAddress();
						draw_desc_buffers.draw_commands_ptr = argument_buffer->GetAddress();
			
						auto& constants = self.GetConstantBlock();
						constants.Set(render_label::call_id, 0u);
						constants.Set(render_label::camera_id, 0u);
			
						self.PushConstantRanges(command_list);
			
						self.BindResources(command_list);
			
						// TODO: USE CURRENT CAMERA HERE FOR CALCULATE OFFSET!!!!
						gfx::IndirectDrawCommand draw_command{};
						draw_command.indexed_draw = true;
						draw_command.draw_count = render_world->GetDrawCount();
						draw_command.argument_buffer = argument_buffer;
						draw_command.argument_offset = 0;
						draw_command.command_stride = sizeof(shlib::DrawIndexedIndirectData);
						draw_command.count_buffer = count_buffer;
						draw_command.count_offset = 0;
			
						self.Begin(command_list);
						command_list.DrawIndirect(draw_command);
						self.End(command_list);
					};
			
				render_graph->AddNode(render_graph->CreateNode(create_info));
			}
			
			// Resolve materials
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "visibilitybuffer:shade:HAS_TEXCOORD:HAS_NORMAL:TANGENT_VEC4:INDEX_16BIT";
			
				create_info.draw_callback = [this, 
					alive_commands_buffer,
					indirect_draw_commands,
					hdr0,
					brdf_lut_target,
					irradiance_cube_target,
					prefiltered_cube_target,
					visbuffer_geometry_target,
					visbuffer_vboinst_target,
					zbuffer_depth_target,
					cascaded_shadows_target,
					directional_shadows_target,
					omni_shadows_target
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;

						auto* alive_meshes = render_graph->GetBuffer(alive_commands_buffer);
						auto* argument_buffer = render_graph->GetBuffer(indirect_draw_commands);

						// Indirect generated buffers
						self.ReadResource(alive_commands_buffer, ~0u);
						self.ReadResource(indirect_draw_commands, ~0u);
			
						self.BindWriteResource(render_label::write_image0, hdr0, ~0u);
						self.BindResource(render_label::sample_image0, brdf_lut_target, ~0u); // binding 1
						self.BindResource(render_label::sample_image1, irradiance_cube_target, ~0u); // binding 2
						self.BindResource(render_label::sample_image2, prefiltered_cube_target, ~0u); // binding 3
			
						self.BindReadResource(render_label::sample_image3, visbuffer_geometry_target, ~0u);
						self.BindReadResource(render_label::sample_image4, visbuffer_vboinst_target, ~0u);
						self.BindReadResource(render_label::sample_image5, zbuffer_depth_target, ~0u);
			
						self.BindReadResource(render_label::sample_image6, cascaded_shadows_target, ~0u);
						self.BindReadResource(render_label::sample_image7, directional_shadows_target, ~0u);
						self.BindReadResource(render_label::sample_image8, omni_shadows_target, ~0u);
			
						self.ResolveDependencies(command_list);
			
						auto& geometry_buffer = *geometry_buffers.back();
						geometry_buffer.BindIndexBuffer(command_list);
			
						auto& buffers = self.GetUniformBlock(render_label::geometry_buffers);
						buffers.Set(render_label::position, geometry_buffer.GetBufferAddress(VertexAttributeFlag::ePosition));
						buffers.Set(render_label::texcoord, geometry_buffer.GetBufferAddress(VertexAttributeFlag::eTexCoord));
						buffers.Set(render_label::normal, geometry_buffer.GetBufferAddress(VertexAttributeFlag::eNormal));
						buffers.Set(render_label::tangent, geometry_buffer.GetBufferAddress(VertexAttributeFlag::eTangent));
						buffers.Set(render_label::indices, geometry_buffer.GetIndexBufferAddress());

						auto& draw_data_block = self.GetUniformBlock(shlib::DrawDescriptionBuffers_label);
						auto& draw_desc_buffers = draw_data_block.As<shlib::DrawDescriptionBuffers>();
						draw_desc_buffers.draw_descriptions_ptr = alive_meshes->GetAddress();
						draw_desc_buffers.draw_commands_ptr = argument_buffer->GetAddress();
			
						auto& constants = self.GetConstantBlock();
						constants.Set(render_label::call_id, 0u);
						constants.Set(render_label::camera_id, 0u);
			
						self.PushConstantRanges(command_list);
			
						self.BindResources(command_list);
			
						self.DispatchForTarget(command_list, hdr0, true);
					};
			
				render_graph->AddNode(render_graph->CreateNode(create_info));
			}
			
			// Compute static motion vectors (can be started in another queue)
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = use_low_precision_velocity ? "postprocess:generate_motion_vectors:LOW_PRECISION_VELOCITY" : "postprocess:generate_motion_vectors";
				create_info.draw_callback = [this,
					motion_vectors_target,
					zbuffer_depth_target
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;
			
						self.BindWriteResource(render_label::write_image0, motion_vectors_target, ~0u);
						self.BindReadResource(render_label::sample_image0, zbuffer_depth_target, ~0u);
			
						self.ResolveDependencies(command_list);
						self.BindResources(command_list);
			
						self.DispatchForTarget(command_list, zbuffer_depth_target, true);
					};
			
				render_graph->AddNode(render_graph->CreateNode(create_info));
			}
			
			uint32_t input_target = hdr0;
			uint32_t output_target = hdr1;
			
			// Copy history
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "utility:copy_compute";
			
				create_info.draw_callback = [this,
					input_target,
					hdr0_history
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;
			
						self.BindReadResource(render_label::sample_image0, input_target, ~0u);
						self.BindWriteResource(render_label::write_image0, hdr0_history, ~0u);
			
						self.ResolveDependencies(command_list);
						self.BindResources(command_list);
			
						self.DispatchForTarget(command_list, input_target, true);
					};
			
				render_graph->AddNode(render_graph->CreateNode(create_info));
			}
			
			uint32_t final_target = input_target;
			
			// Post processing
			{
				u32 pass_count{ 0u };
			
				// FXAA 
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.shader_name = "postprocess:apply_effect:FXAA";
			
					create_info.enabled_callback = []() -> bool { return postprocess.fxaa.enable; };
					create_info.draw_callback = [this,
						input_target,
						output_target,
						enabled = &postprocess.fxaa.enable
					](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
						{
							ZoneScoped;
			
							self.BindReadResource(render_label::sample_image0, input_target, ~0u);
							self.BindWriteResource(render_label::write_image0, output_target, ~0u);
			
							self.ResolveDependencies(command_list);
							self.BindResources(command_list);
			
							self.DispatchForTarget(command_list, input_target, true);
						};
			
					if (postprocess.fxaa.enable)
						SWAP_COUNT(input_target, output_target);
			
					render_graph->AddNode(render_graph->CreateNode(create_info));
				}
			
				// Apply motion blur
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.shader_name = use_low_precision_velocity ? "postprocess:apply_effect:MOTION_BLUR:LOW_PRECISION_VELOCITY" : "postprocess:apply_effect:MOTION_BLUR";
			
					create_info.enabled_callback = []() -> bool { return postprocess.motionblur.enable; };
					create_info.draw_callback = [this,
						input_target,
						output_target,
						motion_vectors_target
					](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
						{
							ZoneScoped;
			
							self.BindReadResource(render_label::sample_image0, input_target, ~0u);
							self.BindReadResource(render_label::sample_image1, motion_vectors_target, ~0u);
							self.BindWriteResource(render_label::write_image0, output_target, ~0u);
			
							self.ResolveDependencies(command_list);
							self.BindResources(command_list);
			
							self.DispatchForTarget(command_list, input_target, true);
						};
			
					if (postprocess.motionblur.enable)
						SWAP_COUNT(input_target, output_target);
			
					render_graph->AddNode(render_graph->CreateNode(create_info));
				}
			
				// Compute dof
				{
					auto group_node = render_graph->CreateNode("DOF", 0xFFFF33FF);
					auto* node = static_cast<GroupNode*>(group_node.get());
					node->SetGroupEnabledCallback([]() -> bool { return postprocess.dof.enable; });
			
					// Compute bokeh
					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.shader_name = "postprocess:bokeh";
			
						create_info.draw_callback = [this,
							input_target,
							hdr0_div2
						](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
							{
								ZoneScoped;
			
								self.BindReadResource(render_label::sample_image0, input_target, ~0u);
								self.BindWriteResource(render_label::write_image0, hdr0_div2, ~0u);
								self.ResolveDependencies(command_list);
								self.BindResources(command_list);
			
								self.DispatchForTarget(command_list, input_target, true);
							};
			
						node->AddNode(std::move(render_graph->CreateNode(create_info)));
					}
			
					// Apply dof
					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.shader_name = "postprocess:apply_effect:DOF";
			
						create_info.draw_callback = [this,
							input_target,
							output_target,
							hdr0_div2,
							zbuffer_depth_target
						](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
							{
								ZoneScoped;
			
								self.BindReadResource(render_label::sample_image0, input_target, ~0u);
								self.BindReadResource(render_label::sample_image1, hdr0_div2, ~0u);
								self.BindReadResource(render_label::sample_image2, zbuffer_depth_target, ~0u);
								self.BindWriteResource(render_label::write_image0, output_target, ~0u);
								self.ResolveDependencies(command_list);
								self.BindResources(command_list);
			
								self.DispatchForTarget(command_list, input_target, true);
							};
			
						node->AddNode(std::move(render_graph->CreateNode(create_info)));
					}
			
					if (postprocess.dof.enable)
						SWAP_COUNT(input_target, output_target);
			
					render_graph->AddNode(std::move(group_node));
				}
			
				// Compute bloom
				{
					auto group_node = render_graph->CreateNode("bloom", 0xFF3333FF);
					auto* node = static_cast<GroupNode*>(group_node.get());
					node->SetGroupEnabledCallback([]() -> bool { return postprocess.bloom.enable; });
			
					// Starting from brightness detection pass
					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.shader_name = bloom_use_brightness ? "postprocess:brightdetect" : "utility:copy_compute";
			
						create_info.draw_callback = [this,
							input_target,
							hdr0_div2
						](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
							{
								ZoneScoped;
			
								self.BindReadResource(render_label::sample_image0, input_target, ~0u);
								self.BindWriteResource(render_label::write_image0, hdr0_div2, ~0u);
								self.ResolveDependencies(command_list);
								self.BindResources(command_list);
			
								self.DispatchForTarget(command_list, hdr0_div2, true);
							};
			
						node->AddNode(std::move(render_graph->CreateNode(create_info)));
					}
			
					// Begin downsampling
					{
						{
							ComputeShaderPassCreateInfo create_info{};
							create_info.shader_name = "postprocess:downsample:FIRST_PASS";
			
							create_info.draw_callback = [this,
								hdr0_div2,
								bloom_target
							](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
								{
									ZoneScoped;
			
									self.WriteResource(render_label::write_image0, bloom_target, ~0u);
			
									self.BindReadResource(render_label::sample_image0, hdr0_div2, ~0u);
									self.BindResource(render_label::write_image0, bloom_target, ~0u);
			
									self.ResolveDependencies(command_list);
									self.BindResources(command_list);
			
									self.DispatchForTarget(command_list, bloom_target, true);
								};
			
							node->AddNode(std::move(render_graph->CreateNode(create_info)));
						}
			
						{
							ComputeShaderPassCreateInfo create_info{};
							create_info.shader_name = "postprocess:downsample";
			
							create_info.draw_callback = [this,
								bloom_target
							](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
								{
									ZoneScoped;
			
									auto target = render_graph->GetTexture(bloom_target);
			
									for (int32_t mip = 1; mip < bloom_blur_depth; ++mip)
									{
										self.BindReadResource(render_label::sample_image0, bloom_target, mip - 1);
										self.BindWriteResource(render_label::write_image0, bloom_target, mip);
			
										self.ResolveDependencies(command_list);
										self.BindResources(command_list);
			
										uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };
			
										group_x = target->GetWidth() >> mip;
										group_y = target->GetHeight() >> mip;
			
										self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
										command_list.Dispatch(group_x, group_y, group_z);
									}
								};
			
							node->AddNode(std::move(render_graph->CreateNode(create_info)));
						}
					}
			
					// Upsample
					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.shader_name = "postprocess:upsample";
			
						create_info.draw_callback = [this,
							bloom_target
						](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
							{
								ZoneScoped;
			
								auto target = render_graph->GetTexture(bloom_target);
			
								for (int32_t mip = bloom_blur_depth - 1; mip > 0; --mip)
								{
									self.BindReadResource(render_label::sample_image0, bloom_target, mip);
									self.BindWriteResource(render_label::write_image0, bloom_target, mip - 1);
			
									self.ResolveDependencies(command_list);
									self.BindResources(command_list);
			
									uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };
			
									group_x = target->GetWidth() >> (mip - 1);
									group_y = target->GetHeight() >> (mip - 1);
			
									self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
									command_list.Dispatch(group_x, group_y, group_z);
								}
							};
			
						node->AddNode(std::move(render_graph->CreateNode(create_info)));
					}
			
					// Apply bloom
					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.shader_name = "postprocess:apply_effect:BLOOM";
			
						create_info.draw_callback = [this,
							input_target,
							output_target,
							bloom_target,
							dirt_mask_texture
						](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
							{
								ZoneScoped;
			
								self.BindWriteResource(render_label::write_image0, output_target, ~0u);
								self.BindReadResource(render_label::sample_image0, input_target, ~0u);
								self.BindReadResource(render_label::sample_image1, bloom_target, ~0u);
								self.BindResource(render_label::sample_image2, dirt_mask_texture, ~0u);
			
								self.ResolveDependencies(command_list);
								self.BindResources(command_list);
			
								self.DispatchForTarget(command_list, output_target, true);
							};
			
						node->AddNode(std::move(render_graph->CreateNode(create_info)));
					}
			
					render_graph->AddNode(std::move(group_node));
			
					if (postprocess.bloom.enable)
						SWAP_COUNT(input_target, output_target);
				}
			
				// Tonemapping 
				{
					auto group_node = render_graph->CreateNode("tonemapping", 0xFFF9F51F);
					auto* node = static_cast<GroupNode*>(group_node.get());
					node->SetGroupEnabledCallback([]() -> bool { return postprocess.tonemap.enable; });
			
					// Compute histogram
					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.shader_name = "postprocess:compute_histogram";
			
						create_info.draw_callback = [this,
							input_target,
							histogram_buffer
						](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
							{
								ZoneScoped;
			
								self.BindReadResource(render_label::sample_image0, input_target, ~0u);
								self.BindWriteResource(render_label::histogram_buffer, histogram_buffer, ~0u);
								self.ResolveDependencies(command_list);
			
								float log_lum_range = postprocess.tonemap.lumMax - postprocess.tonemap.lumMin;
			
								auto& uniform = self.GetUniformBlock(render_label::histogram_compute_params);
								uniform.Set(render_label::min_log_lum, postprocess.tonemap.lumMin);
								uniform.Set(render_label::log_lum_range, log_lum_range);
			
								self.BindResources(command_list);
			
								self.DispatchForTarget(command_list, input_target, true);
							};
			
						node->AddNode(std::move(render_graph->CreateNode(create_info)));
					}
			
					// Compute avg screen space brightness
					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.shader_name = "postprocess:compute_brightness";
			
						create_info.draw_callback = [this,
							avg_brightness_target,
							histogram_buffer,
							input_target
						](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
							{
								ZoneScoped;
			
								self.BindReadResource(render_label::histogram_buffer, histogram_buffer, ~0u);
								self.BindWriteResource(render_label::write_image0, avg_brightness_target, ~0u);
								self.ResolveDependencies(command_list);
			
								float log_lum_range = postprocess.tonemap.lumMax - postprocess.tonemap.lumMin;
								float time_coeff = glm::clamp<f32>(1.0f - glm::exp(-delta_time * postprocess.tonemap.tau), 0.0, 1.0);
			
								auto* texture = render_graph->GetTexture(input_target);
			
								auto& uniform = self.GetUniformBlock(render_label::histogram_compute_params);
								uniform.Set(render_label::min_log_lum, postprocess.tonemap.lumMin);
								uniform.Set(render_label::log_lum_range, log_lum_range);
								uniform.Set(render_label::time_coeff, time_coeff);
								uniform.Set(render_label::num_pixels_x, static_cast<float>(texture->GetWidth()));
								uniform.Set(render_label::num_pixels_y, static_cast<float>(texture->GetHeight()));
			
								self.BindResources(command_list);
			
								command_list.Dispatch(1u, 1u, 1u);
							};
			
						node->AddNode(std::move(render_graph->CreateNode(create_info)));
					}
			
					// Apply tonemapping
					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.shader_name = "postprocess:apply_effect:TONEMAP";
			
						create_info.draw_callback = [this,
							input_target,
							output_target,
							avg_brightness_target
						](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
							{
								ZoneScoped;
			
								self.BindWriteResource(render_label::write_image0, output_target, ~0u);
								self.BindReadResource(render_label::sample_image0, input_target, ~0u);
								self.BindReadResource(render_label::sample_image1, avg_brightness_target, ~0u);
			
								self.ResolveDependencies(command_list);
								self.BindResources(command_list);
			
								self.DispatchForTarget(command_list, input_target, true);
							};
			
						if (postprocess.tonemap.enable)
							SWAP_COUNT(input_target, output_target);
			
						node->AddNode(render_graph->CreateNode(create_info));
					}
			
					render_graph->AddNode(std::move(group_node));
				}
			
				final_target = pass_count % 2 ? output_target : input_target;
			}
			
			
			// Test output shader
			{
				RenderShaderPassCreateInfo create_info{};
				create_info.shader_name = "testing:screenspace";
				create_info.AddWriteAttachment(backbuffer_resource, ResourceAttachmentTypeFlag::eColor, gfx::ClearColor{ 1.f, 0.5f, 1.f, 1.f });
			
				create_info.pipeline_state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
				create_info.pipeline_state.color_blending_state.attachment_count = 1;
			
				create_info.draw_callback = [this, 
					final_target
				](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						ZoneScoped;
			
						auto* backbuffer = render_graph->GetTexture(backbuffer_resource);
			
						auto viewport_width = static_cast<float>(backbuffer->GetWidth());
						auto viewport_height = static_cast<float>(backbuffer->GetHeight());
			
						// Flip viewport
						command_list.SetViewport(0.f, viewport_height, viewport_width, -viewport_height, 0.f, 1.f);
						command_list.SerScissor(0u, 0u, viewport_width, viewport_height);
			
						self.BindReadResource(render_label::sample_image0, final_target, ~0u);
			
						self.ResolveDependencies(command_list);
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

					TextureResourceCreateInfo font_texture_ci{};
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
							ZoneScoped;

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

										self.ReadResource(render_label::sample_image0, resource_binding, ~0u);
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
												self.BindResource(render_label::sample_image0, resource_binding, ~0u);
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