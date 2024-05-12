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

			void create(const std::string& name, const FBufferCreateInfo& create_info) override;
			void resize(const std::string& name, const FBufferCreateInfo& create_info) override;
			void upload(utl::vector<u8>&& data, const utl::vector<FBufferUploadRegion>& regions) override;

			void map() override;
			void unmap() override;
			void write(const void* data, u64 size = INVALID_HANDLE, u64 offset = 0ull) override;
			void flush(u64 size = INVALID_HANDLE, u64 offset = 0ull) override;
			void clear() override;

			u64 size() override;
			u64 instanceSize() override;
			u64 count() override;

			u64 handle() override;
			uint64_t address() override;

			void* data() override;

		private:
			u64 m_bufferId{ INVALID_HANDLE };
			u64 m_elemSize{ 0ull };
			u64 m_elemCount{ 0ull };
			uint64_t m_virtualAddress{ 0ull };
			void* m_pMapped{ nullptr };

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}