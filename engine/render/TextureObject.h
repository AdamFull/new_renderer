#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CTextureObject : public ITextureObject
		{
		public:
			CTextureObject(IRenderInterface* render_interface);
			~CTextureObject() override;

			void Create(const std::string& name, const FTextureCreateInfo& create_info) override;
			void Create(const std::string& name, const FTextureCreateInfo& create_info, FTextureUploadInfo&& upload_info) override;

			void GetDimensions(u32* width, u32* height, u32* depth) override;
			void GetMipLevelCount(u32* mip_count) override;
			void GetLayerCount(u32* layer_count) override;

			FAPIHandle Handle() override;
		private:
			FAPIHandle m_textureId{};

			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}