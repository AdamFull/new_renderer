#include <edge/Edge.h>
#include <edge/util/Logger.h>
#include <edge/util/utl_profiler.h>

#include <edge/gfx/gfx_context.h>
#include <edge/gfx/gfx_resource_loader.h>

#include <edge/fs/fs_virtual_filesystem.h>

#include <regex>

#include "HelloTriangleVert.h"
#include "HelloTriangleFrag.h"

struct BasicVertex
{
	struct Vec2 { float x, y; } pos;
	struct Vec3 { float x, y, z; } color;
};

struct RenderFrame
{
	RenderFrame(const edge::gfx::ISwapchain& swapchain, const edge::gfx::IQueue* queue, uint32_t frame_index)
	{
		backbuffer = edge::gfx::create_swapchain_texture(swapchain, frame_index);
		command_pool = edge::gfx::create_command_pool(*queue);
		descriptor_allocator = edge::gfx::create_descriptor_allocator();

		image_avaliable = edge::gfx::create_semaphore(edge::gfx::SemaphoreType::eBinary);
		image_avaliable->set_resource_name(std::format("image_avaliable_{}", frame_index));

		render_finished = edge::gfx::create_semaphore(edge::gfx::SemaphoreType::eBinary);
		render_finished->set_resource_name(std::format("render_finished_{}", frame_index));

		upload_avaliable = edge::gfx::create_semaphore(edge::gfx::SemaphoreType::eBinary);
		render_finished->set_resource_name(std::format("upload_avaliable_{}", frame_index));

		fence = edge::gfx::create_fence();
	}

	~RenderFrame()
	{
		edge::gfx::destroy(backbuffer);
		edge::gfx::destroy(command_pool);
		edge::gfx::destroy(descriptor_allocator);
		edge::gfx::destroy(image_avaliable);
		edge::gfx::destroy(render_finished);
		edge::gfx::destroy(upload_avaliable);
		edge::gfx::destroy(fence);
	}

	void UpdateBackbuffer(const edge::gfx::ISwapchain& swapchain, uint32_t frame_index)
	{
		edge::gfx::destroy(backbuffer);
		backbuffer = edge::gfx::create_swapchain_texture(swapchain, frame_index);
	}

	void Reset()
	{
		descriptor_allocator->reset();
		command_pool->reset();
	}

	edge::gfx::ITexture* backbuffer{ nullptr };
	edge::gfx::ICommandPool* command_pool{ nullptr };
	edge::gfx::IDescriptorAllocator* descriptor_allocator{ nullptr };
	edge::gfx::ISemaphore* image_avaliable{ nullptr };
	edge::gfx::ISemaphore* render_finished{ nullptr };
	edge::gfx::ISemaphore* upload_avaliable{ nullptr };
	edge::gfx::IFence* fence{ nullptr };
};


class Application final : public edge::Application
{
public:
	~Application() override
	{
		queue->wait_idle();

		auto image_count = swapchain->get_image_count();
		for (int32_t i = 0; i < static_cast<int32_t>(image_count); ++i)
		{
			render_frames[i] = nullptr;
		}

		edge::gfx::destroy(swapchain);
	}

