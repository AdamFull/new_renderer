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

			void create(const stl::string& name, const FGraphicsShaderCreateInfo& create_info) override;
			void create(const stl::string& name, const FComputeShaderCreateInfo& create_info) override;

			const stl::unique_ptr<CUniformHandle>& getUniformBlock(const char* name) override;
			const stl::unique_ptr<CUniformHandle>& getConstantBlock(const char* name) override;

			void bindBuffer(const char* name, u64 buffer_resource) override;
			void bindTexture(const char* name, u64 texture_resource, u32 mip = 0u) override;

			void flushDescriptors() override;
			void flushConstants(EQueueTypeFlagBits queue) override;

			void bind(EQueueTypeFlagBits queue) override;

			u64 handle() const;

			const FShaderReflectionData& getReflection() const override;

		protected:
			void buildReflection();
		private:
			stl::unordered_map<u64, stl::unique_ptr<CUniformHandle>> m_uniformBlocks{};
			stl::unordered_map<u64, stl::unique_ptr<CUniformHandle>> m_constantBlocks{};

			stl::unordered_map<u64, u64> m_bindBuffers{};
			stl::unordered_map<u64, FTextureBindInfo> m_bindTextures{};
			u64 m_shaderId{ ~0ull };

			FShaderReflectionData m_reflection{};

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}