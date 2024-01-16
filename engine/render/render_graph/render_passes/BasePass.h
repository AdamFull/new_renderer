#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		struct FBaseFrameInfo
		{
			float deltaTime{};
		};

		struct FRenderFrameInfo : public FBaseFrameInfo
		{
			FRect2D<float> renderRect{};
		};

		struct FComputeFrameInfo : public FBaseFrameInfo
		{
		};

		class IBasePass
		{
		public:
			friend class CRenderGraph;
			virtual ~IBasePass() = default;

			virtual void bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info) = 0;

			virtual void execute(float dt) = 0;

		protected:
			EQueueTypeFlagBits m_queue{};

			ERenderingSizeType m_sizePolicy{};
			size_t m_sizeTarget{ ~0ull };

			std::unordered_map<ERenderResourceGroupType, std::vector<size_t>> m_renderResources{};
			std::unordered_map<std::string, size_t> m_bakedBindings{};

#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string m_debugName{};
#endif

			IRenderInterface* m_pRenderInterface{ nullptr };
		};

		class IRenderPass : public IBasePass
		{
		public:
			~IRenderPass() = default;

			virtual void init(const FRenderPassCreateInfo& pass_ci);
			virtual void bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info);

			void execute(float dt) override;
		protected:
			virtual void _render(const FRenderFrameInfo& frame_info) = 0;
		protected:
			bool m_bFlipViewport{ false };
			size_t m_depthTarget{ ~0ull };
			size_t m_stencilTarget{ ~0ull };
			uint32_t m_layerCount{ 1u };

			std::vector<FRenderGraphAttachmentInfo> m_vColorAttachments{};
			FRenderGraphAttachmentInfo m_depthAttachment{};
			FRenderGraphAttachmentInfo m_stencilAttachment{};
		};

		class IComputePass : public IBasePass
		{
		public:
			~IComputePass() = default;

			virtual void init(const FComputePassCreateInfo& pass_ci);
			virtual void bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info);

			void execute(float dt) override;

		protected:
			virtual void _execute(const FComputeFrameInfo& frame_info) = 0;
		protected:
			FDispatch2Group m_dispatchGroup{};
		};
	}
}