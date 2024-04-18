#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/SharedStructure.h>
#include <engine/interface/filesystem/VirtualFileSystemInterface.h>

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

			bool loadTexture(const utl::vector<uint8_t>& data, EMimeType mime, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadShaderData(const FShaderUploadInfo& load_info, utl::vector<FShaderCreationBlockInfo>& shader_stages);
		private:
			bool loadSTBFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadKTXFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadKTX2FromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadDDSFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
		private:
			shader_compiler::FShaderCacheData m_shaderCache{};
		};
	}
}