#include "DebugDraw.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "monoisome_regular.ttf.h"

constexpr const u32 start_glyph = 33 - 1; 
constexpr const u32 max_glyph_count = 128;
constexpr const u32 max_atlas_width = 1024;

static ImDbgContext* context{ nullptr };

struct ImDbgFontGlyph
{
    glm::vec2 advance{};
    glm::vec2 size{};
    glm::vec2 corners{};
    glm::vec2 uv{};
};

struct FontAtlas
{
    FT_Face face;

    utl::vector<ImDbgFontGlyph> glyphs{};

    u8* texture{ nullptr };
    u32 width{};
    u32 height{};
    u64 size{};
    u32 stride{};
};

struct FontImpl
{
    FT_Library ft;
    utl::vector<FontAtlas> atlases;
};

namespace ImDbg
{
    u32 ColorToUint32(const glm::vec3& color);
    glm::vec4 CalculateClipRect(const glm::vec2& start, f32 width, f32 height);
    glm::vec2 WorldToScreen(const glm::vec3& pos);

    void CreateContext()
    {
        static constexpr const std::array<ImDbgGeometryType, 3ull> geometry_types{ ImDbgGeometryType::ePoints, ImDbgGeometryType::eLines, ImDbgGeometryType::eTriangles };

        context = new ImDbgContext();

        for (const auto& type : geometry_types)
        {
            auto& new_list2d = context->drawData2d.drawLists.emplace_back();
            new_list2d.type = type;

            auto& new_list3d = context->drawData3d.drawLists.emplace_back();
            new_list3d.type = type;
        }

        context->font = new FontImpl();
        auto err = FT_Init_FreeType(&context->font->ft);
        assert(!err);

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
		assert(!err);

		err = FT_New_Memory_Face(context->font->ft, data, data_size, 0, &new_atlas.face);
		assert(!err);

		err = FT_Set_Pixel_Sizes(new_atlas.face, 0, character_size);
		assert(!err);

        // Caqlculating max atlas width
        int max_dim = (1 + (new_atlas.face->size->metrics.height >> 6)) * glm::ceil(glm::sqrt(max_glyph_count - start_glyph));
        new_atlas.width = 1;
        while (new_atlas.width < max_dim) new_atlas.width <<= 1;
        
        u32 maxy{};
        glm::uvec2 pen{};
        for (u64 i = start_glyph; i < max_glyph_count; i++)
        {
            FT_Load_Char(new_atlas.face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
            FT_Bitmap* bmp = &new_atlas.face->glyph->bitmap;

            if (pen.x + bmp->width >= new_atlas.width)
            {
                pen.x = 0;
                pen.y += (new_atlas.face->size->metrics.height >> 6) + 1;
            }

            pen.x += bmp->width + 1;
            if (pen.y + bmp->rows > maxy)
                maxy = pen.y + bmp->rows;
        }

        // Adjust the texture height to the exact height required
        new_atlas.height = 1;
        while (new_atlas.height < maxy) new_atlas.height <<= 1;

        // Fill glyph texture
        new_atlas.stride = sizeof(u32);
        new_atlas.size = (u64)new_atlas.width * (u64)new_atlas.height * (u64)new_atlas.stride;
        new_atlas.texture = (u8*)calloc(new_atlas.size, 1);

        int strip_width = 1; // Width of the filled strip
        for (int y = 0; y < new_atlas.height; ++y)
        {
            for (int x = 0; x < strip_width; ++x)
            {
                int pixel_index = (y * new_atlas.width + x) * new_atlas.stride;
                for (u64 idx = 0; idx < new_atlas.stride; ++idx)
                    new_atlas.texture[pixel_index + idx] = 0xFF;
            }
        }

        glm::uvec2 cursor{ strip_width * 2, 0};
        for (u64 i = start_glyph; i < max_glyph_count; i++)
        {
            FT_Load_Char(new_atlas.face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
            FT_Bitmap* bmp = &new_atlas.face->glyph->bitmap;

            if (cursor.x + bmp->width >= new_atlas.width)
            {
                cursor.x = 0;
                cursor.y += ((new_atlas.face->size->metrics.height >> 6) + 1);
            }

            for (u64 row = 0; row < bmp->rows; ++row)
            {
                for (u64 col = 0; col < bmp->width; ++col)
                {
                    int x = cursor.x + col;
                    int y = cursor.y + row;
                    int pixel_index = (y * new_atlas.width + x) * new_atlas.stride;
                    auto value = bmp->buffer[row * bmp->pitch + col];

                    for (u64 idx = 0; idx < new_atlas.stride; ++idx)
                        new_atlas.texture[pixel_index + idx] = value;
                }
            }

            auto& new_glyph = new_atlas.glyphs.emplace_back();
            new_glyph.advance.x = new_atlas.face->glyph->advance.x >> 6;
            new_glyph.advance.y = new_atlas.face->glyph->advance.y >> 6;
            new_glyph.size.x = new_atlas.face->glyph->bitmap.width;
            new_glyph.size.y = new_atlas.face->glyph->bitmap.rows;
            new_glyph.corners.x = new_atlas.face->glyph->bitmap_left;
            new_glyph.corners.y = new_atlas.face->glyph->bitmap_top;
            new_glyph.uv.x = (f32)cursor.x / (f32)new_atlas.width;
            new_glyph.uv.y = (f32)cursor.y / (f32)new_atlas.height;

            cursor.x += bmp->width + 1;
        }

        //stbi_write_png("font.png", new_atlas.width, new_atlas.height, new_atlas.stride, new_atlas.texture, new_atlas.width * new_atlas.stride);
	}

    void GetTexDataAsRGBA32(u32 index, u8** out_pixels, i32* out_width, i32* out_height, i32* out_bytes_per_pixel)
    {
        auto& atlas = context->font->atlases[index];
        *out_pixels = atlas.texture;
        *out_width = atlas.width;
        *out_height = atlas.height;
        *out_bytes_per_pixel = atlas.stride;
    }

    void NewFrame()
    {
        for (auto& list : context->drawData2d.drawLists)
        {
            list.vertices.clear();
            list.indices.clear();
            list.drawCommands.clear();
        }

        context->drawData2d.totalIndexCount = 0;
        context->drawData2d.totalVertexCount = 0;

        for (auto& list : context->drawData3d.drawLists)
        {
            list.vertices.clear();
            list.indices.clear();
            list.drawCommands.clear();
        }

        context->drawData3d.totalIndexCount = 0;
        context->drawData3d.totalVertexCount = 0;
    }

    void EndFrame()
    {

    }

    // For 2d drawings
    void DrawPoint2D(const glm::vec2& position, ImDbgColor color, f32 size)
    {
        auto& draw_list = context->drawData2d.drawLists[(u32)ImDbgGeometryType::ePoints];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 1;
        new_draw_cmd.clipRect = CalculateClipRect(position - glm::vec2(size), size * 2, size * 2);

        draw_list.vertices.emplace_back(ImDbgVertex2D{ position, glm::vec2(0.0f), color });
        draw_list.indices.emplace_back(new_draw_cmd.vertexOffset);

        context->drawData2d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData2d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawLine2D(const glm::vec2& from, const glm::vec2& to, ImDbgColor color, f32 thickness)
    {
        auto& draw_list = context->drawData2d.drawLists[(u32)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 2;
        new_draw_cmd.clipRect = CalculateClipRect(glm::min(from, to) - thickness, glm::distance(from, to), thickness);

        draw_list.vertices.emplace_back(ImDbgVertex2D{ from, glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ to, glm::vec2(0.0f), color });
        draw_list.indices.emplace_back(new_draw_cmd.vertexOffset);
        draw_list.indices.emplace_back(new_draw_cmd.vertexOffset + 1);

        context->drawData2d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData2d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawRect2D(const glm::vec2& min, const glm::vec2& max, ImDbgColor color, f32 thickness)
    {
        const utl::vector<glm::vec2> points{ min, {max.x, min.y}, max, {min.x, max.y} };
        DrawPolygon2D(points, color, thickness);
    }

    void DrawFillRect2D(const glm::vec2& min, const glm::vec2& max, ImDbgColor color)
    {
        const utl::vector<glm::vec2> points{ min, {max.x, min.y}, max, {min.x, max.y} };
        DrawFillPolygon2D(points, color);
    }

    void DrawCircle2D(const glm::vec2& center, f32 radius, ImDbgColor color, f32 thickness)
    {
        auto& draw_list = context->drawData2d.drawLists[(u32)ImDbgGeometryType::eLines];

        const int segments = 36;
        float increment = 2.0f * glm::pi<float>() / segments;

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = segments * 2;
        new_draw_cmd.clipRect = CalculateClipRect(center - glm::vec2(radius), radius * 2, radius * 2);

        u32 vertex_offset = draw_list.vertices.size();

        // Add vertices for the circle
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment;
            glm::vec2 pos = center + radius * glm::vec2(glm::cos(theta), glm::sin(theta));
            draw_list.vertices.emplace_back(ImDbgVertex2D{ pos, glm::vec2(0.0f), color });
        }

        // Add indices for the circle
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset + i);
            draw_list.indices.emplace_back(vertex_offset + i + 1);
        }

        context->drawData2d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData2d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawFillCircle2D(const glm::vec2& center, f32 radius, ImDbgColor color)
    {
        auto& draw_list = context->drawData2d.drawLists[(u32)ImDbgGeometryType::eTriangles];

        const int segments = 36;
        float increment = 2.0f * glm::pi<float>() / segments;

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = segments * 3;
        new_draw_cmd.clipRect = CalculateClipRect(center - glm::vec2(radius), radius * 2, radius * 2);

        u32 vertex_offset = draw_list.vertices.size();
        draw_list.vertices.emplace_back(ImDbgVertex2D{ center, glm::vec2(0.0f), color });

        for (int i = 0; i <= segments; i++)
        {
            float theta = i * increment;
            glm::vec2 pos = center + radius * glm::vec2(glm::cos(theta), glm::sin(theta));
            draw_list.vertices.emplace_back(ImDbgVertex2D{ pos, glm::vec2(0.0f), color });

            if (i > 0)
            {
                draw_list.indices.emplace_back(vertex_offset);
                draw_list.indices.emplace_back(vertex_offset + i);
                draw_list.indices.emplace_back(vertex_offset + i + 1);
            }
        }

        context->drawData2d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData2d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawTriangle2D(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, ImDbgColor color, f32 thickness)
    {
        const utl::vector<glm::vec2> points{ p1, p2, p3 };
        DrawPolygon2D(points, color, thickness);
    }

    void DrawFillTriangle2D(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, ImDbgColor color)
    {
        const utl::vector<glm::vec2> points{ p1, p2, p3 };
        DrawFillPolygon2D(points, color);
    }

    void DrawPolygon2D(const utl::vector<glm::vec2>& points, ImDbgColor color, f32 thickness)
    {
        if (points.size() < 2) return;

        // Calculate polygon aabb
        glm::vec2 min{ points[0] }, max{ points[0] };
        for (i32 i = 1; i < points.size(); ++i)
        {
            min = glm::min(min, points[i]);
            max = glm::max(max, points[i]);
        }

        glm::vec2 range = max - min;

        auto& draw_list = context->drawData2d.drawLists[(u32)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = points.size() * 2;
        new_draw_cmd.clipRect = CalculateClipRect(min - thickness, range.x + thickness, range.y + thickness);

        u32 vertex_offset = draw_list.vertices.size();

        for (const auto& point : points)
            draw_list.vertices.emplace_back(ImDbgVertex2D{ point, glm::vec2(0.0f), color });

        for (i32 i = 0; i < points.size(); ++i)
        {
            draw_list.indices.emplace_back(vertex_offset + i);
            draw_list.indices.emplace_back(vertex_offset + ((i + 1) % points.size()));
        }

        context->drawData2d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData2d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawFillPolygon2D(const utl::vector<glm::vec2>& points, ImDbgColor color)
    {
        if (points.size() < 3) return;

        // Calculate polygon aabb
        glm::vec2 min{ points[0] }, max{ points[0] };
        for (i32 i = 1; i < points.size(); ++i)
        {
            min = glm::min(min, points[i]);
            max = glm::max(max, points[i]);
        }

        glm::vec2 range = max - min;

        auto& draw_list = context->drawData2d.drawLists[(u32)ImDbgGeometryType::eTriangles];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = (points.size() - 2) * 3;
        new_draw_cmd.clipRect = CalculateClipRect(min, range.x, range.y);

        for (const auto& point : points)
            draw_list.vertices.emplace_back(ImDbgVertex2D{ point, glm::vec2(0.0f), color });

        u32 vertex_offset = draw_list.vertices.size() - points.size();

        for (i32 i = 1; i < points.size() - 1; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset);
            draw_list.indices.emplace_back(vertex_offset + i);
            draw_list.indices.emplace_back(vertex_offset + i + 1);
        }

        context->drawData2d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData2d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawRoundedRect2D(const glm::vec2& min, const glm::vec2& max, f32 radius, ImDbgColor color, f32 thickness)
    {
        auto& draw_list = context->drawData2d.drawLists[(u32)ImDbgGeometryType::eLines];

        const int segments = 8;
        float increment = glm::half_pi<float>() / segments;

        glm::vec2 top_left = glm::vec2(min.x + radius, min.y + radius);
        glm::vec2 top_right = glm::vec2(max.x - radius, min.y + radius);
        glm::vec2 bottom_left = glm::vec2(min.x + radius, max.y - radius);
        glm::vec2 bottom_right = glm::vec2(max.x - radius, max.y - radius);

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = (segments + 1) * 8;
        new_draw_cmd.clipRect = CalculateClipRect(min - thickness, max.x - min.x - thickness, max.y - min.y - thickness);

        u32 vertex_offset = draw_list.vertices.size();

        // Add vertices for rounded corners
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment;
            draw_list.vertices.emplace_back(ImDbgVertex2D{ top_left - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), glm::vec2(0.0f), color });
        }
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment + glm::half_pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex2D{ top_right - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), glm::vec2(0.0f), color });
        }
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment + glm::pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex2D{ bottom_right - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), glm::vec2(0.0f), color });
        }
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment + 3.0f * glm::half_pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex2D{ bottom_left - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), glm::vec2(0.0f), color });
        }

        // Add vertices for straight edges
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x + radius, min.y), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x - radius, min.y), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x, min.y + radius), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x, max.y - radius), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x - radius, max.y), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x + radius, max.y), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x, max.y - radius), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x, min.y + radius), glm::vec2(0.0f), color });

        // Add indices for lines
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset + i);
            draw_list.indices.emplace_back(vertex_offset + i + 1);
        }
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset + segments + 1 + i);
            draw_list.indices.emplace_back(vertex_offset + segments + 1 + i + 1);
        }
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 2 + i);
            draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 2 + i + 1);
        }
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 3 + i);
            draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 3 + i + 1);
        }

        draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 4);
        draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 4 + 1);
        draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 4 + 2);
        draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 4 + 3);
        draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 4 + 4);
        draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 4 + 5);
        draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 4 + 6);
        draw_list.indices.emplace_back(vertex_offset + (segments + 1) * 4 + 7);

        context->drawData2d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData2d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawFillRoundedRect2D(const glm::vec2& min, const glm::vec2& max, f32 radius, ImDbgColor color)
    {
        auto& draw_list = context->drawData2d.drawLists[(u32)ImDbgGeometryType::eTriangles];

        // Define corners
        glm::vec2 top_left = glm::vec2(min.x + radius, min.y + radius);
        glm::vec2 top_right = glm::vec2(max.x - radius, min.y + radius);
        glm::vec2 bottom_left = glm::vec2(min.x + radius, max.y - radius);
        glm::vec2 bottom_right = glm::vec2(max.x - radius, max.y - radius);

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 0;

        // Center rectangle
        u32 vertex_offset = draw_list.vertices.size();
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x + radius, min.y), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x - radius, min.y), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x - radius, max.y), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x + radius, max.y), glm::vec2(0.0f), color });

        draw_list.indices.emplace_back(vertex_offset);
        draw_list.indices.emplace_back(vertex_offset + 1);
        draw_list.indices.emplace_back(vertex_offset + 2);
        draw_list.indices.emplace_back(vertex_offset);
        draw_list.indices.emplace_back(vertex_offset + 2);
        draw_list.indices.emplace_back(vertex_offset + 3);
        new_draw_cmd.indexCount += 6;

        // Side rectangles
        vertex_offset = draw_list.vertices.size();
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x, min.y + radius), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x + radius, min.y + radius), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x + radius, max.y - radius), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(min.x, max.y - radius), glm::vec2(0.0f), color });

        draw_list.indices.emplace_back(vertex_offset);
        draw_list.indices.emplace_back(vertex_offset + 1);
        draw_list.indices.emplace_back(vertex_offset + 2);
        draw_list.indices.emplace_back(vertex_offset);
        draw_list.indices.emplace_back(vertex_offset + 2);
        draw_list.indices.emplace_back(vertex_offset + 3);
        new_draw_cmd.indexCount += 6;

        vertex_offset = draw_list.vertices.size();
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x - radius, min.y + radius), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x, min.y + radius), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x, max.y - radius), glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(max.x - radius, max.y - radius), glm::vec2(0.0f), color });

        draw_list.indices.emplace_back(vertex_offset);
        draw_list.indices.emplace_back(vertex_offset + 1);
        draw_list.indices.emplace_back(vertex_offset + 2);
        draw_list.indices.emplace_back(vertex_offset);
        draw_list.indices.emplace_back(vertex_offset + 2);
        draw_list.indices.emplace_back(vertex_offset + 3);
        new_draw_cmd.indexCount += 6;

        // Corners
        const int segments = 8;
        float increment = glm::half_pi<float>() / segments;

        vertex_offset = draw_list.vertices.size();
        for (i32 i = 0; i <= segments; ++i)
        {
            float theta = i * increment;
            draw_list.vertices.emplace_back(ImDbgVertex2D{ top_left - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), glm::vec2(0.0f), color });
        }

        for (i32 i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset);
            draw_list.indices.emplace_back(vertex_offset + i);
            draw_list.indices.emplace_back(vertex_offset + i + 1);
            new_draw_cmd.indexCount += 3;
        }

        vertex_offset = draw_list.vertices.size();
        for (i32 i = 0; i <= segments; ++i)
        {
            float theta = i * increment + glm::half_pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex2D{ top_right - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), glm::vec2(0.0f), color });
        }

        for (i32 i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset);
            draw_list.indices.emplace_back(vertex_offset + i);
            draw_list.indices.emplace_back(vertex_offset + i + 1);
            new_draw_cmd.indexCount += 3;
        }

        vertex_offset = draw_list.vertices.size();
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment + glm::pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex2D{ bottom_right - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), glm::vec2(0.0f), color });
        }

        for (i32 i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset);
            draw_list.indices.emplace_back(vertex_offset + i);
            draw_list.indices.emplace_back(vertex_offset + i + 1);
            new_draw_cmd.indexCount += 3;
        }

        vertex_offset = draw_list.vertices.size();
        for (i32 i = 0; i <= segments; ++i)
        {
            f32 theta = i * increment + 3.0f * glm::half_pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex2D{ bottom_left - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), glm::vec2(0.0f), color });
        }

        for (i32 i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_offset);
            draw_list.indices.emplace_back(vertex_offset + i);
            draw_list.indices.emplace_back(vertex_offset + i + 1);
            new_draw_cmd.indexCount += 3;
        }

        context->drawData2d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData2d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawText2D(const glm::vec2& position, const std::string& text, ImDbgColor color)
    {
        auto& draw_list = context->drawData2d.drawLists[(u32)ImDbgGeometryType::eTriangles];

        auto& font_atlas = context->font->atlases[0]; // Assuming you have only one font atlas

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = text.size() * 6;

        // TODO: add \n support 

        glm::vec2 cursor = position; // Initial position of the text
        f32 height{};
        for (char c : text)
        {
            if (c < start_glyph || c >= max_glyph_count)
                continue;

            const ImDbgFontGlyph& glyph = font_atlas.glyphs[c - start_glyph];

            if (glyph.size.x == 0.f || glyph.size.y == 0.f)
            {
                cursor.x += glyph.advance.x;
                continue;
            }

            float x0 = cursor.x + glyph.corners.x;
            float y0 = cursor.y + glyph.corners.y;
            float x1 = x0 + glyph.size.x;
            float y1 = y0 - glyph.size.y;

            float u0 = glyph.uv.x;
            float v0 = glyph.uv.y;
            float u1 = u0 + glyph.size.x / font_atlas.width;
            float v1 = v0 + glyph.size.y / font_atlas.height;

            u32 vertex_offset = draw_list.vertices.size();

            // Add vertices for the glyph quad
            draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(x0, y0), glm::vec2(u0, v0), color });
            draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(x1, y0), glm::vec2(u1, v0), color });
            draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(x1, y1), glm::vec2(u1, v1), color });
            draw_list.vertices.emplace_back(ImDbgVertex2D{ glm::vec2(x0, y1), glm::vec2(u0, v1), color });

            // Add indices for the glyph quad
            draw_list.indices.emplace_back(vertex_offset);
            draw_list.indices.emplace_back(vertex_offset + 1);
            draw_list.indices.emplace_back(vertex_offset + 2);
            draw_list.indices.emplace_back(vertex_offset);
            draw_list.indices.emplace_back(vertex_offset + 2);
            draw_list.indices.emplace_back(vertex_offset + 3);

            cursor.x += glyph.advance.x;
            height = std::max(glyph.size.y, height);
        }

        context->drawData2d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData2d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;

        new_draw_cmd.clipRect = CalculateClipRect(position, cursor.x - position.x, height);
    }

    // For 3d drawings
    void DrawPoint3D(const glm::vec3& position, const glm::vec3& color, float size)
    {
        auto& draw_list = context->drawData3d.drawLists[(u32)ImDbgGeometryType::ePoints];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 1;
        new_draw_cmd.clipRect = CalculateClipRect(glm::vec3(0.0f), context->io.screenSize.x, context->io.screenSize.y);

        draw_list.vertices.emplace_back(ImDbgVertex3D{ position, glm::vec2(0.0f), ColorToUint32(color) });
        draw_list.indices.emplace_back(new_draw_cmd.vertexOffset);

        context->drawData3d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData3d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawLine3D(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color, float thickness)
    {
        auto& draw_list = context->drawData3d.drawLists[(u32)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 2;
        new_draw_cmd.clipRect = CalculateClipRect(glm::vec3(0.0f), context->io.screenSize.x, context->io.screenSize.y);

        draw_list.vertices.emplace_back(ImDbgVertex3D{ from, glm::vec2(0.0f), ColorToUint32(color) });
        draw_list.vertices.emplace_back(ImDbgVertex3D{ to, glm::vec2(0.0f), ColorToUint32(color) });
        draw_list.indices.emplace_back(new_draw_cmd.vertexOffset);
        draw_list.indices.emplace_back(new_draw_cmd.vertexOffset + 1);

        context->drawData3d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData3d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawBox3D(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color)
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

        auto& draw_list = context->drawData3d.drawLists[(u32)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 24;
        new_draw_cmd.clipRect = CalculateClipRect(glm::vec2(0.f), context->io.screenSize.x, context->io.screenSize.y);

        u32 packedColor = ColorToUint32(color);

        for (const auto& pos : verticesPos)
            draw_list.vertices.emplace_back(ImDbgVertex3D{ pos, glm::vec2(0.0f), packedColor });

        for (const auto& idx : boxIndices)
            draw_list.indices.emplace_back(new_draw_cmd.vertexOffset + idx);

        context->drawData3d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData3d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawSphere3D(const glm::vec3& center, float radius, const glm::vec3& color)
    {
        const int segments = 16;
        static const glm::vec3 axis[] =
        {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f}
        };

        auto& draw_list = context->drawData3d.drawLists[(u32)ImDbgGeometryType::eLines];

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

                draw_list.vertices.emplace_back(ImDbgVertex3D{ p1, glm::vec2(0.0f), ColorToUint32(color) });
                draw_list.vertices.emplace_back(ImDbgVertex3D{ p2, glm::vec2(0.0f), ColorToUint32(color) });

                draw_list.indices.emplace_back(new_draw_cmd.vertexOffset + j * 2);
                draw_list.indices.emplace_back(new_draw_cmd.vertexOffset + j * 2 + 1);
            }

            context->drawData3d.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
            context->drawData3d.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
        }
    }

    void DrawText3D(const glm::vec3& position, const std::string& text, const glm::vec3& color)
    {
        
    }

    ImDbgIO* GetIO()
    {
        return &context->io;
    }

    ImDbgDrawData<ImDbgVertex2D>* GetDrawData2D()
    {
        return &context->drawData2d;
    }

    ImDbgDrawData<ImDbgVertex3D>* GetDrawData3D()
    {
        return &context->drawData3d;
    }

    // Other functions
    u32 ColorToUint32(const glm::vec3& color)
    {
        uint8_t r = static_cast<uint8_t>(glm::clamp(color.z, 0.0f, 1.0f) * 255.0f);
        uint8_t g = static_cast<uint8_t>(glm::clamp(color.y, 0.0f, 1.0f) * 255.0f);
        uint8_t b = static_cast<uint8_t>(glm::clamp(color.x, 0.0f, 1.0f) * 255.0f);
        return (r << 16) | (g << 8) | b;
    }

    glm::vec4 CalculateClipRect(const glm::vec2& start, f32 width, f32 height)
    {
        return glm::vec4(start.x, start.y, start.x + width, start.y + height);
    }

    glm::vec2 WorldToScreen(const glm::vec3& pos)
    {
        const glm::vec4 cs_pos = context->io.worldToClip * glm::vec4(pos, 1.f);
        const glm::vec3 ndc_pos = glm::vec3(cs_pos) / cs_pos.w;
        return glm::vec2(
            (ndc_pos.x + 1.0f) * 0.5f * context->io.screenSize.x,
            (1.0f - ndc_pos.y) * 0.5f * context->io.screenSize.y
        );
    }
}