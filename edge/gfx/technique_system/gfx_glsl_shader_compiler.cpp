#include "gfx_glsl_shader_compiler.h"

#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/build_info.h>

#include "../../util/utl_hash.hpp"
#include "../../util/Logger.h"
#include "../../fs/fs_helper.h"
#include "../../fs/fs_virtual_filesystem.h"

#include <set>

#include <spirv_glsl.hpp>

namespace edge::gfx
{
	glslang::EShTargetClientVersion k_client_version{ glslang::EShTargetVulkan_1_1 };
	glslang::EShClient k_client{ glslang::EShClientVulkan };
	glslang::EShTargetLanguageVersion k_language_version{ glslang::EShTargetSpv_1_4 };
	EShMessages k_messages{ EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault | EShMsgCascadingErrors | EShMsgKeepUncalled };

	// Just copy of https://github.com/KhronosGroup/glslang/blob/main/StandAlone/DirStackFileIncluder.h
	class shader_includer : public glslang::TShader::Includer {
	public:
		shader_includer(std::string_view base_path) :
			base_path{ base_path } {

			directoryStack.reserve(16);
		}

		virtual IncludeResult* includeLocal(const char* headerName,
			const char* includerName,
			size_t inclusionDepth) override {

			return readLocalPath(headerName, includerName, (int)inclusionDepth);
		}

		virtual IncludeResult* includeSystem(const char* headerName,
			const char* /*includerName*/,
			size_t /*inclusionDepth*/) override {
			return readSystemPath(headerName);
		}

		// Externally set directories. E.g., from a command-line -I<dir>.
		//  - Most-recently pushed are checked first.
		//  - All these are checked after the parse-time stack of local directories
		//    is checked.
		//  - This only applies to the "local" form of #include.
		//  - Makes its own copy of the path.
		virtual void pushExternalLocalDirectory(const std::string& dir) {
			directoryStack.emplace_back(base_path + '/' + dir);
			externalLocalDirectoryCount = (int)directoryStack.size();
		}

		virtual void releaseInclude(IncludeResult* result) override {
			if (result) {
				delete result;
			}
		}

		uint32_t getDependencyHash() {
			return hash;
		}

		void reset(bool collect_hash = false) {
			this->collect_hash = collect_hash;
			hash = 0;
		}

		virtual ~shader_includer() override { }
	protected:
		std::vector<std::string> directoryStack;
		int externalLocalDirectoryCount;
		std::string base_path;
		std::unordered_map<std::string, std::string> includer_cache{};

		uint32_t hash{ 0u };
		bool collect_hash{ false };

		// Search for a valid "local" path based on combining the stack of include
		// directories and the nominal name of the header.
		virtual IncludeResult* readLocalPath(const char* headerName, const char* includerName, int depth) {
			// Discard popped include directories, and
			// initialize when at parse-time first level.
			directoryStack.resize(depth + externalLocalDirectoryCount);
			if (depth == 1)
				directoryStack.back() = getDirectory(includerName);

			// Find a directory that works, using a reverse search of the include stack.
			for (auto it = directoryStack.rbegin(); it != directoryStack.rend(); ++it) {
				std::string path = *it + '/' + headerName;
				std::replace(path.begin(), path.end(), '\\', '/');

				if (!fs::exists(path)) {
					continue;
				}

				directoryStack.emplace_back(getDirectory(path));

				std::string* file_data{ nullptr };
				if (auto found = includer_cache.find(path); found != includer_cache.end()) {
					file_data = &found->second;
				}
				else {
					file_data = &includer_cache[path];
					if (fs::read_file(path, *file_data)) {
					}
				}

				if (collect_hash) {
					hash ^= murmur_hash(file_data->c_str(), file_data->size(), 0);
				}

				return new IncludeResult(path, reinterpret_cast<const char*>(file_data->data()), file_data->size(), file_data);
			}

			return nullptr;
		}

		// Search for a valid <system> path.
		// Not implemented yet; returning nullptr signals failure to find.
		virtual IncludeResult* readSystemPath(const char* /*headerName*/) const {
			return nullptr;
		}

