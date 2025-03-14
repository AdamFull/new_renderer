#include "RenderContext.h"
#include "UniformPool.h"
#include "RendererInterface.h"

#include "GroupExecutionNode.h"

#include <edge/util/Profiler.h>

#include <format>
#include <edge/util/Logger.h>

#include "render_graph/RenderPass.h"
#include "render_graph/ComputePass.h"

#include <edge/runtime/Window.h>

#include <edge/gfx/gfx_util.h>
#include <edge/gfx/gfx_context.h>
#include <edge/gfx/technique_system/gfx_technique_system.h>

#include <edge/fs/fs_virtual_filesystem.h>

namespace engine
{
	namespace render
	{
		class RenderContext::RenderFrame
		{
		public:
			RenderFrame(edge::gfx::IContext& context, edge::gfx::ISwapchain* swapchain, const edge::gfx::IQueue* queue, uint32_t frame_index) :
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

			void UpdateBackbuffer(edge::gfx::ISwapchain* swapchain, uint32_t frame_index)
			{
				context.Destroy(backbuffer);
				backbuffer = context.CreateSwapchainTexture(swapchain, frame_index);
			}

			void Reset()
			{
				descriptor_allocator->Reset();
				command_pool->Reset();
				uniform_pool->Reset();
			}

			edge::gfx::ITexture* backbuffer{ nullptr };
			edge::gfx::ICommandPool* command_pool{ nullptr };
			edge::gfx::IDescriptorAllocator* descriptor_allocator{ nullptr };
			std::unique_ptr<UniformPool> uniform_pool{};
			edge::gfx::ISemaphore* image_avaliable{ nullptr };
			edge::gfx::ISemaphore* render_finished{ nullptr };
			edge::gfx::ISemaphore* upload_avaliable{ nullptr };
			edge::gfx::IFence* fence{ nullptr };

		private:
			edge::gfx::IContext& context;
		};

		RenderContext::RenderContext(edge::gfx::IContext& gfx_context, edge::fs::VirtualFilesystem* filesystem, edge::Window* window) :
			context{ gfx_context },
			window{ *window }
		{
			// Select queue that can present
			queue = context.RequestQueue(edge::gfx::QueueTypeFlag::ePresent | edge::gfx::QueueTypeFlag::eGraphics);
			
			edge::gfx::SwapChainCreateInfo swapchain_ci{};
			swapchain_ci.window = window;
			swapchain_ci.image_count = 3u;

			swapchain = context.CreateSwapchain(swapchain_ci);

			auto image_count = swapchain->GetImageCount();

			for (int32_t i = 0; i < image_count; ++i)
			{
				render_frames[i] = std::make_unique<RenderFrame>(context, swapchain, queue, i);
			}

			// Create default image sampler
			edge::gfx::SamplerCreateInfo sampler_ci{};
			sampler_ci.address_u = sampler_ci.address_v = sampler_ci.address_w = edge::gfx::SamplerAddressMode::eClampToEdge;
			sampler_ci.min_filter = edge::gfx::Filter::eLinear;
			sampler_ci.mag_filter = edge::gfx::Filter::eLinear;
			sampler_ci.mip_map_mode = edge::gfx::Filter::eLinear;

			default_sampler = context.CreateSampler(sampler_ci);

			// Create timestamp query for collecting pass info
			edge::gfx::QueryPoolCreateInfo query_pool_ci{};
			query_pool_ci.type = edge::gfx::QueryType::eTimestamp;
			query_pool_ci.query_count = 2u;
			timestamp_query = context.CreateQueryPool(query_pool_ci);
			timestamp_query->Reset(0u);

			context.RequestTimestampFrequency(&timestamp_frequency);

			// TODO: now it creates just default one
			edge::gfx::SharedDescriptorCreateInfo shared_descriptor_ci{};
			shared_descriptor_ci.descriptions.emplace_back(edge::gfx::DescriptorType::eUniformBuffer, 0u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(edge::gfx::DescriptorType::eUniformBuffer, 1u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(edge::gfx::DescriptorType::eUniformBuffer, 2u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(edge::gfx::DescriptorType::eSampledImage, 3u, shlib::MAX_SAMPLED_TEXTURES);
			shared_descriptor_ci.descriptions.emplace_back(edge::gfx::DescriptorType::eSampler, 4u, shlib::MAX_SAMPLED_TEXTURES);

			shared_descriptor = context.CreateSharedDescriptor(shared_descriptor_ci);

			// Create uniform pool
			global_uniform_pool = std::make_unique<UniformPool>(context);

			current_frame = render_frames[active_frame_index].get();

			technique_db = std::make_unique<edge::gfx::TechniqueDatabase>(&gfx_context, filesystem);
			technique_db->Load("/root/tdbcache");

			// Register "culling" techniques
			technique_db->AddTechnique("clear_counters", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/culling/clear_counters.comp" } }
				});
			technique_db->AddTechnique("cull_and_batch", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/culling/cull_and_batch.comp" } }
				});
			technique_db->AddTechnique("scan_bit_array", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {
						.path = "/source/embed/shaders/culling/scan.comp",
						.definitions = { "INPUT_BIT_ARRAY" }
					} }
				});
			technique_db->AddTechnique("assemble_commands", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/culling/assemble_commands.comp" } }
				});

