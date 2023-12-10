#pragma once

#include <engine/interface/render/RenderInterface.h>
#include "BufferObject.h"

namespace engine
{
	namespace render
	{
		template<class _Ty>
		class CGPUArrayHandle
		{
		public:
			CGPUArrayHandle(IRenderInterface* render_interface, IRenderContext* render_context)
			{
				m_pRenderInterface = render_interface;
				m_pRenderContext = render_context;

				m_pBuffer = std::make_unique<CBufferObject>(m_pRenderInterface, m_pRenderContext);
			}

			void create(const std::string& name, size_t count)
			{
				FBufferCreateInfo buffer_ci{};
				buffer_ci.flags = BUFFER_TYPE_STORAGE_FLAG_BIT;
				buffer_ci.size = sizeof(_Ty);
				buffer_ci.count = count;

				m_pBuffer->create(name, buffer_ci);
			}

			void flush()
			{
				m_pBuffer->map();
				for (size_t idx = 0ull; idx < m_elements.size(); ++idx)
				{
					auto& element = m_elements.at(idx);
					auto* dst = m_pBuffer->at(idx);
					std::memcpy(dst, &element, sizeof(_Ty));
				}
				m_pBuffer->unmap();
			}
		private:
			std::vector<_Ty> m_elements{};
			std::unique_ptr<IBufferObject> m_pBuffer{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}