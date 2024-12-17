#pragma once

#include <vector>
#include <string_view>
#include <span>

#include "imdbg_math.h"

namespace ImDbg
{
	using Index = uint16_t;
	using Color = uint32_t;

	struct Vertex
	{
		Vec4 position;
		UVec2 uv;
		Color color;
	};

	enum class CommandType
	{
		Point2D,
		Line2D,

		Point3D,
		Line3D
	};

	void CreateContext();
	void DestroyContext();

	void NewFrame();
	void EndFrame();

	void DrawPoint2D(Vec2 position, Color color, float size = 1.f);
	void DrawLine2D(Vec2 from, Vec2 to, Color color, float thickness = 1.f);
	void DrawRect2D(Vec2 min, Vec2 max, Color color, float thickness = 1.f);
	void DrawFillRect2D(Vec2 min, Vec2 max, Color color);
	void DrawArc2D(Vec2 center, float radius, float start_angle, float end_angle, Color color, int32_t segments = 36, float thickness = 1.0f);
	void DrawPie2D(Vec2 center, float radius, float start_angle, float end_angle, Color color, int32_t segments = 36);
	void DrawRingArc2D(Vec2 center, float inner_radius, float outer_radius, float start_angle, float end_angle, Color color, int segments = 36);
	void DrawRing2D(Vec2 center, float inner_radius, float outer_radius, Color color, int32_t segments = 36);
	void DrawCircle2D(Vec2 center, float radius, Color color, float thickness = 1.0f);
	void DrawFillCircle2D(Vec2 center, float radius, Color color);
	void DrawTriangle2D(Vec2 p1, Vec2 p2, Vec2 p3, Color color, float thickness = 1.0f);
	void DrawFillTriangle2D(Vec2 p1, Vec2 p2, Vec2 p3, Color color);
	void DrawPolygon2D(std::span<const Vec2> points, Color color, float thickness = 1.0f);
	void DrawFillPolygon2D(std::span<const Vec2> points, Color color);
	void DrawRoundedRect2D(Vec2 min, Vec2 max, float radius, Color color, float thickness = 1.0f);
	void DrawFillRoundedRect2D(Vec2 min, Vec2 max, float radius, Color color);
	void DrawText2D(Vec2 position, std::string_view text, Color color);
}