#pragma once 

#include <standart_library.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#ifdef DrawText
#undef DrawText
#endif

typedef void* ImDbgHandle;

typedef uint16_t ImDbgIndex;

enum class ImDbgGeometryType
{
	ePoints,
	eLines,
	eTriangles
};

struct ImDbgVertex
{
	glm::vec3 position;
	glm::vec2 uv;
	u32 color;
};

struct ImDbgDrawCmd
{
	u32 vertexOffset;
	u32 indexOffset;
	u32 indexCount;

	ImDbgHandle handle;

	glm::vec4 clipRect;
};

struct ImDbgDrawList
{
	ImDbgGeometryType type;

	utl::vector<ImDbgVertex> vertices;
	utl::vector<ImDbgIndex> indices;

	utl::vector<ImDbgDrawCmd> drawCommands;
};

struct ImDbgDrawData
{
	u32 totalVertexCount;
	u32 totalIndexCount;

	glm::mat4 worldToClipSpace;
	glm::mat4 clipSpaceToScreenSpace;
	glm::mat4 screenSpaceToClipSpace;

	utl::vector<ImDbgDrawList> drawLists;
};

struct ImDbgIO
{
	glm::vec2 screenSize;
};

struct ImDbgContext
{
	ImDbgIO io;
	ImDbgDrawData drawData;
};

namespace ImDbg
{
	void CreateContext();
	void DestroyContext();

	void NewFrame();
	void EndFrame();

	void DrawPoint(const glm::vec3& position, const glm::vec3& color, float size = 0.5f);
	void DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float thickness = 0.01f);
	void DrawBox(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
	void DrawSphere(const glm::vec3& center, float radius, const glm::vec3& color);
	void DrawText(const glm::vec3& position, const std::string& text, const glm::vec3& color);

	ImDbgIO* GetIO();
	ImDbgDrawData* GetDrawData();
}