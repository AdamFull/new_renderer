#include <engine/rendering/UploadContext.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <ktx.h>

#include <engine/rendering/native_format/vk_format.h>

#include <engine/filesystem/vfs_helper.h>
#include <util/Logger.h>

#include <shader_compiler/compiler_util.h>

#include <gfx/Context.h>

namespace engine
{
	namespace render
	{
		struct FShaderNameInfo
		{
			std::string library_name;
			std::string shader_name;
			utl::vector<std::string> definitions;
		};

		FShaderNameInfo GetShaderNameInfo(const std::string& shader_name)
		{
			FShaderNameInfo name_info{};

			std::string token;
			std::stringstream ss(shader_name);

			if (std::getline(ss, token, ':'))
				name_info.library_name = token;

			if (std::getline(ss, token, ':'))
				name_info.shader_name = token;

			while (std::getline(ss, token, ':'))
				name_info.definitions.emplace_back(token);

			return name_info;
		}

		gfx::ShaderStageFlag GetShaderStageFlag(shader_compiler::EShaderType type)
		{
			if (type == shader_compiler::EShaderType::eVertex)
				return gfx::ShaderStageFlag::eVertex;
			else if (type == shader_compiler::EShaderType::eControl)
				return gfx::ShaderStageFlag::eTessellationControl;
			else if (type == shader_compiler::EShaderType::eEvaluation)
				return gfx::ShaderStageFlag::eTessellationEvaluation;
			else if (type == shader_compiler::EShaderType::eGeometry)
				return gfx::ShaderStageFlag::eGeometry;
			else if (type == shader_compiler::EShaderType::eFragment)
				return gfx::ShaderStageFlag::eFragment;
			else if (type == shader_compiler::EShaderType::eCompute)
				return gfx::ShaderStageFlag::eCompute;

			return static_cast<gfx::ShaderStageFlag>(0);
		}

		int32_t GetShaderIndex(shader_compiler::EShaderType type)
		{
			if (type == shader_compiler::EShaderType::eVertex)
				return 0;
			else if (type == shader_compiler::EShaderType::eControl)
				return 1;
			else if (type == shader_compiler::EShaderType::eEvaluation)
				return 2;
			else if (type == shader_compiler::EShaderType::eGeometry)
				return 3;
			else if (type == shader_compiler::EShaderType::eFragment)
				return 4;
			else if (type == shader_compiler::EShaderType::eCompute)
				return 5;

			return -1;
		}


		UploadContext::UploadContext(gfx::Context& context, filesystem::IVirtualFileSystemInterface* vfs) :
			context{ context },
			vfs{ vfs }
		{
			queue = context.RequestQueue(gfx::QueueTypeFlag::eGraphics, 1u);

			assert(k_max_upload_frames > upload_frame_count);

			for (int32_t i = 0; i < upload_frame_count; ++i)
			{
				upload_frames[i] = std::make_unique<UploadFrame>(context, *queue);
			}

			log_info("Loading shader cache...");
			if (!vfs->ReadBson("/root/embed/rendering/shaders/shader_cache.shc", shader_cache))
				log_error("Failed to load shader cache. ");

			const char* mip_generate_shaders[] = { 
				"utility:generate_mips_4x", 
				"utility:generate_mips_4x:MIP_2", 
				"utility:generate_mips_4x:MIP_2:MIP_3", 
				"utility:generate_mips_4x:MIP_2:MIP_3:MIP_4"
			};

			for (int32_t i = 0; i < mip_generators.size(); ++i)
			{
				auto& generator = mip_generators[i];

				generator.shader = LoadShader(mip_generate_shaders[i]);

				gfx::RootSignatureCreateInfo root_sign_ci{};
				root_sign_ci.shader = generator.shader;

				generator.root_signature = context.CreateRootSignature(root_sign_ci);

				gfx::ComputePipelineCreateInfo pipeline_ci{};
				pipeline_ci.shader = generator.shader;
				pipeline_ci.root_signature = generator.root_signature;

				generator.pipeline = context.CreatePipeline(pipeline_ci);
			}

			gfx::SamplerCreateInfo sampler_ci{};
			sampler_ci.address_u = sampler_ci.address_v = sampler_ci.address_w = gfx::SamplerAddressMode::eClampToEdge;
			sampler_ci.min_filter = gfx::Filter::eLinear;
			sampler_ci.mag_filter = gfx::Filter::eLinear;
			sampler_ci.mip_map_mode = gfx::Filter::eLinear;

			mg_sampler = context.CreateSampler(sampler_ci);

			// Create descriptor allocator for mip generation
			mg_descriptor_allocator = context.CreateDescriptorAllocator();
		}

