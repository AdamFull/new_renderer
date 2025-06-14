#include "RenderContext.h"
#include "RendererInterface.h"

#include "GroupExecutionNode.h"

#include <edge/util/utl_profiler.h>

#include <format>
#include <edge/util/Logger.h>

#include "render_graph/RenderPass.h"
#include "render_graph/ComputePass.h"

#include <edge/runtime/rt_window.h>

#include <edge/gfx/gfx_util.h>
#include <edge/gfx/gfx_context.h>
#include <edge/gfx/technique_system/gfx_technique_system.h>

#include <edge/fs/fs_virtual_filesystem.h>

#include <edge/util/utl_hash.hpp>

namespace engine::render {
	class RenderContext::RenderFrame {
	public:
		RenderFrame(const edge::gfx::ISwapchain& swapchain, const edge::gfx::IQueue* queue, uint32_t frame_index) {
			backbuffer = edge::gfx::create_swapchain_texture(swapchain, frame_index);
			command_pool = edge::gfx::create_command_pool(*queue);
			descriptor_allocator = edge::gfx::create_descriptor_allocator();

			//descriptor_allocator->allocate({ &global_descriptor, 1 });

			uniform_pool = edge::gfx::create_uniform_pool_unique(16384ull);
			image_avaliable = edge::gfx::create_semaphore(edge::gfx::SemaphoreType::eBinary);
			image_avaliable->set_resource_name(std::format("image_avaliable_{}", frame_index));
			render_finished = edge::gfx::create_semaphore(edge::gfx::SemaphoreType::eBinary);
			render_finished->set_resource_name(std::format("render_finished_{}", frame_index));
			upload_avaliable = edge::gfx::create_semaphore(edge::gfx::SemaphoreType::eBinary);
			render_finished->set_resource_name(std::format("upload_avaliable_{}", frame_index));
			fence = edge::gfx::create_fence();
		}

		~RenderFrame() {
			reset();

			edge::gfx::destroy(backbuffer);
			edge::gfx::destroy(command_pool);
			edge::gfx::destroy(descriptor_allocator);
			edge::gfx::destroy(image_avaliable);
			edge::gfx::destroy(render_finished);
			edge::gfx::destroy(upload_avaliable);
			edge::gfx::destroy(fence);
			uniform_pool.reset();
		}

		void update_backbuffer(const edge::gfx::ISwapchain& swapchain, uint32_t frame_index) {
			edge::gfx::destroy(backbuffer);
			backbuffer = edge::gfx::create_swapchain_texture(swapchain, frame_index);
		}

		void reset() {
			fence->wait();

			// Free layout bindings for this frame
			for (auto [layout_hash, binding_layouts] : binding_layout_table) {
				descriptor_allocator->free(binding_layouts);

				for (auto& binding_layout : binding_layouts) {
					delete binding_layout;
				}
			}
			binding_layout_table.clear();

			//descriptor_allocator->reset();
			command_pool->reset();
			uniform_pool->reset();
		}

		auto lookup_and_allocate_binding_layout(edge::gfx::IDescriptorLayout* descriptor_layout) -> edge::gfx::IBindingLayout* {
			auto found_layout_binding = binding_layout_table.find(descriptor_layout->get_hash());
			if (found_layout_binding != binding_layout_table.end()) {
				
				auto& binding_layouts = found_layout_binding->second;
				auto found_binding_layout = std::find_if(binding_layouts.begin(), binding_layouts.end(),
					[](edge::gfx::IBindingLayout* binding_layout) { return binding_layout->valid(); });

				// We found valid binding layout that we can use
				if (found_binding_layout != binding_layouts.end()) {
					return *found_binding_layout;
				}
			}

			// Binding layout is not found, create new one
			auto& binding_layouts = binding_layout_table[descriptor_layout->get_hash()];
			auto& new_binding_layout = binding_layouts.emplace_back();
			new_binding_layout = edge::gfx::create_binding_layout(*descriptor_layout);
			descriptor_allocator->allocate({ &new_binding_layout , 1 });
			return new_binding_layout;
		}

		edge::gfx::ITexture* backbuffer{ nullptr };
		edge::gfx::ICommandPool* command_pool{ nullptr };
		std::unique_ptr<edge::gfx::IUniformPool> uniform_pool{};
		edge::gfx::ISemaphore* image_avaliable{ nullptr };
		edge::gfx::ISemaphore* render_finished{ nullptr };
		edge::gfx::ISemaphore* upload_avaliable{ nullptr };
		edge::gfx::IFence* fence{ nullptr };

