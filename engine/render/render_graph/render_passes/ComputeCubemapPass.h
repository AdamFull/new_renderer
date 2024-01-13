#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CComputeCubemapPass : public IComputePass
		{
		public:
			CComputeCubemapPass(IRenderInterface* render_interface, IRenderContext* render_context, IRenderGraph* render_graph);

			~CComputeCubemapPass() = default;

			void init(const FComputePassCreateInfo& pass_ci) override;
			void bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info) override;
		private:
			void _execute(const FComputeFrameInfo& frame_info) override;
		private:
			std::unique_ptr<IShaderObject> m_pShader{};
		};
	}
}