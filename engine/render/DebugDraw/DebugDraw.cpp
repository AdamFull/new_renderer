#include "DebugDraw.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <ft2build.h>
//#include FT_FREETYPE_H

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "monoisome_regular.ttf.h"

constexpr const uint32_t start_glyph = 33 - 1; 
constexpr const uint32_t max_glyph_count = 128;
constexpr const uint32_t max_atlas_width = 1024;
constexpr const glm::vec2 default_uv = { 0.01f, 0.01f };

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
    //FT_Face face;

    //std::vector<ImDbgFontGlyph> glyphs{};

    float lineHeight{};

    uint8_t* texture{ nullptr };
    uint32_t width{};
    uint32_t height{};
    uint64_t size{};
    uint32_t stride{};
};

struct FontImpl
{
    //FT_Library ft;
    //std::vector<FontAtlas> atlases;
};

namespace ImDbg
{
    glm::vec4 CalculateClipRect(const glm::vec2& start, float width, float height);
    glm::vec2 WorldToScreen(const glm::vec3& pos);

    void create_context()
    {
        static constexpr const std::array<ImDbgGeometryType, 3ull> geometry_types{ ImDbgGeometryType::ePoints, ImDbgGeometryType::eLines, ImDbgGeometryType::eTriangles };

        context = new ImDbgContext();

        for (const auto& type : geometry_types)
        {
            auto& new_list2d = context->drawData.drawLists.emplace_back();
            new_list2d.type = type;

            auto& new_list3d = context->drawData.drawLists.emplace_back();
            new_list3d.type = type;
        }

        //context->font = new FontImpl();
        //auto err = FT_Init_FreeType(&context->font->ft);
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

            //if(font_atlas.face)
            //    FT_Done_Face(font_atlas.face);
        }
        
        //FT_Done_FreeType(context->font->ft);

        delete context;
        context = nullptr;

