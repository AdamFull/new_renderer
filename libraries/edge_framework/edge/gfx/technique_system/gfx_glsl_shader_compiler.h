#pragma once

#include "gfx_shader_compiler.h"

#include <memory>

namespace edge::gfx
{
	class shader_includer;

	class GLSLShaderCompiler final : public IShaderCompiler {
	public:
		GLSLShaderCompiler(const ShaderCompilerCreateInfo& create_info);
		~GLSLShaderCompiler() override;

		auto collect_dependency_hash(const ShaderCompileInfo& compile_info) -> uint32_t override;
		auto compile(const ShaderCompileInfo& compile_info, std::vector<uint8_t>& output) -> bool override;
	private:
		std::unique_ptr<shader_includer> includer;
	};
}