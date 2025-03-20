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

		class TextureLoader {
		public:
			virtual ~TextureLoader() = default;

			auto get_texture() -> ITexture*;

			virtual auto upload(ICommandList* cmd_list, StagingBufferAllocator& allocator) -> void = 0;
		protected:
			TextureCreateInfo texture_ci{};
			ITexture* texture{ nullptr };
		};

		auto make_texture_loader(std::string_view path, IContext* gfx_context, fs::VirtualFilesystem& fs) -> std::unique_ptr<TextureLoader>;
	}
}