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
			void Load(const std::string& path) override;
		private:
			u64 m_TextureId{ ~0ull };

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}