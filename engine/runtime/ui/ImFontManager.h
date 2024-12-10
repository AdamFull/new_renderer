#pragma once

#include <reactive/function.h>
#include <design_pattern/pattern.h>

#include <imgui.h>

namespace engine
{
	using read_file_t = utl::reactive::function<bool(std::string_view, utl::vector<u8>&)>;
	class ImFontManager : public utl::pattern::singleton<ImFontManager>
	{
	public:
		void AddFont(std::string_view name, std::string_view path, f32 size, const ImFontConfig* config = nullptr, const ImWchar* glyph_range = nullptr);
		void AddFont(std::string_view name, const utl::vector<u8>& bytes, f32 size, const ImFontConfig* config = nullptr, const ImWchar* glyph_range = nullptr);
		ImFont* GetFont(std::string_view name);

		void SetFileReaderFn(read_file_t&& readerfn);

	private:
		utl::hash_map<uint32_t, ImFont*> m_fonts;
		read_file_t m_fnReadFile;
	};
}