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

			void create(const stl::string& name, const FBufferCreateInfo& create_info) override;

			void map() override;
			void unmap() override;
			void write(const void* data, u64 size = ~0ull, u64 offset = 0ull) override;
			void flush(u64 size = ~0ull, u64 offset = 0ull) override;
			void clear() override;

			u64 handle() override;
			uint64_t address() override;

			char* at(u64 index) override;

		private:
			u64 m_bufferId{ ~0ull };
			u64 m_elemSize{ 0ull };
			u64 m_elemCount{ 0ull };
			uint64_t m_virtualAddress{ 0ull };
			void* m_pMapped{ nullptr };

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}