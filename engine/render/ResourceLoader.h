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
			FTextureCreateInfo loadTexture(const std::vector<uint8_t>& data, EMimeType mime);
		private:
			FTextureCreateInfo loadSTBFromMemory(const std::vector<uint8_t>& data);
			FTextureCreateInfo loadKTXFromMemory(const std::vector<uint8_t>& data);
			FTextureCreateInfo loadKTX2FromMemory(const std::vector<uint8_t>& data);
			FTextureCreateInfo loadDDSFromMemory(const std::vector<uint8_t>& data);
		};
	}
}