#include <base/Sampler.h>
#include <base/Device.h>
#include <base/DebugUtils.h>
#include <base/PhysicalDevice.h>
#include <RenderDevice.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			SamplerBuilder::SamplerBuilder()
			{
				create_info.mipmapMode = vk::SamplerMipmapMode::eNearest;
				create_info.mipLodBias = 0.0f;
				create_info.anisotropyEnable = VK_FALSE;
				create_info.compareEnable = VK_FALSE;
				create_info.compareOp = vk::CompareOp::eLess;
				create_info.minLod = 0.0f;
				create_info.maxLod = 1.0f;
				create_info.borderColor = vk::BorderColor::eIntOpaqueWhite;
				create_info.unnormalizedCoordinates = VK_FALSE;
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

			uint64_t SamplerBuilder::Hash() const
			{
				uint32_t hash{ 0u };

				hash = utl::murmur_hash(static_cast<uint32_t>(create_info.flags), hash);
				hash = utl::murmur_hash(utl::enum_cast(create_info.magFilter), hash);
				hash = utl::murmur_hash(utl::enum_cast(create_info.minFilter), hash);
				hash = utl::murmur_hash(utl::enum_cast(create_info.mipmapMode), hash);
				hash = utl::murmur_hash(utl::enum_cast(create_info.addressModeU), hash);
				hash = utl::murmur_hash(utl::enum_cast(create_info.addressModeV), hash);
				hash = utl::murmur_hash(utl::enum_cast(create_info.addressModeW), hash);
				hash = utl::murmur_hash(create_info.mipLodBias, hash);
				hash = utl::murmur_hash(create_info.anisotropyEnable, hash);
				hash = utl::murmur_hash(create_info.maxAnisotropy, hash);
				hash = utl::murmur_hash(create_info.compareEnable, hash);
				hash = utl::murmur_hash(utl::enum_cast(create_info.compareOp), hash);
				hash = utl::murmur_hash(create_info.minLod, hash);
				hash = utl::murmur_hash(create_info.maxLod, hash);
				hash = utl::murmur_hash(utl::enum_cast(create_info.borderColor), hash);
				hash = utl::murmur_hash(create_info.unnormalizedCoordinates, hash);

				return hash;
			}


			Sampler SamplerBuilder::Build(RenderDevice& render_device) const
			{
				Finalize(render_device);
				return Sampler(render_device, *this);
			}

			std::unique_ptr<Sampler> SamplerBuilder::BuildUnique(RenderDevice& render_device) const
			{
				Finalize(render_device);
				return std::make_unique<Sampler>(render_device, *this);
			}

			void SamplerBuilder::Finalize(RenderDevice& render_device) const
			{
				if (create_info.anisotropyEnable && create_info.maxAnisotropy == 0.0f)
				{
					auto& physical_device = render_device.GetPhysicalDevice();
					create_info.maxAnisotropy = physical_device.GetProperties().limits.maxSamplerAnisotropy;
				}
			}


			Sampler::Sampler(RenderDevice& render_device, const SamplerBuilder& builder) :
				VulkanResource{ nullptr },
				render_device{ render_device }
			{
				handle = render_device.CreateSampler(builder.create_info, builder.debug_name);
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