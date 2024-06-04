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
				u64 elementCount{ 0ull };
				u64 elementSize{ 0ull };
			};

			CVertexHandler(IRenderInterface* render_interface);
			~CVertexHandler() override;

			void Create(const FVertexHandlerCreateInfo& create_info) override;
			void Allocate() override;

			void Bind(EQueueTypeFlagBits queue) override;

			const FVertexRange& AddMeshData(const utl::vector<FVertexAttributePointer>& attributes_data, const FVertexAttributePointer& indices) override;

			u64 AddVertices(const utl::vector<FVertexAttributePointer>& attributes_data) override;
			u64 AddVertices(const void* data, u64 count, u64 stride) override;
			u64 AddIndices(const FVertexAttributePointer& indices) override;
			u64 AddIndices(const void* data, u64 count, u64 stride) override;
			u64 addMeshlets(const utl::vector<FVertexAttributePointer>& attributes_data, const utl::vector<u32>& indices);

			const FVertexDeclarationInfo& GetVertexDeclaration() const override;
			u64 GetHash() const override;
			const std::string& GetDefinitions() const override;

			const std::unique_ptr<IGPUVector>& GetGeometryBuffer(EGeometryBufferIndices buffer_index) override;
		private:
			utl::vector<std::unique_ptr<IGPUVector>> m_vGeometryBuffers{};

			// Vertex cache
			utl::hash_map<u64, FVertexRange> m_cachedRanges{};

			// For shader creation
			std::string m_definitions{};
			u64 m_attributeHash{ 0ull };
			EVertexHandlerFlags m_flags{};
			FVertexDeclarationInfo m_declaration{};

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}