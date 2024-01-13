#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <udetail.hpp>

namespace engine
{
	namespace render
	{
		class CShaderObject : public IShaderObject
		{
		public:
			CShaderObject(IRenderInterface* render_interface, IRenderContext* render_context);
			~CShaderObject() override;

			void create(const std::string& name, const FGraphicsShaderCreateInfo& create_info) override;
			void create(const std::string& name, const FComputeShaderCreateInfo& create_info) override;

			const std::unique_ptr<CUniformHandle>& getUniformBlock(const char* name) override;
			const std::unique_ptr<CUniformHandle>& getConstantBlock(const char* name) override;

			void bindBuffer(const char* name, size_t buffer_resource) override;
			void bindTexture(const char* name, size_t texture_resource, uint32_t mip = 0u) override;

			void flushDescriptors() override;
			void flushConstants(uint32_t queue) override;

			void bind(uint32_t queue) override;

			size_t handle() const;

			const FShaderReflectionData& getReflection() const override;

		protected:
			void buildReflection();
		private:
			std::unordered_map<size_t, std::unique_ptr<CUniformHandle>> m_uniformBlocks{};
			std::unordered_map<size_t, std::unique_ptr<CUniformHandle>> m_constantBlocks{};

			std::unordered_map<size_t, size_t> m_bindBuffers{};
			std::unordered_map<size_t, FTextureBindInfo> m_bindTextures{};
			size_t m_shaderId{ ~0ull };

			FShaderReflectionData m_reflection{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}