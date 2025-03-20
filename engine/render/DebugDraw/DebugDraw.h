#pragma once 

#include <standart_library.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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

enum class ImDbgDrawType
{
    e2D,
    e3D
};

struct ImDbgVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    uint32_t color;
};

struct ImDbgDrawCmd
{
    ImDbgDrawType type;

	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t indexCount;

	ImDbgHandle handle;

	glm::vec4 clipRect;
};

struct ImDbgDrawList
{
	ImDbgGeometryType type;

	std::vector<ImDbgVertex> vertices;
	std::vector<ImDbgIndex> indices;

	std::vector<ImDbgDrawCmd> drawCommands;
};

struct ImDbgDrawData
{
	uint32_t totalVertexCount;
	uint32_t totalIndexCount;
	std::vector<ImDbgDrawList> drawLists;
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
    ImDbgDrawData drawData;
	struct FontImpl* font;
};

namespace ImDbg
{
	template<class _Ty>
	class ArrayProxy
	{
	public:
		constexpr ArrayProxy() noexcept : m_data{ nullptr }, m_size{ 0ull }{}
		ArrayProxy(_Ty const& val) noexcept : m_data{ &val }, m_size{ 1ull }{}
		ArrayProxy(uint64_t size, _Ty const* data) : m_data{ data }, m_size{ size }{}

		template<uint64_t _Size>
		ArrayProxy(_Ty const(&arr)[_Size]) noexcept : m_data{ arr }, m_size{ _Size }{}

		ArrayProxy(std::initializer_list<_Ty> const& list) noexcept : m_data{ list.begin() }, m_size{ list.size() }{}

		template <class  _B = _Ty, typename std::enable_if<std::is_const<_B>::value, int>::type = 0>
		ArrayProxy(std::initializer_list<typename std::remove_const<_Ty>::type> const& list) noexcept : m_data(list.begin()), m_size(list.size()){}

		template <typename _V, typename std::enable_if<std::is_convertible<decltype(std::declval<_V>().data()), _Ty*>::value && std::is_convertible<decltype(std::declval<_V>().size()), std::size_t>::value>::type* = nullptr>
		ArrayProxy(_V const& v) noexcept : m_data{ v.data() }, m_size{ v.size() }{}

		const _Ty* begin() const noexcept { return m_data; }
		const _Ty* end() const noexcept { return m_data + m_size; }

		_Ty& operator[](std::size_t pos) noexcept
		{
			assert(pos < m_size);
			return m_data[pos];
		}

		const _Ty& operator[](std::size_t pos) const noexcept
		{
			assert(pos < m_size);
			return m_data[pos];
		}

		const _Ty& front() const noexcept
		{
			assert(m_size && m_data);
			return *m_data;
		}

		const _Ty& back() const noexcept
		{
			assert(m_size && m_data);
			return *(m_data + m_size - 1);
		}

		bool empty() const noexcept { return (m_size == 0); }

		uint64_t size() const noexcept { return m_size; }
		_Ty const* data() const noexcept { return m_data; }
	private:
		const _Ty* m_data{ nullptr };
		uint64_t m_size{ 0ull };
	};

