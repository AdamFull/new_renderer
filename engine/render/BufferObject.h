#pragma once

#include <interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CBufferObject : public IBufferObject
		{
		public:
			CBufferObject(IRenderInterface* render_interface, IRenderContext* render_context);
			~CBufferObject() override;

			void create(class FBufferCreateInfo& create_info) override;

			void map() override;
			void unmap() override;
			void write(void* data, size_t size, size_t offset) override;
			void flush(size_t size, size_t offset) override;

			char* at(size_t index) override;

		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}