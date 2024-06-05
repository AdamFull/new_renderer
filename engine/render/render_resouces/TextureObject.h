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
		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}