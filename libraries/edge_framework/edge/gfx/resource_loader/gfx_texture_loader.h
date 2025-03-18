#pragma once

#include "../gfx_structures.h"

namespace edge
{
	namespace fs
	{
		class VirtualFilesystem;
	}

	namespace gfx
	{
		class StagingBufferAllocator;

		class TextureLoader
		{
		public:
			virtual ~TextureLoader() = default;

			auto GetTexture() -> ITexture*;

			virtual auto Upload(ICommandList* cmd_list, StagingBufferAllocator& allocator) -> void = 0;
		protected:
			TextureCreateInfo texture_ci{};
			ITexture* texture{ nullptr };
		};

		auto MakeTextureLoader(std::string_view path, IContext* gfx_context, fs::VirtualFilesystem& fs) -> std::unique_ptr<TextureLoader>;
	}
}