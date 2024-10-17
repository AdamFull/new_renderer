#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CBufferObject : public IBufferObject
		{
		public:
			CBufferObject(IRenderInterface* render_interface);
			~CBufferObject() override;

			void Create(const std::string& name, const FBufferCreateInfo& create_info) override;
			void Resize(const std::string& name, const FBufferCreateInfo& create_info) override;
			void Lock(BufferUpdateInfo& update_info) override;
			void Unlock(BufferUpdateInfo& update_info) override;

			void Map() override;
			void Unmap() override;
			void Write(const void* data, u64 size = INVALID_HANDLE, u64 offset = 0ull) override;
			void Flush(u64 size = INVALID_HANDLE, u64 offset = 0ull) override;
			void Clear() override;

			u64 Size() override;
			u64 InstanceSize() override;
			u64 Count() override;

			FAPIHandle Handle() override;
			uint64_t Address() override;
			FGPUBufferDesc GetDescription() override;

			void* Data() override;

		private:
			FAPIHandle m_bufferId{ INVALID_HANDLE };
			u64 m_elemSize{ 0ull };
			u64 m_elemCount{ 0ull };
			uint64_t m_virtualAddress{ 0ull };
			void* m_pMapped{ nullptr };

			IRenderBackend* m_pRenderContext{ nullptr };
		};
	}
}