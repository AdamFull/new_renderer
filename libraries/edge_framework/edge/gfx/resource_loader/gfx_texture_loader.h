#pragma once

#include "../gfx_structures.h"

namespace edge
{
	namespace gfx
	{
		class ITextureLoader
		{
		public:
			virtual ~ITextureLoader() = default;

			virtual auto Load(std::span<const uint8_t> data, TextureCreateInfo& create_info) -> bool = 0;
			virtual auto IsNeedToGenerateMips() const noexcept -> bool = 0;
			virtual auto GetSize() -> uint64_t = 0;
			virtual auto Upload(ICommandList* cmd_list, IBuffer* src_buffer, uint64_t src_offset, ITexture* dst_texture) -> void = 0;
		};

		auto MakeTextureLoader(std::string_view path) -> std::unique_ptr<ITextureLoader>;
	}
}