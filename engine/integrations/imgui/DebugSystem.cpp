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

	// Draw point
	//ImDbg::DrawPoint2D({50, 50}, "#FF0001"_color);

	// Draw line
	//ImDbg::DrawLine2D({ 51, 51 }, {101, 101}, "#FFFF00"_color);

	// Draw rects
	//ImDbg::DrawRect2D({ 100, 100 }, { 300, 300 }, "#FFFF00"_color);
	//ImDbg::DrawFillRect2D({ 100, 100 }, { 300, 300 }, "#FFFF00"_color);

	// Circle test
	//ImDbg::DrawCircle2D({ 150, 150 }, 110, "#00FF00"_color);
	//ImDbg::DrawFillCircle2D({ 150, 150 }, 100, "#FF0000"_color);

	// Triangle test
	//ImDbg::DrawTriangle2D(glm::vec2(300, 150), glm::vec2(500, 150), glm::vec2(500, 300), "#00FF3F"_color);
	//ImDbg::DrawFillTriangle2D(glm::vec2(300, 300), glm::vec2(500, 300), glm::vec2(500, 500), "#00FF3F"_color);

	// Polygon test
	//utl::vector<glm::vec2> points{ {300, 300}, {500, 300}, {500, 500}, {300, 500} };
	//ImDbg::DrawPolygon2D(points, "#CCCC0F"_color);
	//ImDbg::DrawFillPolygon2D(points, "#CCCC0F"_color);

	// Rounded rect
	//ImDbg::DrawRoundedRect2D({ 200, 200 }, {400, 400}, 10, "#0F0F0F"_color);
	//ImDbg::DrawFillRoundedRect2D({ 200, 200 }, { 400, 400 }, 10, "#0F0F0F"_color);

	// Drawing text
	//ImDbg::DrawText2D(glm::vec2(150, 150), "Hello, World!", ImDbg::Color::Blue);
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