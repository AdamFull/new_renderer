#include "GraphNodes.h"

using namespace engine::render;

CDynamicConditionNode::CDynamicConditionNode(utl::reactive::function<u64()>&& condition, std::unique_ptr<IRenderGraphNode>&& pass, std::unique_ptr<IRenderGraphNode>&& fail)
{
	m_fnChechCondition = std::move(condition);
	m_pPass = std::move(pass);
	m_pFail = std::move(fail);
}

void CDynamicConditionNode::Traverse(utl::vector<u64>& passes)
{
	if (m_fnChechCondition() && m_pPass)
		m_pPass->Traverse(passes);
	else
	{
		if(m_pFail)
			m_pFail->Traverse(passes);
	}
}


CGroupNode::CGroupNode(utl::vector<std::unique_ptr<IRenderGraphNode>>&& nodes, const std::string& name, u32 color)
{
	m_vNodes = std::move(nodes);
}

void CGroupNode::Traverse(utl::vector<u64>& passes)
{
	// TODO: Begin zone
	for (auto& node : m_vNodes)
		node->Traverse(passes);
	// TODO: End zone
}


CRenderNode::CRenderNode(u64 pass_index)
{
	m_renderPass = pass_index;
}

void CRenderNode::Traverse(utl::vector<u64>& passes)
{
	passes.emplace_back(m_renderPass);
}


CStaticConditionNode::CStaticConditionNode(std::unique_ptr<IRenderGraphNode>&& node)
{
	m_pNode = std::move(node);
}

void CStaticConditionNode::Traverse(utl::vector<u64>& passes)
{
	// It's static condition, just call traverse
	m_pNode->Traverse(passes);
}