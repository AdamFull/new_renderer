#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CBufferObject : public IBufferObject
		{
		public:
			CBufferObject(IRenderInterface* render_interface, IRenderContext* render_context);
			~CBufferObject() override;

			void create(const std::string& name, const FBufferCreateInfo& create_info) override;

			void map() override;
			void unmap() override;
			void write(void* data, size_t size = ~0ull, size_t offset = 0ull) override;
			void flush(size_t size = ~0ull, size_t offset = 0ull) override;

			char* at(size_t index) override;

		private:
			size_t m_bufferId{ ~0ull };
			size_t m_elemSize{ 0ull };
			size_t m_elemCount{ 0ull };
			void* m_pMapped{ nullptr };

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}