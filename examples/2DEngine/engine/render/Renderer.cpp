#include "Renderer.h"

#include <edge/gfx/gfx_context.h>
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

		command_pool->reset();
		//uniform_pool->reset();
	}

	void RendererFrame::update(const edge::gfx::ISwapchain& swapchain) {
		backbuffer = edge::gfx::create_swapchain_texture_unique(swapchain, index);
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

	void RendererFrame::create_resources(const edge::gfx::IQueue& queue, const edge::gfx::ISwapchain& swapchain) {
		update(swapchain);

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
	}

	void Renderer::end_frame(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;
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