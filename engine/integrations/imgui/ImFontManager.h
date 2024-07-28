#pragma once

#include <reactive/function.h>
#include <design_pattern/pattern.h>

#include <imgui.h>

namespace engine
{
	using read_file_t = utl::reactive::function<bool(const std::string&, utl::vector<u8>&)>;
	class CImFontManager : public utl::pattern::singleton<CImFontManager>
	{
	public:
		void AddFont(const char* name, const std::string& path, f32 size, const ImFontConfig* config = nullptr, const ImWchar* glyph_range = nullptr);
		void AddFont(const char* name, const utl::vector<u8>& bytes, f32 size, const ImFontConfig* config = nullptr, const ImWchar* glyph_range = nullptr);
		ImFont* GetFont(const char* name);

		void SetFileReaderFn(read_file_t&& readerfn);

	private:
		utl::hash_map<u64, ImFont*> m_fonts;
		read_file_t m_fnReadFile;
	};
}