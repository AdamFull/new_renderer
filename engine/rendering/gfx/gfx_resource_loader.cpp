#include "gfx_resource_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <ktx.h>

#include <util/FilesystemHelper.h>
#include <logger.hpp>

#include <tracy/Tracy.hpp>

#include "native_format/vk_format.h"

#include "gfx_context.h"
#include "shader_compiler/gfx_shader_effect_compiler.h"

namespace engine
{
	namespace gfx
	{
		class ResourceUploaderImpl
		{
		public:
			ResourceUploaderImpl(Context& context, const Queue& queue) :
				context{ context }, queue{ queue }
			{
				command_pool = context.CreateCommandPool(&queue);

				signal_semaphore = context.CreateSemaphore();
				signal_semaphore->SetResourceName("resource_upload_signal_semaphore");
				fence = context.CreateFence();
				fence->SetResourceName("resource_upload_fence");
			}

			~ResourceUploaderImpl()
			{
				// Clear all allocated buffers if it's exists
				for (auto& buffer : staging_buffers)
				{
					context.Destroy(buffer);
				}
				staging_buffers.clear();

				context.Destroy(command_pool);
				context.Destroy(signal_semaphore);
				context.Destroy(fence);
			}

			ResourceUploaderImpl(const ResourceUploaderImpl&) = delete;
			ResourceUploaderImpl(ResourceUploaderImpl&&) = delete;
			ResourceUploaderImpl& operator=(const ResourceUploaderImpl&) = delete;
			ResourceUploaderImpl& operator=(ResourceUploaderImpl&&) = delete;

			Buffer& RequestBuffer(size_t size, size_t* buffer_index = nullptr)
			{
				if (buffer_index)
				{
					*buffer_index = staging_buffers.size();
				}

				BufferCreateInfo buffer_create_info{};
				buffer_create_info.usage = BufferUsageFlag::eStagingBuffer;
				buffer_create_info.size = size;

				auto& new_buffer = staging_buffers.emplace_back(context.CreateBuffer(buffer_create_info));
				return *new_buffer;
			}

			Buffer& GetBuffer(size_t buffer_index)
			{
				return *staging_buffers[buffer_index];
			}

			CommandList& RequestCommandList()
			{
				if (!active_command_list)
				{
					active_command_list = command_pool->Allocate();
				}

				return *active_command_list;
			}

			SemaphoreSubmitInfo Submit(SemaphoreSubmitInfo* wait_infos, uint32_t semaphore_count)
			{
				constexpr size_t k_max_wait_semaphores{ 8ull };

				SubmitInfo submit_info{};

				SemaphoreSubmitInfo wait_semaphores[k_max_wait_semaphores]{};
				std::copy(wait_infos, wait_infos + semaphore_count, &wait_semaphores[0]);

				submit_info.wait_semaphore_infos = &wait_semaphores[0];
				submit_info.wait_semaphore_count = semaphore_count;

				// Not shure in witch state i have to signal it
				SemaphoreSubmitInfo signal_semaphore_info{ signal_semaphore, 0ull, StageFlag::eTransfer };

				submit_info.signal_semaphore_infos = &signal_semaphore_info;
				submit_info.signal_semaphore_count = 1u;

				submit_info.command_lists = &active_command_list;
				submit_info.command_buffer_count = 1u;

				queue.Submit(submit_info, fence);

				// Not sure that it's the good one
				SemaphoreSubmitInfo wait_semaphore_info{ signal_semaphore, 0ull, StageFlag::eAllTransfer | StageFlag::eAllGraphics | StageFlag::eComputeShader | StageFlag::eDrawIndirect };
				return wait_semaphore_info;
			}

			void Reset()
			{
				active_command_list = nullptr;

				fence->Wait();
				fence->Reset();

				for (auto& buffer : staging_buffers)
				{
					context.Destroy(buffer);
				}
				staging_buffers.clear();

				command_pool->Reset();
			}
		private:
			Context& context;

			const Queue& queue;

			CommandPool* command_pool{ nullptr };
			CommandList* active_command_list{ nullptr };

			std::vector<Buffer*> staging_buffers;

			Semaphore* signal_semaphore{ nullptr };
			Fence* fence{ nullptr };
		};

