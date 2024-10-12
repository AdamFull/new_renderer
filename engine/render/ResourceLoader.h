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
		class CResourceLoader : public IResourceLoader
		{
		public:
			void Initialize(filesystem::IVirtualFileSystemInterface* vfs) override;

			bool LoadTexture(const utl::vector<uint8_t>& data, EMimeType mime, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle) override;
			bool LoadShaderData(const FShaderUploadInfo& load_info, utl::vector<FShaderCreationBlockInfo>& shader_stages) override;
			void DestroyHandle(FTextureResourceHandle& handle) override;
		private:
			bool LoadSTBFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle);
			bool LoadKTXFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle);
			bool LoadKTX2FromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle);
			bool LoadDDSFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle);
		private:
			shader_compiler::FShaderCacheData m_shaderCache{};
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
		};
	}
}