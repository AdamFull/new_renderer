#pragma once

#include <interface/RenderInterface.h>
#include <interface/SharedStructure.h>

namespace engine
{
	namespace render
	{
		class CResourceLoader
		{
		public:
			bool loadTexture(const std::vector<uint8_t>& data, EMimeType mime, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
		private:
			bool loadSTBFromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadKTXFromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadKTX2FromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
			bool loadDDSFromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info);
		};
	}
}