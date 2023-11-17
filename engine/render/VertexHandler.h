#pragma once

#include <interface/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CVertexHandler : public IVertexHandler
		{
		public:
			struct FBufferInfo
			{
				std::vector<uint8_t> data{};
				size_t elementCount{ 0ull };
				size_t elementSize{ 0ull };
				size_t buffer{ ~0ull };
			};

			CVertexHandler(IRenderInterface* render_interface, IRenderContext* render_context);
			~CVertexHandler() override = default;

			void create() override;

			void bind(uint32_t queue) override;

			size_t addVertices(const std::vector<FVertexAttributePointer>& attributes_data) override;
			size_t addIndices(const FVertexAttributePointer& indices) override;

			size_t getLastVertex() override;
			size_t getLastIndex() override;
		private:
			FBufferInfo m_vertexBuffer{};
			FBufferInfo m_indexBuffer{};

			FVertexDeclarationInfo m_declaration{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}