		edge::gfx::IDescriptorAllocator* descriptor_allocator{ nullptr };
		std::unordered_map<uint32_t, std::vector<edge::gfx::IBindingLayout*>> binding_layout_table{};
	};

	RenderContext::RenderContext(edge::Window* window) :
		window{ *window } {
		// Select queue that can present
		queue = edge::gfx::get_queue(edge::gfx::QueueType::eDirect);
		queue->set_resource_name("GRAPHICS");

		edge::gfx::SwapChainCreateInfo swapchain_ci{};
		swapchain_ci.window = window;
		swapchain_ci.image_count = 3u;

		swapchain = edge::gfx::create_swapchain(swapchain_ci);

		auto image_count = swapchain->get_image_count();

		for (int32_t i = 0; i < static_cast<int32_t>(image_count); ++i) {
			render_frames[i] = std::make_unique<RenderFrame>(*swapchain, queue, i);
		}

		// Create default image sampler
		edge::gfx::SamplerCreateInfo sampler_ci{};
		sampler_ci.address_u = sampler_ci.address_v = sampler_ci.address_w = edge::gfx::SamplerAddressMode::eClampToEdge;
		sampler_ci.min_filter = edge::gfx::Filter::eLinear;
		sampler_ci.mag_filter = edge::gfx::Filter::eLinear;
		sampler_ci.mip_map_mode = edge::gfx::Filter::eLinear;

		default_sampler = edge::gfx::create_sampler(sampler_ci);

		// Create timestamp query for collecting pass info
		edge::gfx::QueryPoolCreateInfo query_pool_ci{};
		query_pool_ci.type = edge::gfx::QueryType::eTimestamp;
		query_pool_ci.query_count = 2u;
		timestamp_query = edge::gfx::create_query_pool(query_pool_ci);
		timestamp_query->reset(0u);

		edge::gfx::request_timestamp_frequency(&timestamp_frequency);

		// TODO: Load this layout from bindles_layout.slang
		std::vector<edge::gfx::DescrptorLayoutBindingDesc> binding_descriptions{};
		binding_descriptions.emplace_back(FRAME_DATA_SLOT, 1u, edge::gfx::ShaderStageFlag::eAll, edge::gfx::DescriptorType::eUniformBuffer, edge::murmur_hash("g_fd"));
		binding_descriptions.emplace_back(SAMPLER_SLOT, MAX_SAMPLER_SLOTS, edge::gfx::ShaderStageFlag::eAll, edge::gfx::DescriptorType::eSampler, edge::murmur_hash("g_samplers"));
		binding_descriptions.emplace_back(SRV_TEXTURE_SLOT, MAX_SRV_TEXTURE_SLOTS, edge::gfx::ShaderStageFlag::eAll, edge::gfx::DescriptorType::eSampledImage, edge::murmur_hash("g_srv_textures"));
		binding_descriptions.emplace_back(UAV_TEXTURE_SLOT, MAX_UAV_TEXTURE_SLOTS, edge::gfx::ShaderStageFlag::eAll, edge::gfx::DescriptorType::eStorageImage, edge::murmur_hash("g_uav_textures"));

		edge::gfx::DescriptorLayoutCreateInfo descriptor_layout_ci{};
		descriptor_layout_ci.bindings = binding_descriptions;
		descriptor_layout_ci.support_bindless = true;
		global_descriptor = edge::gfx::create_descriptor_layout(descriptor_layout_ci);
		global_descriptor->set_resource_name("bindless descriptor");

		edge::gfx::TextureCreateInfo dummy_texture_ci{};
		dummy_texture_ci.usage = edge::gfx::TextureUsageFlag::eSamplable | edge::gfx::TextureUsageFlag::eTransferable;
		dummy_texture_ci.width = 4;
		dummy_texture_ci.height = 4;
		dummy_texture_ci.format = edge::gfx::Format::eR8G8B8A8Unorm;

		dummy_texture = edge::gfx::create_texture(dummy_texture_ci);
		dummy_texture->set_resource_name("dummy texture");

		uint8_t dummy_data[64] = { 0 };
		edge::gfx::ResourceStateFlags resource_state{};
		edge::gfx::update_texture(dummy_texture, resource_state, dummy_data);

		// Create uniform pool
		global_uniform_pool = edge::gfx::create_uniform_pool_unique();

		active_frame_index = image_count - 1;
		current_frame = render_frames[active_frame_index].get();

		technique_db = std::make_unique<edge::gfx::TechniqueDatabase>();
		technique_db->add_descriptor_layout(global_descriptor);
		technique_db->load("/internal/tdbcache");
		technique_db->load_config({
			//"/internal_res/assets/shaders/technique_db.json",
			"/internal_res/assets/shaders/technique_db_slang.json",
			current_frame->backbuffer->get_format()
			});

		technique_db->save("/internal/tdbcache");
	}