		// If no path markers, return current working directory.
		// Otherwise, strip file name and return path leading up to it.
		virtual std::string getDirectory(const std::string path) const {
			size_t last = path.find_last_of("/\\");
			return last == std::string::npos ? base_path : path.substr(0, last);
		}
	};

	static uint32_t make_hash_name(std::string_view input) {
		uint32_t hash{ 0u };

		// Current substring start index
		size_t start = 0;
		while (start < input.size()) {
			// Find the next ':' separator or the end of the string
			size_t end = input.find(':', start);

			// Extract the current substring as a view
			std::string_view substring = input.substr(start, (end == std::string_view::npos) ? input.size() - start : end - start);

			// Combine the hash of the substring into the total hash
			hash ^= murmur_hash(substring);

			// Move to the next substring
			if (end == std::string_view::npos) break;
			start = end + 1;
		}

		return hash;
	}

	static EShLanguage get_language(std::string_view path) {
		EShLanguage language{};

		auto ext = fs::get_ext(path);
		if (ext.compare(".vert") == 0)
			language = EShLangVertex;
		else if (ext.compare(".tesc") == 0)
			language = EShLangTessControl;
		else if (ext.compare(".tese") == 0)
			language = EShLangTessEvaluation;
		else if (ext.compare(".geom") == 0)
			language = EShLangGeometry;
		else if (ext.compare(".frag") == 0)
			language = EShLangFragment;
		else if (ext.compare(".comp") == 0)
			language = EShLangCompute;

		return language;
	}

	static glslang::TShader create_shader(const ShaderCompileInfo& compile_info, EShMessages& messages) {
		EShLanguage language = get_language(compile_info.path);

		glslang::TShader shader(language);
		shader.setOverrideVersion(460);
		shader.setEnvInput(glslang::EShSourceGlsl, language, k_client, 110);
		shader.setEnvClient(k_client, k_client_version);
		shader.setEnvTarget(glslang::EShTargetSpv, k_language_version);
		//shader.setAutoMapBindings(true);

		messages = k_messages;
#ifndef NDEBUG
		messages = static_cast<EShMessages>(messages | EShMsgDebugInfo);
		//shader.setDebugInfo(true);
#endif

		return shader;
	}


	auto find_shader_resource_by_name_hash(ShaderReflection& reflection, uint32_t name_hash) {
		return std::find_if(reflection.shader_resources.begin(), reflection.shader_resources.end(),
			[&name_hash](const ShaderResource& resource) {
				return resource.name_hash == name_hash;
			});
	}

	void collect_execution_modes(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, uint32_t index) {
		auto mode = static_cast<spv::ExecutionMode>(index);
		switch (mode) {
		case spv::ExecutionMode::ExecutionModeInvocations:
			reflection.invocation_count = compiler->get_execution_mode_argument(mode, 0);
			break;
		case spv::ExecutionMode::ExecutionModeLocalSize: {
			for (int32_t dim = 0; dim < 3; ++dim)
				reflection.local_sizes[dim] = compiler->get_execution_mode_argument(mode, dim);
		} break;
		case spv::ExecutionMode::ExecutionModeOutputVertices:
			reflection.output_vertex_count = compiler->get_execution_mode_argument(mode, 0);
			break;
		}
	}

	void collect_uniform_block_reflection(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler,
		const spirv_cross::Resource& res, ShaderStageFlag stage, DescriptorType type) {

		uint32_t name_hash = murmur_hash(res.name);
		auto found = find_shader_resource_by_name_hash(reflection, name_hash);
		if (found != reflection.shader_resources.end())
		{
			found->stages |= stage;
			return;
		}

		const auto& refl_type = compiler->get_type(res.type_id);
		auto& decoration = compiler->get_decoration_bitset(refl_type.self);
		uint32_t member_count = static_cast<uint32_t>(refl_type.member_types.size());

		ShaderResource& uniform_block = reflection.add_shader_resource();

		std::strcpy(uniform_block.name, res.name.c_str());
		uniform_block.name_hash = name_hash;
		uniform_block.set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
		uniform_block.binding = compiler->get_decoration(res.id, spv::DecorationBinding);
		uniform_block.stages |= stage;
		uniform_block.type = type;
		uniform_block.size = static_cast<uint32_t>(compiler->get_declared_struct_size(refl_type));

		// Add internal variables if have them
		if (member_count > 0) {
			uniform_block.range_begin = reflection.shader_variables.size();

			for (int32_t i = 0; i < static_cast<int32_t>(member_count); ++i) {
				uint64_t array_stride{ 0ull }, matrix_stride{ 0ull };

				auto& member_type = compiler->get_type(refl_type.member_types[i]);
				ShaderVariable& variable = reflection.add_shader_variable();
				variable.size = static_cast<uint32_t>(compiler->get_declared_struct_member_size(refl_type, i));
				variable.offset = static_cast<uint32_t>(compiler->type_struct_member_offset(refl_type, i));

				auto& variable_name = compiler->get_member_name(refl_type.self, i);
				std::strcpy(variable.name, variable_name.c_str());
				variable.name_hash = murmur_hash(variable_name);
			}

			uniform_block.variable_count = reflection.shader_variables.size();
		}
	}

