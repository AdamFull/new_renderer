#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CGeometryPass : public IRenderPass
		{
		public:
			CGeometryPass(IRenderInterface* render_interface);

			~CGeometryPass() = default;

			void init(const FRenderPassCreateInfo& pass_ci) override;
			void bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info) override;
		private:
			void _render(const FRenderFrameInfo& frame_info) override;
		protected:
			const std::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);
		private:
			std::unordered_map<size_t, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};
		};
	}
}