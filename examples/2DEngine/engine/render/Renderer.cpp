#include "Renderer.h"

#include <edge/gfx/gfx_context.h>
#include <edge/gfx/gfx_resource_loader.h>
#include <edge/util/utl_profiler.h>
#include <edge/util/Logger.h>

namespace e2d {
	// Renderer frame impl
	RendererFrame::~RendererFrame() {
		reset();
	}

	auto RendererFrame::construct(const edge::gfx::IQueue& queue, const edge::gfx::ISwapchain& swapchain, 
		uint32_t frame_index) -> std::unique_ptr<IRendererFrame> {
		std::unique_ptr<RendererFrame> self = std::make_unique<RendererFrame>();
		self->index = frame_index;
		self->create_resources(queue, swapchain);

		return self;
	}

	void RendererFrame::reset() {
		fence->wait();

		// Free layout bindings for this frame
		for (auto& [layout_hash, binding_layouts] : binding_layout_table) {
			descriptor_allocator->free(binding_layouts);
		}
		binding_layout_table.clear();

		command_list = nullptr;
		command_pool->reset();
		uniform_pool->reset();
	}

	void RendererFrame::update(const edge::gfx::ISwapchain& swapchain) {
		backbuffer = edge::gfx::create_swapchain_texture_unique(swapchain, index);
	}

	void RendererFrame::begin() {
		if (!command_list) {
			command_list = command_pool->allocate();
			command_list->begin();
		}
	}

	void RendererFrame::end() {
		uniform_pool->frame_mark();
		command_list->end();
		fence->reset();
		command_list = nullptr;
	}

	auto RendererFrame::get_fence() -> edge::gfx::IFence& {
		return *fence;
	}

	auto RendererFrame::get_command_list() -> edge::gfx::ICommandList& {
		return *command_list;
	}

	auto RendererFrame::get_acquired_semaphore() -> edge::gfx::ISemaphore& {
		return *image_avaliable;
	}
	auto RendererFrame::get_ready_semaphore() -> edge::gfx::ISemaphore& {
		return *render_finished;
	}

	auto RendererFrame::get_backbuffer() const -> edge::gfx::ITexture& {
		return *backbuffer;
	}

	auto RendererFrame::get_or_allocate_binding_layout(edge::gfx::IDescriptorLayout& descriptor_layout) -> edge::gfx::IBindingLayout& {
		auto found_layout_binding = binding_layout_table.find(descriptor_layout.get_hash());
		if (found_layout_binding != binding_layout_table.end()) {

			auto& binding_layouts = found_layout_binding->second;
			auto found_binding_layout = std::find_if(binding_layouts.begin(), binding_layouts.end(),
				[](const std::unique_ptr<edge::gfx::IBindingLayout>& binding_layout) { return binding_layout->valid(); });

			// We found valid binding layout that we can use
			if (found_binding_layout != binding_layouts.end()) {
				return *(*found_binding_layout);
			}
		}

		// Binding layout is not found, create new one
		auto& binding_layouts = binding_layout_table[descriptor_layout.get_hash()];
		auto& new_binding_layout = binding_layouts.emplace_back();
		new_binding_layout = edge::gfx::create_binding_layout_unique(descriptor_layout);

		auto* binding_layout = new_binding_layout.get();
		descriptor_allocator->allocate({ &binding_layout , 1 });
		return *new_binding_layout;
	}

	auto RendererFrame::allocate_uniform_block(uint64_t size) -> edge::gfx::BufferSuballocation {
		return uniform_pool->request_block(size);
	}

