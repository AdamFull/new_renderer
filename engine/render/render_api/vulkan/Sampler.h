#pragma once

#include <engine/render/render_api/vulkan/VkResource.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Device;
			class Sampler;
			class VkRenderDevice;

			class SamplerBuilder
			{
			public:
				std::string debug_name;
				vk::SamplerCreateInfo create_info;

				SamplerBuilder();

				SamplerBuilder& SetDebugName(const std::string& name);
				SamplerBuilder& SetFlags(vk::SamplerCreateFlags flags);
				SamplerBuilder& SetMagFilter(vk::Filter filter);
				SamplerBuilder& SetMinFilter(vk::Filter filter);
				SamplerBuilder& SetMipmapMode(vk::SamplerMipmapMode mipmap_mode);
				SamplerBuilder& SetAddressModeU(vk::SamplerAddressMode mode);
				SamplerBuilder& SetAddressModeV(vk::SamplerAddressMode mode);
				SamplerBuilder& SetAddressModeW(vk::SamplerAddressMode mode);
				SamplerBuilder& SetAddressMode(vk::SamplerAddressMode mode);
				SamplerBuilder& SetLodBias(float bias);
				SamplerBuilder& SetEnableAnisotropy(bool value);
				SamplerBuilder& SetMaxAnisotropy(float value);
				SamplerBuilder& SetCompareEnable(bool value);
				SamplerBuilder& SetCompareOp(vk::CompareOp op);
				SamplerBuilder& SetMinLod(float value);
				SamplerBuilder& SetMaxLod(float value);
				SamplerBuilder& SetBorderColor(vk::BorderColor border_color);
				SamplerBuilder& SetUnnormalizedCoordinates(bool value);

				Sampler Build(VkRenderDevice& context) const;
				std::unique_ptr<Sampler> BuildUnique(VkRenderDevice& context) const;
			};

			class Sampler : public VulkanResource<vk::Sampler>
			{
			public:
				Sampler(VkRenderDevice& render_device, const SamplerBuilder& builder);
				~Sampler() override;

				Sampler(Sampler&) = delete;
				Sampler(Sampler&& other) noexcept;

				Sampler& operator=(const Sampler&) = delete;
				Sampler& operator=(Sampler&&) = delete;
			private:
				Device& device;
			};
		}
	}
}