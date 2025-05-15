#include "Rendering.h"

#include <edge/gfx/gfx_context.h>
#include <edge/gfx/gfx_resource_loader.h>
#include <edge/gfx/gfx_frame_graph.h>
#include <edge/gfx/technique_system/gfx_technique_system.h>

#include "render_graph/ComputePass.h"
#include "render_graph/RenderPass.h"
#include "RenderContext.h"
#include "GroupExecutionNode.h"

#include "render_world/RenderWorld.h"

#include "GeometryBuffer.h"

#include <edge/util/Logger.h>

#include "Globals.h"

#include "../world/EntityManager.h"

#include <regex>

#include <imgui.h>
//#include <imdbg.h>
#include <edge/util/utl_profiler.h>

#include <edge/fs/fs_virtual_filesystem.h>

#if USE_NSIGHT_AFTERMATH
#include <edge/gfx/gfx_nsight_aftermath.h>
#endif

// Pass data includes
#include "../../assets/shaders/culling/cull_and_batch.h"

#include "../../assets/shaders/deferred/visbufferwrite.h"

#include "../../assets/shaders/lighting/prefilteredmap.h"
#include "../../assets/shaders/lighting/unidirectional_shadow.h"

#include "../../assets/shaders/postprocess/apply_effect.h"
#include "../../assets/shaders/postprocess/bokeh.h"
#include "../../assets/shaders/postprocess/brightdetect.h"
#include "../../assets/shaders/postprocess/comp_histogram.h"
#include "../../assets/shaders/postprocess/downsampler.h"

#include "../../assets/shaders/testing/debug_text.h"
#include "../../assets/shaders/testing/imgui.h"

#include "../../assets/shaders/utility/clear_buffer.h"
#include "../../assets/shaders/utility/reduce_4x.h"

#define DISABLE_SHADOWS 1
//#define SKINNING_TEST 1

namespace render_label {
	using namespace edge;

	constexpr const uint32_t image0 = "image0"_hash32;
	constexpr const uint32_t image1 = "image1"_hash32;
	constexpr const uint32_t image2 = "image2"_hash32;
	constexpr const uint32_t image3 = "image3"_hash32;
	constexpr const uint32_t image4 = "image4"_hash32;
	constexpr const uint32_t image5 = "image5"_hash32;
	constexpr const uint32_t image6 = "image6"_hash32;
	constexpr const uint32_t image7 = "image7"_hash32;

	constexpr const uint32_t texture0 = "texture0"_hash32;
	constexpr const uint32_t texture1 = "texture1"_hash32;
	constexpr const uint32_t texture2 = "texture2"_hash32;
	constexpr const uint32_t texture3 = "texture3"_hash32;
	constexpr const uint32_t texture4 = "texture4"_hash32;
	constexpr const uint32_t texture5 = "texture5"_hash32;
	constexpr const uint32_t texture6 = "texture6"_hash32;
	constexpr const uint32_t texture7 = "texture7"_hash32;
	constexpr const uint32_t texture8 = "texture8"_hash32;
	constexpr const uint32_t texture9 = "texture9"_hash32;
	constexpr const uint32_t texture10 = "texture10"_hash32;
	constexpr const uint32_t texture11 = "texture11"_hash32;

	constexpr const uint32_t sampler0 = "sampler0"_hash32;
	constexpr const uint32_t sampler1 = "sampler1"_hash32;
	constexpr const uint32_t sampler2 = "sampler2"_hash32;
	constexpr const uint32_t sampler3 = "sampler3"_hash32;
	constexpr const uint32_t sampler4 = "sampler4"_hash32;
	constexpr const uint32_t sampler5 = "sampler5"_hash32;
	constexpr const uint32_t sampler6 = "sampler6"_hash32;

	constexpr const uint32_t sampler_point = "sampler_point"_hash32;
	constexpr const uint32_t sampler_linear = "sampler_linear"_hash32;
	constexpr const uint32_t sampler_shadow_linear = "sampler_shadow_linear"_hash32;

	constexpr const uint32_t roughness = "roughness"_hash32;

	constexpr const uint32_t position = "position"_hash32;
	constexpr const uint32_t color = "color"_hash32;
	constexpr const uint32_t texcoord = "texcoord"_hash32;
	constexpr const uint32_t normal = "normal"_hash32;
	constexpr const uint32_t tangent = "tangent"_hash32;
	constexpr const uint32_t joints = "joints"_hash32;
	constexpr const uint32_t weights = "weights"_hash32;

	constexpr const uint32_t indices = "indices"_hash32;

	constexpr const uint32_t camera_id = "camera_id"_hash32;
}

namespace engine::render
{
	uint32_t main_camera_index{ ~0u };
	PostProcessData postprocess{};

	Renderer::Renderer(edge::Window* window) {
		edge::gfx::ContextCreateInfo gfx_create_info{};
		gfx_create_info.app_name = "MyVulkanEngine";
		gfx_create_info.render_api = edge::gfx::RenderApi::eVulkan;
		gfx_create_info.window = window;

		edge::gfx::make_context(gfx_create_info);

		edge::gfx::ResourceUpdaterCreateInfo resource_updater_init{};
		edge::gfx::make_resource_uploader(resource_updater_init);

		render_context = std::make_unique<RenderContext>(window);

		render_world = std::make_unique<RenderWorld>(*this);


		// Temporary func to create test render graph
		build_graph();
	}

	Renderer::~Renderer() {
		edge::gfx::shutdown_resource_uploader();
		edge::gfx::shutdown_context();
	}

	auto Renderer::add_render_resource(std::string_view path) -> uint32_t {
		return render_context->add_resource(path);
	}

	auto Renderer::add_render_resource(const TextureResourceCreateInfo& create_info) -> uint32_t {
		return render_context->add_resource(create_info);
	}

	auto Renderer::add_render_resource(const edge::gfx::SamplerCreateInfo& create_info) -> uint32_t {
		return render_context->add_resource(create_info);
	}

	auto Renderer::add_render_resource(const edge::gfx::BufferCreateInfo& create_info) -> uint32_t {
		return render_context->add_resource(create_info);
	}

	void Renderer::add_material(const interop::Material& material) {
		// "Patch" material before add

		// TODO: write resource index on material creation
		interop::Material new_material{ material };

		if (new_material.base_color.valid()) {
			auto& tex = render_context->get_resource(new_material.base_color.texture);
			new_material.base_color.texture = tex.resource_id;
			auto& sam = render_context->get_resource(new_material.base_color.sampler);
			new_material.base_color.sampler = sam.resource_id;
		}

		if (new_material.metallic_roughness.valid()) {
			auto& tex = render_context->get_resource(new_material.metallic_roughness.texture);
			new_material.metallic_roughness.texture = tex.resource_id;
			auto& sam = render_context->get_resource(new_material.metallic_roughness.sampler);
			new_material.metallic_roughness.sampler = sam.resource_id;
		}

		if (new_material.normal.valid()) {
			auto& tex = render_context->get_resource(new_material.normal.texture);
			new_material.normal.texture = tex.resource_id;
			auto& sam = render_context->get_resource(new_material.normal.sampler);
			new_material.normal.sampler = sam.resource_id;
		}

		if (new_material.occlusion.valid()) {
			auto& tex = render_context->get_resource(new_material.occlusion.texture);
			new_material.occlusion.texture = tex.resource_id;
			auto& sam = render_context->get_resource(new_material.occlusion.sampler);
			new_material.occlusion.sampler = sam.resource_id;
		}

		if (new_material.emissive.valid()) {
			auto& tex = render_context->get_resource(new_material.emissive.texture);
			new_material.emissive.texture = tex.resource_id;
			auto& sam = render_context->get_resource(new_material.occlusion.sampler);
			new_material.occlusion.sampler = sam.resource_id;
		}

		render_world->add_material(new_material);
	}

	void Renderer::render(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		// Update geometry buffers
		for (auto& geometry_buffer : geometry_buffers) {
			geometry_buffer->flush();
		}

		render_context->begin();
		render_world->begin_frame(delta_time);
#if USE_NSIGHT_AFTERMATH
		edge::gfx::NsightAftermath::try_begin_frame();
#endif

		render_context->execute(delta_time);

#if USE_NSIGHT_AFTERMATH
		edge::gfx::NsightAftermath::try_end_frame();
#endif
		render_world->end_frame();
		render_context->end();
	}

	auto Renderer::request_interleaved_geometry_buffer(const GeometryBufferCreateInfo& create_info) -> uint32_t {
		uint32_t h = hash(create_info);

		if (auto found = interleaved_geometry_buffers.find(h); found != interleaved_geometry_buffers.end()) {
			return found->second;
		}

		auto& new_geometry_buffer = geometry_buffers.emplace_back(std::make_unique<InterleavedGeometryBuffer>(create_info));
		return interleaved_geometry_buffers[h] = static_cast<uint32_t>(geometry_buffers.size() - 1ull);
	}

	auto Renderer::request_deinterleaved_geometry_buffer(const GeometryBufferCreateInfo& create_info) -> uint32_t {
		uint32_t h = hash(create_info);
		if (auto found = deinterleaved_geometry_buffers.find(h); found != deinterleaved_geometry_buffers.end()) {
			return found->second;
		}

		auto& new_geometry_buffer = geometry_buffers.emplace_back(std::make_unique<DeinterleavedGeometryBuffer>(create_info));
		return deinterleaved_geometry_buffers[h] = static_cast<uint32_t>(geometry_buffers.size() - 1ull);
	}

	auto Renderer::lookup_geometry_buffer(uint32_t geometry_buffer_id) -> GeometryBuffer& {
		return *geometry_buffers[geometry_buffer_id];
	}

	auto Renderer::get_render_context() -> IExecutionContext& {
		return *render_context;
	}

	auto Renderer::get_render_context() const -> IExecutionContext const& {
		return *render_context;
	}

	auto Renderer::get_render_world() -> IRenderWorld& {
		return *render_world;
	}

	auto Renderer::get_render_world() const -> IRenderWorld const& {
		return *render_world;
	}

	void Renderer::build_frame_graph() {
		constexpr const std::string_view k_frame_graph_path = "/root/fgb";


	}