	void RendererFrame::create_resources(const edge::gfx::IQueue& queue, const edge::gfx::ISwapchain& swapchain) {
		update(swapchain);

		uniform_pool = edge::gfx::create_uniform_pool_unique(65535ull);

		command_pool = edge::gfx::create_command_pool_unique(queue);
		descriptor_allocator = edge::gfx::create_descriptor_allocator_unique();

		image_avaliable = edge::gfx::create_semaphore_unique(edge::gfx::SemaphoreType::eBinary);
		image_avaliable->set_resource_name(std::format("image_avaliable_{}", index));

		render_finished = edge::gfx::create_semaphore_unique(edge::gfx::SemaphoreType::eBinary);
		render_finished->set_resource_name(std::format("render_finished_{}", index));

		fence = edge::gfx::create_fence_unique();
		fence->set_resource_name(std::format("frame_{}", index));
	}




	Renderer::Renderer(const edge::gfx::IQueue& queue) :
		graphics_queue{ queue } {

	}

	Renderer::~Renderer() {
		graphics_queue.wait_idle();
	}

	auto Renderer::construct(const RendererCreateInfo& create_info) -> std::unique_ptr<IRenderer> {
		std::unique_ptr<Renderer> self{};

		auto* direct_queue = edge::gfx::get_queue(edge::gfx::QueueType::eDirect);
		self = std::make_unique<Renderer>(*direct_queue);
		self->create_resources(*create_info.window);

		return self;
	}

	void Renderer::begin_frame(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		if (frame_active) {
			EDGE_LOGE("[Renderer] Frame is already started");
			return;
		}

		bool surface_updated{ false };
		if (swapchain) {
			surface_updated = handle_surface_change();
		}

		auto* previous_frame = frames_in_flight[active_frame_index].get();

		// We will use the acquired semaphore in a different frame context,
		// so we need to hold ownership.
		acquired_semaphore = &previous_frame->get_acquired_semaphore();
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

		current_frame = frames_in_flight[active_frame_index].get();

		if (!acquired_semaphore) {
			EDGE_LOGE("[Renderer] Couldn't begin frame");
		}

		current_frame->begin();
		auto& cmd_list = current_frame->get_command_list();

		// Get previous frame time 
		uint64_t timestamps[2] = {};
		query_pool->get_data(0u, &timestamps[0]);

		uint64_t elapsed_time = timestamps[1] - timestamps[0];
		if (timestamps[1] <= timestamps[0]) {
			elapsed_time = 0ull;
		}

		gpu_delta_time = static_cast<double>(elapsed_time) * timestamp_frequency / 1000000.0;

		// Reset performance query
		cmd_list.reset_query(query_pool.get(), 0u, 2u);
		cmd_list.begin_query(query_pool.get(), 0u);
	}

	void Renderer::end_frame(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		std::vector<edge::gfx::SemaphoreSubmitInfo> wait_semaphores{};
		wait_semaphores.emplace_back(acquired_semaphore, 0ull, edge::gfx::StageFlag::eColorAttachmentOutput);
		edge::gfx::flush_resource_uploads(wait_semaphores, {});

		// Finish performance query
		edge::gfx::ICommandList* cmd_list = &current_frame->get_command_list();
		cmd_list->end_query(query_pool.get(), 1u);

		// TODO: needed only for now
		edge::gfx::TextureBarrierDesc texture_barrier{};
		texture_barrier.src_state = edge::gfx::ResourceStateFlag::eUndefined;
		texture_barrier.dst_state = edge::gfx::ResourceStateFlag::ePresent;
		texture_barrier.texture = &current_frame->get_backbuffer();

		edge::gfx::ResourceBarrierInfo barrier{};
		barrier.texture_barriers = { &texture_barrier, 1 };
		cmd_list->resource_barrier(barrier);

		current_frame->end();

		auto* ready_sem = &current_frame->get_ready_semaphore();

		try {
			EDGE_PROFILE_ZONE_SCOPED_NAMED("Submit");

			// TODO: COMMAND BUFFERS
			edge::gfx::SubmitInfo submit_info{};

			submit_info.wait_semaphore_infos = wait_semaphores;

			edge::gfx::SemaphoreSubmitInfo signal_semaphores[] = {
				edge::gfx::SemaphoreSubmitInfo{ ready_sem, 0ull, edge::gfx::StageFlag::eColorAttachmentOutput}
			};

			submit_info.signal_semaphore_infos = signal_semaphores;
			submit_info.command_lists = { &cmd_list, 1 };

			auto& frame_fence = current_frame->get_fence();
			graphics_queue.submit(submit_info, &frame_fence);
		}
		catch (std::exception error) {
			EDGE_LOGE("failed to submit Draw command buffer!");
		}

		if (swapchain) {
			EDGE_PROFILE_ZONE_SCOPED_NAMED("Present");

			auto* swapchain_ptr = swapchain.get();

			edge::gfx::PresentInfo present_info = {};
			present_info.wait_semaphores = { &ready_sem, 1 };
			present_info.swapchains = { &swapchain_ptr, 1};
			present_info.image_indices = &active_frame_index;

			graphics_queue.present(present_info);
		}

		frame_active = false;
	}

