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

			virtual void execute(f32 dt) = 0;

			virtual EPassType type() = 0;

		protected:
			EQueueTypeFlagBits m_queue{};

			ERenderingSizeType m_sizePolicy{};
			u64 m_sizeTarget{ ~0ull };

			std::unordered_map<ERenderResourceGroupType, std::vector<u64>> m_renderResources{};
			std::unordered_map<std::string, u64> m_bakedBindings{};

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
			virtual void _render(const FRenderFrameInfo& frame_info) = 0;
		protected:
			bool m_bFlipViewport{ false };
			u64 m_depthTarget{ ~0ull };
			u64 m_stencilTarget{ ~0ull };
			u32 m_layerCount{ 1u };

			std::vector<FRenderGraphAttachmentInfo> m_vColorAttachments{};
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
		protected:
			FDispatch2Group m_dispatchGroup{};
		};
	}
}