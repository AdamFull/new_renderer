#pragma once

#include "gfx_shader_compiler.h"

#include <memory>

namespace edge::gfx
{
	class shader_includer;

	class GLSLCompilationResult final : public ICompilationResult {
	public:
		GLSLCompilationResult();
		~GLSLCompilationResult() override = default;

		auto collect_result(std::vector<uint8_t>& output) -> bool override;
		auto collect_reflection(ShaderReflection& refl) -> bool override;
	
		std::vector<uint32_t> spirv_code{};
	};

	class GLSLShaderCompiler final : public IShaderCompiler {
	public:
		GLSLShaderCompiler(const ShaderCompilerCreateInfo& create_info);
		~GLSLShaderCompiler() override;

		auto collect_dependency_hash(const ShaderCompileInfo& compile_info) -> uint32_t override;
		auto compile(const ShaderCompileInfo& compile_info) -> ICompilationResult* override;
	private:
		std::unique_ptr<shader_includer> includer;
	};
}