	void Renderer::on_surface_update() {
		EDGE_PROFILE_ZONE_SCOPED;

		auto image_count = swapchain->get_image_count();

		auto frame_it = frames_in_flight.begin();
		for (int32_t i = 0; i < static_cast<int32_t>(image_count); ++i) {
			if (frame_it != frames_in_flight.end()) {
				(*frame_it)->update(*swapchain);
			}
			else {
				// Create a new frame if the new swapchain has more images than current frames
				frames_in_flight.emplace_back(RendererFrame::construct(graphics_queue, *swapchain, i));
			}

			++frame_it;
		}
	}

	void Renderer::create_resources(const edge::Window& window) {
		this->window = &window;

		// Create swapchain resource
		edge::gfx::SwapChainCreateInfo swapchain_ci{};
		swapchain_ci.window = &window;
		swapchain_ci.image_count = 3u;
		swapchain = edge::gfx::create_swapchain_unique(swapchain_ci);

		auto image_count = swapchain->get_image_count();
		for (int32_t i = 0; i < static_cast<int32_t>(image_count); ++i) {
			frames_in_flight.emplace_back(RendererFrame::construct(graphics_queue, *swapchain, i));
		}

		active_frame_index = image_count - 1;
		current_frame = frames_in_flight[active_frame_index].get();

		// For performance measurements
		edge::gfx::QueryPoolCreateInfo query_ci{};
		query_ci.type = edge::gfx::QueryType::eTimestamp;
		query_ci.query_count = 2;
		query_pool = edge::gfx::create_query_pool_unique(query_ci);
		query_pool->reset(0u);

		edge::gfx::request_timestamp_frequency(&timestamp_frequency);
	}

	void Renderer::update_swapchain(uint32_t width, uint32_t height) {
		edge::gfx::SwapChainCreateInfo new_swapchain_ci{};
		new_swapchain_ci.window = window;
		new_swapchain_ci.old_swapchain = swapchain.get();
		new_swapchain_ci.width = width;
		new_swapchain_ci.height = height;

		swapchain = edge::gfx::create_swapchain_unique(new_swapchain_ci);
		on_surface_update();
	}

	void Renderer::update_swapchain(uint32_t image_count) {
		edge::gfx::SwapChainCreateInfo new_swapchain_ci{};
		new_swapchain_ci.window = window;
		new_swapchain_ci.old_swapchain = swapchain.get();
		new_swapchain_ci.image_count = image_count;

		swapchain = edge::gfx::create_swapchain_unique(new_swapchain_ci);
		on_surface_update();
	}

	bool Renderer::handle_surface_change(bool force) {
		EDGE_PROFILE_ZONE_SCOPED;

		if (!swapchain) {
			EDGE_LOGW("[Renderer] Can't handle surface changes in headless mode, skipping.");
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
			surface_height != current_height || force) {
			EDGE_PROFILE_ZONE_SCOPED_NAMED("ISwapchain update");
			// Recreate swapchain
			graphics_queue.wait_idle();
			update_swapchain(surface_width, surface_height);

			active_frame_index = 0u;

			return true;
		}

		return false;
	}
}