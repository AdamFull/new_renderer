#pragma once
/*
namespace engine
{
	namespace render
	{
		struct MipGenerator
		{
			gfx::Shader* shader{ nullptr };
			gfx::RootSignature* root_signature{ nullptr };
			gfx::Pipeline* pipeline{ nullptr };
		};

		class UploadContext final : public gfx::ResourceLoader
		{
		public:
			UploadContext(gfx::Context& context, filesystem::IVirtualFileSystemInterface* vfs);
			~UploadContext();

			UploadContext(const UploadContext&) = delete;
			UploadContext(UploadContext&&) = delete;
			UploadContext& operator=(const UploadContext&) = delete;
			UploadContext& operator=(UploadContext&&) = delete;

			gfx::Texture* LoadTexture(const std::string& texture_path) override;

			gfx::Texture* LoadTexture(const gfx::TextureCreateInfo& create_info, std::span<uint8_t> texture_data) override;

			gfx::Shader* LoadShader(const std::string& shader_name) override;

			gfx::Sampler* GetSampler(const gfx::SamplerCreateInfo& create_info) override;

			//void UpdateTexture(gfx::Texture* texture_handle, const FTextureUploadInfo& upload_info);

			//void BeginTextureUpdate(VkTextureUpdateInfo& update_info);
			//void EndTextureUpdate(VkTextureUpdateInfo& update_info);

			void BeginBufferUpdate(gfx::BufferUpdateInfo& update_info) override;
			void EndBufferUpdate(gfx::BufferUpdateInfo& update_info) override;

			gfx::SemaphoreSubmitInfo Submit(gfx::SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count) override;

			gfx::Context& GetContext() override;
			gfx::Context const& GetContext() const override;
		private:
			UploadFrame& GetActiveFrame();
			void BeginFrame();
		private:
			gfx::Context& context;
			filesystem::IVirtualFileSystemInterface* vfs{};

			const gfx::Queue* queue{ nullptr };

			static constexpr size_t k_max_upload_frames{ 8ull };
			std::array<std::unique_ptr<UploadFrame>, k_max_upload_frames> upload_frames{};
			uint32_t upload_frame_count{ 1u };
			uint32_t active_upload_frame{ 0u };

			bool writing_commands{ false };

			std::unordered_map<uint32_t, gfx::Sampler*> sampler_cache{};

			// Shader cache
			shader_compiler::FShaderCacheData shader_cache{};

			std::array<MipGenerator, 4ull> mip_generators{};

			gfx::Sampler* mg_sampler{ nullptr };

			gfx::DescriptorAllocator* mg_descriptor_allocator{ nullptr };
		};
	}
}

*/