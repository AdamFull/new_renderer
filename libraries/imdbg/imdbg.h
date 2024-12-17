#pragma once

#include <vector>
#include <string_view>
#include <span>

using ImDbgIndex = uint16_t;
using ImDbgColor = uint32_t;

struct ImDbgVec2 { int16_t x, y; };
struct ImDbgVec3 { int16_t x, y, z; };
struct ImDbgVec4 { int16_t x, y, z, w; };

struct ImDbgUVec2 { uint16_t x, y; };
struct ImDbgUVec3 { uint16_t x, y, z; };
struct ImDbgUVec4 { uint16_t x, y, z, w; };

struct ImDbgVertex
{
	ImDbgVec4 position;
	ImDbgUVec2 uv;
	ImDbgColor color;
};

namespace ImDbg
{
	enum class CommandType
	{
		Point2D,
		Line2D,

		Point3D,
		Line3D
	};

	struct Command
	{
		CommandType type;
	};

	struct CommandList
	{
		std::vector<Command> commands;
	};

	void CreateContext();
	void DestroyContext();

	void NewFrame();
	void EndFrame();

	void DrawPoint2D(ImDbgVec2 position, ImDbgColor color, float size = 1.f);
	void DrawLine2D(ImDbgVec2 from, ImDbgVec2 to, ImDbgColor color, float thickness = 1.f);
	void DrawRect2D(ImDbgVec2 min, ImDbgVec2 max, ImDbgColor color, float thickness = 1.f);
	void DrawFillRect2D(ImDbgVec2 min, ImDbgVec2 max, ImDbgColor color);
	void DrawArc2D(ImDbgVec2 center, float radius, float start_angle, float end_angle, ImDbgColor color, int32_t segments = 36, float thickness = 1.0f);
	void DrawPie2D(ImDbgVec2 center, float radius, float start_angle, float end_angle, ImDbgColor color, int32_t segments = 36);
	void DrawRingArc2D(ImDbgVec2 center, float inner_radius, float outer_radius, float start_angle, float end_angle, ImDbgColor color, int segments = 36);
	void DrawRing2D(ImDbgVec2 center, float inner_radius, float outer_radius, ImDbgColor color, i32 segments = 36);
	void DrawCircle2D(ImDbgVec2 center, float radius, ImDbgColor color, float thickness = 1.0f);
	void DrawFillCircle2D(ImDbgVec2 center, float radius, ImDbgColor color);
	void DrawTriangle2D(ImDbgVec2 p1, ImDbgVec2 p2, ImDbgVec2 p3, ImDbgColor color, float thickness = 1.0f);
	void DrawFillTriangle2D(ImDbgVec2 p1, ImDbgVec2 p2, ImDbgVec2 p3, ImDbgColor color);
	void DrawPolygon2D(std::span<const ImDbgVec2> points, ImDbgColor color, float thickness = 1.0f);
	void DrawFillPolygon2D(std::span<const ImDbgVec2> points, ImDbgColor color);
	void DrawRoundedRect2D(ImDbgVec2 min, ImDbgVec2 max, float radius, ImDbgColor color, float thickness = 1.0f);
	void DrawFillRoundedRect2D(ImDbgVec2 min, ImDbgVec2 max, float radius, ImDbgColor color);
	void DrawText2D(ImDbgVec2 position, std::string_view text, ImDbgColor color);
}