#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CGPUVector : public IGPUVector
		{
		public:
			CGPUVector(IRenderInterface* render_interface);
			~CGPUVector();

			void resize(u64 new_size) override;

			u64 capacity() override;

			const stl::unique_ptr<IBufferObject>& buffer() override;
		protected:
			void alloc(const std::string& name, EBufferUsageFlags flag, u64 stride, u64 capacity) override;
			void realloc(u64 capacity) override;

			void* at(u64 index) override;
			const void* at(u64 index) const override;
			
		private:
			std::string m_srOriginalName{};
			EBufferUsageFlags m_bufferFlag{};

			stl::unique_ptr<IBufferObject> m_pBuffer{};
		};
	}
}