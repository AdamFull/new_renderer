#include "RenderCamera.h"

using namespace engine::render;

CRenderCamera::CRenderCamera(IRenderInterface* render_interface)
{
	m_pRenderWorld = render_interface->GetRenderWorld().get();
}

void CRenderCamera::Create()
{
	auto& camera_buffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eCameraBuffer);
	auto& frustum_buffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eFrustumBuffer);
	auto& matrices_buffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eMatricesBuffer);

	m_cameraId = camera_buffer->AddByteBlock();

	m_matricesId = matrices_buffer->AddByteBlock(matrix_offset_count);
}