	void collect_uniform_reflection(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, ShaderStageFlag stage, DescriptorType type) {
		uint32_t name_hash = murmur_hash(res.name);
		auto found = find_shader_resource_by_name_hash(reflection, name_hash);
		if (found != reflection.shader_resources.end()) {
			found->stages |= stage;
			return;
		}

		ShaderResource& uniform = reflection.add_shader_resource();

		std::strcpy(uniform.name, res.name.c_str());
		uniform.name_hash = name_hash;
		uniform.set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
		uniform.binding = compiler->get_decoration(res.id, spv::DecorationBinding);
		uniform.type = type;
		uniform.stages = stage;

		auto type_handle = compiler->get_type(res.type_id);
		if (!type_handle.array.empty()) {
			for (auto dim : type_handle.array) {
				uniform.count *= dim;
			}
		}
	}

	void collect_constants_reflection(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, ShaderStageFlag stage) {
		const auto& type = compiler->get_type(res.type_id);
		unsigned member_count = static_cast<uint32_t>(type.member_types.size());

		auto& constants = reflection.constants;

		if (!constants.name_hash) {
			std::strcpy(constants.name, res.name.c_str());
			constants.name_hash = murmur_hash(res.name);
		}

		constants.size = std::max(constants.size, static_cast<uint32_t>(compiler->get_declared_struct_size(type)));
		constants.stages |= stage;

		if (member_count > 0) {
			constants.range_begin = reflection.shader_variables.size();

			for (int32_t i = 0; i < static_cast<int32_t>(member_count); ++i) {
				uint64_t array_stride{ 0ull }, matrix_stride{ 0ull };

				auto& member_type = compiler->get_type(type.member_types[i]);
				ShaderVariable& variable = reflection.add_shader_variable();
				variable.size = static_cast<uint32_t>(compiler->get_declared_struct_member_size(type, i));
				variable.offset = static_cast<uint32_t>(compiler->type_struct_member_offset(type, i));

				auto& variable_name = compiler->get_member_name(type.self, i);
				std::strcpy(variable.name, variable_name.c_str());
				variable.name_hash = murmur_hash(variable_name);
			}

			constants.variable_count = reflection.shader_variables.size();
		}
	}

	void collect_vertex_inputs(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res) {
		VertexInput& vertex_input = reflection.add_vertex_input();

		std::strcpy(vertex_input.name, res.name.c_str());
		vertex_input.name_hash = murmur_hash(res.name);
		vertex_input.location = compiler->get_decoration(res.id, spv::DecorationLocation);

		spirv_cross::SPIRType type = compiler->get_type(res.type_id);
		vertex_input.vec_size = type.vecsize;
		vertex_input.columns = type.columns;
		vertex_input.size = 4u * vertex_input.vec_size * vertex_input.columns;
	}

	auto get_shader_stage(spv::ExecutionModel exec_model) -> ShaderStageFlag {
		switch (exec_model)
		{
		case spv::ExecutionModelVertex: return ShaderStageFlag::eVertex;
		case spv::ExecutionModelTessellationControl: return ShaderStageFlag::eTessellationControl;
		case spv::ExecutionModelTessellationEvaluation: return ShaderStageFlag::eTessellationEvaluation;
		case spv::ExecutionModelGeometry: return ShaderStageFlag::eGeometry;
		case spv::ExecutionModelFragment: return ShaderStageFlag::eFragment;
		case spv::ExecutionModelGLCompute: return ShaderStageFlag::eCompute;
		default: return (ShaderStageFlag)0;
		}
	}

