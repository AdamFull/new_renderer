#include "ImFontManager.h"

#include <hash.hpp>

#include <imgui.h>

using namespace engine;

void CImFontManager::AddFont(const char* name, const std::string& path, f32 size, const ImFontConfig* config, const ImWchar* glyph_range)
{
	ImGuiIO& io = ImGui::GetIO();

	utl::vector<u8> bytes;
	if (m_fnReadFile && m_fnReadFile(path, bytes))
		AddFont(name, bytes, size, config, glyph_range);
}

void CImFontManager::AddFont(const char* name, const utl::vector<u8>& bytes, f32 size, const ImFontConfig* config, const ImWchar* glyph_range)
{
	ImGuiIO& io = ImGui::GetIO();

	auto hash = utl::fnv1a_64_hash_cstr(name);
	m_fonts[hash] = io.Fonts->AddFontFromMemoryTTF((void*)bytes.data(), bytes.size(), size, config, glyph_range);
}

ImFont* CImFontManager::GetFont(const char* name)
{
	auto hash = utl::fnv1a_64_hash_cstr(name);
	if (auto found = m_fonts.find(hash); found != m_fonts.end())
		return found->second;

	return nullptr;
}

void CImFontManager::SetFileReaderFn(read_file_t&& readerfn)
{
	m_fnReadFile = std::move(readerfn);
}