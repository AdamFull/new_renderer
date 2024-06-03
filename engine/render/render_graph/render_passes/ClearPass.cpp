#include "ClearPass.h"

using namespace engine::render;

CClearPass::CClearPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CClearPass::Init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::Init(pass_ci);

	m_cameraIndex = 0ull;
}

void CClearPass::Build(const FPassBuildInfo& build_info)
{
	IRenderPass::Build(build_info);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = "clear";
#endif
}

void CClearPass::_Render(const FRenderFrameInfo& frame_info)
{

}