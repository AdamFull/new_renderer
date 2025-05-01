#pragma once

#include <slang-com-ptr.h>

#include "gfx_shader_compiler.h"

#include <memory>

namespace edge::gfx {
	class EdgeVfsSlangFileSystem;

	class SlangCompilationResult final : public ICompilationResult {
	public:
		SlangCompilationResult(Slang::ComPtr<slang::ISession> session, Slang::ComPtr<slang::IComponentType> program);
		~SlangCompilationResult() override;

		auto collect_result(std::vector<uint8_t>& output) -> bool override;
		auto collect_reflection(ShaderReflection& refl) -> bool override;

	private:
		Slang::ComPtr<slang::ISession> session;
		Slang::ComPtr<slang::IComponentType> linked_program;
	};

	class SlangShaderCompiler final : public IShaderCompiler {
	public:
		SlangShaderCompiler(const ShaderCompilerCreateInfo& create_info);
		~SlangShaderCompiler() override;

		auto collect_dependency_hash(const ShaderCompileInfo& compile_info) -> uint32_t override;
		auto compile(const ShaderCompileInfo& compile_info) -> ICompilationResult* override;
	private:
		EdgeVfsSlangFileSystem* filesystem{ nullptr };
	};
}