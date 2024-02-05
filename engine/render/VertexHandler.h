#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CVertexHandler : public IVertexHandler
		{
		public:
			struct FBufferInfo
			{
				stl::vector<uint8_t> data{};
				u64 elementCount{ 0ull };
				u64 elementSize{ 0ull };
				u64 buffer{ ~0ull };
			};

			CVertexHandler(IRenderInterface* render_interface);
			~CVertexHandler() override;

			void create(const FVertexHandlerCreateInfo& create_info) override;
			void allocate() override;

			void bind(EQueueTypeFlagBits queue) override;

			u64 addVertices(const stl::vector<FVertexAttributePointer>& attributes_data) override;
			u64 addIndices(const FVertexAttributePointer& indices) override;

			u64 getLastVertex() override;
			u64 getLastIndex() override;

			const FVertexDeclarationInfo& getVertexDeclaration() const override;
			u64 getHash() const override;
			const stl::string& getDefinitions() const override;

			u64 getVertexHandle() const override;
			u64 getIndexHandle() const override;
		private:
			FBufferInfo m_vertexBuffer{};
			FBufferInfo m_indexBuffer{};

			// For shader creation
			stl::string m_definitions{};
			u64 m_attributeHash{ 0ull };
			FVertexDeclarationInfo m_declaration{};

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}