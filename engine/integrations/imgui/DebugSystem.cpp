#include "DebugSystem.h"

#include <engine/render/DebugDraw/DebugDraw.h>

using namespace engine;

void DrawRingSegmentProgress(const glm::vec2& center, f32 inner_radius, f32 outer_radius, f32 progress, ImDbgColor color, int segments = 36)
{
    float start_angle = 0.0f;
    float end_angle = progress * 2.0f * glm::pi<float>();
    ImDbg::DrawRingArc2D(center, inner_radius, outer_radius, start_angle, end_angle, color, segments);
}

void DrawPieProgress(const glm::vec2& center, f32 radius, f32 progress, ImDbgColor color, int segments = 36)
{
    float start_angle = 0.0f;
    float end_angle = progress * 2.0f * glm::pi<float>();
    ImDbg::DrawPie2D(center, radius, start_angle, end_angle, color, segments);
}

void DrawComplexShape()
{
    std::vector<glm::vec2> star_points = {
        {300, 100}, {320, 180}, {400, 200}, {320, 220}, {300, 300},
        {280, 220}, {200, 200}, {280, 180}
    };
    ImDbg::DrawPolygon2D(star_points, ImDbg::Color::Blue, 2.0f);
    ImDbg::DrawFillPolygon2D(star_points, ImDbg::Color::Cyan);
}

void DrawProgressBar(const glm::vec2& position, const glm::vec2& size, f32 progress, const std::string& text, ImDbgColor bar_color, ImDbgColor text_color)
{
    // Draw the background
    ImDbg::DrawFillRoundedRect2D(position, position + size, 10.0f, ImDbg::Color::Gray);

    // Draw the progress bar
    glm::vec2 progress_size = glm::vec2(size.x * progress, size.y);
    ImDbg::DrawFillRoundedRect2D(position, position + progress_size, 10.0f, bar_color);

    // Draw the text
    glm::vec2 text_position = position + glm::vec2(size.x / 2, size.y / 2);
    ImDbg::DrawText2D(text_position, text, text_color);
}

void DrawBouncingText(glm::vec2& position, glm::vec2& velocity, const glm::vec2& screen_size, const std::string& text, ImDbgColor color)
{
    // Update position
    position += velocity;

    // Calculate text size
    glm::vec2 text_size = ImDbg::GetTextSize2D(text);

    // Check for screen boundaries and reverse direction if necessary
    if (position.x <= 0 || position.x + text_size.x >= screen_size.x)
    {
        velocity.x = -velocity.x;
        position.x = glm::clamp(position.x, 0.0f, screen_size.x - text_size.x);
    }
    if (position.y <= 0 || position.y + text_size.y >= screen_size.y)
    {
        velocity.y = -velocity.y;
        position.y = glm::clamp(position.y, 0.0f, screen_size.y - text_size.y);
    }

    // Draw the text at the updated position
    ImDbg::DrawText2D(position, text, color);
}

CDebugDrawEngineSystem::CDebugDrawEngineSystem(render::IRenderer& renderer) :
    render_world{ renderer.GetRenderWorld() }
{
}

void CDebugDrawEngineSystem::Initialize()
{
	
}

void CDebugDrawEngineSystem::PreUpdate(f32 dt)
{
    static float progress_increment{ 0.01f };
    static float progress{ 0.f };

    // Bouncing text initial position and velocity
    static glm::vec2 text_position = glm::vec2(400, 300);
    static glm::vec2 text_velocity = glm::vec2(2.0f, 3.0f);
    static std::string bouncing_text = "Bouncing\nText";

	using namespace ImDbg;

	//auto& render_backend = m_pRenderInterface->GetRenderBackend();
	//auto& render_world = m_pRenderInterface->GetRenderWorld();
	//auto resolution = render_backend->GetScreenRenderRect();

	//const auto& camera = render_world->GetDefaultCamera();

	//auto* io = ImDbg::GetIO();
	//io->screenSize = glm::vec2(resolution.size.x, resolution.size.y);
	//io->deltaTime = dt;
	////io->worldToClip = render_world->GetMatrix(camera.matrices_offset + view_projection_matrix_offset);
    //
	//ImDbg::NewFrame();
    //
    //// Draw static shapes
    ////ImDbg::DrawPoint2D(glm::vec2(100, 100), ImDbg::Color::Red, 5.0f);
    //ImDbg::DrawLine2D(glm::vec2(200, 200), glm::vec2(300, 300), ImDbg::Color::Green, 2.0f);
    //ImDbg::DrawRect2D(glm::vec2(350, 350), glm::vec2(450, 450), ImDbg::Color::Blue, 2.0f);
    //ImDbg::DrawCircle2D(glm::vec2(150, 450), 50.0f, ImDbg::Color::Green);
    //ImDbg::DrawFillCircle2D(glm::vec2(250, 450), 50.0f, ImDbg::Color::Blue);
    //ImDbg::DrawTriangle2D(glm::vec2(100, 500), glm::vec2(200, 500), glm::vec2(150, 600), ImDbg::Color::Yellow, 2.0f);
    //ImDbg::DrawFillTriangle2D(glm::vec2(300, 500), glm::vec2(400, 500), glm::vec2(350, 600), ImDbg::Color::Magenta);
    //ImDbg::DrawRoundedRect2D(glm::vec2(50, 50), glm::vec2(150, 150), 20.0f, ImDbg::Color::Purple, 2.0f);
    //ImDbg::DrawFillRoundedRect2D(glm::vec2(200, 50), glm::vec2(300, 150), 20.0f, ImDbg::Color::Lime);
    //ImDbg::DrawArc2D(glm::vec2(350, 200), 50.0f, 0.0f, glm::half_pi<float>(), ImDbg::Color::Red);
    //ImDbg::DrawRing2D(glm::vec2(550, 200), 30.0f, 50.0f, ImDbg::Color::Blue);
    //ImDbg::DrawText2D(glm::vec2(50, 600), "Hello, Debug Draw!", ImDbg::Color::White);
    //
    //// Draw animated ring segment
    //DrawRingSegmentProgress(glm::vec2(200, 300), 50.0f, 70.0f, progress, ImDbg::Color::Green);
    //
    //// Draw animated pie
    //DrawPieProgress(glm::vec2(400, 300), 50.0f, progress, ImDbg::Color::Blue);
    //
    //// Draw progress bars with text
    //DrawProgressBar(glm::vec2(100, 700), glm::vec2(600, 50), progress, "Loading...", ImDbg::Color::Red, ImDbg::Color::White);
    //
    //// Draw complex shape
    //DrawComplexShape();
    //
    //// Draw bouncing text
    //DrawBouncingText(text_position, text_velocity, io->screenSize, bouncing_text, ImDbg::Color::Yellow);
    //
    //progress += progress_increment;
    //if (progress >= 1.0f || progress <= 0.0f)
    //    progress_increment = -progress_increment;
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