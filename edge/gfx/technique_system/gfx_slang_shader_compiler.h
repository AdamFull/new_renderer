#pragma once

#include "gfx_shader_compiler.h"

#include <memory>

namespace edge::gfx {
	class EdgeVfsSlangFileSystem;

	class SlangShaderCompiler final : public IShaderCompiler {
	public:
		SlangShaderCompiler(const ShaderCompilerCreateInfo& create_info);
		~SlangShaderCompiler() override;

		auto collect_dependency_hash(const ShaderCompileInfo& compile_info) -> uint32_t override;
		auto compile(const ShaderCompileInfo& compile_info, std::vector<uint8_t>& output) -> bool override;
	private:
		EdgeVfsSlangFileSystem* filesystem{ nullptr };
	};
}