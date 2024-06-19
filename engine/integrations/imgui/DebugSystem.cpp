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
	
}

void CDebugDrawEngineSystem::PreUpdate(f32 dt)
{
	using namespace ImDbg;

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_world = m_pRenderInterface->GetRenderWorld();
	auto resolution = render_context->GetScreenRenderRect();

	const auto& camera = render_world->GetDefaultCamera();

	auto* io = ImDbg::GetIO();
	io->screenSize = glm::vec2(resolution.size.x, resolution.size.y);
	io->deltaTime = dt;
	io->worldToClip = render_world->GetMatrix(camera.matricesOffset + view_projection_matrix_offset);

	ImDbg::NewFrame();
	
	// Drawing a box
	//ImDbg::DrawBox2D(glm::vec2(50, 50), glm::vec2(200, 200), ImDbg::Color::Red, 1.0f);

	// Drawing a line
	ImDbg::DrawLine2D(glm::vec2(500, 100), glm::vec2(300, 300), ImDbg::Color::Green, 1.0f);

	ImDbg::DrawCircle2D(glm::vec2(500, 500), 500, "#00FF00"_color);

	// Drawing text
	ImDbg::DrawText2D(glm::vec2(150, 150), "Hello, World!", ImDbg::Color::Blue);
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
	
}