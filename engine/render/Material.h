#pragma once

#include <interface/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CMaterial : public IRenderMaterial
		{
		public:
			friend class CRenderInterface;

			CMaterial(IRenderInterface* render_interface, IRenderContext* render_context);
			~CMaterial() override = default;
			
			void addTexture(const char* name, size_t texture_id) override;
			void bind() override;
		private:
			FMaterial m_materialParameters{};
			std::unique_ptr<IShaderObject> m_pShader{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}