	GLSLCompilationResult::GLSLCompilationResult() {
	}

	auto GLSLCompilationResult::collect_result(std::vector<uint8_t>& output) -> bool {
		output.resize(spirv_code.size() * sizeof(uint32_t));

		uint8_t* begin = reinterpret_cast<uint8_t*>(&spirv_code[0]);
		uint8_t* end = begin + output.size();
		memcpy(output.data(), begin, output.size());
		//std::copy(begin, end, output.begin());

		return true;
	}

	auto GLSLCompilationResult::collect_reflection(ShaderReflection& refl) -> bool {
		spirv_cross::CompilerGLSL compiler(spirv_code);
		//auto active = compiler.get_active_interface_variables();
		//auto resources = compiler.get_shader_resources(active);
		//compiler.set_enabled_interface_variables(std::move(active));
		auto resources = compiler.get_shader_resources();

		// Parse execution modes
		auto& execution_modes = compiler.get_execution_mode_bitset();
		execution_modes.for_each_bit([&](uint32_t i) { collect_execution_modes(refl, &compiler, i); });

		auto entry_point = compiler.get_entry_points_and_stages()[0];
		auto stage = get_shader_stage(entry_point.execution_model);

		// Parse uniform buffers
		for (const auto& res : resources.uniform_buffers) {
			collect_uniform_block_reflection(refl, &compiler, res, stage, DescriptorType::eUniformBuffer);
		}

		// Parse storage buffers
		for (const auto& res : resources.storage_buffers) {
			collect_uniform_block_reflection(refl, &compiler, res, stage, DescriptorType::eStorageBuffer);
		}

		// Parse push constants blocks
		for (const auto& res : resources.push_constant_buffers) {
			collect_constants_reflection(refl, &compiler, res, stage);
		}

		for (const auto& res : resources.sampled_images) {
			collect_uniform_reflection(refl, &compiler, res, stage, DescriptorType::eCombinedImageSampler);
		}

		for (const auto& res : resources.separate_images) {
			collect_uniform_reflection(refl, &compiler, res, stage, DescriptorType::eSampledImage);
		}

		for (const auto& res : resources.separate_samplers) {
			collect_uniform_reflection(refl, &compiler, res, stage, DescriptorType::eSampler);
		}

		for (const auto& res : resources.storage_images) {
			collect_uniform_reflection(refl, &compiler, res, stage, DescriptorType::eStorageImage);
		}

		for (const auto& res : resources.subpass_inputs) {
			collect_uniform_reflection(refl, &compiler, res, stage, DescriptorType::eInputAttachment);
		}

		if (stage == ShaderStageFlag::eVertex) {
			for (const auto& res : resources.stage_inputs) {
				collect_vertex_inputs(refl, &compiler, res);
			}
		}

		std::unordered_set<uint32_t> descriptor_sets{};

		std::sort(refl.shader_resources.begin(), refl.shader_resources.end(),
			[&descriptor_sets](const ShaderResource& a, const ShaderResource& b) {
				descriptor_sets.emplace(a.set);

				// First compare by set
				if (a.set != b.set) {
					return a.set < b.set;
				}

				// If sets are equal, compare by binding
				return a.binding < b.binding;
			});

		return true;
	}

	GLSLShaderCompiler::GLSLShaderCompiler(const ShaderCompilerCreateInfo& create_info) {
		if (!glslang::InitializeProcess()) {
			EDGE_LOGE("Failed to initialize glslang process.");
			return;
		}

		includer = std::make_unique<shader_includer>(create_info.base_path);

		for (std::string_view include_dir : create_info.additional_include_dirs)
			includer->pushExternalLocalDirectory(include_dir.data());
	}

	GLSLShaderCompiler::~GLSLShaderCompiler() {
		glslang::FinalizeProcess();
	}

