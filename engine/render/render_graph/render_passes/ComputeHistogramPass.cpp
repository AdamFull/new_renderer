#include "ComputeCubemapPass.h"

using namespace engine::render;

CComputeHistogramPass::CComputeHistogramPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CComputeHistogramPass::init(const FComputePassCreateInfo& pass_ci)
{
	IComputePass::init(pass_ci);
}

void CComputeHistogramPass::build(const FPassBuildInfo& build_info) 
{
	IComputePass::build(build_info);
}

void CComputeHistogramPass::_execute(const FComputeFrameInfo& frame_info)
{

}