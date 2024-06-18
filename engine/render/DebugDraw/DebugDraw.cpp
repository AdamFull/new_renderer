#include "DebugDraw.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

static ImDbgContext* context{ nullptr };

namespace ImDbg
{
    u32 ColorToUint32(const glm::vec3& color);
    glm::vec2 GetUVForChar(char c);
    glm::vec4 CalculateClipRect(const glm::vec3& min, const glm::vec3& max);

    void CreateContext()
    {
        static constexpr const std::array<ImDbgGeometryType, 3ull> geometry_types{ ImDbgGeometryType::ePoints, ImDbgGeometryType::eLines, ImDbgGeometryType::eTriangles };

        context = new ImDbgContext();

        for (const auto& type : geometry_types)
        {
            auto& new_list = context->drawData.drawLists.emplace_back();
            new_list.type = type;
        }
    }

    void DestroyContext()
    {
        delete context;
        context = nullptr;
    }

    void NewFrame()
    {
        for (auto& list : context->drawData.drawLists)
        {
            list.vertices.clear();
            list.indices.clear();
            list.drawCommands.clear();
        }

        context->drawData.totalIndexCount = 0;
        context->drawData.totalVertexCount = 0;

        context->drawData.clipSpaceToScreenSpace = 
            glm::mat4(
            context->io.screenSize.x / 2.0f, 0.0f,                             0.0f, context->io.screenSize.x / 2.0f,
            0.0f,                            -context->io.screenSize.y / 2.0f, 0.0f, context->io.screenSize.y / 2.0f,
            0.0f,                            0.0f,                             1.0f, 0.0f,
            0.0f,                            0.0f,                             0.0f, 1.0f
        );

        context->drawData.screenSpaceToClipSpace = glm::inverse(context->drawData.clipSpaceToScreenSpace);
    }

    void EndFrame()
    {

    }

