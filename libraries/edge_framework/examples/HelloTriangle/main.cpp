#include <edge/Edge.h>
#include <edge/util/Logger.h>
#include <edge/util/Profiler.h>

#include <edge/gfx/gfx_context.h>
#include <edge/gfx/ext/resource_loader/gfx_resource_loader.h>

#include <regex>

#include "HelloTriangleVert.h"
#include "HelloTriangleFrag.h"

using namespace edge;

struct BasicVertex
{
	struct Vec2 { float x, y; } pos;
	struct Vec3 { float x, y, z; } color;
};

class VFSWrapper final : public gfx::FilesystemWrapper
{
public:
	VFSWrapper(edge::filesystem::IVirtualFileSystemInterface& vfs) :
		vfs{ vfs }
	{
	}

	bool Exists(std::string_view path) override
	{
		return vfs.Exists(path);
	}

	bool Mkdir(std::string_view path) override
	{
		return vfs.CreateDirectories(path);
	}

	bool ReadFile(std::string_view path, std::vector<uint8_t>& data) override
	{
		return vfs.ReadFile(path, data);
	}

	bool ReadFile(std::string_view path, std::string& data) override
	{
		return vfs.ReadFile(path, data);
	}

	bool WriteFile(std::string_view path, const std::vector<uint8_t>& data) override
	{
		return vfs.WriteFile(path, data);
	}

	size_t FileSize(std::string_view path) override
	{
		return vfs.FileSize(path);
	}

	std::vector<std::string> ListDir(std::string_view path, std::span<const std::string> include_rules, std::span<const std::string> exclude_rules) override
	{
		std::vector<std::string> list_files{};

		auto iterator = vfs.Walk(path, true);
		for (auto& it = iterator->Begin(); it != iterator->End(); ++it)
		{
			const auto& entry = *it;

			// Skip files that do not match the include rules
			if (!include_rules.empty() && !MatchesPattern(entry, include_rules))
				continue;

			// Skip files that match the exclude rules
			if (!exclude_rules.empty() && MatchesPattern(entry, exclude_rules))
				continue;

			list_files.emplace_back(entry);
		}

		return list_files;
	}

private:
	// Helper function to convert wildcard pattern to regex pattern
	std::string WildcardToRegex(const std::string& pattern)
	{
		std::string regex_pattern;
		regex_pattern.reserve(pattern.size() * 2);

		for (char ch : pattern)
		{
			switch (ch) {
			case '*':
				regex_pattern += ".*"; // Match zero or more characters
				break;
			case '?':
				regex_pattern += "."; // Match exactly one character
				break;
			case '.':
				regex_pattern += "\\."; // Escape dot
				break;
			default:
				regex_pattern += ch; // Add character as is
				break;
			}
		}
		return regex_pattern;
	}

	bool MatchesPattern(const std::string& file_name, const std::span<const std::string>& patterns)
	{
		for (const auto& pattern : patterns)
		{
			std::regex regex(WildcardToRegex(pattern), std::regex_constants::icase);
			if (std::regex_match(file_name, regex))
				return true;
		}
		return false;
	}
private:
	edge::filesystem::IVirtualFileSystemInterface& vfs;
};

struct RenderFrame
{
	RenderFrame(gfx::Context& context, gfx::Swapchain* swapchain, const gfx::Queue* queue, uint32_t frame_index) :
		context{ context }
	{
		backbuffer = context.CreateSwapchainTexture(swapchain, frame_index);
		command_pool = context.CreateCommandPool(queue);
		descriptor_allocator = context.CreateDescriptorAllocator();

		image_avaliable = context.CreateSemaphore();
		image_avaliable->SetResourceName(std::format("image_avaliable_{}", frame_index));

		render_finished = context.CreateSemaphore();
		render_finished->SetResourceName(std::format("render_finished_{}", frame_index));

		upload_avaliable = context.CreateSemaphore();
		render_finished->SetResourceName(std::format("upload_avaliable_{}", frame_index));

		fence = context.CreateFence();
	}

	void UpdateBackbuffer(gfx::Swapchain* swapchain, uint32_t frame_index)
	{
		backbuffer = context.CreateSwapchainTexture(swapchain, frame_index);
	}

	void Reset()
	{
		descriptor_allocator->Reset();
		command_pool->Reset();
	}

