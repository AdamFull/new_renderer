#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CVertexHandler : public IVertexHandler
		{
		public:
			template<class _Ty>
			struct FBufferInfo
			{
				std::vector<_Ty> data{};
				u64 elementCount{ 0ull };
				u64 elementSize{ 0ull };
				u64 buffer{ ~0ull };
			};

			CVertexHandler(IRenderInterface* render_interface);
			~CVertexHandler() override;

			void create(const FVertexHandlerCreateInfo& create_info) override;
			void allocate() override;

			void bind(EQueueTypeFlagBits queue) override;

			const FVertexRange& addMeshData(const std::vector<FVertexAttributePointer>& attributes_data, const FVertexAttributePointer& indices) override;

			u64 addVertices(const std::vector<FVertexAttributePointer>& attributes_data) override;
			u64 addIndices(const FVertexAttributePointer& indices) override;
			u64 addMeshlets(const std::vector<FVertexAttributePointer>& attributes_data, const FVertexAttributePointer& indices);

			u64 getLastVertex() override;
			u64 getLastIndex() override;

			const FVertexDeclarationInfo& getVertexDeclaration() const override;
			u64 getHash() const override;
			const std::string& getDefinitions() const override;

			const std::unique_ptr<IGPUVector>& getGeometryBuffer(EGeometryBufferIndices buffer_index) override;

			u64 getVertexHandle() const override;
			u64 getIndexHandle() const override;
		private:
			FBufferInfo<u8> m_vertexBuffer{};
			FBufferInfo<u8> m_indexBuffer{};

			std::vector<std::unique_ptr<IGPUVector>> m_vGeometryBuffers{};

			// Vertex cache
			std::unordered_map<u64, FVertexRange> m_cachedRanges{};

			// For shader creation
			std::string m_definitions{};
			u64 m_attributeHash{ 0ull };
			FVertexDeclarationInfo m_declaration{};

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}