        delete context;
        context = nullptr;
    }

	void LoadFont(const uint8_t* data, uint32_t data_size, uint32_t character_size)
	{
		//auto& new_atlas = context->font->atlases.emplace_back();
        //
		//auto err = FT_New_Memory_Face(context->font->ft, monoisome_regular_ttf, monoisome_regular_ttf_len, 0, &new_atlas.face);
		//assert(!err);
        //
		//err = FT_New_Memory_Face(context->font->ft, data, data_size, 0, &new_atlas.face);
		//assert(!err);
        //
		//err = FT_Set_Pixel_Sizes(new_atlas.face, 0, character_size);
		//assert(!err);
        //
        //// Caqlculating max atlas width
        //int max_dim = (1 + (new_atlas.face->size->metrics.height >> 6)) * glm::ceil(glm::sqrt(max_glyph_count - start_glyph));
        //new_atlas.width = 1;
        //while (new_atlas.width < max_dim) new_atlas.width <<= 1;
        //
        //uint32_t maxy{};
        //glm::uvec2 pen{};
        //for (uint64_t i = start_glyph; i < max_glyph_count; i++)
        //{
        //    FT_Load_Char(new_atlas.face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
        //    FT_Bitmap* bmp = &new_atlas.face->glyph->bitmap;
        //
        //    if (pen.x + bmp->width >= new_atlas.width)
        //    {
        //        pen.x = 0;
        //        pen.y += (new_atlas.face->size->metrics.height >> 6) + 1;
        //    }
        //
        //    pen.x += bmp->width + 1;
        //    if (pen.y + bmp->rows > maxy)
        //        maxy = pen.y + bmp->rows;
        //}
        //
        //// Get scaled line height
        //new_atlas.lineHeight = new_atlas.face->size->metrics.height >> 6;
        //
        //// Adjust the texture height to the exact height required
        //new_atlas.height = 1;
        //while (new_atlas.height < maxy) new_atlas.height <<= 1;
        //
        //// Fill glyph texture
        //new_atlas.stride = sizeof(uint32_t);
        //new_atlas.size = (uint64_t)new_atlas.width * (uint64_t)new_atlas.height * (uint64_t)new_atlas.stride;
        //new_atlas.texture = (uint8_t*)calloc(new_atlas.size, 1);
        //
        //int strip_width = 3; // Width of the filled strip
        //for (int y = 0; y < new_atlas.height; ++y)
        //{
        //    for (int x = 0; x < strip_width; ++x)
        //    {
        //        int pixel_index = (y * new_atlas.width + x) * new_atlas.stride;
        //        for (uint64_t idx = 0; idx < new_atlas.stride; ++idx)
        //            new_atlas.texture[pixel_index + idx] = 0xFF;
        //    }
        //}
        //
        //glm::uvec2 cursor{ strip_width + 1, 0};
        //for (uint64_t i = start_glyph; i < max_glyph_count; i++)
        //{
        //    FT_Load_Char(new_atlas.face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
        //    FT_Bitmap* bmp = &new_atlas.face->glyph->bitmap;
        //
        //    if (cursor.x + bmp->width >= new_atlas.width)
        //    {
        //        cursor.x = strip_width + 1;
        //        cursor.y += ((new_atlas.face->size->metrics.height >> 6) + 1);
        //    }
        //
        //    for (uint64_t row = 0; row < bmp->rows; ++row)
        //    {
        //        for (uint64_t col = 0; col < bmp->width; ++col)
        //        {
        //            int x = cursor.x + col;
        //            int y = cursor.y + row;
        //            int pixel_index = (y * new_atlas.width + x) * new_atlas.stride;
        //            auto value = bmp->buffer[row * bmp->pitch + col];
        //
        //            for (uint64_t idx = 0; idx < new_atlas.stride; ++idx)
        //                new_atlas.texture[pixel_index + idx] = value;
        //        }
        //    }
        //
        //    auto& new_glyph = new_atlas.glyphs.emplace_back();
        //    new_glyph.advance.x = new_atlas.face->glyph->advance.x >> 6;
        //    new_glyph.advance.y = new_atlas.face->glyph->advance.y >> 6;
        //    new_glyph.size.x = new_atlas.face->glyph->bitmap.width;
        //    new_glyph.size.y = new_atlas.face->glyph->bitmap.rows;
        //    new_glyph.corners.x = new_atlas.face->glyph->bitmap_left;
        //    new_glyph.corners.y = new_atlas.face->glyph->bitmap_top;
        //    new_glyph.uv.x = (float)cursor.x / (float)new_atlas.width;
        //    new_glyph.uv.y = (float)cursor.y / (float)new_atlas.height;
        //
        //    cursor.x += bmp->width + 1;
        //}

        //stbi_write_png("font.png", new_atlas.width, new_atlas.height, new_atlas.stride, new_atlas.texture, new_atlas.width * new_atlas.stride);
	}

    void GetTexDataAsRGBA32(uint32_t index, uint8_t** out_pixels, int32_t* out_width, int32_t* out_height, int32_t* out_bytes_per_pixel)
    {
        //auto& atlas = context->font->atlases[index];
        //*out_pixels = atlas.texture;
        //*out_width = atlas.width;
        //*out_height = atlas.height;
        //*out_bytes_per_pixel = atlas.stride;
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

        for (auto& list : context->drawData.drawLists)
        {
            list.vertices.clear();
            list.indices.clear();
            list.drawCommands.clear();
        }

        context->drawData.totalIndexCount = 0;
        context->drawData.totalVertexCount = 0;
    }

    void end_frame()
    {

    }

    // For 2d drawings
    void DrawPoint2D(const glm::vec2& position, ImDbgColor color, float size)
    {
        if (!context)
            return;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::ePoints];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e2D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 1;
        new_draw_cmd.clipRect = CalculateClipRect(position - glm::vec2(size), size * 2, size * 2);

        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(position, 0.f), default_uv, color });
        draw_list.indices.emplace_back(0);

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawLine2D(const glm::vec2& from, const glm::vec2& to, ImDbgColor color, float thickness)
    {
        if (!context)
            return;

        auto min = glm::min(from, to);
        auto range = glm::max(from, to) - min;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e2D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 2;
        new_draw_cmd.clipRect = CalculateClipRect(min - thickness, range.x + thickness, range.y + thickness);

        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(from, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(to, 0.f), default_uv, color });
        draw_list.indices.emplace_back(0);
        draw_list.indices.emplace_back(1);

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawRect2D(const glm::vec2& min, const glm::vec2& max, ImDbgColor color, float thickness)
    {
        const std::array<glm::vec2, 4ull> points{ min, {max.x, min.y}, max, {min.x, max.y} };
        DrawPolygon2D(points, color, thickness);
    }

    void DrawFillRect2D(const glm::vec2& min, const glm::vec2& max, ImDbgColor color)
    {
        const std::array<glm::vec2, 4ull> points{ min, {max.x, min.y}, max, {min.x, max.y} };
        DrawFillPolygon2D(points, color);
    }

    void DrawArc2D(const glm::vec2& center, float radius, float start_angle, float end_angle, ImDbgColor color, int32_t segments, float thickness)
    {
        if (!context)
            return;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eLines];

        float increment = (end_angle - start_angle) / segments;

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e2D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = segments * 2;
        new_draw_cmd.clipRect = CalculateClipRect(center - glm::vec2(radius) - thickness, radius * 2 + thickness, radius * 2 + thickness);

        // Add vertices for the arc
        for (int32_t i = 0; i <= segments; ++i)
        {
            float theta = start_angle + i * increment;
            glm::vec2 pos = center + radius * glm::vec2(glm::cos(theta), glm::sin(theta));
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(pos, 0.f), default_uv, color });
        }

        // Add indices for the arc
        for (int32_t i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(i);
            draw_list.indices.emplace_back(i + 1);
        }

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawPie2D(const glm::vec2& center, float radius, float start_angle, float end_angle, ImDbgColor color, int32_t segments)
    {
        if (!context)
            return;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eTriangles];

        float increment = (end_angle - start_angle) / segments;

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e2D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = segments * 3;  // Each segment forms a triangle
        new_draw_cmd.clipRect = CalculateClipRect(center - glm::vec2(radius), radius * 2, radius * 2);

        // Center vertex
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(center, 0.f), default_uv, color });

        // Add vertices for the arc
        for (int i = 0; i <= segments; ++i)
        {
            float theta = start_angle + i * increment;
            glm::vec2 pos = center + radius * glm::vec2(glm::cos(theta), glm::sin(theta));
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(pos, 0.f), default_uv, color });
        }

        // Add indices for the pie
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back();       // Center vertex
            draw_list.indices.emplace_back(i + 1);
            draw_list.indices.emplace_back(i + 2);
        }

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawRingArc2D(const glm::vec2& center, float inner_radius, float outer_radius, float start_angle, float end_angle, ImDbgColor color, int32_t segments)
    {
        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eTriangles];

        float increment = (end_angle - start_angle) / segments;

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e2D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = segments * 6;  // Each segment forms two triangles
        new_draw_cmd.clipRect = CalculateClipRect(center - glm::vec2(outer_radius), outer_radius * 2, outer_radius * 2);

        // Add vertices for the inner and outer arc
        for (int i = 0; i <= segments; ++i)
        {
            float theta = start_angle + i * increment;
            glm::vec2 inner_pos = center + inner_radius * glm::vec2(glm::cos(theta), glm::sin(theta));
            glm::vec2 outer_pos = center + outer_radius * glm::vec2(glm::cos(theta), glm::sin(theta));
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(inner_pos, 0.f), default_uv, color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(outer_pos, 0.f), default_uv, color });
        }

        // Add indices for the ring segment
        for (int i = 0; i < segments; ++i)
        {
            int inner_idx1 = i * 2;
            int outer_idx1 = i * 2 + 1;
            int inner_idx2 = (i + 1) * 2;
            int outer_idx2 = (i + 1) * 2 + 1;

            draw_list.indices.emplace_back(inner_idx1);
            draw_list.indices.emplace_back(outer_idx1);
            draw_list.indices.emplace_back(inner_idx2);

            draw_list.indices.emplace_back(outer_idx1);
            draw_list.indices.emplace_back(outer_idx2);
            draw_list.indices.emplace_back(inner_idx2);
        }

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawRing2D(const glm::vec2& center, float inner_radius, float outer_radius, ImDbgColor color, int32_t segments)
    {
        DrawRingArc2D(center, inner_radius, outer_radius, 0.f, 2.f * glm::pi<float>(), color, segments);
    }

    void DrawCircle2D(const glm::vec2& center, float radius, ImDbgColor color, float thickness)
    {
        DrawArc2D(center, radius, 0.0f, 2.0f * glm::pi<float>(), color, 36, thickness);
    }

    void DrawFillCircle2D(const glm::vec2& center, float radius, ImDbgColor color)
    {
        DrawPie2D(center, radius, 0.0f, 2.0f * glm::pi<float>(), color, 36);
    }

    void DrawTriangle2D(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, ImDbgColor color, float thickness)
    {
        const std::array<glm::vec2, 3ull> points{ p1, p2, p3 };
        DrawPolygon2D(points, color, thickness);
    }

    void DrawFillTriangle2D(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, ImDbgColor color)
    {
        if (!context)
            return;

        const std::array<glm::vec2, 3ull> points{ p1, p2, p3 };
        DrawFillPolygon2D(points, color);
    }

    void DrawPolygon2D(const ArrayProxy<glm::vec2>& points, ImDbgColor color, float thickness)
    {
        if (!context)
            return;

        if (points.size() < 2) return;

        // Calculate polygon aabb
        glm::vec2 min{ points[0] }, max{ points[0] };
        for (int32_t i = 1; i < points.size(); ++i)
        {
            min = glm::min(min, points[i]);
            max = glm::max(max, points[i]);
        }

        glm::vec2 range = max - min;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e2D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = points.size() * 2;
        new_draw_cmd.clipRect = CalculateClipRect(min - thickness, range.x + thickness, range.y + thickness);

        for (const auto& point : points)
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(point, 0.f), default_uv, color });

        for (int32_t i = 0; i < points.size(); ++i)
        {
            draw_list.indices.emplace_back(i);
            draw_list.indices.emplace_back(((i + 1) % points.size()));
        }

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawFillPolygon2D(const ArrayProxy<glm::vec2>& points, ImDbgColor color)
    {
        if (!context)
            return;

        if (points.size() < 3) return;

        // Calculate polygon aabb
        glm::vec2 min{ points[0] }, max{ points[0] };
        for (int32_t i = 1; i < points.size(); ++i)
        {
            min = glm::min(min, points[i]);
            max = glm::max(max, points[i]);
        }

        glm::vec2 range = max - min;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eTriangles];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e2D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = (points.size() - 2) * 3;
        new_draw_cmd.clipRect = CalculateClipRect(min, range.x, range.y);

        for (const auto& point : points)
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(point, 0.f), default_uv, color });

        for (int32_t i = 0; i < points.size() - 1; ++i)
        {
            if (i > 1)
            {
                draw_list.indices.emplace_back(0);
                draw_list.indices.emplace_back(i);
                draw_list.indices.emplace_back(i + 1);
            }
        }

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawRoundedRect2D(const glm::vec2& min, const glm::vec2& max, float radius, ImDbgColor color, float thickness)
    {
        if (!context)
            return;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eLines];

        const int segments = 8;
        float increment = glm::half_pi<float>() / segments;

        glm::vec2 top_left = glm::vec2(min.x + radius, min.y + radius);
        glm::vec2 top_right = glm::vec2(max.x - radius, min.y + radius);
        glm::vec2 bottom_left = glm::vec2(min.x + radius, max.y - radius);
        glm::vec2 bottom_right = glm::vec2(max.x - radius, max.y - radius);

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e2D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = (segments + 1) * 8;
        new_draw_cmd.clipRect = CalculateClipRect(min - thickness, max.x - min.x + thickness, max.y - min.y + thickness);

        // Add vertices for rounded corners
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment;
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(top_left - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), 0.f), default_uv, color });
        }
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment + glm::half_pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(top_right - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), 0.f), default_uv, color });
        }
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment + glm::pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(bottom_right - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), 0.f), default_uv, color });
        }
        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment + 3.0f * glm::half_pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(bottom_left - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), 0.f), default_uv, color });
        }

        // Add vertices for straight edges
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x + radius, min.y, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x - radius, min.y, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x, min.y + radius, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x, max.y - radius, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x - radius, max.y, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x + radius, max.y, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x, max.y - radius, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x, min.y + radius, 0.f), default_uv, color });

        // Add indices for lines
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(i);
            draw_list.indices.emplace_back(i + 1);
        }
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(segments + 1 + i);
            draw_list.indices.emplace_back(segments + 1 + i + 1);
        }
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back((segments + 1) * 2 + i);
            draw_list.indices.emplace_back((segments + 1) * 2 + i + 1);
        }
        for (int i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back((segments + 1) * 3 + i);
            draw_list.indices.emplace_back((segments + 1) * 3 + i + 1);
        }

        draw_list.indices.emplace_back((segments + 1) * 4);
        draw_list.indices.emplace_back((segments + 1) * 4 + 1);
        draw_list.indices.emplace_back((segments + 1) * 4 + 2);
        draw_list.indices.emplace_back((segments + 1) * 4 + 3);
        draw_list.indices.emplace_back((segments + 1) * 4 + 4);
        draw_list.indices.emplace_back((segments + 1) * 4 + 5);
        draw_list.indices.emplace_back((segments + 1) * 4 + 6);
        draw_list.indices.emplace_back((segments + 1) * 4 + 7);

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawFillRoundedRect2D(const glm::vec2& min, const glm::vec2& max, float radius, ImDbgColor color)
    {
        if (!context)
            return;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eTriangles];

        // Define corners
        glm::vec2 top_left = glm::vec2(min.x + radius, min.y + radius);
        glm::vec2 top_right = glm::vec2(max.x - radius, min.y + radius);
        glm::vec2 bottom_left = glm::vec2(min.x + radius, max.y - radius);
        glm::vec2 bottom_right = glm::vec2(max.x - radius, max.y - radius);

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e2D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 0;
        new_draw_cmd.clipRect = CalculateClipRect(min, max.x - min.x, max.y - min.y);

        // Center rectangle
        uint32_t vertex_count{ 0u };
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x + radius, min.y, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x - radius, min.y, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x - radius, max.y, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x + radius, max.y, 0.f), default_uv, color });

        draw_list.indices.emplace_back(vertex_count);
        draw_list.indices.emplace_back(vertex_count + 1);
        draw_list.indices.emplace_back(vertex_count + 2);
        draw_list.indices.emplace_back(vertex_count);
        draw_list.indices.emplace_back(vertex_count + 2);
        draw_list.indices.emplace_back(vertex_count + 3);

        vertex_count += 4;
        new_draw_cmd.indexCount += 6;

        // Side rectangles
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x, min.y + radius, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x + radius, min.y + radius, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x + radius, max.y - radius, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(min.x, max.y - radius, 0.f), default_uv, color });

        draw_list.indices.emplace_back(vertex_count);
        draw_list.indices.emplace_back(vertex_count + 1);
        draw_list.indices.emplace_back(vertex_count + 2);
        draw_list.indices.emplace_back(vertex_count);
        draw_list.indices.emplace_back(vertex_count + 2);
        draw_list.indices.emplace_back(vertex_count + 3);

        vertex_count += 4;
        new_draw_cmd.indexCount += 6;

        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x - radius, min.y + radius, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x, min.y + radius, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x, max.y - radius, 0.f), default_uv, color });
        draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(max.x - radius, max.y - radius, 0.f), default_uv, color });

        draw_list.indices.emplace_back(vertex_count);
        draw_list.indices.emplace_back(vertex_count + 1);
        draw_list.indices.emplace_back(vertex_count + 2);
        draw_list.indices.emplace_back(vertex_count);
        draw_list.indices.emplace_back(vertex_count + 2);
        draw_list.indices.emplace_back(vertex_count + 3);

        vertex_count += 4;
        new_draw_cmd.indexCount += 6;

        // Corners
        const int segments = 8;
        float increment = glm::half_pi<float>() / segments;

        for (int32_t i = 0; i <= segments; ++i)
        {
            float theta = i * increment;
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(top_left - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), 0.f), default_uv, color });
        }

        for (int32_t i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_count);
            draw_list.indices.emplace_back(vertex_count + i);
            draw_list.indices.emplace_back(vertex_count + i + 1);
            new_draw_cmd.indexCount += 3;
        }
        vertex_count += segments;

        for (int32_t i = 0; i <= segments; ++i)
        {
            float theta = i * increment + glm::half_pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(top_right - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), 0.f), default_uv, color });
        }

        for (int32_t i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_count);
            draw_list.indices.emplace_back(vertex_count + i);
            draw_list.indices.emplace_back(vertex_count + i + 1);
            new_draw_cmd.indexCount += 3;
        }
        vertex_count += segments;

        for (int i = 0; i <= segments; ++i)
        {
            float theta = i * increment + glm::pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(bottom_right - radius * glm::vec2(glm::cos(theta), glm::sin(theta)), 0.f), default_uv, color });
        }

        for (int32_t i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_count);
            draw_list.indices.emplace_back(vertex_count + i);
            draw_list.indices.emplace_back(vertex_count + i + 1);
            new_draw_cmd.indexCount += 3;
        }
        vertex_count += segments;

        for (int32_t i = 0; i <= segments; ++i)
        {
            float theta = i * increment + 3.0f * glm::half_pi<float>();
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(bottom_left + radius * glm::vec2(glm::cos(theta), glm::sin(theta)), 0.f), default_uv, color });
        }

        for (int32_t i = 0; i < segments; ++i)
        {
            draw_list.indices.emplace_back(vertex_count);
            draw_list.indices.emplace_back(vertex_count + i);
            draw_list.indices.emplace_back(vertex_count + i + 1);
            new_draw_cmd.indexCount += 3;
        }
        vertex_count += segments;

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawText2D(const glm::vec2& position, const std::string& text, ImDbgColor color)
    {
        if (!context)
            return;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eTriangles];

        auto& font_atlas = context->font->atlases[0]; // Assuming you have only one font atlas

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = text.size() * 6;

        // TODO: add \n support 

        glm::vec3 min{ position, 0.f }, max{ position, 0.f };

        uint32_t vertex_count{ 0u };
        glm::vec2 cursor = position; // Initial position of the text
        for (char c : text)
        {
            if (c == '\n')
            {
                cursor.x = position.x;
                cursor.y -= font_atlas.lineHeight;
            }

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

            // Add vertices for the glyph quad
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(x0, y0, 0.f), glm::vec2(u0, v0), color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(x1, y0, 0.f), glm::vec2(u1, v0), color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(x1, y1, 0.f), glm::vec2(u1, v1), color });
            draw_list.vertices.emplace_back(ImDbgVertex{ glm::vec3(x0, y1, 0.f), glm::vec2(u0, v1), color });

            for (int32_t i = draw_list.vertices.size() - 4; i < draw_list.vertices.size(); ++i)
            {
                const auto& vertex = draw_list.vertices[i];
                min = glm::min(min, vertex.position);
                max = glm::max(max, vertex.position);
            }

            // Add indices for the glyph quad
            draw_list.indices.emplace_back(vertex_count);
            draw_list.indices.emplace_back(vertex_count + 1);
            draw_list.indices.emplace_back(vertex_count + 2);
            draw_list.indices.emplace_back(vertex_count);
            draw_list.indices.emplace_back(vertex_count + 2);
            draw_list.indices.emplace_back(vertex_count + 3);

            vertex_count += 4u;

            cursor.x += glyph.advance.x;
        }

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;

        auto range = max - min;

        new_draw_cmd.clipRect = CalculateClipRect(min, range.x, range.y);
    }

    // For 3d drawings
    void DrawPoint3D(const glm::vec3& position, ImDbgColor color, float size)
    {
        if (!context)
            return;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::ePoints];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e3D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 1;

        draw_list.vertices.emplace_back(ImDbgVertex{ position, glm::vec2(0.0f), color });
        draw_list.indices.emplace_back(0);

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawLine3D(const glm::vec3& from, const glm::vec3& to, ImDbgColor color, float thickness)
    {
        if (!context)
            return;

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e3D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 2;

        draw_list.vertices.emplace_back(ImDbgVertex{ from, glm::vec2(0.0f), color });
        draw_list.vertices.emplace_back(ImDbgVertex{ to, glm::vec2(0.0f), color });
        draw_list.indices.emplace_back(0);
        draw_list.indices.emplace_back(1);

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawBox3D(const glm::vec3& min, const glm::vec3& max, ImDbgColor color)
    {
        if (!context)
            return;

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

        static const uint32_t boxIndices[] =
        {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        auto& draw_list = context->drawData.drawLists[(uint32_t)ImDbgGeometryType::eLines];

        auto& new_draw_cmd = draw_list.drawCommands.emplace_back();
        new_draw_cmd.type = ImDbgDrawType::e3D;
        new_draw_cmd.vertexOffset = draw_list.vertices.size();
        new_draw_cmd.indexOffset = draw_list.indices.size();
        new_draw_cmd.indexCount = 24;

        for (const auto& pos : verticesPos)
            draw_list.vertices.emplace_back(ImDbgVertex{ pos, glm::vec2(0.0f), color });

        for (const auto& idx : boxIndices)
            draw_list.indices.emplace_back(idx);

        context->drawData.totalVertexCount += draw_list.vertices.size() - new_draw_cmd.vertexOffset;
        context->drawData.totalIndexCount += draw_list.indices.size() - new_draw_cmd.indexOffset;
    }

    void DrawSphere3D(const glm::vec3& center, float radius, ImDbgColor color)
    {
        
    }

    void DrawText3D(const glm::vec3& position, const std::string& text, ImDbgColor color)
    {
        
    }

    glm::vec2 GetTextSize2D(const std::string& text)
    {
        auto& font_atlas = context->font->atlases[0];

        glm::vec2 min{}, max{};
        glm::vec2 cursor{};

        for (char c : text)
        {
            if (c == '\n')
            {
                cursor.x = 0.f;
                cursor.y -= font_atlas.lineHeight;
            }

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

            const std::array<glm::vec2, 4ull> vertices{ glm::vec2(x0, y0), glm::vec2(x1, y0), glm::vec2(x1, y1), glm::vec2(x0, y1) };

            for (const auto& vertex : vertices)
            {
                min = glm::min(min, vertex);
                max = glm::max(max, vertex);
            }

            cursor.x += glyph.advance.x;
        }

        return max - min;
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
    glm::vec4 CalculateClipRect(const glm::vec2& start, float width, float height)
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