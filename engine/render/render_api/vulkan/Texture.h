#pragma once

#include <memory>
#include <VkResourceStructures.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Image;
			class ImageView;
			class RenderDevice;
			class Texture;

			class TextureBuilder
			{
			public:
				std::string debug_name;

				vk::Format image_format;
				ETextureCreateFlags texture_flags;

				ERenderTargetSizePolicy sizing_policy;
				vk::Extent3D image_extent;
				float scale_factor;

				uint32_t mip_count;
				uint32_t layer_count;

				vk::SampleCountFlagBits sample_count;

				uint32_t sampler_id;

				vk::Image external_image;
				vk::ImageView external_image_view;
				

				TextureBuilder();

				TextureBuilder& SetDebugName(const std::string& name);
				TextureBuilder& SetFormat(vk::Format fmt);
				TextureBuilder& AddFlag(ETextureCreateFlagBits flag);
				TextureBuilder& SetFlags(ETextureCreateFlags flags);
				TextureBuilder& SetSizingPolicy(ERenderTargetSizePolicy policy);
				TextureBuilder& SetExtent(vk::Extent3D extent);
				TextureBuilder& SetScaleFactor(float scale);

				TextureBuilder& SetMipCount(uint32_t count);
				TextureBuilder& SetLayerCount(uint32_t count);

				TextureBuilder& SetSampleCount(vk::SampleCountFlagBits count);
				TextureBuilder& SetSampler(uint32_t id);

				TextureBuilder& SetExternalImage(vk::Image handle);
				TextureBuilder& SetExternalImageView(vk::ImageView handle);

				Texture Build(RenderDevice& render_device) const;
				std::unique_ptr<Texture> BuildUnique(RenderDevice& render_device) const;
			};

			class Texture
			{
			public:
				Texture(RenderDevice& render_device, const TextureBuilder& builder);
				Texture(Texture& other, vk::Extent3D new_extent);
				~Texture();

				Image& GetImage();
				const Image& GetImage() const;

				ImageView& GetImageView(uint32_t idx = ~0u);
				const ImageView& GetImageView(uint32_t idx = ~0u) const;
				const std::vector<std::unique_ptr<ImageView>>& GetImageViews();

				ETextureCreateFlags const& GetFlags() const;
				uint32_t GetSamplerID() const;
				ERenderTargetSizePolicy GetSizingPolicy() const;
				float GetScaleFactor() const;

				void GenerateMipMaps(vk::CommandBuffer& command_buffer);

				vk::ImageLayout GetFinalLayout() const;
				vk::ImageLayout GetIntermediateLayout() const;

				vk::AttachmentLoadOp GetLoadOp();
				vk::AttachmentStoreOp GetStoreOp();

				void MakeImageLayoutTranslation(vk::ImageMemoryBarrier2KHR& translation_info, const FResourceBarrierInfo& barrier);
				void MakeImageLayoutTranslation(vk::MemoryBarrier2KHR& translation_info, const FResourceBarrierInfo& barrier);

				vk::ImageMemoryBarrier2KHR GetTranslationInfo(const FVkTextureTranslationInfo& translation_info);

				vk::DescriptorImageInfo GetDescriptor(uint32_t level);
			private:
				std::unique_ptr<Image> image;

				std::unique_ptr<ImageView> base;
				std::vector<std::unique_ptr<ImageView>> mips;

				uint32_t sampler{ ~0u };
				ETextureCreateFlags flags{ 0u };
				ERenderTargetSizePolicy sizing_policy{};
				f32 scale_factor{ 1.f };
				std::string debug_name;
			};
		}
	}
}