		UploadContext::~UploadContext()
		{
			for (auto& [hash, sampler] : sampler_cache)
			{
				context.Destroy(sampler);
			}

			// Delete mip generation resources
			for (int32_t i = 0; i < mip_generators.size(); ++i)
			{
				context.Destroy(mip_generators[i].pipeline);
				context.Destroy(mip_generators[i].root_signature);
				context.Destroy(mip_generators[i].shader);
			}

			context.Destroy(mg_descriptor_allocator);
		}

		gfx::Texture* UploadContext::LoadTexture(const std::string& texture_path)
		{
			BeginFrame();
			using namespace utl;

			auto name = filesystem::fs::get_stem(texture_path);
			auto ext = filesystem::fs::get_ext(texture_path);
			auto ext_hash = utl::murmur_hash(ext);
			bool generate_mips{ false };

			std::vector<uint8_t> data{};
			vfs->ReadFile(texture_path, data);

			gfx::TextureCreateInfo create_info{};
			gfx::CopyTextureInfo copy_texture{};

			create_info.usage |= gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eTransferable;

			auto& upload_frame = GetActiveFrame();

			switch (ext_hash)
			{
			case ".jpg"_hash32:
			case ".jpeg"_hash32:
			case ".png"_hash32:
			case ".tga"_hash32:
			case ".bmp"_hash32:
			case ".psd"_hash32:
			case ".gif"_hash32:
			case ".hdr"_hash32:
			case ".pic"_hash32:
			case ".pnm"_hash32:
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
				auto& src_buffer = upload_frame.RequestStagingBuffer(data_size);

				// Copy image data to staging buffer
				src_buffer.Update(rawdata, data_size);
				stbi_image_free(rawdata);

				copy_texture.src_buffer = &src_buffer;

				auto& region = copy_texture.copy_regions[copy_texture.copy_region_count++];
				region.size_x = width;
				region.size_y = height;
			} break;
			case ".ktx"_hash32:
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
				auto& src_buffer = upload_frame.RequestStagingBuffer(kTexture->dataSize);
				src_buffer.Update(kTexture->pData, kTexture->dataSize);
				copy_texture.src_buffer = &src_buffer;
				
				// Collect image copy regions
				auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
				for (u32 layer = 0; layer < num_layers; layer++)
				{
					for (u32 level = 0u; level < kTexture->numLevels; level++)
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
			case ".ktx2"_hash32:
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
				auto& src_buffer = upload_frame.RequestStagingBuffer(kTexture->dataSize);
				src_buffer.Update(kTexture->pData, kTexture->dataSize);
				copy_texture.src_buffer = &src_buffer;

				// Collect image copy regions
				auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
				for (u32 layer = 0; layer < num_layers; layer++)
				{
					for (u32 level = 0u; level < kTexture->numLevels; level++)
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

			gfx::Texture* texture = context.CreateTexture(create_info);
			copy_texture.dst_texture = texture;

			auto& command_list = upload_frame.AcquireOrGetCommandList();

			gfx::TextureBarrier texture_barrier{};
			texture_barrier.texture = texture;
			texture_barrier.src_state = gfx::ResourceStateFlag::eUndefined;
			texture_barrier.dst_state = gfx::ResourceStateFlag::eCopyDst;

			gfx::ResourceBarrierInfo resource_barrier{};
			resource_barrier.texture_barriers = &texture_barrier;
			resource_barrier.texture_barrier_count = 1u;

			// Transfer to intermediate layout
			command_list.ResourceBarrier(resource_barrier);

			// Copy texture regions
			command_list.CopyTexture(copy_texture);

			gfx::ResourceStateFlags current_resource_state{ gfx::ResourceStateFlag::eCopyDst };
			if (generate_mips)
			{
				uint32_t total_mip_count = texture->GetMipLevelCount();
				for (int32_t mip = 1; mip < total_mip_count;)
				{
					uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

					group_x = texture->GetWidth() >> mip;
					group_y = texture->GetHeight() >> mip;

					uint32_t mip_count{ 4u };
					if(mip + mip_count > total_mip_count)
						mip_count = total_mip_count - mip;

					auto& generator = mip_generators[mip_count - 1];

					gfx::ResourceDescriptor* descriptor = mg_descriptor_allocator->AllocateDescriptors(generator.root_signature);

					command_list.BindPipeline(generator.pipeline);

					// At first make texture readable
					texture_barrier.src_state = current_resource_state;
					texture_barrier.dst_state = gfx::ResourceStateFlag::eRead | gfx::ResourceStateFlag::eCompute | gfx::ResourceStateFlag::eSample | gfx::ResourceStateFlag::eGeneral;
					command_list.ResourceBarrier(resource_barrier);
					current_resource_state = texture_barrier.dst_state;

					gfx::TextureDescriptor bind_resource{};
					bind_resource.type = gfx::DescriptorType::eCombinedImageSampler;
					bind_resource.set = 0;
					bind_resource.binding = 5;

					bind_resource.texture = texture;
					bind_resource.sampler = mg_sampler;
					bind_resource.subresource_index = mip - 1;
					bind_resource.resource_state = current_resource_state;

					// Bind texture slot
					descriptor->BindTexture(bind_resource);

					// Make texture writable now
					texture_barrier.src_state = current_resource_state;
					texture_barrier.dst_state = gfx::ResourceStateFlag::eWrite | gfx::ResourceStateFlag::eCompute | gfx::ResourceStateFlag::eStorage | gfx::ResourceStateFlag::eGeneral;
					command_list.ResourceBarrier(resource_barrier);
					current_resource_state = texture_barrier.dst_state;

					bind_resource.type = gfx::DescriptorType::eStorageImage;
					bind_resource.resource_state = current_resource_state;

					for (int32_t i = 0; i < mip_count; ++i)
					{
						bind_resource.binding = i + 1;
						bind_resource.subresource_index = mip + i;
						descriptor->BindTexture(bind_resource);
					}

					// Finally bind descriptor
					gfx::BindDescriptorsInfo bind_info{};
					bind_info.pipeline_type = gfx::PipelineType::eCompute;
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
			}

			// Transfer to final layout
			texture_barrier.src_state = current_resource_state;
			texture_barrier.dst_state = gfx::ResourceStateFlag::eShaderReadOnlyResource;
			command_list.ResourceBarrier(resource_barrier);

			return texture;
		}

		gfx::Texture* UploadContext::LoadTexture(const gfx::TextureCreateInfo& create_info, std::span<uint8_t> texture_data)
		{
			gfx::Texture* texture = context.CreateTexture(create_info);

			gfx::CopyTextureInfo copy_texture{};
			copy_texture.dst_texture = texture;

			auto& upload_frame = GetActiveFrame();
			auto& src_buffer = upload_frame.RequestStagingBuffer(texture_data.size());
			src_buffer.Update(texture_data.data(), texture_data.size());
			copy_texture.src_buffer = &src_buffer;

			auto& copy_region = copy_texture.copy_regions[copy_texture.copy_region_count++];
			copy_region.size_x = create_info.width;
			copy_region.size_y = create_info.height;
			copy_region.size_z = create_info.depth;

			auto& command_list = upload_frame.AcquireOrGetCommandList();

			gfx::TextureBarrier texture_barrier{};
			texture_barrier.texture = texture;
			texture_barrier.src_state = gfx::ResourceStateFlag::eUndefined;
			texture_barrier.dst_state = gfx::ResourceStateFlag::eCopyDst;

			gfx::ResourceBarrierInfo resource_barrier{};
			resource_barrier.texture_barriers = &texture_barrier;
			resource_barrier.texture_barrier_count = 1u;

			// Transfer to intermediate layout
			command_list.ResourceBarrier(resource_barrier);

			// Copy texture regions
			command_list.CopyTexture(copy_texture);

			// Transfer to final layout
			texture_barrier.src_state = gfx::ResourceStateFlag::eCopyDst;
			texture_barrier.dst_state = gfx::ResourceStateFlag::eShaderReadOnlyResource;
			command_list.ResourceBarrier(resource_barrier);

			return texture;
		}

		gfx::Shader* UploadContext::LoadShader(const std::string& shader_name)
		{
			auto name_info = GetShaderNameInfo(shader_name);

			uint32_t library_id = utl::murmur_hash(name_info.library_name);
			uint32_t shader_id = shader_compiler::make_module_unique_name(name_info.shader_name, name_info.definitions);

			// Looking for shader library (container with shader code and with shader modules)
			auto found_library = shader_cache.libraries.find(library_id);
			if (found_library == shader_cache.libraries.end())
			{
				log_error("Failed to find shader library named: {}", name_info.library_name);
				assert(false && "FAILED TO FIND SHADER LIBRARY.");
				return nullptr;
			}

			shader_compiler::FCompiledLibrary& shader_library = found_library->second;

			// Looking for shader module (shader type with shader association links)
			auto found_shader_module = shader_library.modules.find(shader_id);
			if (found_shader_module == shader_library.modules.end())
			{
				log_error("Failed to find shader with name: \"{}\" in library: {}", name_info.shader_name, name_info.library_name);
				assert(false && "FAILED TO FIND SHADER.");
				return nullptr;
			}

			const utl::vector<shader_compiler::FShaderModule>& shader_modules = found_shader_module->second;

			gfx::ShaderCreateInfo shader_create_info{};

			for (const auto& shader_module : shader_modules)
			{
				auto pool_name = shader_compiler::get_shader_type(shader_module.type);

				auto found_shader_pool = shader_library.shaders.find(pool_name);
				if (found_shader_pool == shader_library.shaders.end())
				{
					log_error("Shader pool for module named \"{}\" was not found.", name_info.shader_name);
					return nullptr;
				}

				auto& shader_pool = found_shader_pool->second;

				auto found_shader_code = shader_pool.find(shader_module.link);
				if (found_shader_code == shader_pool.end())
					continue;

				shader_compiler::FShaderData& shader_data = found_shader_code->second;
				
				if (shader_data.data.empty())
				{
					auto file_path = filesystem::fs::path_append("/root/embed/rendering/shaders", shader_data.path);
					if (!vfs->ReadFile(file_path, shader_data.data))
					{
						log_error("Failed to load shader: {}.", shader_data.path);
						return nullptr;
					}
				}

				auto shader_stage_flag = GetShaderStageFlag(shader_module.type);
				auto shader_stage_index = GetShaderIndex(shader_module.type);

				shader_create_info.stages |= shader_stage_flag;

				auto& shader_stage = shader_create_info.shader_stages[shader_stage_index];
				shader_stage.stage = shader_stage_flag;
				shader_stage.entry_point = "main";
				shader_stage.code = std::span<uint8_t>(reinterpret_cast<uint8_t*>(shader_data.data.data()), shader_data.data.size() * sizeof(uint32_t));
			}

			return context.CreateShader(shader_create_info);
		}

		gfx::Sampler* UploadContext::GetSampler(const gfx::SamplerCreateInfo& create_info)
		{
			uint32_t sampler_hash = gfx::Hash(create_info);
			if (auto found = sampler_cache.find(sampler_hash); found != sampler_cache.end())
				return found->second;

			return sampler_cache[sampler_hash] = context.CreateSampler(create_info);
		}

		void UploadContext::BeginBufferUpdate(gfx::BufferUpdateInfo& update_info)
		{
			BeginFrame();

			gfx::Buffer* dst_buffer = update_info.dst_buffer;
			
			if (!(dst_buffer->GetUsageFlags() & gfx::BufferUsageFlag::eGpuOnly))
			{
				log_warning("This buffer can be updated from the CPU, no additional memory needs to be allocated.");
				update_info.map_info.data = dst_buffer->Map();
				update_info.map_info.avaliable_size = dst_buffer->GetSize();
			}
			else
			{
				uint32_t allocation_size = update_info.update_size > 0ull ? update_info.update_size : dst_buffer->GetSize();
			
				auto& upload_frame = GetActiveFrame();
				auto& src_buffer = upload_frame.RequestStagingBuffer(allocation_size, &update_info.map_info.buffer_index);
			
				update_info.map_info.data = src_buffer.Map();
				update_info.map_info.avaliable_size = src_buffer.GetSize();
			}
		}

		void UploadContext::EndBufferUpdate(gfx::BufferUpdateInfo& update_info)
		{
			gfx::Buffer* dst_buffer = update_info.dst_buffer;

			if (update_info.map_info.avaliable_size == ~0ull)
			{
				dst_buffer->Unmap();
			}
			else
			{
				auto& upload_frame = GetActiveFrame();
				auto& command_list = upload_frame.AcquireOrGetCommandList();
				auto& src_buffer = upload_frame.GetStagingBuffer(update_info.map_info.buffer_index);

				// TODO: Add memory barrier befor copy for dst buffer
				gfx::BufferBarrier buffer_barrier{};
				buffer_barrier.buffer = dst_buffer;
				buffer_barrier.src_state = gfx::ResourceStateFlag::eCopyDst | gfx::ResourceStateFlag::eCopySrc;
				buffer_barrier.dst_state = gfx::ResourceStateFlag::eCopyDst;

				gfx::ResourceBarrierInfo barrier_info{};
				barrier_info.buffer_barriers = &buffer_barrier;
				barrier_info.buffer_barrier_count = 1u;

				command_list.ResourceBarrier(barrier_info);

				gfx::CopyBufferInfo copy_buffer{};
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

		gfx::SemaphoreSubmitInfo UploadContext::Submit(gfx::SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count)
		{
			if (writing_commands)
			{
				auto& upload_frame = GetActiveFrame();
				auto& command_list = upload_frame.AcquireOrGetCommandList();

				command_list.EndMarker();
				command_list.End();
				writing_commands = false;

				auto signal_semaphore_info = upload_frame.Submit(wait_infos, wait_semaphore_count);

				active_upload_frame = (active_upload_frame + 1u) % upload_frame_count;

				return signal_semaphore_info;
			}

			return {};
		}

		gfx::Context& UploadContext::GetContext()
		{
			return context;
		}

		gfx::Context const& UploadContext::GetContext() const
		{
			return context;
		}

		UploadFrame& UploadContext::GetActiveFrame()
		{
			return *upload_frames[active_upload_frame];
		}

		void UploadContext::BeginFrame()
		{
			if (!writing_commands)
			{
				auto& upload_frame = GetActiveFrame();
				upload_frame.Reset();

				auto& command_list = upload_frame.AcquireOrGetCommandList();

				command_list.Begin();
				command_list.BeginMarker("Upload resources", 0xFFFFFFFF);

				mg_descriptor_allocator->Reset();

				writing_commands = true;
			}
		}
	}
}