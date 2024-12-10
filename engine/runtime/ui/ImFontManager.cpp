#include "ImFontManager.h"

#include <hash.hpp>

#include <imgui.h>

using namespace engine;

void ImFontManager::AddFont(std::string_view name, std::string_view path, f32 size, const ImFontConfig* config, const ImWchar* glyph_range)
{
	ImGuiIO& io = ImGui::GetIO();

	utl::vector<u8> bytes;
	if (m_fnReadFile && m_fnReadFile(path, bytes))
		AddFont(name, bytes, size, config, glyph_range);
}

void ImFontManager::AddFont(std::string_view name, const utl::vector<u8>& bytes, f32 size, const ImFontConfig* config, const ImWchar* glyph_range)
{
	ImGuiIO& io = ImGui::GetIO();

	auto hash = utl::murmur_hash(name);
	m_fonts[hash] = io.Fonts->AddFontFromMemoryTTF((void*)bytes.data(), bytes.size(), size, config, glyph_range);
}

ImFont* ImFontManager::GetFont(std::string_view name)
{
	auto hash = utl::murmur_hash(name);
	if (auto found = m_fonts.find(hash); found != m_fonts.end())
		return found->second;

	return nullptr;
}

void ImFontManager::SetFileReaderFn(read_file_t&& readerfn)
{
	m_fnReadFile = std::move(readerfn);
}