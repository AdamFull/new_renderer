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
			virtual void bindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt);

			EQueueTypeFlagBits m_queue{};

			utl::hash_map<ERenderResourceGroupType, utl::vector<FRenderResourceBinding>> m_renderResources{};
			utl::hash_map<std::string, FRenderResourceBinding> m_bakedBindings{};

			bool m_bOneTime{ false };
			bool m_bCanExecute{ true };

#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string m_debugName{};
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
			virtual void bindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt) override;
		protected:
			bool m_bFlipViewport{ false };

			u64 m_viewportIndex{ ~0ull };
			u64 m_depthTarget{ ~0ull };
			u64 m_stencilTarget{ ~0ull };
			u32 m_layerCount{ 1u };

			utl::vector<FRenderGraphAttachmentInfo> m_vColorAttachments{};
			utl::hash_map<u64, FClearValues> m_clearValues;
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
			virtual void bindShaderResources(const std::unique_ptr<IShaderObject>& shader, f32 dt) override;
		protected:
			utl::vector<FDispatch2Info> m_vDispatchGroups{};
			utl::vector<FComputeDispatchGroupInfo> m_vDispatchInfos{};
		};
	}
}