#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CGPUArrayHandle : public IGPUArrayHandle
		{
		public:
			~CGPUArrayHandle() override;

			CGPUArrayHandle(IRenderInterface* render_interface);

			void create(const stl::string& name, u64 stride, u64 count) override;
			void flush(u64 count = ~0ull, u64 offset = 0ull) override;
			void sync(u64 count) override;

			uint8_t* at(u64 index) override;
			const uint8_t* at(u64 index) const override;

			void insert(const void* data, u64 index = 0ull) override;

			uint8_t* emplace_back(const void* data) override;

			uint8_t* back() override;
			const uint8_t* back() const override;

			void clear() override;

			const stl::vector<uint8_t>& getData() const override;
			const stl::unique_ptr<IBufferObject>& getBufferObject() const override;

			u64 size() const override;
			
		private:
			stl::vector<uint8_t> m_elements{};
			stl::unique_ptr<IBufferObject> m_pBuffer{};

			u64 m_elementSize{ 0ull };
			u64 m_elementCount{ 0ull };

			stl::atomic<u64> m_currentElementCount{ 0ull };

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}