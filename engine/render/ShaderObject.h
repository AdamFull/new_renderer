#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CShaderObject : public IShaderObject
		{
		public:
			CShaderObject(IRenderInterface* render_interface);
			~CShaderObject() override;

			void Create(const std::string& name, const FGraphicsShaderCreateInfo& create_info, const std::vector<FShaderCreationBlockInfo>& shader_stages) override;
			void Create(const std::string& name, const FComputeShaderCreateInfo& create_info, const FShaderCreationBlockInfo& shader_stage) override;

			const std::unique_ptr<CUniformHandle>& GetUniformBlock(u64 name) override;
			const std::unique_ptr<CUniformHandle>& GetConstantBlock(u64 name) override;

			void BindBuffer(u64 name, FAPIHandle buffer_resource) override;
			void BindTexture(u64 name, FAPIHandle texture_resource, u32 mip = ~0u) override;

			void FlushDescriptors() override;
			void FlushConstants(EQueueTypeFlagBits queue) override;

			void Bind(EQueueTypeFlagBits queue) override;
			void BindResources(EQueueTypeFlagBits queue) override;

			FAPIHandle Handle() const override;

			const FShaderReflectionData& GetReflection() const override;

		protected:
			void BuildReflection();
		private:
			utl::hash_map<u64, std::unique_ptr<CUniformHandle>> m_uniformBlocks{};
			utl::hash_map<u64, std::unique_ptr<CUniformHandle>> m_constantBlocks{};

			utl::hash_map<u64, FAPIHandle> m_bindBuffers{};
			utl::hash_map<u64, FTextureBindInfo> m_bindTextures{};
			FAPIHandle m_shaderId{ INVALID_HANDLE };

			FShaderReflectionData m_reflection{};

			IRenderBackend* m_pRenderContext{ nullptr };
		};
	}
}