			// Register "deferred" technique
			technique_db->AddTechnique("visibilitybuffer_write", {
				.type = edge::gfx::TechniqueType::eGraphics,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/deferred/visbufferwrite.vert",
						.definitions = { "GEOMETRY_BUFFERS", "HAS_TEXCOORD", "HAS_NORMAL", "INDEX_16BIT", "HAS_TANGENT" }
					},{
						.path = "/source/embed/shaders/deferred/visbufferwrite.frag",
						.definitions = { "GEOMETRY_BUFFERS", "HAS_TEXCOORD", "HAS_NORMAL", "INDEX_16BIT", "HAS_TANGENT" }
					}},
				.color_blending_attachments = {
					edge::gfx::ColorBlendAttachment{},
					edge::gfx::ColorBlendAttachment{}
				},
				.rasterization_state = {
						.cull_mode = edge::gfx::CullModeFlag::eFront
					},
				.depth_stencil_state = {
					.depth_test = true,
					.depth_write = true,
					.depth_compare_op = edge::gfx::CompareOp::eLess
					},
				.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList,
				.color_formats = {
					edge::gfx::Format::eR8G8B8A8Unorm,
					edge::gfx::Format::eR8G8B8A8Unorm
					},
				.depth_format = edge::gfx::Format::eD32Sfloat
				});
			technique_db->AddTechnique("visibilitybuffer_shade", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/deferred/visbuffershade.comp",
						.definitions = { "LIGHT_DATA", "SHADOW_DATA", "GEOMETRY_BUFFERS", "HAS_TEXCOORD", "HAS_NORMAL", "INDEX_16BIT", "HAS_TANGENT" }
					}}
				});

			// Register "lighting" techniques
			technique_db->AddTechnique("generate_brdflut", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/lighting/brdflut.comp" } }
				});
			technique_db->AddTechnique("generate_irradiancecube", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/lighting/irradiancecube.comp" } }
				});
			technique_db->AddTechnique("generate_prefilteredmap", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/lighting/prefilteredmap.comp" } }
				});
			technique_db->AddTechnique("directional_shadow", {
				.type = edge::gfx::TechniqueType::eGraphics,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/lighting/unidirectional_shadow.vert",
						.definitions = { "SHADOW_DATA", "GEOMETRY_BUFFERS", "HAS_TEXCOORD", "HAS_NORMAL", "INDEX_16BIT", "HAS_TANGENT" }
					},{
						.path = "/source/embed/shaders/lighting/unidirectional_shadow.frag",
						.definitions = { "HAS_TEXCOORD", "HAS_NORMAL", "INDEX_16BIT", "HAS_TANGENT" }
					}},

				.rasterization_state = {
						.cull_mode = edge::gfx::CullModeFlag::eFront
					},
				.depth_stencil_state = {
					.depth_test = true,
					.depth_write = true,
					.depth_compare_op = edge::gfx::CompareOp::eLess
					},
				.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList,
				.depth_format = edge::gfx::Format::eD16Unorm
				});
			technique_db->AddTechnique("cascaded_shadow", {
				.type = edge::gfx::TechniqueType::eGraphics,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/lighting/unidirectional_shadow.vert",
						.definitions = { "SHADOW_DATA", "GEOMETRY_BUFFERS", "HAS_TEXCOORD", "HAS_NORMAL", "CASCADED_SHADOW", "INDEX_16BIT", "HAS_TANGENT" }
					},{
						.path = "/source/embed/shaders/lighting/unidirectional_shadow.frag",
						.definitions = { "HAS_TEXCOORD", "HAS_NORMAL", "CASCADED_SHADOW", "INDEX_16BIT", "HAS_TANGENT" }
					}},

				.rasterization_state = {
						.cull_mode = edge::gfx::CullModeFlag::eFront
					},
				.depth_stencil_state = {
					.depth_test = true,
					.depth_write = true,
					.depth_compare_op = edge::gfx::CompareOp::eLess
					},
				.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList,
				.depth_format = edge::gfx::Format::eD32Sfloat
				});
			technique_db->AddTechnique("omni_shadow", {
				.type = edge::gfx::TechniqueType::eGraphics,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/lighting/unidirectional_shadow.vert",
						.definitions = { "SHADOW_DATA", "GEOMETRY_BUFFERS", "HAS_TEXCOORD", "HAS_NORMAL", "OMNI_SHADOW", "INDEX_16BIT", "HAS_TANGENT" }
					},{
						.path = "/source/embed/shaders/lighting/unidirectional_shadow.frag",
						.definitions = { "HAS_TEXCOORD", "HAS_NORMAL", "OMNI_SHADOW", "INDEX_16BIT", "HAS_TANGENT" }
					}},

				.rasterization_state = {
						.cull_mode = edge::gfx::CullModeFlag::eFront
					},
				.depth_stencil_state = {
					.depth_test = true,
					.depth_write = true,
					.depth_compare_op = edge::gfx::CompareOp::eLess
					},
				.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList,
				.depth_format = edge::gfx::Format::eD16Unorm
				});

			// Register "postprocess" techniques
			technique_db->AddTechnique("generate_motion_vectors_low_precision", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/postprocess/generate_motion_vectors.comp",
						.definitions = { "LOW_PRECISION_VELOCITY" }
					}}
				});
			technique_db->AddTechnique("generate_motion_vectors", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/postprocess/generate_motion_vectors.comp" } }
				});
			technique_db->AddTechnique("apply_fxaa_effect", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/postprocess/apply_effect.comp",
						.definitions = { "FXAA" }
					}}
				});
			technique_db->AddTechnique("apply_motion_blur_effect_low", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/postprocess/apply_effect.comp",
						.definitions = { "MOTION_BLUR", "LOW_PRECISION_VELOCITY"}
					}}
				});
			technique_db->AddTechnique("apply_motion_blur_effect", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/postprocess/apply_effect.comp",
						.definitions = { "MOTION_BLUR" }
					}}
				});
			technique_db->AddTechnique("apply_dof_effect", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/postprocess/apply_effect.comp",
						.definitions = { "DOF" }
					}}
				});
			technique_db->AddTechnique("apply_bloom_effect", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/postprocess/apply_effect.comp",
						.definitions = { "BLOOM" }
					}}
				});
			technique_db->AddTechnique("apply_tonemap_effect", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/postprocess/apply_effect.comp",
						.definitions = { "TONEMAP" }
					}}
				});
			technique_db->AddTechnique("bokeh_filter", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/postprocess/bokeh.comp" } }
				});
			technique_db->AddTechnique("brightdetect", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/postprocess/brightdetect.comp" } }
				});
			technique_db->AddTechnique("downsample", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/postprocess/downsample.comp" } }
				});
			technique_db->AddTechnique("downsample_first_pass", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = {{
						.path = "/source/embed/shaders/postprocess/downsample.comp",
						.definitions = { "FIRST_PASS" }
					}}
				});
			technique_db->AddTechnique("upsample", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/postprocess/upsample.comp" } }
				});
			technique_db->AddTechnique("compute_histogram", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/postprocess/comp_histogram.comp" } }
				});
			technique_db->AddTechnique("compute_brightness", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/postprocess/comp_average_brightness.comp" } }
				});

			// Register "testing" techniques
			technique_db->AddTechnique("screenspace", {
				.type = edge::gfx::TechniqueType::eGraphics,
				.shared_descriptor = shared_descriptor,
				.stages = {
					{.path = "/source/embed/shaders/common/screenspace.vert" },
					{.path = "/source/embed/shaders/testing/screenspace.frag" } },

				.color_blending_attachments = {
					edge::gfx::ColorBlendAttachment{}
				},
				.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList,
				.color_formats = {current_frame->backbuffer->GetFormat()}
				});
			technique_db->AddTechnique("debug_draw", {
				.type = edge::gfx::TechniqueType::eGraphics,
				.shared_descriptor = shared_descriptor,
				.stages = {
					{.path = "/source/embed/shaders/testing/debug_draw.vert" },
					{.path = "/source/embed/shaders/testing/debug_draw.frag" } },
				.vertex_layout = {
					.attributes = {
						{
							.location = 0,
							.format = edge::gfx::Format::eR32G32B32Sfloat
						},
						{
							.location = 1,
							.format = edge::gfx::Format::eR32G32Sfloat,
							.offset = 12
						},
						{
							.location = 2,
							.format = edge::gfx::Format::eR8G8B8A8Unorm,
							.offset = 20
						}
					},
					.bindings = {
						{
							.stride = 24
						}
					}
				},
				.color_blending_attachments = {
					edge::gfx::ColorBlendAttachment{}
				},
				.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList,
				.color_formats = {current_frame->backbuffer->GetFormat()}
				});
			technique_db->AddTechnique("debug_text", {
				.type = edge::gfx::TechniqueType::eGraphics,
				.shared_descriptor = shared_descriptor,
				.stages = {
					{.path = "/source/embed/shaders/testing/debug_text.vert" },
					{.path = "/source/embed/shaders/testing/debug_text.frag" } },
				.vertex_layout = {
					.attributes = {
						{
							.location = 0,
							.format = edge::gfx::Format::eR32G32Sfloat
						}
					},
					.bindings = {
						{
							.stride = 8
						}
					}
				},
				.color_blending_attachments = {
					edge::gfx::ColorBlendAttachment{
					.enable = true,
					.src_color = edge::gfx::BlendFactor::eSrcAlpha,
					.dst_color = edge::gfx::BlendFactor::eOneMinusSrcAlpha,
					.src_alpha = edge::gfx::BlendFactor::eOneMinusSrcAlpha,
					.dst_alpha = edge::gfx::BlendFactor::eZero
					}
				},
				.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList,
				.color_formats = {current_frame->backbuffer->GetFormat()}
				});
			technique_db->AddTechnique("imgui", {
				.type = edge::gfx::TechniqueType::eGraphics,
				.shared_descriptor = shared_descriptor,
				.stages = {
					{.path = "/source/embed/shaders/testing/imgui.vert" },
					{.path = "/source/embed/shaders/testing/imgui.frag" } },
				.vertex_layout = {
					.attributes = {
						{
							.location = 0,
							.format = edge::gfx::Format::eR32G32Sfloat
						},
						{
							.location = 1,
							.format = edge::gfx::Format::eR32G32Sfloat,
							.offset = 8
						},
						{
							.location = 2,
							.format = edge::gfx::Format::eR8G8B8A8Unorm,
							.offset = 16
						}
					},
					.bindings = {
						{
							.stride = 20
						}
					}
				},
				.color_blending_attachments = {
					edge::gfx::ColorBlendAttachment{
					.enable = true,
					.src_color = edge::gfx::BlendFactor::eSrcAlpha,
					.dst_color = edge::gfx::BlendFactor::eOneMinusSrcAlpha,
					.src_alpha = edge::gfx::BlendFactor::eOneMinusSrcAlpha,
					.dst_alpha = edge::gfx::BlendFactor::eZero
					}
				},
				.primitive_topology = edge::gfx::PrimitiveTopology::eTriangleList,
				.color_formats = {current_frame->backbuffer->GetFormat()}
				});

			// Register "utility" techniques
			technique_db->AddTechnique("copy_image", {
				.type = edge::gfx::TechniqueType::eCompute,
				.shared_descriptor = shared_descriptor,
				.stages = { {.path = "/source/embed/shaders/utility/copy.comp" } }
				});

			technique_db->Save("/root/tdbcache");
		}

		RenderContext::~RenderContext()
		{
			technique_db->Save("/root/tdbcache");
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

		uint32_t RenderContext::AddResource(std::string_view path, edge::gfx::ISampler* sampler)
		{
			auto& new_resource = render_resources.emplace_back();
			new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			edge::gfx::ITexture* texture = edge::gfx::ResourceUploader::LoadTexture(path, new_resource.current_state);
			new_resource.resource_id = static_textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });

			return render_resources.size() - 1ull;
		}

		uint32_t RenderContext::AddResource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data)
		{
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
			
			edge::gfx::ITexture* texture = context.CreateTexture(texture_create_info);
			edge::gfx::ResourceUpdater::UpdateTexture(texture, new_resource.current_state, texture_data);
			new_resource.resource_id = static_textures.Add(TextureObject{ texture, default_sampler });
			
			//new_resource.current_state = edge::gfx::ResourceStateFlag::eShaderReadOnlyResource;

			return render_resources.size() - 1ull;
		}

		uint32_t RenderContext::AddResource(const TextureResourceCreateInfo& create_info, edge::gfx::ISampler* sampler)
		{
			auto& new_resource = render_resources.emplace_back();

			// TODO: What if it's storage target and render target at the same time?
			if (create_info.usage & edge::gfx::TextureUsageFlag::eStorage)
				new_resource.resource_type = RenderResourceTypeFlag::eStorageTarget;
			else if (create_info.usage & edge::gfx::TextureUsageFlag::eRenderTarget)
				new_resource.resource_type = RenderResourceTypeFlag::eRenderTarget;
			else
				new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			edge::gfx::TextureCreateInfo texture_create_info{};
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

			if ((create_info.usage & edge::gfx::TextureUsageFlag::eRenderTarget) || (create_info.usage & edge::gfx::TextureUsageFlag::eStorage))
				new_resource.resource_id = render_targets.Add(std::move(texture_object));
			else
				new_resource.resource_id = static_textures.Add(std::move(texture_object));

			return render_resources.size() - 1ull;
		}

		uint32_t RenderContext::AddResource(const edge::gfx::BufferCreateInfo& create_info)
		{
			edge::gfx::IBuffer* buffer = context.CreateBuffer(create_info);
			auto handle = buffers.Add(BufferObject{ buffer });
			if (create_info.usage != edge::gfx::BufferUsageFlag::eStagingBuffer && create_info.usage != edge::gfx::BufferUsageFlag::eUniformBuffer)
			{
				auto& new_resource = render_resources.emplace_back();
				if (create_info.usage & edge::gfx::BufferUsageFlag::eIndirect)
					new_resource.resource_type = RenderResourceTypeFlag::eIndirectBuffer;
				else if (create_info.usage & edge::gfx::BufferUsageFlag::eStorage)
					new_resource.resource_type = RenderResourceTypeFlag::eStorageBuffer;

				new_resource.resource_id = handle;

				return render_resources.size() - 1ull;
			}

			return ~0u;
		}

		void RenderContext::SetResourceName(uint32_t resource_index, std::string_view name)
		{
			auto& render_resource = render_resources[resource_index];

			edge::gfx::IGFXResource* resource{};
			if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				resource = current_frame->backbuffer;
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
			auto& technique = technique_db->GetTechnique(create_info.technique_name);

			RenderPassCreateInfo pass_ci{};
			pass_ci.name = technique.name;
			pass_ci.shader = technique.shader;
			pass_ci.root_signature = technique.root_signature;
			pass_ci.pipeline = technique.pipeline;
			pass_ci.shared_descriptor = shared_descriptor;
			pass_ci.draw_callback = create_info.draw_callback;
			pass_ci.enabled_callback = create_info.enabled_callback;

			pass_ci.resource_attachments = create_info.resource_attachments;

			auto new_pass = std::make_unique<RenderPass>(*this, pass_ci);
			return new_pass;
		}

		std::unique_ptr<ExecutionNode> RenderContext::CreateNode(const ComputeShaderPassCreateInfo& create_info)
		{
			auto& technique = technique_db->GetTechnique(create_info.technique_name);

			ComputePassCreateInfo pass_ci{};
			pass_ci.name = technique.name;
			pass_ci.shader = technique.shader;
			pass_ci.root_signature = technique.root_signature;
			pass_ci.pipeline = technique.pipeline;
			pass_ci.shared_descriptor = shared_descriptor;
			pass_ci.one_time = create_info.one_time;
			pass_ci.draw_callback = create_info.draw_callback;
			pass_ci.enabled_callback = create_info.enabled_callback;

			auto new_pass = std::make_unique<ComputePass>(*this, pass_ci);
			return new_pass;
		}

		void RenderContext::AddNode(std::unique_ptr<ExecutionNode>&& node)
		{
			graph_nodes.emplace_back(std::move(node));
		}

		void RenderContext::Execute(float delta_time)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto& command_list = *current_command_list;

			for (auto& node : graph_nodes)
				node->Execute(command_list, delta_time);

			auto& backbuffer_resource = GetResource(0);

			edge::gfx::TextureBarrier texture_barrier{};
			edge::gfx::ResourceBarrierInfo resource_barriers{};
			resource_barriers.texture_barriers = &texture_barrier;
			resource_barriers.texture_barrier_count = 1;

			texture_barrier.texture = current_frame->backbuffer;
			// TODO: get current resource state from resource itself
			texture_barrier.src_state = backbuffer_resource.GetState();
			texture_barrier.dst_state = edge::gfx::ResourceStateFlag::ePresentSrc;

			command_list.ResourceBarrier(resource_barriers);
		}

		void RenderContext::Begin()
		{
			EDGE_PROFILE_ZONE_SCOPED;

			global_uniform_pool->Reset();

			shared_descriptor->Reset();
			if (!global_resource_descriptor)
			{
				global_resource_descriptor = shared_descriptor->RequestDescriptors();
			}

			bool surface_updated{ false };
			if (!frame_active)
			{
				if (swapchain)
				{
					surface_updated = HandleSurfaceChange();
				}

				auto previous_frame = render_frames[active_frame_index].get();

				// We will use the acquired semaphore in a different frame context,
				// so we need to hold ownership.
				acquired_semaphore = previous_frame->image_avaliable;

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

				// Now the frame is active again
				frame_active = true;
				previous_frame->Reset();

				current_frame = render_frames[active_frame_index].get();
			}

			if (!acquired_semaphore)
			{
				throw std::runtime_error("Couldn't begin frame");
			}

			// Begin new command list
			current_command_list = current_frame->command_pool->Allocate();
			current_command_list->Begin();

			// Update render targets if needed
			if (surface_updated)
			{
				UpdateRenderTargets();
			}

			// Get previous frame time 
			uint64_t timestamps[2] = {};
			timestamp_query->GetData(0u, &timestamps[0]);

			uint64_t elapsed_time = timestamps[1] - timestamps[0];
			if (timestamps[1] <= timestamps[0])
			{
				elapsed_time = 0ull;
			}

			gpu_delta_time = static_cast<float>(elapsed_time) * timestamp_frequency / 1000000000.f;

			// Reset performance query
			current_command_list->ResetQuery(timestamp_query, 0u, 2u);
			current_command_list->BeginQuery(timestamp_query, 0u);

			std::vector<edge::gfx::TextureBarrier> texture_barriers{};

			// Update bindless textures
			uint32_t array_index{ 0u };
			for (auto& resource : render_resources)
			{
				if (resource.resource_type != RenderResourceTypeFlag::eStaticImage)
					continue;

				auto& texture_obj = static_textures.Get(resource.resource_id);
				if (resource.current_state != edge::gfx::ResourceStateFlag::eShaderReadOnlyResource)
				{
					auto& barrier = texture_barriers.emplace_back(texture_obj.texture, ~0u, resource.current_state, edge::gfx::ResourceStateFlag::eShaderReadOnlyResource);
					resource.current_state = edge::gfx::ResourceStateFlag::eShaderReadOnlyResource;
				}

				edge::gfx::TextureDescriptor texture_descriptor{};
				texture_descriptor.type = edge::gfx::DescriptorType::eSampledImage;
				texture_descriptor.set = 0;
				texture_descriptor.binding = 3;

				texture_descriptor.array_index = array_index;
				texture_descriptor.texture = texture_obj.texture;
				texture_descriptor.resource_state = edge::gfx::ResourceStateFlag::eShaderReadOnlyResource;

				global_resource_descriptor->BindTexture(texture_descriptor);

				edge::gfx::TextureDescriptor sampler_descriptor{};
				sampler_descriptor.type = edge::gfx::DescriptorType::eSampler;
				sampler_descriptor.set = 0;
				sampler_descriptor.binding = 4;

				sampler_descriptor.array_index = array_index;
				sampler_descriptor.sampler = texture_obj.sampler;

				global_resource_descriptor->BindTexture(sampler_descriptor);

				++array_index;
			}

			// Some textures can be just uploaded and we need to change them's state to shader readable
			if (!texture_barriers.empty())
			{
				edge::gfx::ResourceBarrierInfo resource_barrier{};
				resource_barrier.texture_barriers = texture_barriers.data();
				resource_barrier.texture_barrier_count = texture_barriers.size();

				current_command_list->ResourceBarrier(resource_barrier);
			}
		}

		void RenderContext::End()
		{
			EDGE_PROFILE_ZONE_SCOPED;

			global_resource_descriptor->Flush();

			std::vector<edge::gfx::SemaphoreSubmitInfo> wait_semaphores{};
			wait_semaphores.emplace_back(acquired_semaphore, 0ull, edge::gfx::StageFlag::eColorAttachmentOutput);

			edge::gfx::SemaphoreSubmitInfo update_wait_semaphore{};
			edge::gfx::ResourceUpdater::Flush(update_wait_semaphore, {});

			if (update_wait_semaphore.semaphore)
				wait_semaphores.emplace_back(update_wait_semaphore);

			edge::gfx::SemaphoreSubmitInfo upload_wait_semaphore{};
			edge::gfx::ResourceUploader::Flush(upload_wait_semaphore, {});

			if (upload_wait_semaphore.semaphore)
				wait_semaphores.emplace_back(upload_wait_semaphore);

			// Finish performance query
			current_command_list->EndQuery(timestamp_query, 1u);

			// Submit commands
			current_command_list->End();
			current_frame->fence->Reset();

			try
			{
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

				queue->Submit(submit_info, current_frame->fence);
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
				present_info.image_indices = &GetCurrentFrameIndex();

				queue->Present(present_info);
				current_frame->fence->Wait();

				//current_frame_index = (current_frame_index + 1) % swapchain->GetImageCount();
			}

			previous_frame_semaphore = current_frame->upload_avaliable;
			if (acquired_semaphore)
			{
				acquired_semaphore = nullptr;
			}

			// Update render resources state
			for (auto& render_resource : render_resources)
			{
				if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage || render_resource.finalized)
					continue;

				render_resource.current_state = edge::gfx::ResourceStateFlag::eUndefined;
			}

			global_resource_descriptor = nullptr;
			frame_active = false;
		}

		edge::gfx::IContext& RenderContext::GetContext()
		{
			return context;
		}

		edge::gfx::IContext const& RenderContext::GetContext() const
		{
			return context;
		}

		edge::gfx::ITexture* RenderContext::GetTexture(uint32_t resource_index)
		{
			auto& render_resource = render_resources[resource_index];
			return GetTexture(render_resource);
		}

		edge::gfx::ITexture* RenderContext::GetTexture(const RenderResource& render_resource)
		{
			if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				return current_frame->backbuffer;
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

		edge::gfx::ISampler* RenderContext::GetSampler(uint32_t resource_index)
		{
			auto& render_resource = render_resources[resource_index];
			return GetSampler(render_resource);
		}

		edge::gfx::ISampler* RenderContext::GetSampler(const RenderResource& render_resource)
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

		edge::gfx::IBuffer* RenderContext::GetBuffer(uint32_t resource_index)
		{
			auto& render_resource = render_resources[resource_index];
			return GetBuffer(render_resource);
		}

		edge::gfx::IBuffer* RenderContext::GetBuffer(const RenderResource& render_resource)
		{
			if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer)
			{
				auto& buffer_object = buffers.Get(render_resource.resource_id);
				return buffer_object.buffer;
			}

			return nullptr;
		}

		//edge::gfx::SemaphoreSubmitInfo RenderContext::GetUploadAvaliableSemaphore()
		//{
		//	return edge::gfx::SemaphoreSubmitInfo{ previous_frame_semaphore, 0ull, edge::gfx::StageFlag::eAllCommands };
		//}

		edge::gfx::BufferSuballocation RenderContext::RequestUniformBlock(uint64_t block_size)
		{
			return current_frame->uniform_pool->RequestBlock(block_size);
		}

		edge::gfx::ICommandList& RenderContext::GetCurrentCommandList()
		{
			return *current_command_list;
		}

		edge::gfx::IDescriptorAllocator& RenderContext::GetCurrentDescriptorAllocator()
		{
			return *current_frame->descriptor_allocator;
		}

		void RenderContext::UpdateGlobalBinding(uint32_t binding, std::span<const uint8_t> data)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			if (!global_resource_descriptor)
			{
				global_resource_descriptor = shared_descriptor->RequestDescriptors();
			}

			auto buffer_block = global_uniform_pool->RequestBlock(data.size());
			buffer_block.Update(data);

			edge::gfx::BufferDescriptor buffer_descriptor{};
			buffer_descriptor.type = edge::gfx::DescriptorType::eUniformBuffer;
			buffer_descriptor.binding = binding;
			buffer_descriptor.buffer = &buffer_block.GetBuffer();
			buffer_descriptor.offset = buffer_block.GetOffset();
			buffer_descriptor.size = buffer_block.GetSize();

			global_resource_descriptor->BindBuffer(buffer_descriptor);
		}

		const uint32_t& RenderContext::GetCurrentFrameIndex() const
		{
			return active_frame_index;
		}

		void RenderContext::UpdateSwapchain(uint32_t width, uint32_t height)
		{
			edge::gfx::SwapChainCreateInfo new_swapchain_ci{};
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
			edge::gfx::SwapChainCreateInfo new_swapchain_ci{};
			new_swapchain_ci.window = &window;
			new_swapchain_ci.old_swapchain = swapchain;
			new_swapchain_ci.image_count = image_count;

			swapchain = context.CreateSwapchain(new_swapchain_ci);
			context.Destroy(new_swapchain_ci.old_swapchain);
			UpdateResources();
		}

		bool RenderContext::HandleSurfaceChange(bool force_update)
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
			if (surface_width != current_width ||
				surface_height != current_height ||
				force_update)
			{
				EDGE_PROFILE_ZONE_SCOPED_NAMED("ISwapchain update");
				// Recreate swapchain
				queue->WaitIdle();
				UpdateSwapchain(surface_width, surface_height);

				active_frame_index = 0u;

				return true;
			}

			return false;
		}

		void RenderContext::UpdateRenderTargets()
		{
			EDGE_PROFILE_ZONE_SCOPED;

			uint32_t width, height;
			swapchain->GetSurfaceExtent(width, height);

			// Update/resize render targets here if it's needed
			for (auto& target : render_targets.Data())
			{
				// TODO: resize only if texture is able to resize 
				if (!target.resizable)
					continue;

				edge::gfx::TextureCreateInfo create_info{};
				create_info.usage = target.usage;
				create_info.layers = (target.usage & edge::gfx::TextureUsageFlag::eCubemap) ? target.texture->GetArrayLayerCount() / 6u : target.texture->GetArrayLayerCount();
				// TODO: need to recalculate number of mips?
				create_info.levels = target.texture->GetMipLevelCount();
				create_info.format = target.texture->GetFormat();

				if (target.scale_parent_resource != ~0u)
				{
					edge::gfx::ITexture* parent = GetTexture(target.scale_parent_resource);

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
	
		void RenderContext::UpdateResources()
		{
			EDGE_PROFILE_ZONE_SCOPED;

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