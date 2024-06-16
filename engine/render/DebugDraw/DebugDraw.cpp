#include "DebugDraw.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Initialize static members
utl::vector<Vertex> DebugDraw::vertices;
utl::vector<uint16_t> DebugDraw::indices;
utl::hash_map<DebugGeometryType, utl::vector<DebugDrawCmd>> DebugDraw::drawCommands;

void DebugDraw::Init()
{

}

void DebugDraw::Shutdown()
{

}

void DebugDraw::NewFrame()
{
    vertices.clear();
    indices.clear();
    drawCommands.clear();
}

void DebugDraw::Render()
{
    
}

void DebugDraw::DrawPoint(const glm::vec3& position, const glm::vec3& color, float size)
{
    DrawLine(position - glm::vec3(size, size, 0), position + glm::vec3(size, size, 0), color, size);
}

void DebugDraw::DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float thickness)
{
    uint32_t vertexOffset = vertices.size();
    uint32_t indexOffset = indices.size();
    uint32_t packedColor = ColorToUint32(color);

    // Calculate the direction and perpendicular vector
    glm::vec3 dir = glm::normalize(to - from);
    glm::vec3 perp = glm::cross(dir, glm::vec3(0, 0, 1)) * thickness * 0.5f; // Cross product to get perpendicular vector

    // Create a thin rectangle (two triangles) to represent the line
    vertices.push_back(Vertex{ from - perp, glm::vec2(0.0f), packedColor });
    vertices.push_back(Vertex{ from + perp, glm::vec2(0.0f), packedColor });
    vertices.push_back(Vertex{ to + perp, glm::vec2(0.0f), packedColor });
    vertices.push_back(Vertex{ to - perp, glm::vec2(0.0f), packedColor });

    indices.push_back(vertexOffset);
    indices.push_back(vertexOffset + 1);
    indices.push_back(vertexOffset + 2);
    indices.push_back(vertexOffset);
    indices.push_back(vertexOffset + 2);
    indices.push_back(vertexOffset + 3);

    glm::vec4 clipRect = CalculateClipRect(from, glm::length(to - from), thickness);
    AddDrawCmd(vertexOffset, indexOffset, 6, DebugGeometryType::eTriangles, clipRect);
}

void DebugDraw::DrawBox(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color)
{
    glm::vec3 verticesPos[8] = {
        { min.x, min.y, min.z },
        { max.x, min.y, min.z },
        { max.x, max.y, min.z },
        { min.x, max.y, min.z },
        { min.x, min.y, max.z },
        { max.x, min.y, max.z },
        { max.x, max.y, max.z },
        { min.x, max.y, max.z }
    };

    // Bottom face
    DrawLine(verticesPos[0], verticesPos[1], color);
    DrawLine(verticesPos[1], verticesPos[2], color);
    DrawLine(verticesPos[2], verticesPos[3], color);
    DrawLine(verticesPos[3], verticesPos[0], color);

    // Top face
    DrawLine(verticesPos[4], verticesPos[5], color);
    DrawLine(verticesPos[5], verticesPos[6], color);
    DrawLine(verticesPos[6], verticesPos[7], color);
    DrawLine(verticesPos[7], verticesPos[4], color);

    // Vertical edges
    DrawLine(verticesPos[0], verticesPos[4], color);
    DrawLine(verticesPos[1], verticesPos[5], color);
    DrawLine(verticesPos[2], verticesPos[6], color);
    DrawLine(verticesPos[3], verticesPos[7], color);
}

