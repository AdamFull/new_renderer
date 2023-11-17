#pragma once

#include <interface/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CVertexHandler : public IVertexHandler
		{
		public:
			CVertexHandler(IRenderInterface* render_interface, IRenderContext* render_context);
			~CVertexHandler() override = default;

			void create() override;

			size_t addVertices(const std::vector<FVertexAttributePointer>& attributes_data) override;
			size_t addIndices(const FVertexAttributePointer& indices) override;

			size_t getLastVertex() override;
			size_t getLastIndex() override;
		private:
			std::vector<uint8_t> m_vertices{};
			size_t m_lastVertex{ 0ull };
			size_t m_vertexSize{ 0ull };

			std::vector<uint8_t> m_indices{};
			size_t m_lastIndex{ 0ull };
			size_t m_indexSize{ 0ull };

			FVertexDeclarationInfo m_declaration{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}