#pragma once

#include <interface/render/RenderInterface.h>
#include <interface/SharedStructure.h>
#include <interface/filesystem/VirtualFileSystemInterface.h>

#include <shader_compiler/shader_library_parse.hpp>
//#include "shader_compiler/shader_library_parse.hpp"

namespace engine
{
	namespace render
	{
		class CResourceLoader
		{
		public:
			void initialize(filesystem::IVirtualFileSystemInterface* vfs);

			bool loadTexture(const std::vector<uint8_t>& data, EMimeType mime, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadShaderData(const FShaderUploadInfo& load_info, std::vector<FShaderCreationBlockInfo>& shader_stages);
		private:
			bool loadSTBFromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadKTXFromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadKTX2FromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadDDSFromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
		private:
			std::unordered_map<int64_t, shader_compiler::FCompiledLibrary> m_shaderLibraries{};
		};
	}
}