#pragma once

#include <interface/RenderInterface.h>
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

			void bindTexture(const char* name, size_t texture_resource) override;

			void flushDescriptors() override;
			void flushConstants(uint32_t queue) override;

			void bind(uint32_t queue) override;

		protected:
			void buildReflection();
		private:
			std::unordered_map<size_t, std::unique_ptr<CUniformHandle>> m_uniformBlocks{};
			std::unordered_map<size_t, std::unique_ptr<CUniformHandle>> m_constantBlocks{};

			std::unordered_map<size_t, size_t> m_bindTextures{};
			size_t m_shaderId{ ~0ull };

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}