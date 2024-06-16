#pragma once 

#include <standart_library.h>

#include <glm/glm.hpp>

enum class DebugGeometryType
{
	ePoints,
	eLines,
	eTriangles
};

struct Vertex 
{
	glm::vec3 position;
	glm::vec2 uv;
	u32 color;
};

struct DebugDrawCmd
{
	u32 vertex_offset;
	u32 index_offset;
	u32 index_count;

	glm::vec4 clip_rect;
};

class DebugDraw
{
public:
	static void Init();
	static void Shutdown();

	static void NewFrame();
	static void Render();

	static void DrawPoint(const glm::vec3& position, const glm::vec3& color, float size = 0.5f);
	static void DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float thickness = 0.01f);
	static void DrawBox(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
	static void DrawSphere(const glm::vec3& center, float radius, const glm::vec3& color);
	static void DrawText(const glm::vec3& position, const std::string& text, const glm::vec3& color);

	static const utl::vector<Vertex>& GetVertices();
	static const utl::vector<u16>& GetIndices();
	static const utl::vector<DebugDrawCmd>& GetDrawCommands(DebugGeometryType type);
private:
	static utl::vector<Vertex> vertices;
	static utl::vector<u16> indices;
	static utl::hash_map<DebugGeometryType, utl::vector<DebugDrawCmd>> drawCommands;

	static void AddDrawCmd(u32 vertexOffset, u32 indexOffset, u32 elementCount, DebugGeometryType type, const glm::vec4& clipRect);
	static u32 ColorToUint32(const glm::vec3& color);
	static glm::vec2 GetUVForChar(char c);
	static glm::vec4 CalculateClipRect(const glm::vec3& position, float width, float height);
};