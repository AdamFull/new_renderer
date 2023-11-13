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
			~CShaderObject() override = default;
			void create(size_t resource, const FShaderReflectionData& reflection) override;

			const std::unique_ptr<CUniformHandle>& getUniformBlock(const char* name) override;
			const std::unique_ptr<CUniformHandle>& getConstantBlock(const char* name) override;

			void bindTexture(const char* name, size_t texture_resource) override;

			FShaderResourceCommit getResourceCommit() override;
			size_t getId() override;
		private:
			std::unordered_map<size_t, std::unique_ptr<CUniformHandle>> m_uniformBlocks{};
			std::unordered_map<size_t, std::unique_ptr<CUniformHandle>> m_constantBlocks{};

			std::unordered_map<size_t, size_t> m_bindTextures{};
			size_t m_shaderId{ ~0ull };
		};
	}
}