#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CGPUVector2 : public IGPUVector2
		{
		public:
			CGPUVector2(IRenderInterface* render_interface);
			~CGPUVector2() = default;

			void upload() override;
			void resize(u64 new_size) override;
			const std::unique_ptr<IBufferObject>& buffer() override;
		protected:
			void allocate(const std::string& name, EBufferUsageFlags flag, u64 stride, u64 capacity) override;
		private:
			std::string m_srOriginalName{};
			EBufferUsageFlags m_bufferFlag{};

			std::unique_ptr<IBufferObject> m_pBuffer{};
		};
	}
}