	constexpr uint8_t hex_to_value(char c) 
	{
		return (c >= '0' && c <= '9') ? (c - '0') :
			(c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
			(c >= 'a' && c <= 'f') ? (c - 'a' + 10) : 0;
	}

	constexpr ImDbgColor hex_to_rgba(const char* hex)
	{
		return (hex_to_value(hex[1]) << 20) | (hex_to_value(hex[2]) << 16) |
			(hex_to_value(hex[3]) << 12) | (hex_to_value(hex[4]) << 8) |
			(hex_to_value(hex[5]) << 4) | (hex_to_value(hex[6])) | 0xFF000000;
	}

	constexpr ImDbgColor operator "" _color(const char* hex, std::size_t) { return hex_to_rgba(hex); }

	namespace Color
	{
        constexpr const ImDbgColor AliceBlue = "#F0F8FF"_color;
        constexpr const ImDbgColor AntiqueWhite = "#FAEBD7"_color;
        constexpr const ImDbgColor Aquamarine = "#7FFFD4"_color;
        constexpr const ImDbgColor Azure = "#F0FFFF"_color;
        constexpr const ImDbgColor Beige = "#F5F5DC"_color;
        constexpr const ImDbgColor Bisque = "#FFE4C4"_color;
        constexpr const ImDbgColor Black = "#000000"_color;
        constexpr const ImDbgColor BlanchedAlmond = "#FFEBCD"_color;
        constexpr const ImDbgColor Blue = "#0000FF"_color;
        constexpr const ImDbgColor BlueViolet = "#8A2BE2"_color;
        constexpr const ImDbgColor Brown = "#A52A2A"_color;
        constexpr const ImDbgColor BurlyWood = "#DEB887"_color;
        constexpr const ImDbgColor CadetBlue = "#5F9EA0"_color;
        constexpr const ImDbgColor Chartreuse = "#7FFF00"_color;
        constexpr const ImDbgColor Chocolate = "#D2691E"_color;
        constexpr const ImDbgColor Coral = "#FF7F50"_color;
        constexpr const ImDbgColor CornflowerBlue = "#6495ED"_color;
        constexpr const ImDbgColor Cornsilk = "#FFF8DC"_color;
        constexpr const ImDbgColor Crimson = "#DC143C"_color;
        constexpr const ImDbgColor Cyan = "#00FFFF"_color;
        constexpr const ImDbgColor DarkBlue = "#00008B"_color;
        constexpr const ImDbgColor DarkCyan = "#008B8B"_color;
        constexpr const ImDbgColor DarkGoldenRod = "#B8860B"_color;
        constexpr const ImDbgColor DarkGray = "#A9A9A9"_color;
        constexpr const ImDbgColor DarkGreen = "#006400"_color;
        constexpr const ImDbgColor DarkKhaki = "#BDB76B"_color;
        constexpr const ImDbgColor DarkMagenta = "#8B008B"_color;
        constexpr const ImDbgColor DarkOliveGreen = "#556B2F"_color;
        constexpr const ImDbgColor DarkOrange = "#FF8C00"_color;
        constexpr const ImDbgColor DarkOrchid = "#9932CC"_color;
        constexpr const ImDbgColor DarkRed = "#8B0000"_color;
        constexpr const ImDbgColor DarkSalmon = "#E9967A"_color;
        constexpr const ImDbgColor DarkSeaGreen = "#8FBC8F"_color;
        constexpr const ImDbgColor DarkSlateBlue = "#483D8B"_color;
        constexpr const ImDbgColor DarkSlateGray = "#2F4F4F"_color;
        constexpr const ImDbgColor DarkTurquoise = "#00CED1"_color;
        constexpr const ImDbgColor DarkViolet = "#9400D3"_color;
        constexpr const ImDbgColor DeepPink = "#FF1493"_color;
        constexpr const ImDbgColor DeepSkyBlue = "#00BFFF"_color;
        constexpr const ImDbgColor DimGray = "#696969"_color;
        constexpr const ImDbgColor DodgerBlue = "#1E90FF"_color;
        constexpr const ImDbgColor FireBrick = "#B22222"_color;
        constexpr const ImDbgColor FloralWhite = "#FFFAF0"_color;
        constexpr const ImDbgColor ForestGreen = "#228B22"_color;
        constexpr const ImDbgColor Gainsboro = "#DCDCDC"_color;
        constexpr const ImDbgColor GhostWhite = "#F8F8FF"_color;
        constexpr const ImDbgColor Gold = "#FFD700"_color;
        constexpr const ImDbgColor GoldenRod = "#DAA520"_color;
        constexpr const ImDbgColor Gray = "#808080"_color;
        constexpr const ImDbgColor Green = "#008000"_color;
        constexpr const ImDbgColor GreenYellow = "#ADFF2F"_color;
        constexpr const ImDbgColor HoneyDew = "#F0FFF0"_color;
        constexpr const ImDbgColor HotPink = "#FF69B4"_color;
        constexpr const ImDbgColor IndianRed = "#CD5C5C"_color;
        constexpr const ImDbgColor Indigo = "#4B0082"_color;
        constexpr const ImDbgColor Ivory = "#FFFFF0"_color;
        constexpr const ImDbgColor Khaki = "#F0E68C"_color;
        constexpr const ImDbgColor Lavender = "#E6E6FA"_color;
        constexpr const ImDbgColor LavenderBlush = "#FFF0F5"_color;
        constexpr const ImDbgColor LawnGreen = "#7CFC00"_color;
        constexpr const ImDbgColor LemonChiffon = "#FFFACD"_color;
        constexpr const ImDbgColor LightBlue = "#ADD8E6"_color;
        constexpr const ImDbgColor LightCoral = "#F08080"_color;
        constexpr const ImDbgColor LightCyan = "#E0FFFF"_color;
        constexpr const ImDbgColor LightGoldenRodYellow = "#FAFAD2"_color;
        constexpr const ImDbgColor LightGray = "#D3D3D3"_color;
        constexpr const ImDbgColor LightGreen = "#90EE90"_color;
        constexpr const ImDbgColor LightPink = "#FFB6C1"_color;
        constexpr const ImDbgColor LightSalmon = "#FFA07A"_color;
        constexpr const ImDbgColor LightSeaGreen = "#20B2AA"_color;
        constexpr const ImDbgColor LightSkyBlue = "#87CEFA"_color;
        constexpr const ImDbgColor LightSlateGray = "#778899"_color;
        constexpr const ImDbgColor LightSteelBlue = "#B0C4DE"_color;
        constexpr const ImDbgColor LightYellow = "#FFFFE0"_color;
        constexpr const ImDbgColor Lime = "#00FF00"_color;
        constexpr const ImDbgColor LimeGreen = "#32CD32"_color;
        constexpr const ImDbgColor Linen = "#FAF0E6"_color;
        constexpr const ImDbgColor Magenta = "#FF00FF"_color;
        constexpr const ImDbgColor Maroon = "#800000"_color;
        constexpr const ImDbgColor MediumAquaMarine = "#66CDAA"_color;
        constexpr const ImDbgColor MediumBlue = "#0000CD"_color;
        constexpr const ImDbgColor MediumOrchid = "#BA55D3"_color;
        constexpr const ImDbgColor MediumPurple = "#9370DB"_color;
        constexpr const ImDbgColor MediumSeaGreen = "#3CB371"_color;
        constexpr const ImDbgColor MediumSlateBlue = "#7B68EE"_color;
        constexpr const ImDbgColor MediumSpringGreen = "#00FA9A"_color;
        constexpr const ImDbgColor MediumTurquoise = "#48D1CC"_color;
        constexpr const ImDbgColor MediumVioletRed = "#C71585"_color;
        constexpr const ImDbgColor MidnightBlue = "#191970"_color;
        constexpr const ImDbgColor MintCream = "#F5FFFA"_color;
        constexpr const ImDbgColor MistyRose = "#FFE4E1"_color;
        constexpr const ImDbgColor Moccasin = "#FFE4B5"_color;
        constexpr const ImDbgColor NavajoWhite = "#FFDEAD"_color;
        constexpr const ImDbgColor Navy = "#000080"_color;
        constexpr const ImDbgColor OldLace = "#FDF5E6"_color;
        constexpr const ImDbgColor Olive = "#808000"_color;
        constexpr const ImDbgColor OliveDrab = "#6B8E23"_color;
        constexpr const ImDbgColor Orange = "#FFA500"_color;
        constexpr const ImDbgColor OrangeRed = "#FF4500"_color;
        constexpr const ImDbgColor Orchid = "#DA70D6"_color;
        constexpr const ImDbgColor PaleGoldenRod = "#EEE8AA"_color;
        constexpr const ImDbgColor PaleGreen = "#98FB98"_color;
        constexpr const ImDbgColor PaleTurquoise = "#AFEEEE"_color;
        constexpr const ImDbgColor PaleVioletRed = "#DB7093"_color;
        constexpr const ImDbgColor PapayaWhip = "#FFEFD5"_color;
        constexpr const ImDbgColor PeachPuff = "#FFDAB9"_color;
        constexpr const ImDbgColor Peru = "#CD853F"_color;
        constexpr const ImDbgColor Pink = "#FFC0CB"_color;
        constexpr const ImDbgColor Plum = "#DDA0DD"_color;
        constexpr const ImDbgColor PowderBlue = "#B0E0E6"_color;
        constexpr const ImDbgColor Purple = "#800080"_color;
        constexpr const ImDbgColor RebeccaPurple = "#663399"_color;
        constexpr const ImDbgColor Red = "#FF0000"_color;
        constexpr const ImDbgColor RosyBrown = "#BC8F8F"_color;
        constexpr const ImDbgColor RoyalBlue = "#4169E1"_color;
        constexpr const ImDbgColor SaddleBrown = "#8B4513"_color;
        constexpr const ImDbgColor Salmon = "#FA8072"_color;
        constexpr const ImDbgColor SandyBrown = "#F4A460"_color;
        constexpr const ImDbgColor SeaGreen = "#2E8B57"_color;
        constexpr const ImDbgColor SeaShell = "#FFF5EE"_color;
        constexpr const ImDbgColor Sienna = "#A0522D"_color;
        constexpr const ImDbgColor Silver = "#C0C0C0"_color;
        constexpr const ImDbgColor SkyBlue = "#87CEEB"_color;
        constexpr const ImDbgColor SlateBlue = "#6A5ACD"_color;
        constexpr const ImDbgColor SlateGray = "#708090"_color;
        constexpr const ImDbgColor Snow = "#FFFAFA"_color;
        constexpr const ImDbgColor SpringGreen = "#00FF7F"_color;
        constexpr const ImDbgColor SteelBlue = "#4682B4"_color;
        constexpr const ImDbgColor Tan = "#D2B48C"_color;
        constexpr const ImDbgColor Teal = "#008080"_color;
        constexpr const ImDbgColor Thistle = "#D8BFD8"_color;
        constexpr const ImDbgColor Tomato = "#FF6347"_color;
        constexpr const ImDbgColor Turquoise = "#40E0D0"_color;
        constexpr const ImDbgColor Violet = "#EE82EE"_color;
        constexpr const ImDbgColor Wheat = "#F5DEB3"_color;
        constexpr const ImDbgColor White = "#FFFFFF"_color;
        constexpr const ImDbgColor WhiteSmoke = "#F5F5F5"_color;
        constexpr const ImDbgColor Yellow = "#FFFF00"_color;
        constexpr const ImDbgColor YellowGreen = "#9ACD32"_color;
	}

	void create_context();
	void DestroyContext();

	void LoadFont(const uint8_t* data, uint32_t data_size, uint32_t character_size);
	void GetTexDataAsRGBA32(uint32_t index, uint8_t** out_pixels, int32_t* out_width, int32_t* out_height, int32_t* out_bytes_per_pixel);

	void NewFrame();
	void end_frame();

	// 2d drawings
	void DrawPoint2D(const glm::vec2& position, ImDbgColor color, f32 size = 1.0f);
	void DrawLine2D(const glm::vec2& from, const glm::vec2& to, ImDbgColor color, f32 thickness = 1.0f);
	void DrawRect2D(const glm::vec2& min, const glm::vec2& max, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillRect2D(const glm::vec2& min, const glm::vec2& max, ImDbgColor color);
	void DrawArc2D(const glm::vec2& center, f32 radius, f32 start_angle, f32 end_angle, ImDbgColor color, int32_t segments = 36, f32 thickness = 1.0f);
	void DrawPie2D(const glm::vec2& center, f32 radius, f32 start_angle, f32 end_angle, ImDbgColor color, int32_t segments = 36);
	void DrawRingArc2D(const glm::vec2& center, f32 inner_radius, f32 outer_radius, f32 start_angle, f32 end_angle, ImDbgColor color, int segments = 36);
	void DrawRing2D(const glm::vec2& center, f32 inner_radius, f32 outer_radius, ImDbgColor color, int32_t segments = 36);
	void DrawCircle2D(const glm::vec2& center, f32 radius, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillCircle2D(const glm::vec2& center, f32 radius, ImDbgColor color);
	void DrawTriangle2D(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillTriangle2D(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, ImDbgColor color);
	void DrawPolygon2D(const ArrayProxy<glm::vec2>& points, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillPolygon2D(const ArrayProxy<glm::vec2>& points, ImDbgColor color);
	void DrawRoundedRect2D(const glm::vec2& min, const glm::vec2& max, f32 radius, ImDbgColor color, f32 thickness = 1.0f);
	void DrawFillRoundedRect2D(const glm::vec2& min, const glm::vec2& max, f32 radius, ImDbgColor color);
	void DrawText2D(const glm::vec2& position, const std::string& text, ImDbgColor color);

	// 3d drawings
	void DrawPoint3D(const glm::vec3& position, ImDbgColor color, float size = 0.5f);
	void DrawLine3D(const glm::vec3& from, const glm::vec3& to, ImDbgColor color, float thickness = 0.01f);
	void DrawBox3D(const glm::vec3& min, const glm::vec3& max, ImDbgColor color);
	void DrawSphere3D(const glm::vec3& center, float radius, ImDbgColor color);
	void DrawText3D(const glm::vec3& position, const std::string& text, ImDbgColor color);

    glm::vec2 GetTextSize2D(const std::string& text);

	ImDbgIO* GetIO();
	ImDbgDrawData* GetDrawData();
}