	std::unique_ptr<gfx::Texture> backbuffer;
	std::unique_ptr<gfx::CommandPool> command_pool;
	std::unique_ptr<gfx::DescriptorAllocator> descriptor_allocator;
	std::unique_ptr<gfx::Semaphore> image_avaliable;
	std::unique_ptr<gfx::Semaphore> render_finished;
	std::unique_ptr<gfx::Semaphore> upload_avaliable;
	std::unique_ptr<gfx::Fence> fence;

private:
	gfx::Context& context;
};


class Application final : public edge::Application
{
public:
    Application()
    {
        edge::LoggerCreateInfo logger_create_info{};
        logger_create_info.logger_name = "EdgeHelloTriangle";
        edge::CreateLogger(logger_create_info);

        EDGE_LOGI("Starting app: {}", name);
    }

	~Application()
	{
		queue->WaitIdle();

		auto image_count = swapchain->GetImageCount();
		for (int32_t i = 0; i < image_count; ++i)
		{
			render_frames[i] = nullptr;
		}
	}

    bool Prepare(const edge::ApplicationOptions& options) override
    {
		window = options.window;

        gfx::ContextCreateInfo gfx_create_info{};
        gfx_create_info.app_name = "EdgeHelloTriangle";
        gfx_create_info.render_api = gfx::RenderApi::eVulkan;
        gfx_create_info.window = options.window;

        // Init graphics context
        gfx_context = gfx::CreateContext(gfx_create_info);

        // Create filesystem wrapper
        gfx_fs_wrapper = std::make_unique<VFSWrapper>(*options.filesystem);

        gfx::ext::ResourceLoaderCreateInfo loader_create_info{};
        loader_create_info.context = gfx_context.get();
        loader_create_info.filesystem_wrapper = gfx_fs_wrapper.get();

		// You can also implement your own resource loader
        resource_loader = gfx::ext::CreateResourceLoader(loader_create_info);

		// Init everything we need for rendering
		queue = gfx_context->RequestQueue(gfx::QueueTypeFlag::ePresent);

		gfx::SwapChainCreateInfo swapchain_ci{};
		swapchain_ci.window = options.window;
		swapchain_ci.image_count = 2u;

		swapchain = gfx_context->CreateSwapchain(swapchain_ci);

		auto image_count = swapchain->GetImageCount();

		for (int32_t i = 0; i < image_count; ++i)
		{
			render_frames[i] = std::make_unique<RenderFrame>(*gfx_context, swapchain, queue, i);
		}

		// Create shader and pipeline
		const gfx::ShaderStageInfo shader_stages[]
		{
			{
				.stage = gfx::ShaderStageFlag::eVertex,
				.code = { reinterpret_cast<const uint8_t*>(HelloTriangleVert), HelloTriangleVert_size }
			},
			{
				.stage = gfx::ShaderStageFlag::eFragment,
				.code = { reinterpret_cast<const uint8_t*>(HelloTriangleFrag), HelloTriangleFrag_size }
			}
		};

		shader = gfx_context->CreateShader(shader_stages);

		// Create root signature
		gfx::RootSignatureCreateInfo root_sign_ci{};
		root_sign_ci.shader = shader.get();

		root_sign = gfx_context->CreateRootSignature(root_sign_ci);

		gfx::GraphicsPipelineCreateInfo pipeline_ci{};
		pipeline_ci.shader = shader.get();
		pipeline_ci.root_signature = root_sign.get();

		pipeline_ci.color_formats[0] = render_frames[0]->backbuffer->GetFormat();
		pipeline_ci.color_format_count = 1u;

		pipeline_ci.pipeline_state.rasterization_state.front_face = gfx::FrontFace::eCW;

		// Position
		pipeline_ci.pipeline_state.vertex_layout.attributes[0].binding = 0;
		pipeline_ci.pipeline_state.vertex_layout.attributes[0].location = 0;
		pipeline_ci.pipeline_state.vertex_layout.attributes[0].offset = offsetof(BasicVertex, pos);
		pipeline_ci.pipeline_state.vertex_layout.attributes[0].format = gfx::Format::eR32G32Sfloat;

		// UV
		pipeline_ci.pipeline_state.vertex_layout.attributes[1].binding = 0;
		pipeline_ci.pipeline_state.vertex_layout.attributes[1].location = 1;
		pipeline_ci.pipeline_state.vertex_layout.attributes[1].offset = offsetof(BasicVertex, color);
		pipeline_ci.pipeline_state.vertex_layout.attributes[1].format = gfx::Format::eR32G32B32Sfloat;

		pipeline_ci.pipeline_state.vertex_layout.attribute_count = 2u;

		pipeline_ci.pipeline_state.vertex_layout.bindings[0].binding = 0;
		pipeline_ci.pipeline_state.vertex_layout.bindings[0].stride = sizeof(BasicVertex);
		pipeline_ci.pipeline_state.vertex_layout.binding_count = 1u;

		pipeline_ci.pipeline_state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
		pipeline_ci.pipeline_state.color_blending_state.attachment_count = 1u;

		pipeline = gfx_context->CreatePipeline(pipeline_ci);

		// Create vertex buffers
		const BasicVertex vertices[] = 
		{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		gfx::BufferCreateInfo vertex_buffer_ci{};
		vertex_buffer_ci.usage = gfx::BufferUsageFlag::eVertexBuffer;
		vertex_buffer_ci.size = sizeof(vertices);

		vertex_buffer = gfx_context->CreateBuffer(vertex_buffer_ci);

		// Upload vertex data in to buffer
		gfx::BufferUpdateInfo update_info{};
		update_info.dst_buffer = vertex_buffer.get();
		update_info.update_size = sizeof(vertices);

		resource_loader->BeginBufferUpdate(update_info);

		memcpy(update_info.map_info.data, &vertices[0], sizeof(vertices));
		update_info.regions[0].size = sizeof(vertices);
		update_info.region_count = 1u;

		resource_loader->EndBufferUpdate(update_info);

		return true;
    }

    void Update(float delta_time) override
    {
		EDGE_PROFILE_ZONE_SCOPED;

		bool surface_updated{ false };
		if (swapchain)
		{
			surface_updated = HandleSurfaceChange();
		}

		auto& previous_frame = render_frames[active_frame_index];

		acquired_semaphore = previous_frame->image_avaliable.get();

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
					previous_frame->Reset();
					return;
				}
			}
		}

		// New frame begins here
		previous_frame->Reset();

		auto& current_frame = render_frames[active_frame_index];

		auto* command_list = current_frame->command_pool->Allocate();
		command_list->Begin();

		gfx::TextureBarrier texture_translation{};
		texture_translation.texture = current_frame->backbuffer.get();
		texture_translation.src_state = gfx::ResourceStateFlag::eUndefined;
		texture_translation.dst_state = gfx::ResourceStateFlag::eColorAttachmentWrite;

		gfx::ResourceBarrierInfo barrier{};
		barrier.texture_barriers = &texture_translation;
		barrier.texture_barrier_count = 1u;

		command_list->ResourceBarrier(barrier);

		command_list->BindPipeline(pipeline.get());

		uint32_t stride{ 0u };
		const uint64_t offset{ 0ull };
		command_list->BindVertexBuffers(std::span{ (vertex_buffer.get()), 1}, std::span{&stride, 1}, std::span{&offset, 1});

		gfx::RenderingInfo rendering_info{};
		rendering_info.width = current_frame->backbuffer->GetWidth();
		rendering_info.height = current_frame->backbuffer->GetHeight();
		rendering_info.attachments[0].texture = current_frame->backbuffer.get();
		rendering_info.attachments[0].texture_state = texture_translation.dst_state;
		rendering_info.attachments[0].load_op = gfx::LoadOp::eClear;
		rendering_info.attachments[0].store_op = gfx::StoreOp::eStore;
		rendering_info.attachment_count = 1;

		command_list->SetViewport(0, 0, rendering_info.width, rendering_info.height, 0.f, 1.f);
		command_list->SerScissor(0, 0, rendering_info.width, rendering_info.height);

		command_list->BeginRendering(rendering_info);
		command_list->Draw(3u, 0u, 0u, 1u);
		command_list->EndRendering();

		// Prepare image for present
		texture_translation.src_state = texture_translation.dst_state;
		texture_translation.dst_state = gfx::ResourceStateFlag::ePresentSrc;

		command_list->ResourceBarrier(barrier);

		// Submit resource uploader queue
		gfx::SemaphoreSubmitInfo upload_wait_semaphore = resource_loader->Submit({});

		// Here is frame finished
		command_list->End();

		current_frame->fence->Reset();

		try
		{
			EDGE_PROFILE_ZONE_SCOPED_NAMED("Submit");

			// TODO: COMMAND BUFFERS
			gfx::SubmitInfo submit_info{};

			gfx::SemaphoreSubmitInfo wait_semaphores[2] = {
				gfx::SemaphoreSubmitInfo{ acquired_semaphore, 0ull, gfx::StageFlag::eColorAttachmentOutput },
				upload_wait_semaphore
			};

			submit_info.wait_semaphore_infos = wait_semaphores;


			gfx::SemaphoreSubmitInfo signal_semaphores[1] = {
				gfx::SemaphoreSubmitInfo{ current_frame->render_finished.get(), 0ull, gfx::StageFlag::eColorAttachmentOutput},
				//gfx::SemaphoreSubmitInfo{ current_frame->upload_avaliable, 0ull, gfx::StageFlag::eAllCommands }
			};

			submit_info.signal_semaphore_infos = signal_semaphores;

			submit_info.command_lists = { &command_list, 1 };

			queue->Submit(submit_info, current_frame->fence.get());
		}
		catch (std::exception error)
		{
			EDGE_LOGE("failed to submit Draw command buffer!");
		}

		if (swapchain)
		{
			EDGE_PROFILE_ZONE_SCOPED_NAMED("Present");

			auto* wait_semaphore = current_frame->render_finished.get();

			gfx::PresentInfo present_info = {};
			present_info.wait_semaphores = { &wait_semaphore, 1 };

			present_info.swapchains = { &swapchain.get(), 1};
			present_info.image_indices = &active_frame_index;

			queue->Present(present_info);
			current_frame->fence->Wait();

			//current_frame_index = (current_frame_index + 1) % swapchain->GetImageCount();
		}

		if (acquired_semaphore)
		{
			acquired_semaphore = nullptr;
		}
    }

