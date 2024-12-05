#pragma once

#include "gfx_common.h"
#include "gfx_structures.h"

namespace engine
{
	namespace gfx
	{
		class Context;
	
		class ResourceLoader
		{
		public:
			ResourceLoader(Context& context, FilesystemWrapper& filesystem_wrapper);
			~ResourceLoader();

			ResourceLoader(const ResourceLoader&) = delete;
			ResourceLoader(ResourceLoader&&) = delete;
			ResourceLoader& operator=(const ResourceLoader&) = delete;
			ResourceLoader& operator=(ResourceLoader&&) = delete;

			Texture* LoadTexture(const char* texture_path);

			Texture* LoadTexture(const TextureCreateInfo& create_info, std::span<uint8_t> texture_data);

			Shader* LoadShader(const char* shader_name);

			void BeginBufferUpdate(BufferUpdateInfo& update_info);
			void EndBufferUpdate(BufferUpdateInfo& update_info);

			SemaphoreSubmitInfo Submit(SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count);

			Context& GetContext();
			Context const& GetContext() const;
		private:
			class ResourceUploaderImpl;
			class MipGeneratorImpl;

			ResourceUploaderImpl& GetActiveUploader();
			void BeginCommands();
		private:
			Context& context;
			FilesystemWrapper& fs;

			const gfx::Queue* queue{ nullptr };

			std::array<std::unique_ptr<ResourceUploaderImpl>, 8ull> uploaders{};
			uint32_t uploader_count{ 1u };
			uint32_t active_uploader{ 0u };

			bool is_writing_commands{ false };

			std::unique_ptr<MipGeneratorImpl> mip_generator;
		};
	}
}