	RenderContext::~RenderContext() {
		technique_db->save("/internal/tdbcache");
		queue->wait_idle();

		auto image_count = swapchain->get_image_count();
		for (int32_t i = 0; i < static_cast<int32_t>(image_count); ++i) {
			render_frames[i] = nullptr;
		}

		edge::gfx::destroy(swapchain);

		edge::gfx::destroy(default_sampler);
		edge::gfx::destroy(timestamp_query);

		for (auto& texture_object : static_textures.data()) {
			edge::gfx::destroy(texture_object.texture);
		}

		for (auto& texture_object : render_targets.data()) {
			edge::gfx::destroy(texture_object.texture);
		}

		for (auto& buffer : buffers.data()) {
			edge::gfx::destroy(buffer);
		}

		for (auto& sampler : samplers.data()) {
			edge::gfx::destroy(sampler);
		}
	}

	auto RenderContext::add_resource() -> uint32_t {
		// Just creates empty resource)
		auto& new_resource = render_resources.emplace_back();
		return static_cast<uint32_t>(render_resources.size() - 1ull);
	}

	auto RenderContext::add_resource_sync(std::string_view path) -> uint32_t {
		auto& new_resource = render_resources.emplace_back();
		new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

		edge::gfx::ITexture* texture = edge::gfx::load_texture_sync(path.data(), new_resource.current_state);
		new_resource.resource_id = static_textures.add(TextureObject{ texture });

		return static_cast<uint32_t>(render_resources.size() - 1ull);
	}

	auto RenderContext::add_resource(std::string_view path) -> uint32_t {
		auto& new_resource = render_resources.emplace_back();
		new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;
		new_resource.resource_id = static_textures.add(TextureObject{ dummy_texture });
		pending_resources.emplace_back(std::make_pair(edge::gfx::load_texture(path.data()), (uint32_t)(render_resources.size() - 1ull)));

		return static_cast<uint32_t>(render_resources.size() - 1ull);
	}

