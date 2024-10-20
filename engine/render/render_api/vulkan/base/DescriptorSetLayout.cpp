#include <base/DescriptorSetLayout.h>
#include <RenderDevice.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder()
			{
				ext_create_info.pBindingFlags = binding_flags_ext.data();

				create_info.pBindings = bindings.data();
				create_info.pNext = &ext_create_info;
			}

			DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::SetDebugName(const std::string& name)
			{
				debug_name = name;
				return *this;
			}

			DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::AddLayoutBinding(const vk::DescriptorSetLayoutBinding& binding, vk::DescriptorBindingFlagsEXT flags)
			{
				bindings[create_info.bindingCount++] = binding;
				binding_flags_ext[ext_create_info.bindingCount++] = flags;

				if (flags & vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind)
					create_info.flags |= vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT;

				return *this;
			}

			DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::SetCreateFlags(vk::DescriptorSetLayoutCreateFlags flags)
			{
				create_info.flags = flags;
				return *this;
			}

			bool DescriptorSetLayoutBuilder::CanBuild() const
			{
				return create_info.bindingCount > 0;
			}

			DescriptorSetLayout DescriptorSetLayoutBuilder::Build(RenderDevice& render_device) const
			{
				Finalize();
				return DescriptorSetLayout(render_device, *this);
			}

			std::unique_ptr<DescriptorSetLayout> DescriptorSetLayoutBuilder::BuildUnique(RenderDevice& render_device) const
			{
				Finalize();
				return std::make_unique<DescriptorSetLayout>(render_device, *this);
			}

			void DescriptorSetLayoutBuilder::Finalize() const
			{
				// Sort bindings
				std::sort(bindings.begin(), bindings.begin() + create_info.bindingCount, [](const vk::DescriptorSetLayoutBinding& l, const vk::DescriptorSetLayoutBinding& r) { return l.binding < r.binding; });

				// Remove not unique elements
				auto first = std::unique(bindings.begin(), bindings.begin() + create_info.bindingCount, [](const vk::DescriptorSetLayoutBinding& l, const vk::DescriptorSetLayoutBinding& r) { return l.binding == r.binding; });

				if (first != bindings.end());
			}

			DescriptorSetLayout::DescriptorSetLayout(RenderDevice& render_device, const DescriptorSetLayoutBuilder& builder) :
				VulkanResource{ nullptr },
				render_device{ render_device }
			{
				handle = render_device.CreateDescriptorSetLayout(builder.create_info, builder.debug_name);

				for (int32_t i = 0; i < builder.create_info.bindingCount; ++i)
				{
					auto& binding = builder.bindings[i];

					// Count number of required descriptors
					pool_sizes[utl::enum_cast(binding.descriptorType)] += binding.descriptorCount;

					layout_bindings[i] = { binding, builder.binding_flags_ext[i] };
					++binding_count;
				}
			}

			DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) :
				VulkanResource{ std::move(other) },
				render_device{ other.render_device },
				layout_bindings{ std::exchange(other.layout_bindings, {}) },
				binding_count{ std::exchange(other.binding_count, {}) },
				pool_sizes{ std::exchange(other.pool_sizes, {}) }
			{
			}

			DescriptorSetLayout::~DescriptorSetLayout()
			{
				render_device.Destroy(handle);
			}

			const DescriptorSetLayout::LayoutBindingArray& DescriptorSetLayout::GetBindings() const
			{
				return layout_bindings;
			}

			uint32_t DescriptorSetLayout::GetBindingCount() const
			{
				return binding_count;
			}

			vk::DescriptorSetLayoutBinding& DescriptorSetLayout::GetLayoutBinding(uint32_t binding_index)
			{
				return layout_bindings[binding_index].binding;
			}

			vk::DescriptorBindingFlagsEXT& DescriptorSetLayout::GetLayoutBindingFlags(uint32_t binding_index)
			{
				return layout_bindings[binding_index].flags;
			}

			const PoolSizes& DescriptorSetLayout::GetRequiredDescriptorCount() const
			{
				return pool_sizes;
			}
		}
	}
}