#pragma once

#include "../gfx_shader_compiler.h"

#include <memory>

namespace edge
{
	namespace gfx
	{
		class shader_includer;

		class GLSLShaderCompiler final : public IShaderCompiler
		{
		public:
			GLSLShaderCompiler(const ShaderCompilerCreateInfo& create_info);
			~GLSLShaderCompiler() override;

			auto CollectDependencyHash(const ShaderCompileInfo& compile_info) -> uint32_t override;
			auto Compile(const ShaderCompileInfo& compile_info, std::vector<uint8_t>& output) -> bool override;
		private:
			filesystem::IVirtualFileSystemInterface* filesystem{ nullptr };

			std::unique_ptr<shader_includer> includer;
		};
	}
}