private:
	void UpdateSwapchain(uint32_t width, uint32_t height)
	{
		gfx::SwapChainCreateInfo new_swapchain_ci{};
		new_swapchain_ci.window = window;
		new_swapchain_ci.old_swapchain = swapchain;
		new_swapchain_ci.width = width;
		new_swapchain_ci.height = height;

		swapchain = gfx_context->CreateSwapchain(new_swapchain_ci);
		gfx_context->Destroy(new_swapchain_ci.old_swapchain);
		
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
				render_frames[i] = std::make_unique<RenderFrame>(*gfx_context, swapchain, queue, i);
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
		if (surface_width != current_width || surface_height != current_height || force_update)
		{
			EDGE_PROFILE_ZONE_SCOPED_NAMED("Swapchain update");
			// Recreate swapchain
			queue->WaitIdle();
			UpdateSwapchain(surface_width, surface_height);

			active_frame_index = 0u;

			return true;
		}

		return false;
	}
private:
    std::unique_ptr<gfx::Context> gfx_context{};
    std::unique_ptr<gfx::FilesystemWrapper> gfx_fs_wrapper{};
    std::unique_ptr<gfx::ext::ResourceLoader> resource_loader{};

	edge::Window* window{ nullptr };

	const gfx::Queue* queue;

	std::unique_ptr<gfx::Swapchain> swapchain;
	std::array<std::unique_ptr<RenderFrame>, 8ull> render_frames{};

	uint32_t active_frame_index{ 0u };
	gfx::Semaphore* acquired_semaphore{ nullptr };

	std::unique_ptr<gfx::Shader> shader;
	std::unique_ptr<gfx::RootSignature> root_sign;
	std::unique_ptr<gfx::Pipeline> pipeline;

	std::unique_ptr<gfx::Buffer> vertex_buffer;
};

int RuntimeMain(const edge::RuntimeContext& context)
{
    auto runtime = edge::CreateApplicationContext(context);

    edge::ApplicationCreateInfo app_ci{};
    app_ci.name = "EdgeHelloTriangle";
    app_ci.app_allocator = []() { return std::make_unique<Application>(); };

    runtime->RequestApplication(&app_ci);

    auto code = runtime->Initialize();
    if (code == edge::ExitCode::Success)
    {
        code = runtime->MainLoop();
    }

    runtime->Terminate(code);

    return 0;
}