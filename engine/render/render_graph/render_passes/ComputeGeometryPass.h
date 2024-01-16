#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CComputeGeometryPass : public IComputePass
		{
		public:
			CComputeGeometryPass(IRenderInterface* render_interface);

			~CComputeGeometryPass() = default;

			void init(const FComputePassCreateInfo& pass_ci) override;
			void bake(std::unordered_map<u64, FRenderGraphAttachmentOperationInfo>& resource_info) override;
		private:
			void _execute(const FComputeFrameInfo& frame_info) override;
			const std::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);
		private:
			std::unordered_map<u64, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};
		};
	}
}