	auto GLSLShaderCompiler::collect_dependency_hash(const ShaderCompileInfo& compile_info) -> uint32_t {
		EShMessages messages{ };
		auto shader = create_shader(compile_info, messages);

		auto shader_name_cstr = compile_info.path.data();
		auto shader_source_cstr = compile_info.source.data();
		shader.setStringsWithLengthsAndNames(&shader_source_cstr, nullptr, &shader_name_cstr, 1);

		// Create preamble
		std::stringstream preamble_ss;

		// Include extention is required by default
		preamble_ss << "#extension GL_GOOGLE_include_directive : require" << std::endl;

		for (auto& definition : compile_info.definitions)
			preamble_ss << std::format("#define {}", definition) << std::endl;

		auto preamble = preamble_ss.str();
		shader.setPreamble(preamble.c_str());

		includer->reset(true);

		if (!shader.parse(GetDefaultResources(), k_client_version, false, messages, *includer)) {
			EDGE_LOGE("\nParsing error: In shader \"{}\" \n{}\n{}\n", compile_info.path, shader.getInfoLog(), shader.getInfoDebugLog());
			return {};
		}

		return includer->getDependencyHash();
	}

	auto GLSLShaderCompiler::compile(const ShaderCompileInfo& compile_info) -> ICompilationResult* {
		EShMessages messages{};
		auto shader = create_shader(compile_info, messages);

		auto shader_name_cstr = compile_info.path.data();
		auto shader_source_cstr = compile_info.source.data();
		shader.setStringsWithLengthsAndNames(&shader_source_cstr, nullptr, &shader_name_cstr, 1);

		// Create preamble
		std::stringstream preamble_ss;

		// Include extention is required by default
		preamble_ss << "#extension GL_GOOGLE_include_directive : require" << std::endl;

		for (auto& definition : compile_info.definitions)
			preamble_ss << std::format("#define {}", definition) << std::endl;

		auto preamble = preamble_ss.str();
		shader.setPreamble(preamble.c_str());

		//if (!shader.preprocess(&resources, k_client_version, ENoProfile, false, false, messages, &preprocessed_code, *includer))
		//{
		//	EDGE_LOGE("\nPreprocess error: In shader \"{}\" \n{}\n{}\n", compile_info.path, shader.getInfoLog(), shader.getInfoDebugLog());
		//	return nullptr;
		//}

		includer->reset();

		if (!shader.parse(GetDefaultResources(), k_client_version, false, messages, *includer)) {
			EDGE_LOGE("\nParsing error: In shader \"{}\" \n{}\n{}\n", compile_info.path, shader.getInfoLog(), shader.getInfoDebugLog());
			return nullptr;
		}

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages) || !program.mapIO()) {
			EDGE_LOGE("\nLinking error: In shader \"{}\" \n{}\n{}\n", compile_info.path, program.getInfoLog(), program.getInfoDebugLog());
			return nullptr;
		}

		auto stage = shader.getStage();
		glslang::TIntermediate* intermediate = program.getIntermediate(stage);

		// Failed to translate to spirv
		if (!intermediate) {
			EDGE_LOGE("Failed translating to spirv.");
			return nullptr;
		}

		glslang::SpvOptions spvOptions;
#ifndef NDEBUG
		spvOptions.generateDebugInfo = true;
		spvOptions.disableOptimizer = true;
		//spvOptions.emitNonSemanticShaderDebugInfo = true;
		//spvOptions.emitNonSemanticShaderDebugSource = true;
		spvOptions.optimizeSize = false;
#else
		spvOptions.generateDebugInfo = false;
		spvOptions.disableOptimizer = false;
		spvOptions.optimizeSize = true;
#endif

		auto* compilation_result = new GLSLCompilationResult();

		spv::SpvBuildLogger logger;
		GlslangToSpv(*intermediate, compilation_result->spirv_code, &logger, &spvOptions);

		std::string error{};
		logger.error(error);
		if (!error.empty()) {
			EDGE_LOGE("\nSPIRV generation error: In shader \"{}\" \n{}", compile_info.path, error);
			delete compilation_result;
			return nullptr;
		}

		std::string warning;
		logger.warning(warning);
		if (!warning.empty()) {
			EDGE_LOGW("SPIRV generation warnings: In shader \"{}\" \n{}\n", compile_info.path, warning);
		}

		return compilation_result;
	}
}