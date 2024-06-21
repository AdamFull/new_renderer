#pragma once 

#include <standart_library.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#ifdef DrawText
#undef DrawText
#endif

typedef void* ImDbgHandle;

typedef uint16_t ImDbgIndex;
typedef uint32_t ImDbgColor;

// Common 
enum class ImDbgGeometryType
{
	ePoints,
	eLines,
	eTriangles
};

struct ImDbgDrawCmd
{
	u32 vertexOffset;
	u32 indexOffset;
	u32 indexCount;

	ImDbgHandle handle;

	glm::vec4 clipRect;
};

template<class _Vertex>
struct ImDbgDrawList
{
	ImDbgGeometryType type;

	utl::vector<_Vertex> vertices;
	utl::vector<ImDbgIndex> indices;

	utl::vector<ImDbgDrawCmd> drawCommands;
};

// For 2d drawings
struct ImDbgVertex2D
{
	glm::vec2 position;
	glm::vec2 uv;
	u32 color;
};

// For 3d drawings
struct ImDbgVertex3D
{
	glm::vec3 position;
	glm::vec2 uv;
	u32 color;
};

template<class _Vertex>
struct ImDbgDrawData
{
	u32 totalVertexCount;
	u32 totalIndexCount;
	utl::vector<ImDbgDrawList<_Vertex>> drawLists;
};


struct ImDbgIO
{
	glm::vec2 screenSize{};
	f32 deltaTime{};
	glm::mat4 worldToClip{};
};

struct ImDbgContext
{
	ImDbgIO io;
	ImDbgDrawData<ImDbgVertex2D> drawData2d;
	ImDbgDrawData<ImDbgVertex3D> drawData3d;
	struct FontImpl* font;
};

namespace ImDbg
{
	constexpr uint8_t hex_to_value(char c) 
	{
		return (c >= '0' && c <= '9') ? (c - '0') :
			(c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
			(c >= 'a' && c <= 'f') ? (c - 'a' + 10) : 0;
	}

	constexpr uint32_t hex_to_rgba(const char* hex) {
		return (hex[0] == '#') ? (
			(hex[1] ? (hex_to_value(hex[1]) << 28) : 0) |
			(hex[2] ? (hex_to_value(hex[2]) << 24) : 0) |
			(hex[3] ? (hex_to_value(hex[3]) << 20) : 0) |
			(hex[4] ? (hex_to_value(hex[4]) << 16) : 0) |
			(hex[5] ? (hex_to_value(hex[5]) << 12) : 0) |
			(hex[6] ? (hex_to_value(hex[6]) << 8) : 0) |
			(hex[7] ? (hex_to_value(hex[7]) << 4) : 0) |
			(hex[8] ? hex_to_value(hex[8]) : 0)
			) | 0xFF : 0;
	}

	constexpr uint32_t operator "" _color(const char* hex, std::size_t) { return hex_to_rgba(hex); }

	namespace Color
	{
		constexpr const ImDbgColor Red			= "#FF000000"_color;
		constexpr const ImDbgColor Green		= "#00FF0000"_color;
		constexpr const ImDbgColor Blue			= "#0000FF00"_color;
	}

	void CreateContext();
	void DestroyContext();

	void LoadFont(const u8* data, u32 data_size, u32 character_size);
	void GetTexDataAsRGBA32(u32 index, u8** out_pixels, i32* out_width, i32* out_height, i32* out_bytes_per_pixel);

	void NewFrame();
	void EndFrame();

	// 2d drawings
	void DrawPoint2D(const glm::vec2& position, ImDbgColor color, f32 size = 1.0f);
	void DrawLine2D(const glm::vec2& from, const glm::vec2& to, ImDbgColor color, f32 thickness = 1.0f);
	void DrawRect2D(const glm::vec2& min, const glm::vec2& max, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillRect2D(const glm::vec2& min, const glm::vec2& max, ImDbgColor color);
	void DrawCircle2D(const glm::vec2& center, f32 radius, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillCircle2D(const glm::vec2& center, f32 radius, ImDbgColor color);
	void DrawTriangle2D(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillTriangle2D(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, ImDbgColor color);
	void DrawPolygon2D(const utl::vector<glm::vec2>& points, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillPolygon2D(const utl::vector<glm::vec2>& points, ImDbgColor color);
	void DrawRoundedRect2D(const glm::vec2& min, const glm::vec2& max, f32 radius, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillRoundedRect2D(const glm::vec2& min, const glm::vec2& max, f32 radius, ImDbgColor color);
	void DrawText2D(const glm::vec2& position, const std::string& text, ImDbgColor color);

	// 3d drawings
	void DrawPoint3D(const glm::vec3& position, const glm::vec3& color, float size = 0.5f);
	void DrawLine3D(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float thickness = 0.01f);
	void DrawBox3D(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
	void DrawSphere3D(const glm::vec3& center, float radius, const glm::vec3& color);
	void DrawText3D(const glm::vec3& position, const std::string& text, const glm::vec3& color);

	ImDbgIO* GetIO();
	ImDbgDrawData<ImDbgVertex2D>* GetDrawData2D();
	ImDbgDrawData<ImDbgVertex3D>* GetDrawData3D();
}