	auto RenderContext::add_resource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data) -> uint32_t {
		auto& new_resource = render_resources.emplace_back();
		new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

		edge::gfx::TextureCreateInfo texture_create_info{};
		texture_create_info.usage = create_info.usage;
		texture_create_info.width = create_info.width;
		texture_create_info.height = create_info.height;
		texture_create_info.depth = create_info.depth;
		texture_create_info.layers = create_info.layers;
		texture_create_info.levels = create_info.levels;
		texture_create_info.format = create_info.format;

		edge::gfx::ITexture* texture = edge::gfx::create_texture(texture_create_info);
		edge::gfx::update_texture(texture, new_resource.current_state, texture_data);
		new_resource.resource_id = static_textures.add(TextureObject{ texture });

		//new_resource.current_state = edge::gfx::ResourceStateFlag::eShaderReadOnlyResource;

		return static_cast<uint32_t>(render_resources.size() - 1ull);
	}

	auto RenderContext::add_resource(const TextureResourceCreateInfo& create_info) -> uint32_t {
		auto& new_resource = render_resources.emplace_back();

		// TODO: What if it's storage target and render target at the same time?
		if (create_info.usage & edge::gfx::TextureUsageFlag::eStorage) {
			new_resource.resource_type = RenderResourceTypeFlag::eStorageTarget;
		}
		else if (create_info.usage & edge::gfx::TextureUsageFlag::eRenderTarget) {
			new_resource.resource_type = RenderResourceTypeFlag::eRenderTarget;
		}
		else {
			new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;
		}

		edge::gfx::TextureCreateInfo texture_create_info{};
		texture_create_info.usage = create_info.usage;
		texture_create_info.width = static_cast<uint32_t>(create_info.width * create_info.scale);
		texture_create_info.height = static_cast<uint32_t>(create_info.height * create_info.scale);
		texture_create_info.depth = static_cast<uint32_t>(create_info.depth);
		texture_create_info.layers = create_info.layers;
		texture_create_info.levels = create_info.levels;
		texture_create_info.format = create_info.format;

		TextureObject texture_object{};
		texture_object.texture = edge::gfx::create_texture(texture_create_info);
		texture_object.usage = create_info.usage;
		texture_object.resizable = create_info.resizable;
		texture_object.scale = create_info.scale;
		texture_object.scale_parent_resource = create_info.scale_parent_resource;

		if ((create_info.usage & edge::gfx::TextureUsageFlag::eRenderTarget) || (create_info.usage & edge::gfx::TextureUsageFlag::eStorage)) {
			new_resource.resource_id = render_targets.add(std::move(texture_object));
		}
		else {
			new_resource.resource_id = static_textures.add(std::move(texture_object));
		}

		return static_cast<uint32_t>(render_resources.size() - 1ull);
	}

	auto RenderContext::add_resource(const edge::gfx::BufferCreateInfo& create_info) -> uint32_t {
		edge::gfx::IBuffer* buffer = edge::gfx::create_buffer(create_info);
		auto handle = buffers.add(std::move(buffer));
		if (create_info.usage != edge::gfx::BufferUsageFlag::eStagingBuffer && create_info.usage != edge::gfx::BufferUsageFlag::eUniformBuffer) {
			auto& new_resource = render_resources.emplace_back();
			if (create_info.usage & edge::gfx::BufferUsageFlag::eIndirect) {
				new_resource.resource_type = RenderResourceTypeFlag::eIndirectBuffer;
			}
			else if (create_info.usage & edge::gfx::BufferUsageFlag::eStorage) {
				new_resource.resource_type = RenderResourceTypeFlag::eStorageBuffer;
			}

			new_resource.resource_id = handle;

			return static_cast<uint32_t>(render_resources.size() - 1ull);
		}

		return ~0u;
	}

	auto RenderContext::add_resource(const edge::gfx::SamplerCreateInfo& create_info) -> uint32_t {
		auto sampler_hash = edge::gfx::hash(create_info);
		
		for (int32_t rid = 0; rid < static_cast<int32_t>(render_resources.size()); ++rid) {
			auto& render_resource = render_resources[rid];
			if ((render_resource.resource_type & RenderResourceTypeFlag::eSampler) != RenderResourceTypeFlag::eSampler) {
				continue;
			}

			auto& sampler = samplers.get(render_resource.resource_id);
			if (sampler->get_hash() == sampler_hash) {
				return static_cast<uint32_t>(rid);
			}
		}

		auto& new_resource = render_resources.emplace_back();
		new_resource.resource_type = RenderResourceTypeFlag::eSampler;
		new_resource.resource_id = samplers.add(edge::gfx::create_sampler(create_info));
		return static_cast<uint32_t>(render_resources.size() - 1ull);
	}

	void RenderContext::set_resource_name(uint32_t resource_index, std::string_view name) {
		auto& render_resource = render_resources[resource_index];

		edge::gfx::IGFXResource* resource{};
		if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer) {
			resource = current_frame->backbuffer;
		}
		else if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage) {
			auto& texture_object = static_textures.get(render_resource.resource_id);
			resource = texture_object.texture;
		}
		else if (render_resource.resource_type & RenderResourceTypeFlag::eSampler) {
			resource = samplers.get(render_resource.resource_id);
		}
		else if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage) {
			auto& texture_object = render_targets.get(render_resource.resource_id);
			resource = texture_object.texture;
		}
		else if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer) {
			resource = buffers.get(render_resource.resource_id);
		}

		resource->set_resource_name(name);
	}

	auto RenderContext::get_resource(uint32_t resource_index) -> RenderResource& {
		return render_resources[resource_index];
	}

	auto RenderContext::get_resource(uint32_t resource_index) const -> RenderResource const& {
		return render_resources[resource_index];
	}

	auto RenderContext::create_node(std::string_view group_name, uint32_t group_color) -> std::unique_ptr<IExecutionNode> {
		return std::make_unique<GroupNode>(group_name, group_color);
	}

	auto RenderContext::create_node(const RenderShaderPassCreateInfo& create_info) -> std::unique_ptr<IExecutionNode> {
		auto& technique = technique_db->get_technique(create_info.technique_name);

		RenderPassCreateInfo pass_ci{};
		pass_ci.name = technique.name;
		pass_ci.shader = technique.shader;
		pass_ci.root_signature = technique.root_signature;
		pass_ci.pipeline = technique.pipeline;
		pass_ci.descriptor_layouts = technique.descriptor_layouts;
		pass_ci.draw_callback = create_info.draw_callback;
		pass_ci.enabled_callback = create_info.enabled_callback;
		pass_ci.resource_attachments = create_info.resource_attachments;

		return std::make_unique<RenderPass>(*this, pass_ci);
	}

	auto RenderContext::create_node(const ComputeShaderPassCreateInfo& create_info) -> std::unique_ptr<IExecutionNode> {
		auto& technique = technique_db->get_technique(create_info.technique_name);

		ComputePassCreateInfo pass_ci{};
		pass_ci.name = technique.name;
		pass_ci.shader = technique.shader;
		pass_ci.root_signature = technique.root_signature;
		pass_ci.pipeline = technique.pipeline;
		pass_ci.descriptor_layouts = technique.descriptor_layouts;
		pass_ci.one_time = create_info.one_time;
		pass_ci.draw_callback = create_info.draw_callback;
		pass_ci.enabled_callback = create_info.enabled_callback;

		return std::make_unique<ComputePass>(*this, pass_ci);
	}

	void RenderContext::add_node(std::unique_ptr<IExecutionNode>&& node) {
		graph_nodes.emplace_back(std::move(node));
	}

	void RenderContext::execute(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto& command_list = *current_command_list;

		for (auto& node : graph_nodes) {
			node->execute(command_list, delta_time);
		}

		auto& backbuffer_resource = get_resource(0);

		edge::gfx::TextureBarrierDesc texture_barrier{};
		edge::gfx::ResourceBarrierInfo resource_barriers{};
		resource_barriers.texture_barriers = { &texture_barrier, 1 };

		texture_barrier.texture = current_frame->backbuffer;
		// TODO: get current resource state from resource itself
		texture_barrier.src_state = backbuffer_resource.GetState();
		texture_barrier.dst_state = edge::gfx::ResourceStateFlag::ePresent;

		command_list.resource_barrier(resource_barriers);
	}

	void RenderContext::begin() {
		EDGE_PROFILE_ZONE_SCOPED;

		global_uniform_pool->reset();

		bool surface_updated{ false };
		if (!frame_active) {
			if (swapchain) {
				surface_updated = handle_surface_change();
			}

			auto previous_frame = render_frames[active_frame_index].get();

			// We will use the acquired semaphore in a different frame context,
			// so we need to hold ownership.
			acquired_semaphore = previous_frame->image_avaliable;
			previous_frame->reset();

			if (swapchain) {
				if (!swapchain->acquire_next_image(acquired_semaphore, nullptr, &active_frame_index)) {
					surface_updated = handle_surface_change(true);

					bool acquire_result{ false };
					if (surface_updated) {
						acquire_result = swapchain->acquire_next_image(acquired_semaphore, nullptr, &active_frame_index);
					}

					if (!acquire_result) {
						previous_frame->reset();
						return;
					}
				}
			}

			// Now the frame is active again
			frame_active = true;

			current_frame = render_frames[active_frame_index].get();
		}

		if (!acquired_semaphore) {
			throw std::runtime_error("Couldn't begin frame");
		}

		// Begin new command list
		current_command_list = current_frame->command_pool->allocate();
		current_command_list->begin();

		// Update render targets if needed
		if (surface_updated) {
			update_render_targets();
		}

		// Get previous frame time 
		uint64_t timestamps[2] = {};
		timestamp_query->get_data(0u, &timestamps[0]);

		uint64_t elapsed_time = timestamps[1] - timestamps[0];
		if (timestamps[1] <= timestamps[0]) {
			elapsed_time = 0ull;
		}

		gpu_delta_time = static_cast<double>(elapsed_time) * timestamp_frequency / 1000000.0;

		// Reset performance query
		current_command_list->reset_query(timestamp_query, 0u, 2u);
		current_command_list->begin_query(timestamp_query, 0u);

		for (auto it = pending_resources.begin(); it != pending_resources.end();) {
			auto resource_desc = it->first;
			auto resource_uploaded = resource_desc->done.load(std::memory_order_relaxed);
			if (resource_uploaded) {
				auto& render_resource = render_resources[it->second];
				render_resource.current_state = resource_desc->state;

				auto& resource = static_textures.get(render_resource.resource_id);
				resource.texture = resource_desc->resource->as<edge::gfx::ITexture*>();
				it = pending_resources.erase(it);
			}
			else {
				++it;
			}
		}

		std::vector<edge::gfx::TextureBarrierDesc> texture_barriers{};

		// Update bindless textures
		uint32_t array_index{ 0u };
		for (auto& resource : render_resources) {
			if (resource.resource_type != RenderResourceTypeFlag::eStaticImage) {
				continue;
			}

			auto& texture_obj = static_textures.get(resource.resource_id);
			if (resource.current_state != edge::gfx::ResourceStateFlag::eShaderResource && texture_obj.texture != dummy_texture) {
				auto& barrier = texture_barriers.emplace_back(texture_obj.texture, ~0u, resource.current_state, edge::gfx::ResourceStateFlag::eShaderResource);
				resource.current_state = edge::gfx::ResourceStateFlag::eShaderResource;
			}

			edge::gfx::TextureDescriptor texture_descriptor{};
			texture_descriptor.type = edge::gfx::DescriptorType::eSampledImage;
			texture_descriptor.binding = SRV_TEXTURE_SLOT;

			texture_descriptor.array_index = array_index;
			texture_descriptor.texture = texture_obj.texture;
			texture_descriptor.resource_state = edge::gfx::ResourceStateFlag::eShaderResource;

			auto* current_global_binding_layout = current_frame->lookup_and_allocate_binding_layout(global_descriptor);
			current_global_binding_layout->bind(texture_descriptor);

			++array_index;
		}

		array_index = 0u;
		for (auto& resource : render_resources) {
			if (resource.resource_type != RenderResourceTypeFlag::eSampler) {
				continue;
			}

			auto& sampler = samplers.get(resource.resource_id);

			edge::gfx::SamplerDescriptor sampler_descriptor{};
			sampler_descriptor.binding = SAMPLER_SLOT;

			sampler_descriptor.array_index = array_index;
			sampler_descriptor.sampler = sampler;

			auto* current_global_binding_layout = current_frame->lookup_and_allocate_binding_layout(global_descriptor);
			current_global_binding_layout->bind(sampler_descriptor);

			++array_index;
		}

		// Some textures can be just uploaded and we need to change them's state to shader readable
		if (!texture_barriers.empty()) {
			edge::gfx::ResourceBarrierInfo resource_barrier{};
			resource_barrier.texture_barriers = texture_barriers;

			current_command_list->resource_barrier(resource_barrier);
		}
	}

	void RenderContext::end() {
		EDGE_PROFILE_ZONE_SCOPED;

		std::vector<edge::gfx::SemaphoreSubmitInfo> wait_semaphores{};
		wait_semaphores.emplace_back(acquired_semaphore, 0ull, edge::gfx::StageFlag::eColorAttachmentOutput);
		edge::gfx::flush_resource_uploads(wait_semaphores, {});

		// Finish performance query
		current_command_list->end_query(timestamp_query, 1u);

		current_frame->uniform_pool->frame_mark();

		// Submit commands
		current_command_list->end();
		current_frame->fence->reset();

		try {
			EDGE_PROFILE_ZONE_SCOPED_NAMED("Submit");

			// TODO: COMMAND BUFFERS
			edge::gfx::SubmitInfo submit_info{};

			submit_info.wait_semaphore_infos = wait_semaphores;

			edge::gfx::SemaphoreSubmitInfo signal_semaphores[] = {
				edge::gfx::SemaphoreSubmitInfo{ current_frame->render_finished, 0ull, edge::gfx::StageFlag::eColorAttachmentOutput },
				//edge::gfx::SemaphoreSubmitInfo{ current_frame->upload_avaliable, 0ull, edge::gfx::StageFlag::eAllCommands }
			};

			submit_info.signal_semaphore_infos = signal_semaphores;
			submit_info.command_lists = { &current_command_list, 1 };

			queue->submit(submit_info, current_frame->fence);
		}
		catch (std::exception error) {
			EDGE_LOGE("failed to submit Draw command buffer!");
		}

		if (swapchain) {
			EDGE_PROFILE_ZONE_SCOPED_NAMED("Present");

			auto* wait_semaphore = current_frame->render_finished;

			edge::gfx::PresentInfo present_info = {};
			present_info.wait_semaphores = { &wait_semaphore, 1 };
			present_info.swapchains = { &swapchain, 1 };
			present_info.image_indices = &get_current_frame_index();

			queue->present(present_info);
			//current_frame->fence->wait();
			// TODO: This one makes render waiting current frame. Need to remove it in future

			//current_frame_index = (current_frame_index + 1) % swapchain->get_image_count();
		}

		previous_frame_semaphore = current_frame->upload_avaliable;
		if (acquired_semaphore) {
			acquired_semaphore = nullptr;
		}

		// Update render resources state
		for (auto& render_resource : render_resources) {
			if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer) {
				render_resource.current_state = edge::gfx::ResourceStateFlag::eUndefined;
			}
		}

		frame_active = false;
	}

	auto RenderContext::get_texture(uint32_t resource_index) -> edge::gfx::ITexture* {
		auto& render_resource = render_resources[resource_index];
		return get_texture(render_resource);
	}

	auto RenderContext::get_texture(const RenderResource& render_resource) -> edge::gfx::ITexture* {
		if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer) {
			return current_frame->backbuffer;
		}
		else if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage) {
			auto& texture_object = static_textures.get(render_resource.resource_id);
			return texture_object.texture;
		}
		else if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage) {
			auto& texture_object = render_targets.get(render_resource.resource_id);
			return texture_object.texture;
		}

		return nullptr;
	}

	auto RenderContext::get_sampler(uint32_t resource_index) -> edge::gfx::ISampler* {
		auto& render_resource = render_resources[resource_index];
		return get_sampler(render_resource);
	}

	auto RenderContext::get_sampler(const RenderResource& render_resource) -> edge::gfx::ISampler* {
		if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer) {
			return default_sampler;
		}
		else {
			return samplers.get(render_resource.resource_id);
		}

		return default_sampler;
	}

	auto RenderContext::get_buffer(uint32_t resource_index) -> edge::gfx::IBuffer* {
		auto& render_resource = render_resources[resource_index];
		return get_buffer(render_resource);
	}

	auto RenderContext::get_buffer(const RenderResource& render_resource) -> edge::gfx::IBuffer* {
		if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer) {
			return buffers.get(render_resource.resource_id);
		}

		return nullptr;
	}

	//edge::gfx::SemaphoreSubmitInfo RenderContext::GetUploadAvaliableSemaphore() {
	//	return edge::gfx::SemaphoreSubmitInfo{ previous_frame_semaphore, 0ull, edge::gfx::StageFlag::eAllCommands };
	//}

	edge::gfx::BufferSuballocation RenderContext::request_uniform_block(uint64_t block_size) {
		return current_frame->uniform_pool->request_block(block_size);
	}

	edge::gfx::ICommandList& RenderContext::get_current_command_list() {
		return *current_command_list;
	}

	auto RenderContext::get_binding_layouts(std::span<edge::gfx::IDescriptorLayout*> descriptor_layouts) 
		-> std::vector<edge::gfx::IBindingLayout*> {
		// TODO: need to think how to reuse this
		std::vector<edge::gfx::IBindingLayout*> binding_layouts{ descriptor_layouts.size() };

		for (int32_t i = 0; i < descriptor_layouts.size(); ++i) {
			binding_layouts[i] = current_frame->lookup_and_allocate_binding_layout(descriptor_layouts[i]);
		}

		return binding_layouts;
	}

	void RenderContext::update_global_binding(uint32_t binding, std::span<const uint8_t> data) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto buffer_block = global_uniform_pool->request_block(data.size());
		buffer_block.update(data);

		edge::gfx::BufferDescriptor buffer_descriptor{};
		buffer_descriptor.type = edge::gfx::DescriptorType::eUniformBuffer;
		buffer_descriptor.binding = binding;
		buffer_descriptor.buffer = &buffer_block.get_buffer();
		buffer_descriptor.offset = static_cast<uint32_t>(buffer_block.get_offset());
		buffer_descriptor.size = static_cast<uint32_t>(buffer_block.get_size());

		auto* current_global_binding_layout = current_frame->lookup_and_allocate_binding_layout(global_descriptor);
		current_global_binding_layout->bind(buffer_descriptor);
	}

	const uint32_t& RenderContext::get_current_frame_index() const {
		return active_frame_index;
	}

	void RenderContext::update_swapchain(uint32_t width, uint32_t height) {
		edge::gfx::SwapChainCreateInfo new_swapchain_ci{};
		new_swapchain_ci.window = &window;
		new_swapchain_ci.old_swapchain = swapchain;
		new_swapchain_ci.width = width;
		new_swapchain_ci.height = height;

		swapchain = edge::gfx::create_swapchain(new_swapchain_ci);
		edge::gfx::destroy(new_swapchain_ci.old_swapchain);
		update_resources();
	}

	void RenderContext::update_swapchain(uint32_t image_count) {
		edge::gfx::SwapChainCreateInfo new_swapchain_ci{};
		new_swapchain_ci.window = &window;
		new_swapchain_ci.old_swapchain = swapchain;
		new_swapchain_ci.image_count = image_count;

		swapchain = edge::gfx::create_swapchain(new_swapchain_ci);
		edge::gfx::destroy(new_swapchain_ci.old_swapchain);
		update_resources();
	}

	bool RenderContext::handle_surface_change(bool force_update) {
		EDGE_PROFILE_ZONE_SCOPED;

		if (!swapchain) {
			EDGE_LOGW("Can't handle surface changes in headless mode, skipping.");
			return false;
		}

		uint32_t surface_width, surface_height;
		swapchain->get_surface_extent(surface_width, surface_height);
		if (surface_width == 0xFFFFFFFFu) {
			return false;
		}

		auto& current_width = swapchain->get_width();
		auto& current_height = swapchain->get_height();

		// Only recreate the swapchain if the dimensions have changed;
		// handle_surface_changes() is called on VK_SUBOPTIMAL_KHR,
		// which might not be due to a surface resize
		if (surface_width != current_width ||
			surface_height != current_height ||
			force_update) {
			EDGE_PROFILE_ZONE_SCOPED_NAMED("ISwapchain update");
			// Recreate swapchain
			queue->wait_idle();
			update_swapchain(surface_width, surface_height);

			active_frame_index = 0u;

			return true;
		}

		return false;
	}

	void RenderContext::update_render_targets() {
		EDGE_PROFILE_ZONE_SCOPED;

		uint32_t width, height;
		swapchain->get_surface_extent(width, height);

		// Update/resize render targets here if it's needed
		for (auto& target : render_targets.data()) {
			// TODO: resize only if texture is able to resize 
			if (!target.resizable) {
				continue;
			}

			edge::gfx::TextureCreateInfo create_info{};
			create_info.usage = target.usage;
			create_info.layers = (target.usage & edge::gfx::TextureUsageFlag::eCubemap) ? target.texture->get_array_layer_count() / 6u : target.texture->get_array_layer_count();
			// TODO: need to recalculate number of mips?
			create_info.levels = target.texture->get_mip_level_count();
			create_info.format = target.texture->get_format();

			if (target.scale_parent_resource != ~0u) {
				edge::gfx::ITexture* parent = get_texture(target.scale_parent_resource);

				create_info.width = static_cast<uint32_t>(parent->get_width() * target.scale);
				create_info.height = static_cast<uint32_t>(parent->get_height() * target.scale);
			}
			else {
				create_info.width = static_cast<uint32_t>(width * target.scale);
				create_info.height = static_cast<uint32_t>(height * target.scale);
			}

			edge::gfx::destroy(target.texture);
			target.texture = edge::gfx::create_texture(create_info);
		}
	}

	void RenderContext::update_resources() {
		EDGE_PROFILE_ZONE_SCOPED;

		auto image_count = swapchain->get_image_count();

		auto frame_it = render_frames.begin();
		for (int32_t i = 0; i < static_cast<int32_t>(image_count); ++i) {
			if (frame_it != render_frames.end()) {
				(*frame_it)->update_backbuffer(*swapchain, i);
			}
			else {
				// Create a new frame if the new swapchain has more images than current frames
				render_frames[i] = std::make_unique<RenderFrame>(*swapchain, queue, i);
			}

			++frame_it;
		}
	}
}