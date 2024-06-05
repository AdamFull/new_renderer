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
			~CGPUVector() = default;

			void Upload() override;
			void Resize(u64 new_size) override;
			const std::unique_ptr<IBufferObject>& Buffer() override;
		protected:
			void Allocate(const std::string& name, EBufferUsageFlags flag, u64 stride, u64 capacity) override;
		private:
			std::string m_srOriginalName{};
			EBufferUsageFlags m_bufferFlag{};

			std::unique_ptr<IBufferObject> m_pBuffer{};
		};
	}
}