void DebugDraw::DrawSphere(const glm::vec3& center, float radius, const glm::vec3& color)
{
    const int segments = 16;
    glm::vec3 axis[] = 
    {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };

    for (int i = 0; i < 3; ++i) 
    {
        uint32_t vertex_offset = vertices.size();
        uint32_t index_offset = indices.size();

        for (int j = 0; j < segments; ++j) 
        {
            float theta1 = j * glm::two_pi<float>() / segments;
            float theta2 = (j + 1) * glm::two_pi<float>() / segments;

            glm::vec3 p1 = center + radius * (glm::cos(theta1) * axis[i] + glm::sin(theta1) * axis[(i + 1) % 3]);
            glm::vec3 p2 = center + radius * (glm::cos(theta2) * axis[i] + glm::sin(theta2) * axis[(i + 1) % 3]);

            vertices.emplace_back(Vertex{ p1, glm::vec2(0.0f), ColorToUint32(color) });
            vertices.emplace_back(Vertex{ p2, glm::vec2(0.0f), ColorToUint32(color) });

            indices.push_back(vertex_offset + j * 2);
            indices.push_back(vertex_offset + j * 2 + 1);
        }

        const glm::vec4 clip_rect = CalculateClipRect(center, radius * 2, radius * 2);
        AddDrawCmd(vertex_offset, index_offset, segments * 2, DebugGeometryType::eLines, clip_rect);
    }
}

void DebugDraw::DrawText(const glm::vec3& position, const std::string& text, const glm::vec3& color)
{
    uint32_t vertex_offset = vertices.size();
    uint32_t index_offset = indices.size();
    uint32_t packed_color = ColorToUint32(color);

    float x = position.x;
    float y = position.y;

    for (char c : text) {
        glm::vec2 uv = GetUVForChar(c);

        vertices.emplace_back(Vertex{ glm::vec3(x, y, position.z), uv, packed_color });
        vertices.emplace_back(Vertex{ glm::vec3(x + 1, y, position.z), uv, packed_color });
        vertices.emplace_back(Vertex{ glm::vec3(x + 1, y + 1, position.z), uv, packed_color });
        vertices.emplace_back(Vertex{ glm::vec3(x, y + 1, position.z), uv, packed_color });

        indices.emplace_back(vertex_offset);
        indices.emplace_back(vertex_offset + 1);
        indices.emplace_back(vertex_offset + 2);
        indices.emplace_back(vertex_offset + 2);
        indices.emplace_back(vertex_offset + 3);
        indices.emplace_back(vertex_offset);

        vertex_offset += 4;
        index_offset += 6;

        x += 1.0f; // Move to the next character position
    }

    const glm::vec4 clip_rect = CalculateClipRect(position, text.size(), 1.0f);
    AddDrawCmd(vertex_offset, index_offset, text.size() * 6, DebugGeometryType::eTriangles, clip_rect);
}


const utl::vector<Vertex>& DebugDraw::GetVertices()
{
    return vertices;
}

const utl::vector<u16>& DebugDraw::GetIndices()
{
    return indices;
}

const utl::vector<DebugDrawCmd>& DebugDraw::GetDrawCommands(DebugGeometryType type)
{
    static utl::vector<DebugDrawCmd> empty{};
    if (auto found = drawCommands.find(type); found != drawCommands.end())
        return found->second;

    return empty;
}


void DebugDraw::AddDrawCmd(u32 vertex_offset, u32 index_offset, u32 elementCount, DebugGeometryType type, const glm::vec4& clipRect)
{
    drawCommands[type].emplace_back(DebugDrawCmd{vertex_offset, index_offset, elementCount, clipRect});
}

u32 DebugDraw::ColorToUint32(const glm::vec3& color)
{
    uint8_t r = static_cast<uint8_t>(glm::clamp(color.z, 0.0f, 1.0f) * 255.0f);
    uint8_t g = static_cast<uint8_t>(glm::clamp(color.y, 0.0f, 1.0f) * 255.0f);
    uint8_t b = static_cast<uint8_t>(glm::clamp(color.x, 0.0f, 1.0f) * 255.0f);
    return (r << 16) | (g << 8) | b;
}

glm::vec2 DebugDraw::GetUVForChar(char c) 
{
    // Dummy implementation, replace with actual UV lookup based on your font texture atlas
    // For example purposes, we'll map 'a' to 'z' in a 16x16 grid
    int index = c - 'a'; // Assuming 'a' to 'z'
    int col = index % 16;
    int row = index / 16;
    float u = col / 16.0f;
    float v = row / 16.0f;
    return glm::vec2(u, v);
}

glm::vec4 DebugDraw::CalculateClipRect(const glm::vec3& position, float width, float height)
{
    return glm::vec4(position.x, position.y, position.x + width, position.y + height);
}