	void Renderer::build_graph() {
		static bool use_low_precision_velocity{ false };
		static bool bloom_use_brightness{ true };
		static uint32_t bloom_blur_depth = 5;

		// Register backbuffer resource

		backbuffer_resource = render_context->add_resource();

		auto& backbuffer_resource_binding = render_context->get_resource(backbuffer_resource);
		backbuffer_resource_binding.resource_type = RenderResourceTypeFlag::eBackbuffer;

		// Create default samplers

		uint32_t nearest_mirror_sampler{ ~0u };
		uint32_t nearest_repeat_sampler{ ~0u };
		uint32_t nearest_clamp_to_edge_sampler{ ~0u };
		uint32_t nearest_clamp_to_border_sampler{ ~0u };
		{
			edge::gfx::SamplerCreateInfo create_info{};
			create_info.min_filter = edge::gfx::Filter::eNearest;
			create_info.mag_filter = edge::gfx::Filter::eNearest;
			create_info.mip_map_mode = edge::gfx::Filter::eNearest;

			create_info.address_u = create_info.address_v = edge::gfx::SamplerAddressMode::eMirror;
			nearest_mirror_sampler = render_context->add_resource(create_info);

			create_info.address_u = create_info.address_v = edge::gfx::SamplerAddressMode::eRepeat;
			nearest_repeat_sampler = render_context->add_resource(create_info);

			create_info.address_u = create_info.address_v = edge::gfx::SamplerAddressMode::eClampToEdge;
			nearest_clamp_to_edge_sampler = render_context->add_resource(create_info);

			create_info.address_u = create_info.address_v = edge::gfx::SamplerAddressMode::eClampToBorder;
			nearest_clamp_to_border_sampler = render_context->add_resource(create_info);
		}

		uint32_t linear_mirror_sampler{ ~0u };
		uint32_t linear_repeat_sampler{ ~0u };
		uint32_t linear_clamp_to_edge_sampler{ ~0u };
		uint32_t linear_clamp_to_border_sampler{ ~0u };
		{
			edge::gfx::SamplerCreateInfo create_info{};
			create_info.min_filter = edge::gfx::Filter::eLinear;
			create_info.mag_filter = edge::gfx::Filter::eLinear;
			create_info.mip_map_mode = edge::gfx::Filter::eLinear;

			create_info.address_u = create_info.address_v = edge::gfx::SamplerAddressMode::eMirror;
			linear_mirror_sampler = render_context->add_resource(create_info);

			create_info.address_u = create_info.address_v = edge::gfx::SamplerAddressMode::eRepeat;
			linear_repeat_sampler = render_context->add_resource(create_info);

			create_info.address_u = create_info.address_v = edge::gfx::SamplerAddressMode::eClampToEdge;
			linear_clamp_to_edge_sampler = render_context->add_resource(create_info);

			create_info.address_u = create_info.address_v = edge::gfx::SamplerAddressMode::eClampToBorder;
			linear_clamp_to_border_sampler = render_context->add_resource(create_info);
		}

		uint32_t nearest_mipmap_linear_reduce_min{ ~0u };
		{
			edge::gfx::SamplerCreateInfo create_info{};
			create_info.min_filter = edge::gfx::Filter::eLinear;
			create_info.mag_filter = edge::gfx::Filter::eLinear;
			create_info.mip_map_mode = edge::gfx::Filter::eNearest;

			create_info.address_u = create_info.address_v = edge::gfx::SamplerAddressMode::eClampToEdge;
			create_info.min_lod = 0.0f;
			create_info.max_lod = 16.0f;

			create_info.reduction_mode = edge::gfx::ReductionMode::eMin;

			nearest_mipmap_linear_reduce_min = render_context->add_resource(create_info);
		}

		// Shadow samplers
		uint32_t default_shadow_sampler{ ~0u };
		{
			edge::gfx::SamplerCreateInfo create_info{};
			create_info.min_filter = edge::gfx::Filter::eLinear;
			create_info.mag_filter = edge::gfx::Filter::eLinear;
			create_info.mip_map_mode = edge::gfx::Filter::eLinear;
			create_info.depth_compare = edge::gfx::CompareOp::eLess;

			default_shadow_sampler = render_context->add_resource(create_info);
		}

		// Load some static textures
		auto casual_day_4k_cube = render_context->add_resource_sync("/internal_res/assets/textures/CasualDay4K.ktx2");
		auto dirt_mask_texture = render_context->add_resource("/internal_res/assets/textures/DirtMaskTextureExample.png");

		// GI targets
		uint32_t brdf_lut_target{ ~0u };
		{
			// TODO: make it fixed size
			TextureResourceCreateInfo create_info{};
			create_info.width = create_info.height = 512u;
			create_info.format = edge::gfx::Format::eR16G16Sfloat;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage;
			create_info.resizable = false;

			brdf_lut_target = render_context->add_resource(create_info);
			render_context->set_resource_name(brdf_lut_target, "brdf_lut_target");
		}

		//uint32_t environment_cube_target{ ~0u };
		//{
		//	TextureResourceCreateInfo create_info{};
		//	create_info.width = create_info.height = 512;
		//	create_info.format = edge::gfx::Format::eB10G11R11UfloatPack32;
		//	create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage | edge::gfx::TextureUsageFlag::eCubemap;
		//	create_info.resizable = false;
		//
		//	environment_cube_target = render_context->add_resource(create_info);
		//	render_context->set_resource_name(environment_cube_target, "environment_cube_target");
		//}

		uint32_t irradiance_cube_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = create_info.height = 64u;
			create_info.format = edge::gfx::Format::eR32G32B32A32Sfloat;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage | edge::gfx::TextureUsageFlag::eCubemap;
			create_info.resizable = false;

			irradiance_cube_target = render_context->add_resource(create_info);
			render_context->set_resource_name(irradiance_cube_target, "irradiance_cube_target");
		}

		uint32_t prefiltered_cube_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = create_info.height = 512u;
			create_info.format = edge::gfx::Format::eB10G11R11UfloatPack32;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage | edge::gfx::TextureUsageFlag::eCubemap;
			create_info.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(create_info.width, create_info.height)))) + 1u;
			create_info.resizable = false;

			edge::gfx::ISampler* special_sampler{ nullptr };
			{
				edge::gfx::SamplerCreateInfo create_info_s{};
				create_info_s.min_filter = edge::gfx::Filter::eLinear;
				create_info_s.mag_filter = edge::gfx::Filter::eLinear;
				create_info_s.mip_map_mode = edge::gfx::Filter::eLinear;
				create_info_s.min_lod = 0.f;
				create_info_s.max_lod = static_cast<float>(create_info.levels);

				// TODO: need to cache samplers
				special_sampler = edge::gfx::create_sampler(create_info_s);
			}

			prefiltered_cube_target = render_context->add_resource(create_info);
			render_context->set_resource_name(prefiltered_cube_target, "prefiltered_cube_target");
		}

		// Shadow targets
		uint32_t cascaded_shadows_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = create_info.height = constants::cascade_shadow_map_resolution;
			create_info.layers = constants::cascade_shadow_map_cascade_count * constants::max_directional_light_shadow_count;
			create_info.format = edge::gfx::Format::eD32Sfloat;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eRenderTarget;
			create_info.resizable = false;

			cascaded_shadows_target = render_context->add_resource(create_info);
			render_context->set_resource_name(cascaded_shadows_target, "cascaded_shadows_target");
		}

		uint32_t directional_shadows_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = create_info.height = constants::spot_light_shadow_map_resolution;
			create_info.layers = constants::max_spot_light_shadow_count;
			create_info.format = edge::gfx::Format::eD16Unorm;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eRenderTarget;
			create_info.resizable = false;

			directional_shadows_target = render_context->add_resource(create_info);
			render_context->set_resource_name(directional_shadows_target, "directional_shadows_target");
		}

		uint32_t omni_shadows_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = create_info.height = constants::point_light_shadow_map_resolution;
			create_info.layers = constants::max_point_light_shadow_count;
			create_info.format = edge::gfx::Format::eD16Unorm;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eRenderTarget | edge::gfx::TextureUsageFlag::eCubemap;
			create_info.resizable = false;

			omni_shadows_target = render_context->add_resource(create_info);
			render_context->set_resource_name(omni_shadows_target, "omni_shadows_target");
		}

		auto* backbuffer_texture = render_context->get_texture(backbuffer_resource);

		// Visibility buffer targets
		uint32_t visbuffer_geometry_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = backbuffer_texture->get_width();
			create_info.height = backbuffer_texture->get_height();
			create_info.format = edge::gfx::Format::eR8G8B8A8Unorm;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eRenderTarget;
			create_info.resizable = true;

			visbuffer_geometry_target = render_context->add_resource(create_info);
			render_context->set_resource_name(visbuffer_geometry_target, "visbuffer_geometry_target");
		}

		uint32_t visbuffer_vboinst_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = backbuffer_texture->get_width();
			create_info.height = backbuffer_texture->get_height();
			create_info.format = edge::gfx::Format::eR8G8B8A8Unorm;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eRenderTarget;
			create_info.resizable = true;

			visbuffer_vboinst_target = render_context->add_resource(create_info);
			render_context->set_resource_name(visbuffer_vboinst_target, "visbuffer_vboinst_target");
		}

		uint32_t zbuffer_depth_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = backbuffer_texture->get_width();
			create_info.height = backbuffer_texture->get_height();
			create_info.format = edge::gfx::Format::eD32Sfloat;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eRenderTarget;
			create_info.resizable = true;

			zbuffer_depth_target = render_context->add_resource(create_info);
			render_context->set_resource_name(zbuffer_depth_target, "zbuffer_depth_target");
		}

		uint32_t depth_pyramid_terget{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = backbuffer_texture->get_width();
			create_info.height = backbuffer_texture->get_height();
			create_info.format = edge::gfx::Format::eD32Sfloat;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage;
			create_info.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(create_info.width, create_info.height)))) + 1u;
			//create_info.resizable = true;

			depth_pyramid_terget = render_context->add_resource(create_info);
			render_context->set_resource_name(depth_pyramid_terget, "depth_pyramid_terget");
		}

		uint32_t ssao_target{ ~0u };
		uint32_t ssao_target_temp{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = backbuffer_texture->get_width();
			create_info.height = backbuffer_texture->get_height();
			create_info.scale = 0.5f;
			create_info.format = edge::gfx::Format::eR16Sfloat;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage;
			create_info.resizable = true;

			ssao_target = render_context->add_resource(create_info);
			render_context->set_resource_name(ssao_target, "ssao_target");

			ssao_target_temp = render_context->add_resource(create_info);
			render_context->set_resource_name(ssao_target, "ssao_target_temp");
		}

		uint32_t motion_vectors_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = backbuffer_texture->get_width();
			create_info.height = backbuffer_texture->get_height();
			create_info.format = use_low_precision_velocity ? edge::gfx::Format::eR8G8Unorm : edge::gfx::Format::eR16G16Sfloat;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage;
			create_info.resizable = true;

			motion_vectors_target = render_context->add_resource(create_info);
			render_context->set_resource_name(motion_vectors_target, "motion_vectors_target");
		}

		uint32_t hdr0{ ~0u };
		uint32_t hdr1{ ~0u };
		uint32_t hdr0_history{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = backbuffer_texture->get_width();
			create_info.height = backbuffer_texture->get_height();
			create_info.format = edge::gfx::Format::eB10G11R11UfloatPack32;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage;
			create_info.resizable = true;

			hdr0 = render_context->add_resource(create_info);
			render_context->set_resource_name(hdr0, "hdr0");

			hdr1 = render_context->add_resource(create_info);
			render_context->set_resource_name(hdr1, "hdr1");

			hdr0_history = render_context->add_resource(create_info);
			render_context->set_resource_name(hdr0_history, "hdr0_history");
		}

		uint32_t hdr0_div2{ ~0u };
		{
			// TODO: add support for half size targets
			TextureResourceCreateInfo create_info{};
			create_info.width = backbuffer_texture->get_width() / 2;
			create_info.height = backbuffer_texture->get_height() / 2;
			create_info.format = edge::gfx::Format::eB10G11R11UfloatPack32;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage;
			create_info.resizable = true;
			create_info.scale = 0.5f;

			hdr0_div2 = render_context->add_resource(create_info);
			render_context->set_resource_name(hdr0_div2, "hdr0_div2");
		}

		uint32_t bloom_target{ ~0u };
		{
			// TODO: add support for half size targets
			TextureResourceCreateInfo create_info{};
			create_info.width = backbuffer_texture->get_width() / 2;
			create_info.height = backbuffer_texture->get_height() / 2;
			create_info.levels = bloom_blur_depth;
			create_info.format = edge::gfx::Format::eB10G11R11UfloatPack32;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage;
			create_info.resizable = true;
			create_info.scale = 0.5f;

			bloom_target = render_context->add_resource(create_info);
			render_context->set_resource_name(bloom_target, "bloom_target");
		}

		// Eye adaptation targets
		uint32_t avg_brightness_target{ ~0u };
		{
			TextureResourceCreateInfo create_info{};
			create_info.width = 1u;
			create_info.height = 1u;
			create_info.format = edge::gfx::Format::eR32Sfloat;
			create_info.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eStorage;
			create_info.resizable = false;

			avg_brightness_target = render_context->add_resource(create_info);
			render_context->set_resource_name(avg_brightness_target, "avg_brightness_target");
		}

		uint32_t histogram_buffer{ ~0u };
		{
			edge::gfx::BufferCreateInfo create_info{};
			create_info.size = sizeof(uint32_t) * 256u;
			create_info.usage = edge::gfx::BufferUsageFlag::eStorageBuffer;

			histogram_buffer = render_context->add_resource(create_info);
			render_context->set_resource_name(histogram_buffer, "histogram_buffer");
		}

		// Buffer contains draw indirect commands
		uint32_t indirect_draw_commands{ ~0u };
		{
			edge::gfx::BufferCreateInfo create_info{};
			create_info.size = sizeof(interop::DrawIndexedIndirectData) * constants::max_draw_command_count;
			create_info.usage = edge::gfx::BufferUsageFlag::eIndirectBuffer;

			indirect_draw_commands = render_context->add_resource(create_info);
			render_context->set_resource_name(indirect_draw_commands, "indirect_draw_commands");
		}