		// Helper class for generate mip maps
		class MipGeneratorImpl
		{
		public:
			MipGeneratorImpl(ResourceLoader& resource_loader) :
				context{ resource_loader.GetContext() }
			{
				const char* mip_generate_shaders[] = {
				"utility:generate_mips_4x",
				"utility:generate_mips_4x:MIP_2",
				"utility:generate_mips_4x:MIP_2:MIP_3",
				"utility:generate_mips_4x:MIP_2:MIP_3:MIP_4"
				};

				// Load all shaders
				for (int32_t i = 0; i < shader_instances.size(); ++i)
				{
					auto& shader_instance = shader_instances[i];

					shader_instance.shader = resource_loader.LoadShader(mip_generate_shaders[i]);

					RootSignatureCreateInfo root_sign_ci{};
					root_sign_ci.shader = shader_instance.shader;

					shader_instance.root_signature = context.CreateRootSignature(root_sign_ci);

					ComputePipelineCreateInfo pipeline_ci{};
					pipeline_ci.shader = shader_instance.shader;
					pipeline_ci.root_signature = shader_instance.root_signature;

					shader_instance.pipeline = context.CreatePipeline(pipeline_ci);
				}

				// Create descriptor allocator for allocate descriptors in mip passes
				descriptor_allocator = context.CreateDescriptorAllocator();

				// Also we need linear sampler 

				SamplerCreateInfo sampler_ci{};
				sampler_ci.address_u = sampler_ci.address_v = sampler_ci.address_w = SamplerAddressMode::eClampToEdge;
				sampler_ci.min_filter = Filter::eLinear;
				sampler_ci.mag_filter = Filter::eLinear;
				sampler_ci.mip_map_mode = Filter::eLinear;

				linear_sampler = context.CreateSampler(sampler_ci);
			}

			~MipGeneratorImpl()
			{
				// Delete mip generation resources
				for (int32_t i = 0; i < shader_instances.size(); ++i)
				{
					context.Destroy(shader_instances[i].pipeline);
					context.Destroy(shader_instances[i].root_signature);
					context.Destroy(shader_instances[i].shader);
				}

				context.Destroy(descriptor_allocator);
			}

