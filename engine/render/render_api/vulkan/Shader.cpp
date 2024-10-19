#include <engine/render/render_api/vulkan/Shader.h>
#include <engine/render/render_api/vulkan/PhysicalDevice.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>
#include <engine/render/render_api/vulkan/Buffer.h>
#include <engine/render/render_api/vulkan/Texture.h>
#include <engine/render/render_api/vulkan/ImageView.h>
#include <engine/render/render_api/vulkan/Sampler.h>
#include <engine/render/render_api/vulkan/VkCommon.h>
#include <engine/render/render_api/vulkan/RenderContext.h>
#include <engine/render/render_api/vulkan/DescriptorSet.h>
#include <engine/render/render_api/vulkan/CommandBuffer.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			ShaderBuilder& ShaderBuilder::SetBindPoint(vk::PipelineBindPoint bind_point)
			{
				this->bind_point = bind_point;
				return *this;
			}

			ShaderBuilder& ShaderBuilder::SetUsageCount(uint32_t usage_count)
			{
				this->usage_count = usage_count;
				return *this;
			}

			ShaderBuilder& ShaderBuilder::AddShaderModule(vk::ShaderStageFlagBits stage, std::span<uint32_t> code)
			{
				shader_modules[shader_module_count++] = ShaderModuleInfo{ stage, vk::ShaderModuleCreateInfo{ {}, code.size() * sizeof(uint32_t), code.data()}};
				reflection_builder.AddShaderStage(stage, code);
				return *this;
			}

			ShaderBuilder& ShaderBuilder::AddDefaultLayout(vk::DescriptorSetLayout layout)
			{
				default_layouts[default_descriptor_count++] = layout;
				return *this;
			}

			Shader ShaderBuilder::Build(RenderDevice& render_device)
			{
				return Shader(render_device, *this);
			}

			std::unique_ptr<Shader> ShaderBuilder::BuildUnique(RenderDevice& render_device)
			{
				return std::make_unique<Shader>(render_device, *this);
			}


			vk::WriteDescriptorSet& DescriptorUpdate::AddDescriptorWrite()
			{
				return write_descriptor_sets[writes_count++];
			}

			vk::DescriptorImageInfo& DescriptorUpdate::AddImageInfo()
			{
				return image_descriptors[image_count++];
			}

			vk::DescriptorBufferInfo& DescriptorUpdate::AddBufferInfo()
			{
				return buffer_descriptors[buffer_count++];
			}

			void DescriptorUpdate::Clear()
			{
				writes_count = 0u;
				image_count = 0u;
				buffer_count = 0u;
			}


			Shader::Shader(RenderDevice& render_device, ShaderBuilder& builder) :
				render_device{ render_device },
				bind_point{ builder.bind_point }
			{
				// Create shader modules
				for (int32_t i = 0; i < builder.shader_module_count; ++i)
				{
					auto& module_info = builder.shader_modules[i];
					shader_modules[i] = render_device.CreateShaderModule2(module_info.create_info);

					vk::PipelineShaderStageCreateInfo shader_stage{};
					shader_stage.stage = module_info.stage;
					shader_stage.module = shader_modules[i];
					shader_stage.pName = module_info.entry_point;

					if (bind_point == vk::PipelineBindPoint::eGraphics)
					{
						builder.graphics_pipeline_builder.AddShaderStage(shader_stage);
					}
					else if (bind_point == vk::PipelineBindPoint::eCompute)
					{
						builder.compute_pipeline_builder.SetShaderStage(shader_stage);
						break;
					}
					else
					{
						assert(false);
					}
				}

				// Create reflection
				reflection = builder.reflection_builder.Build();

				// Update shader stage specialization info
				//for (auto& shader_stage : pipeline_ci.shaderStages)
				//{
				//	auto found = reflection->specializationConstantStages.find(shader_stage.stage);
				//	if (found != reflection->specializationConstantStages.end())
				//		shader_stage.pSpecializationInfo = &found->second.info;
				//}


				// TODO: generate better object names. Current names can overlap

				// TODO: dynamic per frame resource allocation
				maximumDescriptorSlots = builder.usage_count * 2u;

				std::array<DescriptorSetLayoutBuilder, k_max_descriptor_sets> descriptor_set_layout_builders{};

				// Create descriptor set layouts
				for (const auto& [name, block] : reflection.uniformBlocks)
				{
					vk::DescriptorSetLayoutBinding descriptor_binding{};
					descriptor_binding.binding = block.binding;
					descriptor_binding.descriptorType = block.descriptorType;
					descriptor_binding.descriptorCount = 1u;
					descriptor_binding.stageFlags = block.stageFlags;

					auto& builder = descriptor_set_layout_builders[block.set];
					builder.AddLayoutBinding(descriptor_binding);
				}

				for (const auto& [name, uniform] : reflection.uniforms)
				{
					vk::DescriptorSetLayoutBinding descriptor_binding = {};
					descriptor_binding.binding = uniform.binding;
					descriptor_binding.descriptorType = uniform.descriptorType;
					descriptor_binding.descriptorCount = 1u;
					descriptor_binding.stageFlags = uniform.stageFlags;

					auto& builder = descriptor_set_layout_builders[uniform.set];
					builder.AddLayoutBinding(descriptor_binding);
				}

				for (int32_t set = 0; set < k_max_descriptor_sets; ++set)
				{
					auto& builder = descriptor_set_layout_builders[set];
					if (builder.CanBuild())
					{
						descriptor_set_layouts[set] = builder.BuildUnique(render_device);
					}
				}

				// Minimal buffer offset
				auto& physical = render_device.GetPhysicalDevice();
				auto minOffsetAllignment = std::lcm(physical.GetProperties().limits.minUniformBufferOffsetAlignment, physical.GetProperties().limits.nonCoherentAtomSize);

				// Create uniform buffers
				for (const auto& [id, uniform_block] : reflection.uniformBlocks)
				{
					// Storage buffers is binding from outside
					if (uniform_block.descriptorType == vk::DescriptorType::eStorageBuffer)
						continue;

					auto builder = BufferBuilder(uniform_block.size, maximumDescriptorSlots, minOffsetAllignment);
					builder.SetVmaUsage(vma::MemoryUsage::eCpuToGpu);
					builder.SetSharingMode(vk::SharingMode::eConcurrent);

//#ifndef BUILD_WITHOUT_DEBUGGIND
//					auto* debug_name = debug_mapping::find(id);
//					if (debug_name)
//						builder.SetDebugName(std::format("{}:{}", resources_info.name, debug_name));
//					else
//						builder.SetDebugName(std::format("{}:{:x}", resources_info.name, id));
//#endif

					if (uniform_block.descriptorType == vk::DescriptorType::eUniformBuffer)
						builder.SetUsage(vk::BufferUsageFlagBits::eUniformBuffer);
					else if (uniform_block.descriptorType == vk::DescriptorType::eStorageBuffer)
						builder.SetUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer);

					uniform_buffers.emplace(id, builder.BuildUnique(render_device));
				}

				// Create pipeline layout
				std::array<vk::DescriptorSetLayout, k_max_descriptor_sets> descriptor_set_layouts_ci{};
				vk::PipelineLayoutCreateInfo layout_ci{};
				layout_ci.pSetLayouts = descriptor_set_layouts_ci.data();

				// NOTE: adding global descriptor set layout
				descriptor_set_layouts_ci[0] = builder.default_layouts[0];
				layout_ci.setLayoutCount++;

				// Send descriptor set layouts to Create info
				for (int32_t idx = 1; idx < descriptor_set_layouts.size(); ++idx)
				{
					if (descriptor_set_layouts[idx])
					{
						// I think that should be idx here, but it can cause errors when some layout is not exists
						descriptor_set_layouts_ci[layout_ci.setLayoutCount++] = descriptor_set_layouts[idx]->GetHandle();
					}
				}
				//for (const auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
				//	layout_ci.descriptorSetLayouts.emplace_back(descriptor_set_layout);

				// Prepare push blocks info
				u32 current_offset{ 0u };
				for (const auto& [name, block] : reflection.pushBlocks)
				{
					auto& push_constant_range = pushConstantRanges.emplace_back();
					push_constant_range.stageFlags = block.stageFlags;
					push_constant_range.offset = current_offset;
					push_constant_range.size = utl::cast<u32>(block.size);
					current_offset += push_constant_range.size;
				}

				layout_ci.pPushConstantRanges = pushConstantRanges.data();
				layout_ci.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());

				layout = render_device.CreatePipelineLayout2(layout_ci);

				if (bind_point == vk::PipelineBindPoint::eGraphics)
				{
					builder.graphics_pipeline_builder.SetPipelineLayout(layout);
					pipeline = builder.graphics_pipeline_builder.BuildUnique(render_device);
				}
				else if (bind_point == vk::PipelineBindPoint::eCompute)
				{
					builder.compute_pipeline_builder.SetPipelineLayout(layout);
					pipeline = builder.compute_pipeline_builder.BuildUnique(render_device);
				}
			}

			Shader::~Shader()
			{
				pipeline.reset();

				for (int32_t i = 0; i < shader_module_count; ++i)
				{
					render_device.Destroy(shader_modules[i]);
				}

				uniform_buffers.clear();
				render_device.Destroy(layout);
			}

			Shader::Shader(Shader&& other) noexcept :
				render_device{ other.render_device },
				pipeline{ std::exchange(other.pipeline, {}) },
				shader_modules{ std::exchange(other.shader_modules, {}) },
				shader_module_count{ std::exchange(other.shader_module_count, {}) },
				bind_point{ std::exchange(other.bind_point, {}) },
				layout{ std::exchange(other.layout, {}) },
				descriptor_set_layouts{ std::move(other.descriptor_set_layouts) },
				descriptor_sets{ std::exchange(other.descriptor_sets, {}) },
				pushConstantRanges{ std::exchange(other.pushConstantRanges, {}) }
			{
			}

			Pipeline& Shader::GetPipeline()
			{
				return *pipeline;
			}

			vk::PipelineBindPoint Shader::GetBindPoint()
			{
				return bind_point;
			}

			FShaderReflectionData Shader::GetReflectionData()
			{
				FShaderReflectionData reflection_data{};

				for (const auto& [id, vkuniform] : reflection.uniforms)
				{
					auto& uniform = reflection_data.uniforms[id];
					uniform.binding = vkuniform.binding;
					uniform.set = vkuniform.set;
					uniform.offset = vkuniform.offset;
					uniform.size = vkuniform.size;
				}

				for (const auto& [aid, vkuniform_block] : reflection.uniformBlocks)
				{
					if (vkuniform_block.descriptorType == vk::DescriptorType::eStorageBuffer)
						continue;

					auto& uniform_block = reflection_data.uniformBlocks[aid];
					uniform_block.binding = vkuniform_block.binding;
					uniform_block.set = vkuniform_block.set;
					uniform_block.size = vkuniform_block.size;

					for (const auto& [bid, vkuniform] : vkuniform_block.uniforms)
					{
						auto& uniform = uniform_block.uniforms[bid];
						uniform.binding = vkuniform.binding;
						uniform.set = vkuniform.set;
						uniform.offset = vkuniform.offset;
						uniform.size = vkuniform.size;
					}
				}

				for (const auto& [aid, vkpush_block] : reflection.pushBlocks)
				{
					auto& push_block = reflection_data.pushBlocks[aid];
					push_block.size = vkpush_block.size;

					for (const auto& [bid, vkuniform] : vkpush_block.uniforms)
					{
						auto& uniform = push_block.uniforms[bid];
						uniform.binding = vkuniform.binding;
						uniform.set = vkuniform.set;
						uniform.offset = vkuniform.offset;
						uniform.size = vkuniform.size;
					}
				}

				if (!reflection.localSizes.empty())
					reflection_data.localSizes = reflection.localSizes.front();

				return reflection_data;
			}

			void Shader::FlushConstantRanges(CommandBuffer& command_buffer, const utl::hash_map<u64, FUniformDataView>& constants)
			{
				// TODO: remove all other posibilities to have more than one push block. 
				for (const auto& [constant_name, constant_data] : constants)
				{
					auto found_reflection = reflection.pushBlocks.find(constant_name);
					if (found_reflection == reflection.pushBlocks.end())
						continue;

					auto& constant_block = found_reflection->second;
					command_buffer.PushConstants({ layout, constant_block.stageFlags, 0u, (u32)constant_data.size, constant_data.data });

					// Break, because we can have only one push constants block
					break;
				}
			}

			void Shader::ResetDescriptors(RenderContext& render_context)
			{
				auto& current_frame = render_context.GetCurrentFrame();
				descriptor_sets[0] = Handle(render_context.GetCurrentGlobalDescriptorSet());
				
				for (int32_t i = 1; i < k_max_descriptor_sets; ++i)
				{
					auto& descriptor_set_layout = descriptor_set_layouts[i];
					if (descriptor_set_layout)
					{
						descriptor_sets[i] = Handle(current_frame.RequestDescriptorSet(*descriptor_set_layout));
					}
				}
			}

			void Shader::BindDescriptors(CommandBuffer& command_buffer)
			{
				if (descriptor_update.writes_count)
				{
					auto& logical = render_device.GetLogicalDevice();
					Handle(logical).updateDescriptorSets(descriptor_update.writes_count, descriptor_update.write_descriptor_sets.data(), 0, nullptr);
					descriptor_update.Clear();
				}

				uint32_t descriptor_set_count{ 0u };

				// TODO: I think i can separate bindings by empty sets here. If some set is empty, i have to save last index, bind sets and after that continue bind from index.
				// NOTE: shader.descriptorUsage contains next index of descriptor set to use. 
				// Here i collecting all shader descriptor sets, that was changed in past
				for (int32_t i = 0; i < k_max_descriptor_sets; ++i)
				{
					if (descriptor_sets[i])
					{
						++descriptor_set_count;
					}
				}

				// TODO: determine the lowest descriptor index and start binding from lowest one (for example from 1)
				Handle(command_buffer).bindDescriptorSets(bind_point, layout, 0, descriptor_set_count, descriptor_sets.data(), 0u, nullptr);
			}

			void Shader::Bind(CommandBuffer& command_buffer)
			{
				command_buffer.BindPipeline(bind_point, pipeline->GetHandle());
			}

			void Shader::UpdateUniform(RenderContext& render_context, uint64_t reflection_name, std::span<const char> data)
			{
				auto& current_frame = render_context.GetCurrentFrame();

				// TODO: may be add something like index of buffer?
				auto found_reflection = reflection.uniformBlocks.find(reflection_name);
				if (found_reflection == reflection.uniformBlocks.end())
					return;

				// Prepare descriptor update data
				auto& uniform_block = found_reflection->second;

				auto requested_block = current_frame.RequestUniformBlock(uniform_block.size);

				// Not sure that it is correct, because i don't know how to determine previous buffer
				auto& buffer_descriptor = descriptor_update.AddBufferInfo();
				buffer_descriptor.buffer = requested_block.buffer.GetHandle();
				buffer_descriptor.range = requested_block.size;
				buffer_descriptor.offset = requested_block.offset;

				auto& write_descriptor_set = descriptor_update.AddDescriptorWrite();;
				write_descriptor_set.descriptorType = uniform_block.descriptorType;
				write_descriptor_set.dstBinding = uniform_block.binding;
				write_descriptor_set.dstSet = descriptor_sets[uniform_block.set];
				write_descriptor_set.pBufferInfo = &buffer_descriptor;
				write_descriptor_set.descriptorCount = 1;

				// Write new data to buffer memory
				requested_block.buffer.Update(data.data(), data.size(), requested_block.offset);
			}

			void Shader::UpdateTextureBinding(uint64_t reflection_name, Texture& texture, Sampler& sampler, uint32_t mip_level)
			{
				auto found_reflection = reflection.uniforms.find(reflection_name);
				if (found_reflection == reflection.uniforms.end())
					return;

				auto& texture_subresource = texture.GetImageView(mip_level);
				auto& uniform = found_reflection->second;

				auto& image_descriptor = descriptor_update.AddImageInfo();
				image_descriptor.imageView = texture_subresource.GetHandle();
				image_descriptor.sampler = sampler.GetHandle();
				image_descriptor.imageLayout = texture_subresource.GetImageLayout();

				auto& write_descriptor_set = descriptor_update.AddDescriptorWrite();
				write_descriptor_set.descriptorType = uniform.descriptorType;
				write_descriptor_set.dstBinding = uniform.binding;
				write_descriptor_set.dstSet = descriptor_sets[uniform.set];
				write_descriptor_set.pImageInfo = &image_descriptor;
				write_descriptor_set.descriptorCount = 1;
			}

			void Shader::UpdateBufferBinding(uint64_t reflection_name, Buffer& buffer)
			{
				auto found_reflection = reflection.uniformBlocks.find(reflection_name);
				if (found_reflection == reflection.uniformBlocks.end())
					return;

				// Prepare descriptor update data
				auto& uniform_block = found_reflection->second;

				// Not sure that it is correct, because i don't know how to determine previous buffer
				auto& buffer_descriptor = descriptor_update.AddBufferInfo();
				buffer_descriptor.buffer = buffer.GetHandle();
				buffer_descriptor.range = buffer.GetSize();
				buffer_descriptor.offset = 0ull;

				auto& write_descriptor_set = descriptor_update.AddDescriptorWrite();
				write_descriptor_set.descriptorType = uniform_block.descriptorType;
				write_descriptor_set.dstBinding = uniform_block.binding;
				write_descriptor_set.dstSet = descriptor_sets[uniform_block.set];
				write_descriptor_set.pBufferInfo = &buffer_descriptor;
				write_descriptor_set.descriptorCount = 1;
			}

			void Shader::DivideByLocalSizes(vk::DispatchIndirectCommand& command, uint64_t subgroup_index)
			{
				auto& localSizes = reflection.localSizes[subgroup_index];
				command.x = std::max(utl::cast<u32>(std::ceil(utl::cast<f32>(command.x) / utl::cast<f32>(localSizes[0]))), 1u);
				command.y = std::max(utl::cast<u32>(std::ceil(utl::cast<f32>(command.y) / utl::cast<f32>(localSizes[1]))), 1u);
				command.z = std::max(utl::cast<u32>(std::ceil(utl::cast<f32>(command.z) / utl::cast<f32>(localSizes[2]))), 1u);
			}

			Buffer* Shader::GetUniformBuffer(uint64_t uniform_name) const
			{
				auto found = uniform_buffers.find(uniform_name);
				if (found != uniform_buffers.end())
					return found->second.get();

				return nullptr;
			}
		}
	}
}