#include "Editor.h"

#include <filesystem/VirtualFileSystemManager.h>
#include <filesystem/native/NativeFileSystem.h>

using namespace editor;

void CEditor::create()
{
	m_pVirtualFileSystem = stl::make_unique<engine::filesystem::CVirtualFileSystemManager>();

	m_pVirtualFileSystem->mount("/root", stl::make_unique<engine::filesystem::CNativeFileSystem>("./"));
}

void CEditor::loop()
{

}