    bool prepare(const edge::ApplicationOptions& options) override
    {
		window = options.window;

		// Init everything we need for rendering
		queue = edge::gfx::get_queue(edge::gfx::QueueType::eDirect);

		edge::gfx::SwapChainCreateInfo swapchain_ci{};
		swapchain_ci.window = options.window;
		swapchain_ci.image_count = 2u;

		swapchain = edge::gfx::create_swapchain(swapchain_ci);

		auto image_count = swapchain->get_image_count();

		for (int32_t i = 0; i < static_cast<int32_t>(image_count); ++i)
		{
			render_frames[i] = std::make_unique<RenderFrame>(*swapchain, queue, i);
		}

		// Create shader and pipeline
		const edge::gfx::ShaderStageInfo shader_stages[]
		{
			{
				.stage = edge::gfx::ShaderStageFlag::eVertex,
				.code = { reinterpret_cast<const uint8_t*>(HelloTriangleVert), HelloTriangleVert_size }
			},
			{
				.stage = edge::gfx::ShaderStageFlag::eFragment,
				.code = { reinterpret_cast<const uint8_t*>(HelloTriangleFrag), HelloTriangleFrag_size }
			}
		};

		shader = edge::gfx::create_shader(shader_stages);

		// Create root signature
		edge::gfx::RootSignatureCreateInfo root_sign_ci{};
		root_sign_ci.shader = shader;

		root_sign = edge::gfx::create_root_signature(root_sign_ci);

		edge::gfx::GraphicsPipelineCreateInfo pipeline_ci{};
		pipeline_ci.shader = shader;
		pipeline_ci.root_signature = root_sign;
		pipeline_ci.color_formats.emplace_back(render_frames[0]->backbuffer->get_format());

		pipeline_ci.pipeline_state.rasterization_state.front_face = edge::gfx::FrontFace::eCW;

		// Position
		auto& pos_attr = pipeline_ci.pipeline_state.vertex_layout.attributes.emplace_back();
		pos_attr.binding = 0;
		pos_attr.location = 0;
		pos_attr.offset = offsetof(BasicVertex, pos);
		pos_attr.format = edge::gfx::Format::eR32G32Sfloat;

		// UV
		auto& uv_attr = pipeline_ci.pipeline_state.vertex_layout.attributes.emplace_back();
		uv_attr.binding = 0;
		uv_attr.location = 1;
		uv_attr.offset = offsetof(BasicVertex, color);
		uv_attr.format = edge::gfx::Format::eR32G32B32Sfloat;

		auto& binding = pipeline_ci.pipeline_state.vertex_layout.bindings.emplace_back();
		binding.stride = sizeof(BasicVertex);

		pipeline_ci.pipeline_state.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList;
		pipeline_ci.pipeline_state.color_blending_state.attachments.emplace_back();

		pipeline = edge::gfx::create_pipeline(pipeline_ci);

		// Create vertex buffers
		const BasicVertex vertices[] = 
		{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		edge::gfx::BufferCreateInfo vertex_buffer_ci{};
		vertex_buffer_ci.usage = edge::gfx::BufferUsageFlag::eVertexBuffer;
		vertex_buffer_ci.size = sizeof(vertices);

		vertex_buffer = edge::gfx::create_buffer(vertex_buffer_ci);

		// Upload vertex data in to buffer
		edge::gfx::ResourceStateFlags resource_state{};
		edge::gfx::update_buffer(vertex_buffer, resource_state, { (const uint8_t*)(&vertices[0]), sizeof(vertices) });

		return true;
    }

    void update(float delta_time) override
    {
		EDGE_PROFILE_ZONE_SCOPED;

		bool surface_updated{ false };
		if (swapchain)
		{
			surface_updated = HandleSurfaceChange();
		}

		auto& previous_frame = render_frames[active_frame_index];

		acquired_semaphore = previous_frame->image_avaliable;

		if (swapchain)
		{
			if (!swapchain->acquire_next_image(acquired_semaphore, nullptr, &active_frame_index))
			{
				surface_updated = HandleSurfaceChange(true);

				bool acquire_result{ false };
				if (surface_updated)
				{
					acquire_result = swapchain->acquire_next_image(acquired_semaphore, nullptr, &active_frame_index);
				}

				if (!acquire_result)
				{
					previous_frame->Reset();
					return;
				}
			}
		}

		// New frame begins here
		previous_frame->Reset();

		auto& current_frame = render_frames[active_frame_index];

		auto* command_list = current_frame->command_pool->allocate();
		command_list->begin();

		edge::gfx::TextureBarrierDesc texture_translation{};
		texture_translation.texture = current_frame->backbuffer;
		texture_translation.src_state = edge::gfx::ResourceStateFlag::eUndefined;
		texture_translation.dst_state = edge::gfx::ResourceStateFlag::eRenderTarget;

		edge::gfx::BufferBarrierDesc vertex_buffer_barrier{};
		vertex_buffer_barrier.buffer = vertex_buffer;
		vertex_buffer_barrier.src_state = edge::gfx::ResourceStateFlag::eCopyDst;
		vertex_buffer_barrier.dst_state = edge::gfx::ResourceStateFlag::eVertexRead;
		vertex_buffer_barrier.range.size = vertex_buffer->get_size();

		edge::gfx::ResourceBarrierInfo barrier{};
		barrier.texture_barriers = { &texture_translation, 1 };
		barrier.buffer_barriers = { &vertex_buffer_barrier, 1 };

		command_list->resource_barrier(barrier);
		barrier.buffer_barriers = {};

		command_list->bind_pipeline(pipeline);

		uint32_t stride{ 0u };
		const uint64_t offset{ 0ull };
		command_list->bind_vertex_buffers(std::span{ &vertex_buffer, 1 }, std::span{ &stride, 1 }, std::span{ &offset, 1 });

		edge::gfx::RenderingInfo rendering_info{};
		rendering_info.width = current_frame->backbuffer->get_width();
		rendering_info.height = current_frame->backbuffer->get_height();
		auto& attachment = rendering_info.attachments.emplace_back();
		attachment.texture = current_frame->backbuffer;
		attachment.texture_state = texture_translation.dst_state;
		attachment.load_op = edge::gfx::LoadOp::eClear;
		attachment.store_op = edge::gfx::StoreOp::eStore;

		command_list->set_viewport(0, 0, static_cast<float>(rendering_info.width), static_cast<float>(rendering_info.height), 0.f, 1.f);
		command_list->set_scissor(0, 0, rendering_info.width, rendering_info.height);

		command_list->begin_rendering(rendering_info);
		command_list->draw(3u, 0u, 0u, 1u);
		command_list->end_rendering();

		// Prepare image for present
		texture_translation.src_state = texture_translation.dst_state;
		texture_translation.dst_state = edge::gfx::ResourceStateFlag::ePresent;

		command_list->resource_barrier(barrier);

		std::vector<edge::gfx::SemaphoreSubmitInfo> wait_semaphores{};
		wait_semaphores.emplace_back(acquired_semaphore, 0ull, edge::gfx::StageFlag::eColorAttachmentOutput);

		// Submit resource uploader queue
		edge::gfx::flush_resource_uploads(wait_semaphores, {});

		// Here is frame finished
		command_list->end();

		current_frame->fence->reset();

		try
		{
			EDGE_PROFILE_ZONE_SCOPED_NAMED("Submit");

			// TODO: COMMAND BUFFERS
			edge::gfx::SubmitInfo submit_info{};
			submit_info.wait_semaphore_infos = wait_semaphores;

			edge::gfx::SemaphoreSubmitInfo signal_semaphores[1] = {
				edge::gfx::SemaphoreSubmitInfo{ current_frame->render_finished, 0ull, edge::gfx::StageFlag::eColorAttachmentOutput },
				//edge::gfx::SemaphoreSubmitInfo{ current_frame->upload_avaliable, 0ull, edge::gfx::StageFlag::eAllCommands }
			};

			submit_info.signal_semaphore_infos = signal_semaphores;

			submit_info.command_lists = { &command_list, 1 };

			queue->submit(submit_info, current_frame->fence);
		}
		catch (std::exception error)
		{
			EDGE_LOGE("failed to submit Draw command buffer!");
		}

		if (swapchain)
		{
			EDGE_PROFILE_ZONE_SCOPED_NAMED("Present");

			auto* wait_semaphore = current_frame->render_finished;

			edge::gfx::PresentInfo present_info = {};
			present_info.wait_semaphores = { &wait_semaphore, 1 };

			present_info.swapchains = { &swapchain, 1 };
			present_info.image_indices = &active_frame_index;

			queue->present(present_info);
			current_frame->fence->wait();

			//current_frame_index = (current_frame_index + 1) % swapchain->get_image_count();
		}

		if (acquired_semaphore)
		{
			acquired_semaphore = nullptr;
		}
    }

private:
	void UpdateSwapchain(uint32_t width, uint32_t height)
	{
		edge::gfx::SwapChainCreateInfo new_swapchain_ci{};
		new_swapchain_ci.window = window;
		new_swapchain_ci.old_swapchain = swapchain;
		new_swapchain_ci.width = width;
		new_swapchain_ci.height = height;

		swapchain = edge::gfx::create_swapchain(new_swapchain_ci);
		edge::gfx::destroy(new_swapchain_ci.old_swapchain);
		
		auto image_count = swapchain->get_image_count();
		auto frame_it = render_frames.begin();
		for (int32_t i = 0; i < static_cast<int32_t>(image_count); ++i)
		{
			if (frame_it != render_frames.end())
			{
				(*frame_it)->UpdateBackbuffer(*swapchain, i);
			}
			else
			{
				// Create a new frame if the new swapchain has more images than current frames
				render_frames[i] = std::make_unique<RenderFrame>(*swapchain, queue, i);
			}

			++frame_it;
		}
	}

	bool HandleSurfaceChange(bool force_update = false)
	{
		EDGE_PROFILE_ZONE_SCOPED;

		if (!swapchain)
		{
			EDGE_LOGW("Can't handle surface changes in headless mode, skipping.");
			return false;
		}

		uint32_t surface_width, surface_height;
		swapchain->get_surface_extent(surface_width, surface_height);
		if (surface_width == 0xFFFFFFFFu)
		{
			return false;
		}

		auto& current_width = swapchain->get_width();
		auto& current_height = swapchain->get_height();

		// Only recreate the swapchain if the dimensions have changed;
		// handle_surface_changes() is called on VK_SUBOPTIMAL_KHR,
		// which might not be due to a surface resize
		if (surface_width != current_width || surface_height != current_height || force_update)
		{
			EDGE_PROFILE_ZONE_SCOPED_NAMED("ISwapchain update");
			// Recreate swapchain
			queue->wait_idle();
			UpdateSwapchain(surface_width, surface_height);

			active_frame_index = 0u;

			return true;
		}

		return false;
	}
private:
	edge::Window* window{ nullptr };

	const edge::gfx::IQueue* queue;

	edge::gfx::ISwapchain* swapchain{ nullptr };
	std::array<std::unique_ptr<RenderFrame>, 8ull> render_frames{};

	uint32_t active_frame_index{ 0u };
	edge::gfx::ISemaphore* acquired_semaphore{ nullptr };

	edge::gfx::IShader* shader{ nullptr };
	edge::gfx::IRootSignature* root_sign{ nullptr };
	edge::gfx::IPipeline* pipeline{ nullptr };

	edge::gfx::IBuffer* vertex_buffer{ nullptr };
};

int runtime_main(edge::IRuntimeContext& context)
{
    auto runtime = edge::create_application_context(context);

    edge::ApplicationCreateInfo app_ci{};
	app_ci.features.use_graphics = true;
	app_ci.features.use_uploader = true;
    app_ci.name = "EdgeHelloTriangle";
    app_ci.app_allocator = []() { return std::make_unique<Application>(); };

    runtime->request_application(&app_ci);

    auto code = runtime->initialize();
    if (code == edge::ExitCode::Success)
    {
        code = runtime->main_loop();
    }

    runtime->terminate(code);

    return 0;
}