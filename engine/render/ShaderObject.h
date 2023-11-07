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
			void create(FResourceView resource, const FShaderReflectionData& reflection) override;
			const std::unique_ptr<CUniformHandle>& getUniformBlock(const char* name) override;
			void bindTexture(const char* name, FResourceView texture_resource) override;
		private:
			std::unordered_map<size_t, std::unique_ptr<CUniformHandle>> uniformBlocks{};
			std::unordered_map<size_t, FResourceView> bindTextures{};
			FResourceView shaderView{};
		};
	}
}