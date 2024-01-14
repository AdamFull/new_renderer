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

			void create(const std::string& name, size_t stride, size_t count) override;
			void flush(size_t count = ~0ull, size_t offset = 0ull) override;
			void sync(size_t count) override;

			uint8_t* at(size_t index) override;
			const uint8_t* at(size_t index) const override;

			void insert(const void* data, size_t index = 0ull) override;

			uint8_t* emplace_back(const void* data) override;

			uint8_t* back() override;
			const uint8_t* back() const override;

			void clear() override;

			const std::vector<uint8_t>& getData() const override;
			const std::unique_ptr<IBufferObject>& getBufferObject() const override;

			size_t size() const override;
			
		private:
			std::vector<uint8_t> m_elements{};
			std::unique_ptr<IBufferObject> m_pBuffer{};

			size_t m_elementSize{ 0ull };
			size_t m_elementCount{ 0ull };

			std::atomic<size_t> m_currentElementCount{ 0ull };

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}