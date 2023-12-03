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

		bool load(const std::string& fsPath, FGLTFRoot& root);
	protected:
		bool _load(const std::string& fsPath, FGLTFRoot& root);
		bool _loadGLTFFromMemory(const std::vector<uint8_t>& data, FGLTFRoot& root);
		bool _loadGLBFromMemory(const std::vector<uint8_t>& data, FGLTFRoot& root);
	private:
		std::unique_ptr<IFilesystemWrapper> m_pFS{};
		std::unique_ptr<IDebugMessenger> m_pMessenger{};
	};
}