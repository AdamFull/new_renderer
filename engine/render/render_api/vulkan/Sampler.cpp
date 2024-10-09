#include <engine/render/render_api/vulkan/Sampler.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/DebugUtils.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			SamplerBuilder::SamplerBuilder()
			{

			}

			SamplerBuilder& SamplerBuilder::SetDebugName(const std::string& name)
			{
				debug_name = name;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetFlags(vk::SamplerCreateFlags flags)
			{
				create_info.flags = flags;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetMagFilter(vk::Filter filter)
			{
				create_info.magFilter = filter;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetMinFilter(vk::Filter filter)
			{
				create_info.minFilter = filter;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetMipmapMode(vk::SamplerMipmapMode mipmap_mode)
			{
				create_info.mipmapMode = mipmap_mode;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetAddressModeU(vk::SamplerAddressMode mode)
			{
				create_info.addressModeU = mode;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetAddressModeV(vk::SamplerAddressMode mode)
			{
				create_info.addressModeV = mode;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetAddressModeW(vk::SamplerAddressMode mode)
			{
				create_info.addressModeW = mode;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetAddressMode(vk::SamplerAddressMode mode)
			{
				create_info.addressModeU = create_info.addressModeV = create_info.addressModeW = mode;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetLodBias(float bias)
			{
				create_info.mipLodBias = bias;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetEnableAnisotropy(bool value)
			{
				create_info.anisotropyEnable = value;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetMaxAnisotropy(float value)
			{
				create_info.maxAnisotropy = value;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetCompareEnable(bool value)
			{
				create_info.compareEnable = value;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetCompareOp(vk::CompareOp op)
			{
				create_info.compareOp = op;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetMinLod(float value)
			{
				create_info.minLod = value;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetMaxLod(float value)
			{
				create_info.maxLod = value;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetBorderColor(vk::BorderColor border_color)
			{
				create_info.borderColor = border_color;
				return *this;
			}

			SamplerBuilder& SamplerBuilder::SetUnnormalizedCoordinates(bool value)
			{
				create_info.unnormalizedCoordinates = value;
				return *this;
			}


			Sampler SamplerBuilder::Build(RenderDevice& render_device) const
			{
				return Sampler(render_device, *this);
			}

			std::unique_ptr<Sampler> SamplerBuilder::BuildUnique(RenderDevice& render_device) const
			{
				return std::make_unique<Sampler>(render_device, *this);
			}

			Sampler::Sampler(RenderDevice& render_device, const SamplerBuilder& builder) :
				VulkanResource{ nullptr },
				render_device{ render_device }
			{
				handle = render_device.CreateSampler2(builder.create_info, builder.debug_name);
			}

			Sampler::~Sampler()
			{
				render_device.Destroy(handle);
			}

			Sampler::Sampler(Sampler&& other) noexcept :
				VulkanResource{ std::move(other) },
				render_device{ other.render_device }
			{
			}
		}
	}
}