    void DrawPoint(const glm::vec3& position, const glm::vec3& color, float size)
    {
        auto& draw_list = context->drawData.drawLists[(u32)ImDbgGeometryType::ePoints];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 1;
        new_draw_cmd.clipRect = CalculateClipRect(position, position);

        draw_list.vertices.emplace_back(ImDbgVertex{ position, glm::vec2(0.0f), ColorToUint32(color) });
        draw_list.indices.emplace_back(new_draw_cmd.vertexOffset);

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float thickness)
    {
        auto& draw_list = context->drawData.drawLists[(u32)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 2;
        new_draw_cmd.clipRect = CalculateClipRect(from, to);

        draw_list.vertices.emplace_back(ImDbgVertex{ from, glm::vec2(0.0f), ColorToUint32(color) });
        draw_list.vertices.emplace_back(ImDbgVertex{ to, glm::vec2(0.0f), ColorToUint32(color) });
        draw_list.indices.emplace_back(new_draw_cmd.vertexOffset);
        draw_list.indices.emplace_back(new_draw_cmd.vertexOffset + 1);

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawBox(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color)
    {
        const glm::vec3 verticesPos[8] = 
        {
            { min.x, min.y, min.z },
            { max.x, min.y, min.z },
            { max.x, max.y, min.z },
            { min.x, max.y, min.z },
            { min.x, min.y, max.z },
            { max.x, min.y, max.z },
            { max.x, max.y, max.z },
            { min.x, max.y, max.z }
        };

        static const u32 boxIndices[] =
        {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        auto& draw_list = context->drawData.drawLists[(u32)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 24;
        new_draw_cmd.clipRect = CalculateClipRect(min, max);

        u32 packedColor = ColorToUint32(color);

        for (const auto& pos : verticesPos)
            draw_list.vertices.emplace_back(ImDbgVertex{ pos, glm::vec2(0.0f), packedColor });

        for (const auto& idx : boxIndices)
            draw_list.indices.emplace_back(new_draw_cmd.vertexOffset + idx);

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawSphere(const glm::vec3& center, float radius, const glm::vec3& color)
    {
        const int segments = 16;
        static const glm::vec3 axis[] =
        {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f}
        };

        auto& draw_list = context->drawData.drawLists[(u32)ImDbgGeometryType::eLines];

        for (int i = 0; i < 3; ++i)
        {
            auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
            new_draw_cmd.vertexOffset = draw_list.vertices.size();
            new_draw_cmd.indexOffset = draw_list.indices.size();
            new_draw_cmd.indexCount = segments * 2;
            new_draw_cmd.clipRect = CalculateClipRect(center - radius, center + radius);

            for (int j = 0; j < segments; ++j)
            {
                float theta1 = j * glm::two_pi<float>() / segments;
                float theta2 = (j + 1) * glm::two_pi<float>() / segments;

                glm::vec3 p1 = center + radius * (glm::cos(theta1) * axis[i] + glm::sin(theta1) * axis[(i + 1) % 3]);
                glm::vec3 p2 = center + radius * (glm::cos(theta2) * axis[i] + glm::sin(theta2) * axis[(i + 1) % 3]);

                draw_list.vertices.emplace_back(ImDbgVertex{ p1, glm::vec2(0.0f), ColorToUint32(color) });
                draw_list.vertices.emplace_back(ImDbgVertex{ p2, glm::vec2(0.0f), ColorToUint32(color) });

                draw_list.indices.emplace_back(new_draw_cmd.vertexOffset + j * 2);
                draw_list.indices.emplace_back(new_draw_cmd.vertexOffset + j * 2 + 1);
            }

            context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
            context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
        }
    }

    void DrawText(const glm::vec3& position, const std::string& text, const glm::vec3& color)
    {
        auto& draw_list = context->drawData.drawLists[(u32)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        u32 vertex_offset = new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = text.size() * 6;
        new_draw_cmd.clipRect = CalculateClipRect(position, position + (f32)text.size());

        float x = position.x;
        float y = position.y;

        uint32_t packed_color = ColorToUint32(color);

        for (char c : text)
        {
            glm::vec2 uv = GetUVForChar(c);

            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(x, y, position.z), uv, packed_color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(x + 1, y, position.z), uv, packed_color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(x + 1, y + 1, position.z), uv, packed_color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(x, y + 1, position.z), uv, packed_color });

            draw_list.indices.emplace_back(vertex_offset);
            draw_list.indices.emplace_back(vertex_offset + 1);
            draw_list.indices.emplace_back(vertex_offset + 2);
            draw_list.indices.emplace_back(vertex_offset + 2);
            draw_list.indices.emplace_back(vertex_offset + 3);
            draw_list.indices.emplace_back(vertex_offset);

            vertex_offset += 4;

            x += 1.0f; // Move to the next character position
        }

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    ImDbgIO* GetIO()
    {
        return &context->io;
    }


    ImDbgDrawData* GetDrawData()
    {
        return &context->drawData;
    }

    // Other functions
    u32 ColorToUint32(const glm::vec3& color)
    {
        uint8_t r = static_cast<uint8_t>(glm::clamp(color.z, 0.0f, 1.0f) * 255.0f);
        uint8_t g = static_cast<uint8_t>(glm::clamp(color.y, 0.0f, 1.0f) * 255.0f);
        uint8_t b = static_cast<uint8_t>(glm::clamp(color.x, 0.0f, 1.0f) * 255.0f);
        return (r << 16) | (g << 8) | b;
    }

    glm::vec2 GetUVForChar(char c)
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

    glm::vec4 CalculateClipRect(const glm::vec3& min, const glm::vec3& max)
    {
        auto cs_min = context->drawData.worldToClipSpace * glm::vec4(min, 1.f);
        auto ndc_min = cs_min / cs_min.w;

        auto cs_max = context->drawData.worldToClipSpace * glm::vec4(max, 1.f);
        auto ndc_max = cs_max / cs_max.w;

        glm::vec2 ss_min;
        glm::vec2 ss_max;

        ss_min.x = (ndc_min.x * 0.5f + 0.5f) * context->io.screenSize.x;
        ss_max.x = (ndc_max.x * 0.5f + 0.5f) * context->io.screenSize.x;
        ss_min.y = (1.0f - (ndc_min.y * 0.5f + 0.5f)) * context->io.screenSize.y;
        ss_max.y = (1.0f - (ndc_max.y * 0.5f + 0.5f)) * context->io.screenSize.y;

        // Return the clip rect in screen space coordinates
        return glm::vec4(ss_min.x, ss_min.y, ss_max.x, ss_max.y);
    }
}