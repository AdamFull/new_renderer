#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		struct FBaseFrameInfo
		{
			f32 deltaTime{};
		};

		struct FRenderFrameInfo : public FBaseFrameInfo
		{
			FRect2D<f32> renderRect{};
		};

		struct FComputeFrameInfo : public FBaseFrameInfo
		{
		};

		class IRenderPass : public IBasePass
		{
		public:
			friend class CRenderGraph;
			~IRenderPass() = default;

			virtual void Init(const FRenderPassCreateInfo& pass_ci);
			virtual void Build(const FPassBuildInfo& build_info) override;
			virtual bool CanRender() const { return true; }

			void Execute(f32 dt) override;

			EPassType Type() override { return EPassType::eRender; }
		protected:
			void BuildAttachment(FGraphHandle resource_id, const FPassBuildInfo& build_info, FRenderGraphAttachmentInfo& attachment_info);
			virtual void _Render(const FRenderFrameInfo& frame_info) = 0;

			virtual FRHIHandle MakeShader(const std::string& name, EShaderCullModeFlagBits cull_mode, EShaderFrontFace front_face, bool alphaBlend = false, EDepthCompareOp depthOp = EDepthCompareOp::eLessOrEqual, const IVertexHandler* vbo = nullptr);
			void InitShaderResources(FRHIHandle);
			void BindShaderResources(FRHIHandle, f32 dt);
			void InitShaderResources(const std::unique_ptr<IShaderObject>& shader);
			void BindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt);
		protected:
			bool m_bFlipViewport{ false };

			u64 m_cameraIndex{ ~0ull };
			FGraphHandle m_depthTarget{ ~0ull };
			FGraphHandle m_stencilTarget{ ~0ull };
			u32 m_layerCount{ 1u };

			utl::vector<FRenderGraphAttachmentInfo> m_vColorAttachments{};
			utl::hash_map<FAPIHandle, FClearValues> m_clearValues;
			FRenderGraphAttachmentInfo m_depthAttachment{};
			FRenderGraphAttachmentInfo m_stencilAttachment{};

			IRenderInterface* m_pRenderInterface{ nullptr };
		};

		class IComputePass : public IBasePass
		{
		public:
			friend class CRenderGraph;
			~IComputePass() = default;

			virtual void Init(const FComputePassCreateInfo& pass_ci);
			virtual void Build(const FPassBuildInfo& build_info) override;

			void Execute(f32 dt) override;

			EPassType Type() override { return EPassType::eCompute; }
		protected:
			virtual void _Execute(const FComputeFrameInfo& frame_info) = 0;

			virtual void InitShaderResources(FRHIHandle shader);
			virtual void BindShaderResources(FRHIHandle shader, f32 dt);
			virtual void InitShaderResources(const std::unique_ptr<IShaderObject>& shader);
			virtual void BindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt);
		protected:
			FDispatch2Info m_dispatchGroup{};
			FComputeDispatchGroupInfo m_dispatchInfo{};

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}