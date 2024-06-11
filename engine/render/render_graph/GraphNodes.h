#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <reactive/function.h>

namespace engine
{
	namespace render
	{
		class CDynamicConditionNode : public IRenderGraphNode
		{
		public:
			CDynamicConditionNode(utl::reactive::function<u64()>&& condition, std::unique_ptr<IRenderGraphNode>&& pass, std::unique_ptr<IRenderGraphNode>&& fail);
			~CDynamicConditionNode() = default;
			void Traverse(utl::vector<u64>& passes) override;
			friend class CRenderGraph;
		private:
			utl::reactive::function<u64()> m_fnChechCondition{};
			std::unique_ptr<IRenderGraphNode> m_pPass{};
			std::unique_ptr<IRenderGraphNode> m_pFail{};
		};
		
		class CGroupNode : public IRenderGraphNode
		{
		public:
			CGroupNode(utl::vector<std::unique_ptr<IRenderGraphNode>>&& nodes, const std::string& name, u32 color);
			~CGroupNode() = default;
			void Traverse(utl::vector<u64>& passes) override;
			friend class CRenderGraph;
		private:
			utl::vector<std::unique_ptr<IRenderGraphNode>> m_vNodes{};
		};
		
		class CRenderNode : public IRenderGraphNode
		{
		public:
			CRenderNode(u64 pass_index);
			~CRenderNode() = default;
			void Traverse(utl::vector<u64>& passes) override;
			friend class CRenderGraph;
		private:
			u64 m_renderPass{ ~0ull };
		};
		
		class CStaticConditionNode : public IRenderGraphNode
		{
		public:
			CStaticConditionNode(std::unique_ptr<IRenderGraphNode>&& node);
			~CStaticConditionNode() = default;
			void Traverse(utl::vector<u64>& passes) override;
			friend class CRenderGraph;
		private:
			std::unique_ptr<IRenderGraphNode> m_pNode{};
		};
	}
}