			ResourceStateFlags WriteCommands(CommandList& command_list, Texture& texture)
			{
				// Texture should be in copy dest state
				ResourceStateFlags resource_state{ ResourceStateFlag::eCopyDst };

				TextureBarrier texture_barrier{};
				texture_barrier.texture = &texture;

				ResourceBarrierInfo resource_barrier{};
				resource_barrier.texture_barriers = &texture_barrier;
				resource_barrier.texture_barrier_count = 1u;

				uint32_t total_mip_count = texture.GetMipLevelCount();
				for (int32_t mip = 1; mip < total_mip_count;)
				{
					uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

					group_x = texture.GetWidth() >> mip;
					group_y = texture.GetHeight() >> mip;

					uint32_t mip_count{ 4u };
					if (mip + mip_count > total_mip_count)
						mip_count = total_mip_count - mip;

					auto& generator = shader_instances[mip_count - 1];

					ResourceDescriptor* descriptor = descriptor_allocator->AllocateDescriptors(generator.root_signature);

					command_list.BindPipeline(generator.pipeline);

					// At first make texture readable
					texture_barrier.src_state = resource_state;
					texture_barrier.dst_state = ResourceStateFlag::eRead | ResourceStateFlag::eCompute | ResourceStateFlag::eSample | ResourceStateFlag::eGeneral;
					command_list.ResourceBarrier(resource_barrier);
					resource_state = texture_barrier.dst_state;

					TextureDescriptor bind_resource{};
					bind_resource.type = DescriptorType::eCombinedImageSampler;
					bind_resource.set = 0;
					bind_resource.binding = 5;

					bind_resource.texture = &texture;
					bind_resource.sampler = linear_sampler;
					bind_resource.subresource_index = mip - 1;
					bind_resource.resource_state = resource_state;

					// Bind texture slot
					descriptor->BindTexture(bind_resource);

					// Make texture writable now
					texture_barrier.src_state = resource_state;
					texture_barrier.dst_state = ResourceStateFlag::eWrite | ResourceStateFlag::eCompute | ResourceStateFlag::eStorage | ResourceStateFlag::eGeneral;
					command_list.ResourceBarrier(resource_barrier);
					resource_state = texture_barrier.dst_state;

					bind_resource.type = DescriptorType::eStorageImage;
					bind_resource.resource_state = resource_state;

					for (int32_t i = 0; i < mip_count; ++i)
					{
						bind_resource.binding = i + 1;
						bind_resource.subresource_index = mip + i;
						descriptor->BindTexture(bind_resource);
					}

					// Finally bind descriptor
					BindDescriptorsInfo bind_info{};
					bind_info.pipeline_type = PipelineType::eCompute;
					bind_info.resource_descriptors = &descriptor;
					bind_info.resource_descriptors_count = 1u;
					bind_info.root_signature = generator.root_signature;

					descriptor->Flush();
					command_list.BindDescriptors(bind_info);

					// Divide by local group sizes
					auto& reflection = generator.shader->GetReflection();
					group_x = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_x) / static_cast<float>(reflection.local_sizes[0]))), 1u);
					group_y = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_y) / static_cast<float>(reflection.local_sizes[1]))), 1u);
					group_z = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_z) / static_cast<float>(reflection.local_sizes[2]))), 1u);

					command_list.Dispatch(group_x, group_y, group_z);

					mip += mip_count;
				}

				return resource_state;
			}

			void Reset()
			{
				descriptor_allocator->Reset();
			}
		private:
			Context& context;

			struct ShaderInstance
			{
				Shader* shader{ nullptr };
				RootSignature* root_signature{ nullptr };
				Pipeline* pipeline{ nullptr };
			};

			Sampler* linear_sampler{ nullptr };

			DescriptorAllocator* descriptor_allocator{ nullptr };

			std::array<ShaderInstance, 4ull> shader_instances;
		};



		class ResourceLoaderImpl : public ResourceLoader
		{
		public:
			ResourceLoaderImpl(const ResourceLoaderCreateInfo& create_info) :
				context{ *create_info.context },
				fs{ *create_info.filesystem_wrapper }
			{
				queue = context.RequestQueue(QueueTypeFlag::eGraphics, 1u);

				//assert(k_max_upload_frames > upload_frame_count);

				for (int32_t i = 0; i < uploader_count; ++i)
				{
					uploaders[i] = std::make_unique<ResourceUploaderImpl>(context, *queue);
				}

				// Create shader compiler
				shc::ShaderEffectCompilerCreateInfo shader_compiler_ci{};
				shader_compiler_ci.filesystem_wrapper = create_info.filesystem_wrapper;
				shader_compiler_ci.shader_compile_config_path = create_info.shader_compile_config_path;
				shader_compiler_ci.shader_cache_patch = create_info.shader_cache_patch;

				effect_compiler = std::make_unique<shc::ShaderEffectCompiler>(shader_compiler_ci);

				// Create mip generator
				mip_generator = std::make_unique<MipGeneratorImpl>(*this);
			}

			~ResourceLoaderImpl()
			{
				for (auto& [hash, sampler] : sampler_cache)
				{
					context.Destroy(sampler);
				}
			}

			Texture* LoadTexture(std::string_view texture_path) override
			{
				ZoneScoped;

				BeginCommands();
				using namespace utl;

				auto ext = fsh::get_ext(texture_path);
				auto ext_hash = utl::murmur_hash(ext.data(), ext.size(), 0);
				bool generate_mips{ false };

				std::vector<uint8_t> data{};
				fs.ReadFile(texture_path.data(), data);

				gfx::TextureCreateInfo create_info{};
				gfx::CopyTextureInfo copy_texture{};

				create_info.usage |= gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eTransferable;

				auto& uploader = GetActiveUploader();

				switch (ext_hash)
				{
				case fsh::format::jpg:
				case fsh::format::jpeg:
				case fsh::format::png:
				case fsh::format::tga:
				case fsh::format::bmp:
				case fsh::format::psd:
				case fsh::format::gif:
				case fsh::format::hdr:
				case fsh::format::pic:
				case fsh::format::pnm:
				{
					int32_t width{ 0 }, height{ 0 }, channels{ 0 };
					uint8_t* rawdata = stbi_load_from_memory(data.data(), data.size(), &width, &height, &channels, STBI_rgb_alpha);
					uint64_t data_size = width * height * 4ull;

					create_info.width = width;
					create_info.height = height;
					create_info.format = gfx::Format::eR8G8B8A8Unorm;

					// TODO: Add mip level generation
					//create_info.levels = 1; utl::cast<u32>(std::floor(std::log2((std::max)(width, height)))) + 1u;
					create_info.levels = utl::cast<u32>(std::floor(std::log2((std::max)(width, height)))) + 1u;
					generate_mips = true;

					// For mip generation
					create_info.usage |= gfx::TextureUsageFlag::eStorage;

					// Create staging buffer for uploading
					auto& src_buffer = uploader.RequestBuffer(data_size);

					// Copy image data to staging buffer
					src_buffer.Update(rawdata, data_size);
					stbi_image_free(rawdata);

					copy_texture.src_buffer = &src_buffer;

					auto& region = copy_texture.copy_regions[copy_texture.copy_region_count++];
					region.size_x = width;
					region.size_y = height;
				} break;
				case fsh::format::ktx:
				{
					ktxTexture* kTexture{ nullptr };
					auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
					assert(ktxresult == KTX_SUCCESS && "FAILED TO OPEN KTX IMAGE");
					if (ktxresult != KTX_SUCCESS)
						return nullptr;

					if (kTexture->isCubemap)
						create_info.usage |= gfx::TextureUsageFlag::eCubemap;

					create_info.width = kTexture->baseWidth;
					create_info.height = kTexture->baseHeight;
					create_info.depth = kTexture->baseDepth;

					create_info.levels = kTexture->numLevels;
					create_info.layers = kTexture->numLayers;

					if (kTexture->generateMipmaps)
					{
						generate_mips = true;
						create_info.levels = utl::cast<u32>(std::floor(std::log2((std::max)(create_info.width, create_info.height)))) + 1u;
					}

					// Get texture format
					auto* ktx1 = (ktxTexture1*)kTexture;

					uint32_t vk_format{};
					vk_format = vkGetFormatFromOpenGLInternalFormat(ktx1->glInternalformat);
					if (vk_format == 0u)
						vk_format = vkGetFormatFromOpenGLFormat(ktx1->glFormat, ktx1->glType);

					create_info.format = gfx::FormatFromVk(vk_format);

					// Create staging buffer for uploading
					auto& src_buffer = uploader.RequestBuffer(kTexture->dataSize);
					src_buffer.Update(kTexture->pData, kTexture->dataSize);
					copy_texture.src_buffer = &src_buffer;

					// Collect image copy regions
					auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
					for (int32_t layer = 0; layer < num_layers; layer++)
					{
						for (int32_t level = 0u; level < kTexture->numLevels; level++)
						{
							auto& region = copy_texture.copy_regions[copy_texture.copy_region_count++];
							region.layer = layer;
							region.level = level;
							region.layer_count = kTexture->numLayers;

							region.size_x = create_info.width >> level;
							region.size_z = create_info.height >> level;

							ktxresult = ktxTexture_GetImageOffset(kTexture, level, 0, layer, &region.buffer_offset);
						}
					}

					ktxTexture_Destroy(kTexture);
				} break;
				case fsh::format::ktx2:
				{
					ktxTexture2* kTexture{ nullptr };
					auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)&kTexture);
					assert(ktxresult == KTX_SUCCESS && "FAILED TO OPEN KTX IMAGE");
					if (ktxresult != KTX_SUCCESS)
						return nullptr;

					if (kTexture->isCubemap)
						create_info.usage |= gfx::TextureUsageFlag::eCubemap;

					create_info.width = kTexture->baseWidth;
					create_info.height = kTexture->baseHeight;
					create_info.depth = kTexture->baseDepth;

					create_info.levels = kTexture->numLevels;
					create_info.layers = kTexture->numLayers;

					if (kTexture->generateMipmaps)
					{
						generate_mips = true;
						create_info.levels = utl::cast<u32>(std::floor(std::log2((std::max)(create_info.width, create_info.height)))) + 1u;
					}

					if (ktxTexture2_NeedsTranscoding(kTexture))
						ktxresult = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);

					create_info.format = gfx::FormatFromVk(kTexture->vkFormat);

					// Create staging buffer for uploading
					auto& src_buffer = uploader.RequestBuffer(kTexture->dataSize);
					src_buffer.Update(kTexture->pData, kTexture->dataSize);
					copy_texture.src_buffer = &src_buffer;

					// Collect image copy regions
					auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
					for (int32_t layer = 0; layer < num_layers; layer++)
					{
						for (int32_t level = 0; level < kTexture->numLevels; level++)
						{
							auto& region = copy_texture.copy_regions[copy_texture.copy_region_count++];
							region.layer = layer;
							region.level = level;
							region.layer_count = kTexture->numLayers;

							region.size_x = create_info.width >> level;
							region.size_y = create_info.height >> level;

							ktxresult = ktxTexture_GetImageOffset((ktxTexture*)kTexture, level, 0, layer, &region.buffer_offset);
						}
					}

					ktxTexture_Destroy((ktxTexture*)kTexture);
				} break;
				}

				Texture* texture = context.CreateTexture(create_info);
				copy_texture.dst_texture = texture;

				auto& command_list = uploader.RequestCommandList();

				TextureBarrier texture_barrier{};
				texture_barrier.texture = texture;
				texture_barrier.src_state = ResourceStateFlag::eUndefined;
				texture_barrier.dst_state = ResourceStateFlag::eCopyDst;

				ResourceBarrierInfo resource_barrier{};
				resource_barrier.texture_barriers = &texture_barrier;
				resource_barrier.texture_barrier_count = 1u;

				// Transfer to intermediate layout
				command_list.ResourceBarrier(resource_barrier);

				// Copy texture regions
				command_list.CopyTexture(copy_texture);

				if (generate_mips)
					texture_barrier.src_state = mip_generator->WriteCommands(command_list, *texture);
				else
					texture_barrier.src_state = ResourceStateFlag::eCopyDst;

				// Transfer to final layout
				texture_barrier.dst_state = ResourceStateFlag::eShaderReadOnlyResource;
				command_list.ResourceBarrier(resource_barrier);

				return texture;
			}

			Texture* LoadTexture(const TextureCreateInfo& create_info, std::span<uint8_t> texture_data) override
			{
				ZoneScoped;

				Texture* texture = context.CreateTexture(create_info);

				CopyTextureInfo copy_texture{};
				copy_texture.dst_texture = texture;

				auto& uploader = GetActiveUploader();
				auto& src_buffer = uploader.RequestBuffer(texture_data.size());
				src_buffer.Update(texture_data.data(), texture_data.size());
				copy_texture.src_buffer = &src_buffer;

				auto& copy_region = copy_texture.copy_regions[copy_texture.copy_region_count++];
				copy_region.size_x = create_info.width;
				copy_region.size_y = create_info.height;
				copy_region.size_z = create_info.depth;

				auto& command_list = uploader.RequestCommandList();

				TextureBarrier texture_barrier{};
				texture_barrier.texture = texture;
				texture_barrier.src_state = ResourceStateFlag::eUndefined;
				texture_barrier.dst_state = ResourceStateFlag::eCopyDst;

				ResourceBarrierInfo resource_barrier{};
				resource_barrier.texture_barriers = &texture_barrier;
				resource_barrier.texture_barrier_count = 1u;

				// Transfer to intermediate layout
				command_list.ResourceBarrier(resource_barrier);

				// Copy texture regions
				command_list.CopyTexture(copy_texture);

				// Transfer to final layout
				texture_barrier.src_state = ResourceStateFlag::eCopyDst;
				texture_barrier.dst_state = ResourceStateFlag::eShaderReadOnlyResource;
				command_list.ResourceBarrier(resource_barrier);

				return texture;
			}

			Sampler* GetSampler(const SamplerCreateInfo& create_info)
			{
				uint32_t sampler_hash = Hash(create_info);
				if (auto found = sampler_cache.find(sampler_hash); found != sampler_cache.end())
					return found->second;

				return sampler_cache[sampler_hash] = context.CreateSampler(create_info);
			}

			int32_t GetShaderIndex(ShaderStageFlag stage)
			{
				if (stage == ShaderStageFlag::eVertex)
					return 0;
				else if (stage == ShaderStageFlag::eTessellationControl)
					return 1;
				else if (stage == ShaderStageFlag::eTessellationEvaluation)
					return 2;
				else if (stage == ShaderStageFlag::eGeometry)
					return 3;
				else if (stage == ShaderStageFlag::eFragment)
					return 4;
				else if (stage == ShaderStageFlag::eCompute)
					return 5;

				return -1;
			}

			Shader* LoadShader(std::string_view shader_name) override
			{
				auto shader_key = shc::GetUniqueHashName(shader_name);
				auto found_shader = shader_cache.find(shader_key);
				if (found_shader == shader_cache.end())
				{
					// Try to create it
					auto effect_create_info = effect_compiler->GetEffect(shader_name);
					if (effect_create_info.shader_stages.empty())
						return nullptr;

					ShaderCreateInfo shader_create_info{};

					for (const auto& shader_stage : effect_create_info.shader_stages)
					{
						shader_create_info.stages |= shader_stage.stage;

						auto stage_index = GetShaderIndex(shader_stage.stage);
						auto& stage_ci = shader_create_info.shader_stages[stage_index];
						stage_ci.stage = shader_stage.stage;
						stage_ci.code = shader_stage.code;
					}

					auto* new_shader = context.CreateShader(shader_create_info);
					shader_cache[shader_key] = new_shader;
					return new_shader;
				}
				
				return found_shader->second;
			}

			void BeginBufferUpdate(BufferUpdateInfo& update_info) override
			{
				ZoneScoped;

				BeginCommands();

				Buffer* dst_buffer = update_info.dst_buffer;

				if (!(dst_buffer->GetUsageFlags() & BufferUsageFlag::eGpuOnly))
				{
					log_warning("This buffer can be updated from the CPU, no additional memory needs to be allocated.");
					update_info.map_info.data = dst_buffer->Map();
					update_info.map_info.avaliable_size = dst_buffer->GetSize();
				}
				else
				{
					uint32_t allocation_size = update_info.update_size > 0ull ? update_info.update_size : dst_buffer->GetSize();

					auto& uploader = GetActiveUploader();
					auto& src_buffer = uploader.RequestBuffer(allocation_size, &update_info.map_info.buffer_index);

					update_info.map_info.data = src_buffer.Map();
					update_info.map_info.avaliable_size = src_buffer.GetSize();
				}
			}

			void EndBufferUpdate(BufferUpdateInfo& update_info) override
			{
				ZoneScoped;

				Buffer* dst_buffer = update_info.dst_buffer;

				if (update_info.map_info.avaliable_size == ~0ull)
				{
					dst_buffer->Unmap();
				}
				else
				{
					auto& uploader = GetActiveUploader();
					auto& command_list = uploader.RequestCommandList();
					auto& src_buffer = uploader.GetBuffer(update_info.map_info.buffer_index);

					// TODO: Add memory barrier befor copy for dst buffer
					BufferBarrier buffer_barrier{};
					buffer_barrier.buffer = dst_buffer;
					buffer_barrier.src_state = ResourceStateFlag::eCopyDst | ResourceStateFlag::eCopySrc;
					buffer_barrier.dst_state = ResourceStateFlag::eCopyDst;

					ResourceBarrierInfo barrier_info{};
					barrier_info.buffer_barriers = &buffer_barrier;
					barrier_info.buffer_barrier_count = 1u;

					command_list.ResourceBarrier(barrier_info);

					CopyBufferInfo copy_buffer{};
					copy_buffer.src_buffer = &src_buffer;
					copy_buffer.dst_buffer = dst_buffer;

					copy_buffer.regions = update_info.regions;
					copy_buffer.region_count = update_info.region_count;

					command_list.CopyBuffer(copy_buffer);

					update_info.map_info.buffer_index = ~0ull;
				}

				update_info.map_info.data = nullptr;
				update_info.map_info.avaliable_size = 0ull;
			}

			SemaphoreSubmitInfo Submit(SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count) override
			{
				ZoneScoped;

				if (is_writing_commands)
				{
					auto& uploader = GetActiveUploader();
					auto& command_list = uploader.RequestCommandList();

					command_list.EndMarker();
					command_list.End();
					is_writing_commands = false;

					auto signal_semaphore_info = uploader.Submit(wait_infos, wait_semaphore_count);

					active_uploader = (active_uploader + 1u) % uploader_count;

					return signal_semaphore_info;
				}

				return {};
			}

			Context& GetContext() override
			{
				return context;
			}

			Context const& GetContext() const override
			{
				return context;
			}
		private:
			ResourceUploaderImpl& GetActiveUploader()
			{
				return *uploaders[active_uploader];
			}

			void BeginCommands()
			{
				if (!is_writing_commands)
				{
					auto& uploader = GetActiveUploader();
					uploader.Reset();

					auto& command_list = uploader.RequestCommandList();

					command_list.Begin();
					command_list.BeginMarker("Upload resources", 0xFFFFFFFF);

					mip_generator->Reset();

					is_writing_commands = true;
				}
			}
		private:
			Context& context;
			FilesystemWrapper& fs;

			const gfx::Queue* queue{ nullptr };

			std::array<std::unique_ptr<ResourceUploaderImpl>, 8ull> uploaders{};
			uint32_t uploader_count{ 1u };
			uint32_t active_uploader{ 0u };

			bool is_writing_commands{ false };

			std::unique_ptr<shc::ShaderEffectCompiler> effect_compiler;

			std::unordered_map<uint32_t, Sampler*> sampler_cache{};
			std::unordered_map<uint32_t, Shader*> shader_cache{};

			std::unique_ptr<MipGeneratorImpl> mip_generator;
		};

		std::unique_ptr<ResourceLoader> CreateResourceLoader(const ResourceLoaderCreateInfo& create_info)
		{
			return std::make_unique<ResourceLoaderImpl>(create_info);
		}
	}
}