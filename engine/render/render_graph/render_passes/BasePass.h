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

		class IBasePass
		{
		public:
			friend class CRenderGraph;
			virtual ~IBasePass() = default;

			virtual void build(const FPassBuildInfo& build_info) = 0;

			virtual void execute(f32 dt);

			virtual bool canExecute();

			virtual EPassType type() = 0;

		protected:
			virtual void initShaderResources(const std::unique_ptr<IShaderObject>& shader);
			virtual void bindShaderResources(const std::unique_ptr<IShaderObject>& shader);

			EQueueTypeFlagBits m_queue{};

			ERenderingSizeType m_sizePolicy{};
			u64 m_sizeTarget{ ~0ull };

			stl::unordered_map<ERenderResourceGroupType, stl::vector<u64>> m_renderResources{};
			stl::unordered_map<stl::string, u64> m_bakedBindings{};

			bool m_bOneTime{ false };
			bool m_bCanExecute{ true };

#ifndef BUILD_WITHOUT_DEBUGGIND
			stl::string m_debugName{};
#endif

			IRenderInterface* m_pRenderInterface{ nullptr };
		};

		class IRenderPass : public IBasePass
		{
		public:
			friend class CRenderGraph;
			~IRenderPass() = default;

			virtual void init(const FRenderPassCreateInfo& pass_ci);
			virtual void build(const FPassBuildInfo& build_info) override;

			void execute(f32 dt) override;

			EPassType type() override { return EPassType::eRender; }
		protected:
			void buildAttachment(u64 resource_id, const FPassBuildInfo& build_info, FRenderGraphAttachmentInfo& attachment_info);
			virtual void _render(const FRenderFrameInfo& frame_info) = 0;

			virtual void makeShader(const std::string& name, std::unique_ptr<IShaderObject>& shader, EShaderCullModeFlagBits cull_mode, EShaderFrontFace front_face, const IVertexHandler* vbo = nullptr);
			virtual void initShaderResources(const std::unique_ptr<IShaderObject>& shader) override;
			virtual void bindShaderResources(const std::unique_ptr<IShaderObject>& shader) override;
		protected:
			bool m_bFlipViewport{ false };

			u64 m_viewportIndex{ ~0ull };
			u64 m_depthTarget{ ~0ull };
			u64 m_stencilTarget{ ~0ull };
			u32 m_layerCount{ 1u };

			stl::vector<FRenderGraphAttachmentInfo> m_vColorAttachments{};
			FRenderGraphAttachmentInfo m_depthAttachment{};
			FRenderGraphAttachmentInfo m_stencilAttachment{};
		};

		class IComputePass : public IBasePass
		{
		public:
			friend class CRenderGraph;
			~IComputePass() = default;

			virtual void init(const FComputePassCreateInfo& pass_ci);
			virtual void build(const FPassBuildInfo& build_info) override;

			void execute(f32 dt) override;

			EPassType type() override { return EPassType::eCompute; }
		protected:
			virtual void _execute(const FComputeFrameInfo& frame_info) = 0;

			virtual void initShaderResources(const std::unique_ptr<IShaderObject>& shader) override;
			virtual void bindShaderResources(const std::unique_ptr<IShaderObject>& shader) override;
		protected:
			FDispatch2Group m_dispatchGroup{};
		};
	}
}