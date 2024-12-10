#pragma once

#include <include/easygltf/easygltf_base.h>

namespace easygltf
{
	class CGLTFLoader
	{
	public:
		void useDefaultFilesystemWrapper();
		void setFilesystemWrapper(std::unique_ptr<IFilesystemWrapper>&& wrapper);

		void useDefaultDebugMessenger();
		void setDebugMessenger(std::unique_ptr<IDebugMessenger>&& messenger);

		bool load(std::string_view fsPath, FGLTFRoot& root);
	protected:
		bool _load(std::string_view fsPath, FGLTFRoot& root);
		bool _LoadGLTFFromMemory(const utl::vector<uint8_t>& data, FGLTFRoot& root);
		bool _loadGLBFromMemory(const utl::vector<uint8_t>& data, FGLTFRoot& root);
	private:
		std::unique_ptr<IFilesystemWrapper> m_pFS{};
		std::unique_ptr<IDebugMessenger> m_pMessenger{};
	};
}