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
		IVec4 position;
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

	class IContext
	{
	public:
		virtual ~IContext() = default;

		virtual void Execute() = 0;
		virtual void Reset() = 0;
	};

	void create_context();
	void DestroyContext();

	IContext* GetCurrentContext();

	void NewFrame();
	void end_frame();

	void DrawPoint2D(IVec2 position, Color color, float size = 1.f);
	void DrawLine2D(IVec2 from, IVec2 to, Color color, float thickness = 1.f);
	void DrawRect2D(IVec2 min, IVec2 max, Color color, float thickness = 1.f);
	void DrawFillRect2D(IVec2 min, IVec2 max, Color color);
	void DrawArc2D(IVec2 center, float radius, float start_angle, float end_angle, Color color, int32_t segments = 36, float thickness = 1.0f);
	void DrawPie2D(IVec2 center, float radius, float start_angle, float end_angle, Color color, int32_t segments = 36);
	void DrawRingArc2D(IVec2 center, float inner_radius, float outer_radius, float start_angle, float end_angle, Color color, int segments = 36);
	void DrawRing2D(IVec2 center, float inner_radius, float outer_radius, Color color, int32_t segments = 36);
	void DrawCircle2D(IVec2 center, float radius, Color color, float thickness = 1.0f);
	void DrawFillCircle2D(IVec2 center, float radius, Color color);
	void DrawTriangle2D(IVec2 p1, IVec2 p2, IVec2 p3, Color color, float thickness = 1.0f);
	void DrawFillTriangle2D(IVec2 p1, IVec2 p2, IVec2 p3, Color color);
	void DrawPolygon2D(std::span<const IVec2> points, Color color, float thickness = 1.0f);
	void DrawFillPolygon2D(std::span<const IVec2> points, Color color);
	void DrawRoundedRect2D(IVec2 min, IVec2 max, float radius, Color color, float thickness = 1.0f);
	void DrawFillRoundedRect2D(IVec2 min, IVec2 max, float radius, Color color);
	void DrawText2D(IVec2 position, std::string_view text, Color color);
}