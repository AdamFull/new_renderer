#include "DebugDraw.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "monoisome_regular.ttf.h"

static ImDbgContext* context{ nullptr };

struct FontAtlas
{
    FT_Face face;

    u8* texture;
    u32 width;
    u32 height;
};

struct FontImpl
{
    FT_Library ft;
    utl::vector<FontAtlas> atlases;
};

namespace ImDbg
{
    u32 ColorToUint32(const glm::vec3& color);
    glm::vec2 GetUVForChar(char c);
    glm::vec4 CalculateClipRect(const glm::vec2& start, f32 width, f32 height);

    void CreateContext()
    {
        static constexpr const std::array<ImDbgGeometryType, 3ull> geometry_types{ ImDbgGeometryType::ePoints, ImDbgGeometryType::eLines, ImDbgGeometryType::eTriangles };

        context = new ImDbgContext();

        for (const auto& type : geometry_types)
        {
            auto& new_list = context->drawData.drawLists.emplace_back();
            new_list.type = type;
        }

        context->font = new FontImpl();
        auto err = FT_Init_FreeType(&context->font->ft);
        assert(err);

        LoadFont(monoisome_regular_ttf, monoisome_regular_ttf_len, 48);
    }

    void DestroyContext()
    {
        for (auto& font_atlas : context->font->atlases)
        {
            if (font_atlas.texture)
            {
                free(font_atlas.texture);
                font_atlas.texture = nullptr;
            }

            if(font_atlas.face)
                FT_Done_Face(font_atlas.face);
        }
        
        FT_Done_FreeType(context->font->ft);

        delete context;
        context = nullptr;

        delete context;
        context = nullptr;
    }

    void LoadFont(const u8* data, u32 data_size, u32 character_size)
    {
        auto& new_atlas = context->font->atlases.emplace_back();

        auto err = FT_New_Memory_Face(context->font->ft, monoisome_regular_ttf, monoisome_regular_ttf_len, 0, &new_atlas.face);
        assert(err);

        err = FT_New_Memory_Face(context->font->ft, data, data_size, 0, &new_atlas.face);
        assert(err);

        err = FT_Set_Pixel_Sizes(new_atlas.face, 0, character_size);
        assert(err);

        u32 roww = 0;
        u32 rowh = 0;

        for (int i = 0; i < 128; i++) 
        {
            if (FT_Load_Char(new_atlas.face, i, FT_LOAD_RENDER))
            {
                fprintf(stderr, "Loading character %c failed!\n", i);
                continue;
            }

            if (roww + new_atlas.face->glyph->bitmap.width + 1 >= (u32)context->io.screenSize.x)
            {
                new_atlas.width = std::max(new_atlas.width, roww);
                new_atlas.height += rowh;
                roww = 0;
                rowh = 0;
            }
            roww += new_atlas.face->glyph->bitmap.width + 1;
            rowh = std::max(rowh, new_atlas.face->glyph->bitmap.rows);
        }
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
        new_draw_cmd.clipRect = CalculateClipRect(glm::vec3(0.0f), context->io.screenSize.x, context->io.screenSize.y);

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
        new_draw_cmd.clipRect = CalculateClipRect(glm::vec3(0.0f), context->io.screenSize.x, context->io.screenSize.y);

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
        new_draw_cmd.clipRect = CalculateClipRect(glm::vec3(0.0f), context->io.screenSize.x, context->io.screenSize.y);

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
            new_draw_cmd.clipRect = CalculateClipRect(glm::vec3(0.0f), context->io.screenSize.x, context->io.screenSize.y);

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
        new_draw_cmd.clipRect = CalculateClipRect(position, text.size(), 1.0f);

        glm::vec2 ss_pos{ position.x, position.y };

        uint32_t packed_color = ColorToUint32(color);

        for (char c : text)
        {
            auto err = FT_Load_Char(context->font->face, c, FT_LOAD_RENDER);
            assert(err);

            FT_GlyphSlot g = context->font->face->glyph;

            ss_pos.x = ss_pos.x + g->bitmap_left;
            ss_pos.y = ss_pos.y - (g->bitmap.rows - g->bitmap_top);
            
            f32 w = g->bitmap.width;
            f32 h = g->bitmap.rows;

            glm::vec2 v0{ ss_pos.x / context->io.screenSize.x * 2.f - 1.f, ss_pos.y + h };
            glm::vec2 v1{ ss_pos.x, ss_pos.y };
            glm::vec2 v2{ ss_pos.x + w, ss_pos.y };
            glm::vec2 v3{ ss_pos.x + w, ss_pos.y + h };

            v0 = 

            glm::vec2 uv = GetUVForChar(c);

            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(ss_pos.x, ss_pos.y + h, 0.f), uv, packed_color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(ss_pos.x, ss_pos.y, 0.f), uv, packed_color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(ss_pos.x + w, ss_pos.y, 0.f), uv, packed_color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(ss_pos.x + w, ss_pos.y + h, 0.f), uv, packed_color });

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

    glm::vec4 CalculateClipRect(const glm::vec2& start, f32 width, f32 height)
    {
        return glm::vec4(start.x, start.y, start.x + width, start.y + height);
    }
}