#ifdef TILE_MATERIAL_CLASSIFICATION
		uint32_t indirect_material_technique_buffer{ ~0u };
		{
			edge::gfx::BufferCreateInfo create_info{};
			create_info.size = sizeof(uint32_t) * 3ull;
			create_info.usage = edge::gfx::BufferUsageFlag::eIndirectBuffer;

			indirect_material_technique_buffer = render_context->add_resource(create_info);
			render_context->set_resource_name(indirect_material_technique_buffer, "indirect_material_technique_buffer");
		}

		uint32_t material_tiles_buffer{ ~0u };
		{
			edge::gfx::BufferCreateInfo create_info{};
			create_info.size = sizeof(uint32_t) * ceil(1920 / 8) * ceil(1080 / 8);
			create_info.usage = edge::gfx::BufferUsageFlag::eStorageBuffer;

			material_tiles_buffer = render_context->add_resource(create_info);
			render_context->set_resource_name(material_tiles_buffer, "material_tiles_buffer");
		}
#endif

		// Passes
		// Generate brdflut
		{
			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = "generate_brdflut";
			create_info.one_time = true;

			create_info.draw_callback = [this,
				brdf_lut_target
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time)
				{
					EDGE_PROFILE_ZONE_SCOPED;

					self.bind_write_resource(render_label::image0, brdf_lut_target, ~0u);

					self.resolve_dependencies(command_list);
					self.bind_resources(command_list);

					self.dispatch_for_target(command_list, brdf_lut_target, true);
					self.finalize_target(command_list, render_label::image0, brdf_lut_target);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Generate irradiance cubemap
		{
			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = "generate_irradiancecube";
			create_info.one_time = true;

			create_info.draw_callback = [this,
				irradiance_cube_target,
				casual_day_4k_cube,
				linear_clamp_to_edge_sampler
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time)
				{
					EDGE_PROFILE_ZONE_SCOPED;

					self.bind_write_resource(render_label::image0, irradiance_cube_target, ~0u);

					self.bind_resource(render_label::texture0, casual_day_4k_cube);
					self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);

					self.resolve_dependencies(command_list);
					self.bind_resources(command_list);

					self.dispatch_for_target(command_list, irradiance_cube_target, true);

					self.finalize_target(command_list, render_label::image0, irradiance_cube_target);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Generate prefiltered cubemap
		{
			auto group_node = render_context->create_node("prefiltered map generator", 0x99F9F1FF);

			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = "generate_prefilteredmap";
			create_info.one_time = true;

			create_info.draw_callback = [this,
				prefiltered_cube_target,
				casual_day_4k_cube,
				linear_clamp_to_edge_sampler
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					self.write_resource(render_label::image0, prefiltered_cube_target, ~0u);

					auto target = render_context->get_texture(prefiltered_cube_target);
					for (int32_t mip = 0; mip < static_cast<int32_t>(target->get_mip_level_count()); ++mip) {
						uint32_t current_mip{ 0 };
						if (mip == 0) {
							self.bind_resource(render_label::texture0, casual_day_4k_cube);
						}
						else {
							current_mip = mip - 1;
							self.bind_read_resource(render_label::texture0, prefiltered_cube_target);
						}

						self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);

						self.bind_write_resource(render_label::image0, prefiltered_cube_target, mip);

						self.resolve_dependencies(command_list);
						self.bind_resources(command_list);

						uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

						float layer_roughness = static_cast<float>(mip) / static_cast<float>(target->get_mip_level_count());

						auto& pc = self.get_constant_block();
						auto& pc_data = pc.as<prefilteredmap::PushConstants>();
						pc_data.roughness = layer_roughness;
						pc_data.current_mip = current_mip;
						self.push_constant_ranges(command_list);

						group_x = target->get_width() >> mip;
						group_y = target->get_height() >> mip;

						self.divide_workgroup_by_local_sizes(group_x, group_y, group_z);
						command_list.dispatch(group_x, group_y, group_z);
					}

					self.finalize_target(command_list, render_label::texture0, prefiltered_cube_target);
				};

			static_cast<GroupNode*>(group_node.get())->add_node(std::move(render_context->create_node(create_info)));

			render_context->add_node(std::move(group_node));
		}

		// Cull meshes
		{
			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = "cull_and_batch";

			create_info.draw_callback = [this,
				indirect_draw_commands,
				depth_pyramid_terget,
				nearest_clamp_to_edge_sampler
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					auto* draw_commands = render_world->get_draw_buffer();
					auto* indirect_cmds = render_context->get_buffer(indirect_draw_commands);

					self.bind_read_resource(render_label::texture0, depth_pyramid_terget, ~0u);
					self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);

					self.write_resource(indirect_draw_commands, ~0u);
					self.resolve_dependencies(command_list);

					auto& pc = self.get_constant_block();
					auto& pc_data = pc.as<cull_and_batch::PushConstants>();
					pc_data.input_draw_desc_ptr = draw_commands->get_address();
					pc_data.output_draw_indirect_ptr = indirect_cmds->get_address();
					self.push_constant_ranges(command_list);

					self.bind_resources(command_list);

					uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };
					group_x = render_world->get_draw_count();

					self.divide_workgroup_by_local_sizes(group_x, group_y, group_z);
					command_list.dispatch(group_x, group_y, group_z);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

#if !DISABLE_SHADOWS

		// Draw cascaded shadows
		{
			RenderShaderPassCreateInfo create_info{};
#if SKINNING_TEST
			create_info.technique_name = "cascaded_shadow_skin";
#else
			create_info.technique_name = "cascaded_shadow";
#endif

			create_info.add_write_attachment(cascaded_shadows_target, ResourceAttachmentTypeFlag::eDepth, edge::gfx::ClearColor{ 1.f });

			create_info.draw_callback = [this,
				cascaded_shadows_target,
				indirect_draw_commands
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					auto* shadow_map = render_context->get_texture(cascaded_shadows_target);
					auto* draw_commands = render_world->get_draw_buffer();
					auto* argument_buffer = render_context->get_buffer(indirect_draw_commands);

					auto viewport_width = shadow_map->get_width();
					auto viewport_height = shadow_map->get_height();

					command_list.set_viewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
					command_list.set_scissor(0u, 0u, viewport_width, viewport_height);

					self.read_resource(indirect_draw_commands, ~0u);

					self.resolve_dependencies(command_list);

					auto& geometry_buffer = *geometry_buffers.back();
					geometry_buffer.bind_index_buffer(command_list);

					// Bind buffers that uses in draw
					auto& draw_data_block = self.get_uniform_block(edge::murmur_hash("egd"));
					auto& draw_data_block_data = draw_data_block.as<interop::EncodedGeometryData>();
					draw_data_block_data.position = geometry_buffer.get_buffer_address(VertexAttributeFlag::ePosition);
					draw_data_block_data.texcoord = geometry_buffer.get_buffer_address(VertexAttributeFlag::eTexCoord);
					draw_data_block_data.draw_descriptions_ptr = draw_commands->get_address();
					draw_data_block_data.draw_data_ptr = argument_buffer->get_address();

#if SKINNING_TEST
					draw_data_block_data.joints = geometry_buffer.get_buffer_address(VertexAttributeFlag::eJoint);
					draw_data_block_data.weights = geometry_buffer.get_buffer_address(VertexAttributeFlag::eWeight);
#endif
					self.bind_resources(command_list);

					self.bind_resources(command_list);

					edge::gfx::IndirectDrawCommand draw_command{};
					draw_command.indexed_draw = true;
					draw_command.draw_count = render_world->get_mesh_count();
					draw_command.argument_buffer = argument_buffer;
					draw_command.command_stride = sizeof(interop::DrawIndexedIndirectData);

					uint32_t draw_call_offset{ 0u };
					self.begin(command_list);

					int32_t shadow_index{ 0 };

					auto view = world::EntityManager::view<world::ecs::DirectionalLightComponent>();
					for (auto [entity, light] : view.each()) {
						if (!light.castShadows) {
							continue;
						}

						for (int32_t layer = 0; layer < constants::cascade_shadow_map_cascade_count; ++layer) {
							auto camera_id = render_world->get_resource_id(light.shadow_casters[layer]);
							draw_call_offset = camera_id * render_world->get_mesh_count();

							auto& pc = self.get_constant_block();
							auto& pc_data = pc.as<shadows::PushConstants>();
							pc_data.call_id = draw_call_offset;
							pc_data.shadow_index = shadow_index;
							pc_data.layer = layer;
							self.push_constant_ranges(command_list);

							draw_command.argument_offset = draw_call_offset * draw_command.command_stride;

							command_list.draw_indirect(draw_command);
						}

						shadow_index++;
					}

					self.end(command_list);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Draw omni shadows
		{
			RenderShaderPassCreateInfo create_info{};
#if SKINNING_TEST
			create_info.technique_name = "omni_shadow_skin";
#else
			create_info.technique_name = "omni_shadow";
#endif

			create_info.pipeline_state.rasterization_state.cull_mode = edge::gfx::CullModeFlag::eFront;

			// Depth state
			create_info.pipeline_state.depth_stencil_state.depth_test = true;
			create_info.pipeline_state.depth_stencil_state.depth_write = true;
			create_info.pipeline_state.depth_stencil_state.depth_compare_op = edge::gfx::CompareOp::eLess;

			create_info.add_write_attachment(omni_shadows_target, ResourceAttachmentTypeFlag::eDepth, edge::gfx::ClearColor{ 1.f });

			create_info.pipeline_state.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList;

			create_info.draw_callback = [this,
				omni_shadows_target,
				alive_commands_buffer,
				indirect_draw_commands,
				indirect_count_buffer
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					auto* shadow_map = render_context->get_texture(omni_shadows_target);
					auto* alive_meshes = render_context->get_buffer(alive_commands_buffer);
					auto* argument_buffer = render_context->get_buffer(indirect_draw_commands);

					auto viewport_width = shadow_map->get_width();
					auto viewport_height = shadow_map->get_height();

					command_list.set_viewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
					command_list.set_scissor(0u, 0u, viewport_width, viewport_height);

					self.read_resource(alive_commands_buffer, ~0u);
					self.read_resource(indirect_draw_commands, ~0u);
					self.read_resource(indirect_count_buffer, ~0u);

					self.resolve_dependencies(command_list);

					auto& geometry_buffer = *geometry_buffers.back();
					geometry_buffer.bind_index_buffer(command_list);

					auto& buffers = self.get_uniform_block(render_label::geometry_buffers);
					buffers.set(render_label::position, geometry_buffer.get_buffer_address(VertexAttributeFlag::ePosition));
					buffers.set(render_label::texcoord, geometry_buffer.get_buffer_address(VertexAttributeFlag::eTexCoord));
#if SKINNING_TEST
					buffers.set(render_label::joints, geometry_buffer.get_buffer_address(VertexAttributeFlag::eJoint));
					buffers.set(render_label::weights, geometry_buffer.get_buffer_address(VertexAttributeFlag::eWeight));
#endif

					// Bind buffers that uses in draw
					auto& draw_data_block = self.get_uniform_block(edge::murmur_hash("DrawDescriptionBuffers"));
					draw_data_block.set(edge::murmur_hash("draw_descriptions_ptr"), alive_meshes->get_address());
					draw_data_block.set(edge::murmur_hash("draw_commands_ptr"), argument_buffer->get_address());

					auto& constants = self.get_constant_block();

					self.bind_resources(command_list);

					edge::gfx::IndirectDrawCommand draw_command{};
					draw_command.indexed_draw = true;
					draw_command.draw_count = render_world->get_mesh_count();
					draw_command.argument_buffer = argument_buffer;
					draw_command.command_stride = sizeof(shlib::DrawIndexedIndirectData);
					draw_command.count_buffer = render_context->get_buffer(indirect_count_buffer);

					uint32_t draw_call_id{ 0u };
					self.begin(command_list);

					int32_t shadow_index{ 0 };

					auto view = world::EntityManager::view<world::ecs::PointLightComponent>();
					for (auto [entity, light] : view.each()) {
						if (!light.castShadows) {
							continue;
						}

						for (int32_t layer = 0; layer < 6; ++layer) {
							auto camera_id = render_world->get_resource_id(light.shadow_casters[layer]);
							draw_call_id = camera_id * render_world->get_mesh_count();

							constants.set(render_label::call_id, draw_call_id);
							constants.set(render_label::shadow_index, shadow_index);
							constants.set(render_label::layer, layer);

							self.push_constant_ranges(command_list);

							draw_command.argument_offset = camera_id * draw_command.command_stride;
							draw_command.count_offset = camera_id * sizeof(uint32_t);

							command_list.draw_indirect(draw_command);
						}

						shadow_index++;
					}

					self.end(command_list);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Draw spot shadows
		{
			RenderShaderPassCreateInfo create_info{};
#if SKINNING_TEST
			create_info.technique_name = "cascaded_shadow_skin";
#else
			create_info.technique_name = "cascaded_shadow";
#endif

			create_info.pipeline_state.rasterization_state.cull_mode = edge::gfx::CullModeFlag::eFront;

			// Depth state
			create_info.pipeline_state.depth_stencil_state.depth_test = true;
			create_info.pipeline_state.depth_stencil_state.depth_write = true;
			create_info.pipeline_state.depth_stencil_state.depth_compare_op = edge::gfx::CompareOp::eLess;

			create_info.add_write_attachment(directional_shadows_target, ResourceAttachmentTypeFlag::eDepth, edge::gfx::ClearColor{ 1.f });

			create_info.pipeline_state.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList;

			create_info.draw_callback = [this,
				directional_shadows_target,
				alive_commands_buffer,
				indirect_draw_commands,
				indirect_count_buffer
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					auto* shadow_map = render_context->get_texture(directional_shadows_target);
					auto* alive_meshes = render_context->get_buffer(alive_commands_buffer);
					auto* argument_buffer = render_context->get_buffer(indirect_draw_commands);

					auto viewport_width = shadow_map->get_width();
					auto viewport_height = shadow_map->get_height();

					command_list.set_viewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
					command_list.set_scissor(0u, 0u, viewport_width, viewport_height);

					self.read_resource(alive_commands_buffer, ~0u);
					self.read_resource(indirect_draw_commands, ~0u);
					self.read_resource(indirect_count_buffer, ~0u);

					self.resolve_dependencies(command_list);

					auto& geometry_buffer = *geometry_buffers.back();
					geometry_buffer.bind_index_buffer(command_list);

					auto& buffers = self.get_uniform_block(render_label::geometry_buffers);
					buffers.set(render_label::position, geometry_buffer.get_buffer_address(VertexAttributeFlag::ePosition));
					buffers.set(render_label::texcoord, geometry_buffer.get_buffer_address(VertexAttributeFlag::eTexCoord));
#if SKINNING_TEST
					buffers.set(render_label::joints, geometry_buffer.get_buffer_address(VertexAttributeFlag::eJoint));
					buffers.set(render_label::weights, geometry_buffer.get_buffer_address(VertexAttributeFlag::eWeight));
#endif

					// Bind buffers that uses in draw
					auto& draw_data_block = self.get_uniform_block(edge::murmur_hash("DrawDescriptionBuffers"));
					draw_data_block.set(edge::murmur_hash("draw_descriptions_ptr"), alive_meshes->get_address());
					draw_data_block.set(edge::murmur_hash("draw_commands_ptr"), argument_buffer->get_address());

					auto& constants = self.get_constant_block();

					self.bind_resources(command_list);

					edge::gfx::IndirectDrawCommand draw_command{};
					draw_command.indexed_draw = true;
					draw_command.draw_count = render_world->get_mesh_count();
					draw_command.argument_buffer = render_context->get_buffer(indirect_draw_commands);
					draw_command.command_stride = sizeof(shlib::DrawIndexedIndirectData);
					draw_command.count_buffer = render_context->get_buffer(indirect_count_buffer);

					uint32_t draw_call_id{ 0u };
					self.begin(command_list);

					int32_t shadow_index{ 0 };

					auto view = world::EntityManager::view<world::ecs::SpotLightComponent>();
					for (auto [entity, light] : view.each()) {
						if (!light.castShadows) {
							continue;
						}

						auto camera_id = render_world->get_resource_id(light.shadow_caster);
						draw_call_id = camera_id * render_world->get_mesh_count();

						constants.set(render_label::call_id, draw_call_id);
						constants.set(render_label::shadow_index, shadow_index);
						constants.set(render_label::layer, shadow_index);

						self.push_constant_ranges(command_list);

						draw_command.argument_offset = camera_id * draw_command.command_stride;
						draw_command.count_offset = camera_id * sizeof(uint32_t);

						command_list.draw_indirect(draw_command);

						shadow_index++;
					}

					self.end(command_list);
				};

			render_context->add_node(render_context->create_node(create_info));
		}
#endif

		// Geometry draw
		{
			RenderShaderPassCreateInfo create_info{};
#if SKINNING_TEST
			create_info.technique_name = "visibilitybuffer_write_skin";
#else
			create_info.technique_name = "visibilitybuffer_write";
#endif
			create_info.add_write_attachment(visbuffer_geometry_target, ResourceAttachmentTypeFlag::eColor, edge::gfx::ClearColor{ 0.f, 0.f, 0.f, 0.f });
			create_info.add_write_attachment(visbuffer_vboinst_target, ResourceAttachmentTypeFlag::eColor, edge::gfx::ClearColor{ 0.f, 0.f, 0.f, 0.f });
			create_info.add_write_attachment(zbuffer_depth_target, ResourceAttachmentTypeFlag::eDepth, edge::gfx::ClearColor{ 1.f });

			create_info.draw_callback = [this,
				indirect_draw_commands
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					auto* backbuffer = render_context->get_texture(backbuffer_resource);
					auto* draw_commands = render_world->get_draw_buffer();
					auto* argument_buffer = render_context->get_buffer(indirect_draw_commands);

					auto viewport_width = backbuffer->get_width();
					auto viewport_height = backbuffer->get_height();

					command_list.set_viewport(0.f, 0.f, static_cast<float>(viewport_width), static_cast<float>(viewport_height), 0.f, 1.f);
					command_list.set_scissor(0u, 0u, viewport_width, viewport_height);

					self.read_resource(indirect_draw_commands, ~0u);
					self.resolve_dependencies(command_list);

					auto& geometry_buffer = *geometry_buffers.back();
					geometry_buffer.bind_index_buffer(command_list);

					// Bind buffers that uses in draw
					auto& draw_data_block = self.get_uniform_block(edge::murmur_hash("egd"));
					auto& draw_data_block_data = draw_data_block.as<interop::EncodedGeometryData>();
					draw_data_block_data.vertices = geometry_buffer.get_buffer_address();
					draw_data_block_data.draw_descriptions_ptr = draw_commands->get_address();
					draw_data_block_data.draw_data_ptr = argument_buffer->get_address();
					
					self.bind_resources(command_list);

					auto camera_id = render_world->get_resource_id(main_camera_index);
					auto draw_call_offset = camera_id * render_world->get_mesh_count();

					auto& pc = self.get_constant_block();
					auto& pc_data = pc.as<visbufferwrite::PushConstants>();
					pc_data.call_id = draw_call_offset;
					pc_data.camera_id = camera_id;

					self.push_constant_ranges(command_list);

					edge::gfx::IndirectDrawCommand draw_command{};
					draw_command.indexed_draw = true;
					draw_command.draw_count = render_world->get_mesh_count();
					draw_command.argument_buffer = argument_buffer;
					draw_command.command_stride = sizeof(interop::DrawIndexedIndirectData);
					draw_command.argument_offset = draw_call_offset * draw_command.command_stride;

					self.begin(command_list);
					command_list.draw_indirect(draw_command);
					self.end(command_list);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Copy depth
		{
			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = "copy_image";

			create_info.draw_callback = [this,
				zbuffer_depth_target,
				depth_pyramid_terget,
				nearest_clamp_to_edge_sampler
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
				self.bind_read_resource(render_label::texture0, zbuffer_depth_target, ~0u);
				self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);
				self.bind_write_resource(render_label::image0, depth_pyramid_terget, 0);
				self.resolve_dependencies(command_list);
				self.bind_resources(command_list);

				self.dispatch_for_target(command_list, zbuffer_depth_target, true);
			};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Build depth pyramid
		{
			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = "depth_reduce_4x";

			create_info.draw_callback = [this,
				depth_pyramid_terget,
				nearest_mipmap_linear_reduce_min
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
				auto* target = render_context->get_texture(depth_pyramid_terget);
				auto width = target->get_width();
				auto height = target->get_height();
				auto mip_levels = target->get_mip_level_count();

				self.bind_read_resource(render_label::texture0, depth_pyramid_terget, ~0u);
				self.bind_resource(render_label::sampler0, nearest_mipmap_linear_reduce_min);
				self.bind_write_resource(render_label::image0, depth_pyramid_terget, ~0u);
				self.resolve_dependencies(command_list);
				self.bind_resources(command_list);

				for (int32_t mip = 1; mip < mip_levels;) {
					auto dst_width = width >> mip;
					auto dst_height = height >> mip;

					uint32_t mip_count = reduce::k_mip_per_pass;
					if (mip + mip_count > mip_levels) {
						mip_count = mip_levels - mip;
					}

					auto& pc = self.get_constant_block();
					auto& pc_data = pc.as<reduce::PushConstants>();
					pc_data.mip_count = mip_count;
					pc_data.mip_level = mip - 1;
					pc_data.texel_size = 1.0f / float2(dst_width, dst_height);
					self.push_constant_ranges(command_list);

					uint32_t group_x{ dst_width }, group_y{ dst_height }, group_z{ 1u };
					self.divide_workgroup_by_local_sizes(group_x, group_y, group_z);
					command_list.dispatch(group_x, group_y, group_z);

					mip += mip_count;
				}
			};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Compute ssao
		{
			auto group_node = render_context->create_node("SSAO", 0xFFFF33FF);
			auto* node = static_cast<GroupNode*>(group_node.get());

			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.technique_name = "compute_ssao";

				create_info.draw_callback = [this,
					zbuffer_depth_target,
					ssao_target,
					nearest_clamp_to_edge_sampler
				](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {

					self.bind_read_resource(render_label::texture0, zbuffer_depth_target, ~0u);
					self.bind_write_resource(render_label::image0, ssao_target, ~0u);
					self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);
					self.resolve_dependencies(command_list);

					self.bind_resources(command_list);

					self.dispatch_for_target(command_list, zbuffer_depth_target, true);
					};

				node->add_node(render_context->create_node(create_info));
			}

			// Compute ssao blur vert
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.technique_name = "compute_ssao_blur_vert";

				create_info.draw_callback = [this,
					ssao_target,
					ssao_target_temp,
					zbuffer_depth_target,
					linear_clamp_to_edge_sampler,
					nearest_clamp_to_edge_sampler
				](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					self.bind_read_resource(render_label::texture0, ssao_target);
					self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);
					self.bind_read_resource(render_label::texture1, zbuffer_depth_target);
					self.bind_resource(render_label::sampler1, nearest_clamp_to_edge_sampler);
					self.bind_write_resource(render_label::image0, ssao_target_temp, ~0u);

					self.resolve_dependencies(command_list);

					self.bind_resources(command_list);

					self.dispatch_for_target(command_list, ssao_target_temp, true);
					};

				node->add_node(render_context->create_node(create_info));
			}

			// Compute ssao blur horiz
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.technique_name = "compute_ssao_blur_horiz";

				create_info.draw_callback = [this,
					ssao_target,
					ssao_target_temp,
					zbuffer_depth_target,
					linear_clamp_to_edge_sampler,
					nearest_clamp_to_edge_sampler
				](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					self.bind_read_resource(render_label::texture0, ssao_target_temp);
					self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);

					self.bind_read_resource(render_label::texture1, zbuffer_depth_target);
					self.bind_resource(render_label::sampler1, nearest_clamp_to_edge_sampler);

					self.bind_write_resource(render_label::image0, ssao_target, ~0u);

					self.resolve_dependencies(command_list);

					self.bind_resources(command_list);

					self.dispatch_for_target(command_list, ssao_target_temp, true);
					};

				node->add_node(render_context->create_node(create_info));
			}

			render_context->add_node(std::move(group_node));
		}

#ifdef TILE_MATERIAL_CLASSIFICATION
		// Clear material indirect buffer
		{
			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = "clear_buffer";

			create_info.draw_callback = [this,
				indirect_material_technique_buffer
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {

				auto* indirect_buffer = render_context->get_buffer(indirect_material_technique_buffer);
				"BufferDescription";

				self.resolve_dependencies(command_list);

				auto& constants = self.get_constant_block();
				constants.set(edge::murmur_hash("ptr"), indirect_buffer->get_address());
				constants.set(edge::murmur_hash("size"), indirect_buffer->get_size());

				self.push_constant_ranges(command_list);
				self.bind_resources(command_list);

				self.dispatch_for_buffer(command_list, indirect_material_technique_buffer, true);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Compute per material technique tiles
		{
			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = "material_tile_classification";

			create_info.draw_callback = [this,
				visbuffer_geometry_target,
				indirect_material_technique_buffer,
				material_tiles_buffer
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
				self.bind_write_resource(edge::murmur_hash("IndirectArgumentBuffer"), indirect_material_technique_buffer, ~0u);
				self.bind_write_resource(edge::murmur_hash("MaterialCommandBuffer"), material_tiles_buffer, ~0u);
				self.bind_read_resource(render_label::sample_image0, visbuffer_geometry_target);

				self.resolve_dependencies(command_list);
				self.bind_resources(command_list);

				self.dispatch_for_target(command_list, visbuffer_geometry_target, true);
				};

			render_context->add_node(render_context->create_node(create_info));
		}
#endif
		// Resolve materials
		{
			ComputeShaderPassCreateInfo create_info{};
#if SKINNING_TEST
			create_info.technique_name = "visibilitybuffer_shade_skin";
#else
			create_info.technique_name = "visibilitybuffer_shade";
#endif

			create_info.draw_callback = [this,
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
				omni_shadows_target,
				ssao_target,

				nearest_clamp_to_edge_sampler,
				linear_clamp_to_edge_sampler,
				default_shadow_sampler
#ifdef TILE_MATERIAL_CLASSIFICATION
				, indirect_material_technique_buffer,
				material_tiles_buffer
#endif
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
				EDGE_PROFILE_ZONE_SCOPED;

				auto* draw_commands = render_world->get_draw_buffer();
				auto* argument_buffer = render_context->get_buffer(indirect_draw_commands);

#ifdef TILE_MATERIAL_CLASSIFICATION
				auto* material_argument_buffer = render_context->get_buffer(indirect_material_technique_buffer);
#endif

				// Indirect generated buffers
				self.read_resource(indirect_draw_commands, ~0u);

				self.bind_write_resource(render_label::image0, hdr0, ~0u);
				self.bind_resource(render_label::texture0, brdf_lut_target); // binding 1
				self.bind_resource(render_label::texture1, irradiance_cube_target); // binding 2
				self.bind_resource(render_label::texture2, prefiltered_cube_target); // binding 3

				self.bind_read_resource(render_label::texture3, visbuffer_geometry_target);
				self.bind_read_resource(render_label::texture4, visbuffer_vboinst_target);
				self.bind_read_resource(render_label::texture5, zbuffer_depth_target);

				self.bind_read_resource(render_label::texture6, cascaded_shadows_target);
				self.bind_read_resource(render_label::texture7, directional_shadows_target);
				self.bind_read_resource(render_label::texture8, omni_shadows_target);

				self.bind_read_resource(render_label::texture9, ssao_target, ~0u);

				self.bind_resource(render_label::sampler_point, nearest_clamp_to_edge_sampler);
				self.bind_resource(render_label::sampler_linear, linear_clamp_to_edge_sampler);
				self.bind_resource(render_label::sampler_shadow_linear, default_shadow_sampler);

#ifdef TILE_MATERIAL_CLASSIFICATION
				self.read_resource(indirect_material_technique_buffer, ~0u);
				self.bind_read_resource(edge::murmur_hash("MaterialCommandBuffer"), material_tiles_buffer);
#endif

				self.resolve_dependencies(command_list);

				auto& geometry_buffer = *geometry_buffers.back();
				geometry_buffer.bind_index_buffer(command_list);

				auto& egd = self.get_uniform_block(edge::murmur_hash("egd"));
				auto& egd_data = egd.as<interop::EncodedGeometryData>();
				egd_data.vertices = geometry_buffer.get_buffer_address();
				egd_data.indices = geometry_buffer.get_index_buffer_address();
				egd_data.draw_descriptions_ptr = draw_commands->get_address();
				egd_data.draw_data_ptr = argument_buffer->get_address();

				self.bind_resources(command_list);

#ifdef TILE_MATERIAL_CLASSIFICATION
				command_list.dispatch_indirect(material_argument_buffer, 0);
#else
				self.dispatch_for_target(command_list, hdr0, true);
#endif
			};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Compute static motion vectors (can be started in another queue)
		{
			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = use_low_precision_velocity ? "generate_motion_vectors_low_precision" : "generate_motion_vectors";
			create_info.draw_callback = [this,
				motion_vectors_target,
				zbuffer_depth_target,
				nearest_clamp_to_edge_sampler
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					self.bind_write_resource(render_label::image0, motion_vectors_target, ~0u);

					self.bind_read_resource(render_label::texture0, zbuffer_depth_target);
					self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);

					self.resolve_dependencies(command_list);
					self.bind_resources(command_list);

					self.dispatch_for_target(command_list, zbuffer_depth_target, true);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		input_target_id = hdr0;
		output_target_id = hdr1;

		// Copy history
		{
			ComputeShaderPassCreateInfo create_info{};
			create_info.technique_name = "copy_image";

			create_info.draw_callback = [this,
				hdr0_history,
				nearest_clamp_to_edge_sampler
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					self.bind_read_resource(render_label::texture0, input_target_id);
					self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);

					self.bind_write_resource(render_label::image0, hdr0_history, ~0u);
					self.resolve_dependencies(command_list);
					self.bind_resources(command_list);

					self.dispatch_for_target(command_list, input_target_id, true);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		// Post processing
		{
			uint32_t pass_count{ 0u };

			auto post_proc_node = render_context->create_node("Post Processing", 0xFFFF33FF);
			auto* pp_node = static_cast<GroupNode*>(post_proc_node.get());
			pp_node->set_pre_callback(
				[this](edge::gfx::ICommandList& command_list, float delta_time) {
					current_input_target_id = input_target_id;
					current_output_target_id = output_target_id;
					last_output_target_id = input_target_id;
				});
			pp_node->set_post_callback(
				[this](edge::gfx::ICommandList& command_list, float delta_time) {
					if (current_input_target_id != input_target_id) {
						last_output_target_id = output_target_id;
					}
				});

			// FXAA 
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.technique_name = "apply_fxaa_effect";

				create_info.enabled_callback = []() -> bool { return postprocess.fxaa.enable; };
				create_info.draw_callback = [this,
					linear_clamp_to_edge_sampler
				](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
						EDGE_PROFILE_ZONE_SCOPED;

						if (postprocess.fxaa.enable) {
							self.bind_read_resource(render_label::texture0, current_input_target_id);
							self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);
							self.bind_write_resource(render_label::image0, current_output_target_id, ~0u);
							self.resolve_dependencies(command_list);
							self.bind_resources(command_list);

							auto& pc = self.get_constant_block();
							auto& pc_data = pc.as<apply_effect::PushConstantFXAA>();
							pc_data.enabled = static_cast<int32_t>(postprocess.fxaa.enable);
							pc_data.quality = postprocess.fxaa.quality;
							pc_data.threshold = postprocess.fxaa.threshold;
							pc_data.threshold_min = postprocess.fxaa.threshold_min;
							self.push_constant_ranges(command_list);

							self.dispatch_for_target(command_list, current_input_target_id, true);

							std::swap(current_input_target_id, current_output_target_id);
						}
					};

				pp_node->add_node(render_context->create_node(create_info));
			}

			// Apply motion blur
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.technique_name = use_low_precision_velocity ? "apply_motion_blur_effect_low" : "apply_motion_blur_effect";

				create_info.enabled_callback = []() -> bool { return postprocess.motionblur.enable; };
				create_info.draw_callback = [this,
					motion_vectors_target,
					linear_clamp_to_border_sampler,
					nearest_clamp_to_border_sampler
				](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
						EDGE_PROFILE_ZONE_SCOPED;

						if (postprocess.fxaa.enable) {
							self.bind_read_resource(render_label::texture0, current_input_target_id);
							self.bind_resource(render_label::sampler0, linear_clamp_to_border_sampler);
							self.bind_read_resource(render_label::texture1, motion_vectors_target);
							self.bind_resource(render_label::sampler1, nearest_clamp_to_border_sampler);
							self.bind_write_resource(render_label::image0, current_output_target_id, ~0u);
							self.resolve_dependencies(command_list);
							self.bind_resources(command_list);

							auto& pc = self.get_constant_block();
							auto& pc_data = pc.as<apply_effect::PushConstantMotionBlur>();
							pc_data.enabled = static_cast<int32_t>(postprocess.motionblur.enable);
							pc_data.scale = postprocess.motionblur.scele;
							pc_data.threshold = postprocess.motionblur.threshold;
							pc_data.max_samples = postprocess.motionblur.max_samples;
							self.push_constant_ranges(command_list);

							self.dispatch_for_target(command_list, current_input_target_id, true);

							std::swap(current_input_target_id, current_output_target_id);
						}
					};

				pp_node->add_node(render_context->create_node(create_info));
			}

			// Compute dof
			{
				auto group_node = render_context->create_node("DOF", 0xFFFF33FF);
				auto* node = static_cast<GroupNode*>(group_node.get());

				// Compute bokeh
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.technique_name = "bokeh_filter";

					create_info.draw_callback = [this,
						hdr0_div2,
						linear_clamp_to_edge_sampler
					](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
							EDGE_PROFILE_ZONE_SCOPED;
							if (postprocess.dof.enable) {
								self.bind_read_resource(render_label::texture0, current_input_target_id);
								self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);
								self.bind_write_resource(render_label::image0, hdr0_div2, ~0u);
								self.resolve_dependencies(command_list);
								self.bind_resources(command_list);

								auto& pc = self.get_constant_block(); 
								auto& pc_data = pc.as<bokeh::PushConstant>();
								pc_data.enabled = static_cast<int32_t>(postprocess.dof.enable);
								pc_data.sample_count = postprocess.dof.bokeh_samples;
								pc_data.polygon_count = postprocess.dof.bokeh_poly;
								self.push_constant_ranges(command_list);

								self.dispatch_for_target(command_list, current_input_target_id, true);
							}
						};

					node->add_node(std::move(render_context->create_node(create_info)));
				}

				// Apply dof
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.technique_name = "apply_dof_effect";

					create_info.draw_callback = [this,
						hdr0_div2,
						zbuffer_depth_target,
						linear_clamp_to_edge_sampler,
						nearest_clamp_to_edge_sampler
					](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
							EDGE_PROFILE_ZONE_SCOPED;

							if (postprocess.dof.enable) {
								self.bind_read_resource(render_label::texture0, current_input_target_id);
								self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);

								self.bind_read_resource(render_label::texture1, hdr0_div2);
								self.bind_resource(render_label::sampler1, linear_clamp_to_edge_sampler);

								self.bind_read_resource(render_label::texture2, zbuffer_depth_target);
								self.bind_resource(render_label::sampler2, nearest_clamp_to_edge_sampler);

								self.bind_write_resource(render_label::image0, current_output_target_id, ~0u);
								self.resolve_dependencies(command_list);
								self.bind_resources(command_list);

								auto& pc = self.get_constant_block();
								auto& pc_data = pc.as<apply_effect::PushConstantDoF>();
								pc_data.enabled = static_cast<int32_t>(postprocess.dof.enable);
								pc_data.near_field = postprocess.dof.near_field;
								pc_data.near_transition = postprocess.dof.near_transition;
								pc_data.far_field = postprocess.dof.far_field;
								pc_data.far_transition = postprocess.dof.far_transition;
								pc_data.focus_point = postprocess.dof.focus_point;
								self.push_constant_ranges(command_list);

								self.dispatch_for_target(command_list, current_input_target_id, true);

								std::swap(current_input_target_id, current_output_target_id);
							}
						};

					node->add_node(std::move(render_context->create_node(create_info)));
				}

				pp_node->add_node(std::move(group_node));
			}

			// Compute bloom
			{
				auto group_node = render_context->create_node("bloom", 0xFF3333FF);
				auto* node = static_cast<GroupNode*>(group_node.get());

				// Starting from brightness detection pass
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.technique_name = bloom_use_brightness ? "brightdetect" : "copy_image";

					create_info.draw_callback = [this,
						hdr0_div2,
						nearest_clamp_to_edge_sampler
					](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
							EDGE_PROFILE_ZONE_SCOPED;

							if (postprocess.bloom.enable) {
								self.bind_read_resource(render_label::texture0, current_input_target_id);
								self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);

								self.bind_write_resource(render_label::image0, hdr0_div2, ~0u);
								self.resolve_dependencies(command_list);
								self.bind_resources(command_list);

								auto& pc = self.get_constant_block();
								auto& pc_data = pc.as<brightdetect::PushConstant>();
								pc_data.threshold = postprocess.bloom.threshold;
								self.push_constant_ranges(command_list);

								self.dispatch_for_target(command_list, hdr0_div2, true);
							}
						};

					node->add_node(std::move(render_context->create_node(create_info)));
				}

				// Begin downsampling
				{
					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.technique_name = "downsample_first_pass";

						create_info.draw_callback = [this,
							hdr0_div2,
							bloom_target,
							linear_clamp_to_edge_sampler
						](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
								EDGE_PROFILE_ZONE_SCOPED;

								if (postprocess.bloom.enable) {
									self.write_resource(render_label::image0, bloom_target, ~0u);

									self.bind_read_resource(render_label::texture0, hdr0_div2);
									self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);

									self.bind_resource(render_label::image0, bloom_target);
									self.resolve_dependencies(command_list);
									self.bind_resources(command_list);

									auto& pc = self.get_constant_block();
									auto& pc_data = pc.as<downsampler::PushConstant>();
									pc_data.current_mip = 0;
									self.push_constant_ranges(command_list);

									self.dispatch_for_target(command_list, bloom_target, true);
								}
							};

						node->add_node(std::move(render_context->create_node(create_info)));
					}

					{
						ComputeShaderPassCreateInfo create_info{};
						create_info.technique_name = "downsample";

						create_info.draw_callback = [this,
							bloom_target,
							linear_clamp_to_edge_sampler
						](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
								EDGE_PROFILE_ZONE_SCOPED;

								if (postprocess.bloom.enable) {
									auto target = render_context->get_texture(bloom_target);

									for (int32_t mip = 1; mip < static_cast<int32_t>(bloom_blur_depth); ++mip) {
										self.bind_read_resource(render_label::texture0, bloom_target);
										self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);

										self.bind_write_resource(render_label::image0, bloom_target, mip);
										self.resolve_dependencies(command_list);
										self.bind_resources(command_list);

										auto& pc = self.get_constant_block();
										auto& pc_data = pc.as<downsampler::PushConstant>();
										pc_data.current_mip = mip - 1;
										self.push_constant_ranges(command_list);

										uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

										group_x = target->get_width() >> mip;
										group_y = target->get_height() >> mip;

										self.divide_workgroup_by_local_sizes(group_x, group_y, group_z);
										command_list.dispatch(group_x, group_y, group_z);
									}
								}
							};

						node->add_node(std::move(render_context->create_node(create_info)));
					}
				}

				// Upsample
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.technique_name = "upsample";

					create_info.draw_callback = [this,
						bloom_target,
						linear_clamp_to_edge_sampler
					](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
							EDGE_PROFILE_ZONE_SCOPED;
							if (postprocess.bloom.enable) {
								auto target = render_context->get_texture(bloom_target);

								for (int32_t mip = bloom_blur_depth - 1; mip > 0; --mip) {
									self.bind_read_resource(render_label::texture0, bloom_target);
									self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);

									self.bind_write_resource(render_label::image0, bloom_target, mip - 1);
									self.resolve_dependencies(command_list);
									self.bind_resources(command_list);

									auto& pc = self.get_constant_block();
									auto& pc_data = pc.as<downsampler::PushConstant>();
									pc_data.filter_radius = postprocess.bloom.filter_radius;
									pc_data.current_mip = mip;
									self.push_constant_ranges(command_list);

									uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

									group_x = target->get_width() >> (mip - 1);
									group_y = target->get_height() >> (mip - 1);

									self.divide_workgroup_by_local_sizes(group_x, group_y, group_z);
									command_list.dispatch(group_x, group_y, group_z);
								}
							}
						};

					node->add_node(std::move(render_context->create_node(create_info)));
				}

				// Apply bloom
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.technique_name = "apply_bloom_effect";

					create_info.draw_callback = [this,
						bloom_target,
						dirt_mask_texture,
						linear_clamp_to_edge_sampler
					](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
							EDGE_PROFILE_ZONE_SCOPED;

							if (postprocess.bloom.enable) {
								self.bind_write_resource(render_label::image0, current_output_target_id, ~0u);

								self.bind_read_resource(render_label::texture0, current_input_target_id);
								self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);

								self.bind_read_resource(render_label::texture1, bloom_target);
								self.bind_resource(render_label::sampler1, linear_clamp_to_edge_sampler);

								self.bind_resource(render_label::texture2, dirt_mask_texture);
								self.bind_resource(render_label::sampler2, linear_clamp_to_edge_sampler);

								self.resolve_dependencies(command_list);
								self.bind_resources(command_list);

								auto& pc = self.get_constant_block();
								auto& pc_data = pc.as<apply_effect::PushConstantBloom>();
								pc_data.enabled = static_cast<int32_t>(postprocess.bloom.enable);
								pc_data.strength = postprocess.bloom.strength;
								self.push_constant_ranges(command_list);

								self.dispatch_for_target(command_list, current_output_target_id, true);
								std::swap(current_input_target_id, current_output_target_id);
							}
						};

					node->add_node(std::move(render_context->create_node(create_info)));
				}

				pp_node->add_node(std::move(group_node));
			}

			// Tonemapping 
			{
				auto group_node = render_context->create_node("tonemapping", 0xFFF9F51F);
				auto* node = static_cast<GroupNode*>(group_node.get());

				// Compute histogram
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.technique_name = "compute_histogram";

					create_info.draw_callback = [this,
						histogram_buffer,
						nearest_clamp_to_edge_sampler
					](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
							EDGE_PROFILE_ZONE_SCOPED;

							if (postprocess.tonemap.enable) {
								auto* hist_buffer = render_context->get_buffer(histogram_buffer);

								self.bind_read_resource(render_label::texture0, current_input_target_id);
								self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);

								self.write_resource(histogram_buffer, ~0u);
								self.resolve_dependencies(command_list);
								self.bind_resources(command_list);

								float log_lum_range = postprocess.tonemap.lum_max - postprocess.tonemap.lum_min;

								auto& pc = self.get_constant_block();
								auto& pc_data = pc.as<comp_histogram::PushConstant>();
								pc_data.min_log_lum = postprocess.tonemap.lum_min;
								pc_data.log_lum_range = log_lum_range;
								pc_data.histogram = hist_buffer->get_address();
								self.push_constant_ranges(command_list);

								self.dispatch_for_target(command_list, current_input_target_id, true);
							}
						};

					node->add_node(std::move(render_context->create_node(create_info)));
				}

				// Compute avg screen space brightness
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.technique_name = "compute_brightness";

					create_info.draw_callback = [this,
						avg_brightness_target,
						histogram_buffer,
						nearest_clamp_to_edge_sampler
					](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
							EDGE_PROFILE_ZONE_SCOPED;

							if (postprocess.tonemap.enable) {
								auto* hist_buffer = render_context->get_buffer(histogram_buffer);

								self.read_resource(histogram_buffer, ~0u);

								self.bind_write_resource(render_label::image0, avg_brightness_target, ~0u);

								self.resolve_dependencies(command_list);

								float log_lum_range = postprocess.tonemap.lum_max - postprocess.tonemap.lum_min;
								float time_coeff = glm::clamp<float>(1.0f - glm::exp(-delta_time * postprocess.tonemap.tau), 0.0, 1.0);

								auto* texture = render_context->get_texture(current_input_target_id);

								auto& pc = self.get_constant_block();
								auto& pc_data = pc.as<comp_brightness::PushConstant>();
								pc_data.min_log_lum = postprocess.tonemap.lum_min;
								pc_data.log_lum_range = log_lum_range;
								pc_data.time_coeff = time_coeff;
								pc_data.num_pixels_x = static_cast<float>(texture->get_width());
								pc_data.num_pixels_y = static_cast<float>(texture->get_height());
								pc_data.histogram = hist_buffer->get_address();

								self.push_constant_ranges(command_list);
								self.bind_resources(command_list);

								command_list.dispatch(1u, 1u, 1u);
							}
						};

					node->add_node(std::move(render_context->create_node(create_info)));
				}

				// Apply tonemapping
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.technique_name = "apply_tonemap_effect";

					create_info.draw_callback = [this,
						avg_brightness_target,
						nearest_clamp_to_edge_sampler
					](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
							EDGE_PROFILE_ZONE_SCOPED;

							if (postprocess.tonemap.enable) {
								self.bind_write_resource(render_label::image0, current_output_target_id, ~0u);
								self.bind_read_resource(render_label::image1, avg_brightness_target);

								self.bind_read_resource(render_label::texture0, current_input_target_id);
								self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);

								self.resolve_dependencies(command_list);
								self.bind_resources(command_list);

								auto& pc = self.get_constant_block();
								auto& pc_data = pc.as<apply_effect::PushConstantTonemap>();
								pc_data.enabled = static_cast<int32_t>(postprocess.tonemap.enable);
								pc_data.adaptive = postprocess.tonemap.adaptive;
								pc_data.white_point = postprocess.tonemap.white_point;
								pc_data.exposure = postprocess.tonemap.exposure;
								pc_data.gamma = postprocess.tonemap.gamma;
								self.push_constant_ranges(command_list);

								self.dispatch_for_target(command_list, current_input_target_id, true);

								std::swap(current_input_target_id, current_output_target_id);
							}
						};

					node->add_node(render_context->create_node(create_info));
				}

				pp_node->add_node(std::move(group_node));
			}

			// Chromatic aberration barrel
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.technique_name = "apply_chromatic_aberration_barrel_effect";

				create_info.draw_callback = [this,
					linear_clamp_to_edge_sampler
				](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					if (postprocess.aberration.enable) {
						self.bind_write_resource(render_label::image0, current_output_target_id, ~0u);

						self.bind_read_resource(render_label::texture0, current_input_target_id);
						self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);

						self.resolve_dependencies(command_list);
						self.bind_resources(command_list);

						auto& pc = self.get_constant_block();
						auto& pc_data = pc.as < apply_effect::PushConstantChromaticAberration>();
						pc_data.enabled = static_cast<int32_t>(postprocess.aberration.enable);
						pc_data.distortion = postprocess.aberration.distortion;
						pc_data.iterations = postprocess.aberration.iterations;
						self.push_constant_ranges(command_list);

						self.dispatch_for_target(command_list, current_input_target_id, true);

						std::swap(current_input_target_id, current_output_target_id);
					}
					};

				pp_node->add_node(render_context->create_node(create_info));
			}

			// Film grain
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.technique_name = "apply_film_grain_effect";

				create_info.draw_callback = [this,
					nearest_clamp_to_edge_sampler
				](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					if (postprocess.filmgrain.enable) {
						self.bind_write_resource(render_label::image0, current_output_target_id, ~0u);

						self.bind_read_resource(render_label::texture0, current_input_target_id);
						self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);

						self.resolve_dependencies(command_list);
						self.bind_resources(command_list);

						auto& pc = self.get_constant_block();
						auto& pc_data = pc.as<apply_effect::PushConstantFilmGrain>();
						pc_data.enabled = static_cast<int32_t>(postprocess.filmgrain.enable);
						pc_data.amount = postprocess.filmgrain.amount;
						self.push_constant_ranges(command_list);

						self.dispatch_for_target(command_list, current_input_target_id, true);

						std::swap(current_input_target_id, current_output_target_id);
					}
					};

				pp_node->add_node(render_context->create_node(create_info));
			}

			// Vignette
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.technique_name = "apply_vignette_effect";

				create_info.draw_callback = [this,
					nearest_clamp_to_edge_sampler
				](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					if (postprocess.vignette.enable) {
						self.bind_write_resource(render_label::image0, current_output_target_id, ~0u);

						self.bind_read_resource(render_label::texture0, current_input_target_id);
						self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);

						self.resolve_dependencies(command_list);
						self.bind_resources(command_list);

						auto& pc = self.get_constant_block();
						auto& pc_data = pc.as<apply_effect::PushConstantVignette>();
						pc_data.enabled = static_cast<int32_t>(postprocess.vignette.enable);
						pc_data.opacity = postprocess.vignette.opacity;
						pc_data.inner_radius = postprocess.vignette.inner;
						pc_data.outer_radius = postprocess.vignette.outer;
						self.push_constant_ranges(command_list);

						self.dispatch_for_target(command_list, current_input_target_id, true);

						std::swap(current_input_target_id, current_output_target_id);
					}
					};

				pp_node->add_node(render_context->create_node(create_info));
			}

			render_context->add_node(std::move(post_proc_node));
		}

		// Test output shader
		{
			RenderShaderPassCreateInfo create_info{};
			create_info.technique_name = "screenspace";
			create_info.add_write_attachment(backbuffer_resource, ResourceAttachmentTypeFlag::eColor, edge::gfx::ClearColor{ 1.f, 0.5f, 1.f, 1.f });

			create_info.draw_callback = [this,
				nearest_clamp_to_edge_sampler
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					auto* backbuffer = render_context->get_texture(backbuffer_resource);

					auto viewport_width = static_cast<float>(backbuffer->get_width());
					auto viewport_height = static_cast<float>(backbuffer->get_height());

					// Flip viewport
					command_list.set_viewport(0.f, viewport_height, viewport_width, -viewport_height, 0.f, 1.f);
					command_list.set_scissor(0u, 0u, viewport_width, viewport_height);

					self.bind_read_resource(render_label::texture0, last_output_target_id);
					self.bind_resource(render_label::sampler0, nearest_clamp_to_edge_sampler);

					self.resolve_dependencies(command_list);
					self.bind_resources(command_list);

					self.begin(command_list);
					command_list.draw(3u, 0u, 0u, 1u);
					self.end(command_list);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		// ImDbg renderer
		//{
		//	if (ImDbg::GetCurrentContext())
		//	{
		//		GeometryBufferCreateInfo geometry_buffer_ci{};
		//		geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::ePosition, edge::gfx::Format::eR16G16B16A16Sint);
		//		geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::eTexCoord, edge::gfx::Format::eR16G16Unorm);
		//		geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::eColor, edge::gfx::Format::eR8G8B8A8Unorm);
		//		geometry_buffer_ci.index_description.type = edge::gfx::IndexType::eUint16;
		//
		//		auto geometry_buffer_id = request_interleaved_geometry_buffer(geometry_buffer_ci);
		//		auto& geometry_buffer = lookup_geometry_buffer(geometry_buffer_id);
		//
		//		RenderShaderPassCreateInfo create_info{};
		//		create_info.technique_name = "debug_draw";
		//		create_info.add_write_attachment(backbuffer_resource, ResourceAttachmentTypeFlag::eColor, edge::gfx::ClearColor{ 0.f, 0.f, 0.f, 1.f });
		//	}
		//}

		// Debug text
		{
			GeometryBufferCreateInfo geometry_buffer_ci{};
			geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::ePosition, edge::gfx::Format::eR32G32Sfloat);
			geometry_buffer_ci.index_description.type = edge::gfx::IndexType::eUint16;

			glm::vec2 vertex_positions[] = {
				{0, 16},
				{0, 0},
				{8 * 4, 0},
				{8 * 4, 16}
			};

			uint16_t indices[] = {
				0, 1, 2,
				0, 2, 3,
			};

			auto geometry_buffer_id = request_interleaved_geometry_buffer(geometry_buffer_ci);
			auto& geometry_buffer = lookup_geometry_buffer(geometry_buffer_id);
			geometry_buffer.add_vertex_stream_data(vertex_positions, 4, sizeof(glm::vec2));
			geometry_buffer.add_index_stream_data(indices, 6, sizeof(uint16_t));

			RenderShaderPassCreateInfo create_info{};
			create_info.technique_name = "debug_text";
			create_info.add_write_attachment(backbuffer_resource, ResourceAttachmentTypeFlag::eColor, edge::gfx::ClearColor{ 0.f, 0.f, 0.f, 1.f });

			uint32_t debug_text_buffer{ ~0u };
			{
				edge::gfx::BufferCreateInfo create_info{};
				create_info.size = sizeof(debug_text::DebugTextData) * 512;
				create_info.usage = edge::gfx::BufferUsageFlag::eStorageBufferCPU;

				debug_text_buffer = render_context->add_resource(create_info);
				render_context->set_resource_name(debug_text_buffer, "debug_text_buffer");
			}



			auto debug_text_maker = [](const glm::u16vec2& pos, const char* text, std::vector<debug_text::DebugTextData>& out) {
					int32_t text_len = static_cast<int32_t>(strlen(text));
					int32_t word_count = (text_len + 4) / 4;

					for (int32_t word_idx = 0; word_idx < word_count; ++word_idx) {
						int32_t word_offset = word_idx * 4;
						int32_t char_remain = text_len - word_offset;

						uint32_t word{ 0 };
						memcpy(&word, text + word_offset, char_remain < 4 ? char_remain : 4);

						out.push_back({
								.word = word,
								.pos = {8 * 4 * word_idx * 2.f + pos.x, pos.y},
								.scale = 2.f,
								.fg_color = {255, 255, 255, 255},
								.bg_color = {0, 0, 0, 0}
							});
					}
				};

			static std::vector<debug_text::DebugTextData> text_data{};
			debug_text_maker({ 100, 100 }, "Hello World!", text_data);

			auto* text_buffer = render_context->get_buffer(debug_text_buffer);
			text_buffer->update(text_data.data(), text_data.size() * sizeof(debug_text::DebugTextData));

			create_info.draw_callback = [this,
				&geometry_buffer,
				debug_text_buffer
			](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
					EDGE_PROFILE_ZONE_SCOPED;

					auto* backbuffer = render_context->get_texture(backbuffer_resource);
					glm::vec2 viewport_resolution{ backbuffer->get_width(), backbuffer->get_height() };

					command_list.set_scissor(0, 0, static_cast<uint32_t>(viewport_resolution.x), static_cast<uint32_t>(viewport_resolution.y));
					command_list.set_viewport(0.f, 0.f, viewport_resolution.x, viewport_resolution.y, 0.f, 1.f);

					// Bind vertex and index buffers
					geometry_buffer.bind(command_list);

					auto* text_buffer = render_context->get_buffer(debug_text_buffer);

					auto& pc = self.get_constant_block();
					auto& pc_data = pc.as<debug_text::PushConstants>();
					pc_data.word_data = text_buffer->get_address();
					pc_data.inv_viewport_resolution = 1.f / viewport_resolution;
					self.push_constant_ranges(command_list);
					self.resolve_dependencies(command_list);

					self.begin(command_list);
					command_list.draw_indexed(6u, 0u, static_cast<uint32_t>(text_data.size()), 0u, 0u);
					self.end(command_list);
				};

			render_context->add_node(render_context->create_node(create_info));
		}

		// ImGui renderer
		{
			if (ImGui::GetCurrentContext()) {
				// Create font texture
				unsigned char* pixels;
				int width, height;

				ImGuiIO& io = ImGui::GetIO();
				io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
				size_t upload_size = width * height * 4ull * sizeof(char);

				TextureResourceCreateInfo font_texture_ci{};
				font_texture_ci.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eTransferable;
				font_texture_ci.width = width;
				font_texture_ci.height = height;
				font_texture_ci.format = edge::gfx::Format::eR8G8B8A8Unorm;

				uint32_t imgui_font_texture = render_context->add_resource(font_texture_ci, { pixels, upload_size });

				GeometryBufferCreateInfo geometry_buffer_ci{};
				geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::ePosition, edge::gfx::Format::eR32G32Sfloat);
				geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::eTexCoord, edge::gfx::Format::eR32G32Sfloat);
				geometry_buffer_ci.attribute_descriptions.emplace_back(VertexAttributeFlag::eColor, edge::gfx::Format::eR8G8B8A8Unorm);
				geometry_buffer_ci.index_description.type = edge::gfx::IndexType::eUint16;

				auto geometry_buffer_id = request_interleaved_geometry_buffer(geometry_buffer_ci);
				auto& geometry_buffer = lookup_geometry_buffer(geometry_buffer_id);

				RenderShaderPassCreateInfo create_info{};
				create_info.technique_name = "imgui";
				create_info.add_write_attachment(backbuffer_resource, ResourceAttachmentTypeFlag::eColor, edge::gfx::ClearColor{ 0.f, 0.f, 0.f, 1.f });

				create_info.draw_callback = [this,
					&geometry_buffer,
					imgui_font_texture,
					linear_clamp_to_edge_sampler
				](ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
						EDGE_PROFILE_ZONE_SCOPED;

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
								geometry_buffer.add_vertex_stream_data(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, sizeof(ImDrawVert));
								geometry_buffer.add_index_stream_data(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, sizeof(ImDrawIdx));

								total_vertices += cmd_list->VtxBuffer.Size;
								total_indices += cmd_list->IdxBuffer.Size;
							}

							// Upload geometry to gpu
							geometry_buffer.flush();

							// Bind vertex and index buffers
							geometry_buffer.bind(command_list);

							auto* backbuffer = render_context->get_texture(backbuffer_resource);

							auto viewport_width = backbuffer->get_width();
							auto viewport_height = backbuffer->get_height();

							command_list.set_viewport(0.f, 0.f, static_cast<float>(viewport_width), static_cast<float>(viewport_height), 0.f, 1.f);

							// Setup scale and translation:
							// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
							{
								auto& pc = self.get_constant_block();

								auto& pc_data = pc.as<imgui::PushConstants>();
								pc_data.scale.x = 2.f / draw_data->DisplaySize.x;
								pc_data.scale.y = 2.f / draw_data->DisplaySize.y;
								pc_data.translate.x = -1.0f - draw_data->DisplayPos.x * pc_data.scale.x;
								pc_data.translate.y = -1.0f - draw_data->DisplayPos.y * pc_data.scale.y;

								self.push_constant_ranges(command_list);
							}

							// Will project scissor/clipping rectangles into framebuffer space
							ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
							ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

							// Collect resource dependencies
							for (int32_t n = 0; n < draw_data->CmdListsCount; n++) {
								const ImDrawList* cmd_list = draw_data->CmdLists[n];
								for (int32_t cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
									const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

									uint32_t resource_binding{ (uint32_t)pcmd->TextureId };
									if (sizeof(ImTextureID) < sizeof(ImU64)) {
										// We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
										IM_ASSERT(pcmd->TextureId == (ImTextureID)imgui_font_texture);
										resource_binding = imgui_font_texture;
									}

									if (!pcmd->TextureId) {
										resource_binding = imgui_font_texture;
									}

									self.read_resource(render_label::texture0, resource_binding, ~0u);
									self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);
								}
							}

							self.resolve_dependencies(command_list);

							// Render gui
							uint32_t last_bound_resource{ ~0u };

							self.begin(command_list);

							// Render command lists
							// (Because we merged all buffers into a single one, we maintain our own offset into them)
							int32_t global_vtx_offset = 0;
							int32_t global_idx_offset = 0;
							for (int32_t n = 0; n < draw_data->CmdListsCount; n++) {
								const ImDrawList* cmd_list = draw_data->CmdLists[n];
								for (int32_t cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
									const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
									if (pcmd->UserCallback != nullptr) {
										// User callback, registered via ImDrawList::AddCallback()
										// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
										//if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
										//	ImGui_ImplVulkan_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width, fb_height);
										//else
										pcmd->UserCallback(cmd_list, pcmd);
									}
									else {
										// Project scissor/clipping rectangles into framebuffer space
										ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
										ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

										// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
										if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
										if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
										if (clip_max.x > static_cast<float>(viewport_width)) { clip_max.x = static_cast<float>(viewport_width); }
										if (clip_max.y > static_cast<float>(viewport_height)) { clip_max.y = static_cast<float>(viewport_height); }
										if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
											continue;

										// Apply scissor/clipping rectangle
										command_list.set_scissor((int32_t)(clip_min.x), (int32_t)(clip_min.y), (uint32_t)(clip_max.x - clip_min.x), (uint32_t)(clip_max.y - clip_min.y));

										// Bind DescriptorSet with font or user texture
										uint32_t resource_binding{ (uint32_t)pcmd->TextureId };
										if (sizeof(ImTextureID) < sizeof(ImU64)) {
											// We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
											IM_ASSERT(pcmd->TextureId == (ImTextureID)imgui_font_texture);
											resource_binding = imgui_font_texture;
										}

										if (!pcmd->TextureId) {
											resource_binding = imgui_font_texture;
										}

										if (resource_binding != last_bound_resource) {
											self.bind_resource(render_label::texture0, resource_binding);
											self.bind_resource(render_label::sampler0, linear_clamp_to_edge_sampler);
											self.bind_resources(command_list);
											last_bound_resource = resource_binding;
										}

										command_list.draw_indexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, 1u, pcmd->VtxOffset + global_vtx_offset, 0u);
									}
								}
								global_idx_offset += cmd_list->IdxBuffer.Size;
								global_vtx_offset += cmd_list->VtxBuffer.Size;
							}

							self.end(command_list);
						}
					};

				render_context->add_node(render_context->create_node(create_info));
			}
		}
	}
}