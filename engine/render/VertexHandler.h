#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <unordered_set>

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
			~CVertexHandler() override;

			void create(const FVertexHandlerCreateInfo& create_info) override;
			void allocate() override;

			void bind(uint32_t queue) override;

			size_t addVertices(const std::vector<FVertexAttributePointer>& attributes_data) override;
			size_t addIndices(const FVertexAttributePointer& indices, size_t vertex_offset) override;

			size_t getLastVertex() override;
			size_t getLastIndex() override;

			const FVertexDeclarationInfo& getVertexDeclaration() const override;
			size_t getHash() const override;
			const std::string& getDefinitions() const override;

			size_t getVertexHandle() const override;
			size_t getIndexHandle() const override;
		private:
			FBufferInfo m_vertexBuffer{};
			FBufferInfo m_indexBuffer{};

			// For shader creation
			std::string m_definitions{};
			size_t m_attributeHash{ 0ull };
			FVertexDeclarationInfo m_declaration{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}