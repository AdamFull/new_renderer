#include "DebugSystem.h"

#include <engine/platform/window/WindowCallback.h>
#include <engine/render/DebugDraw/DebugDraw.h>

using namespace engine;
using namespace engine::render;

CDebugDrawEngineSystem::CDebugDrawEngineSystem(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CDebugDrawEngineSystem::Initialize()
{
	ImDbg::CreateContext();
}

void CDebugDrawEngineSystem::PreUpdate(f32 dt)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto resolution = render_context->GetScreenRenderRect();

	auto* io = ImDbg::GetIO();
	io->screenSize = glm::vec2(resolution.size.x, resolution.size.y);
	io->deltaTime = dt;

	ImDbg::NewFrame();

	//ImDbg::DrawLine(glm::vec3(0.f), glm::vec3(20.5f, 50.5f, 0.f), glm::vec3(1.f, 0.f, 0.f));
}

void CDebugDrawEngineSystem::Update(f32 dt)
{
	ImDbg::EndFrame();
}

void CDebugDrawEngineSystem::Render()
{
}

void CDebugDrawEngineSystem::Destroy()
{
	ImDbg::DestroyContext();
}