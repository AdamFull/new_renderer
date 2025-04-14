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

	bool GLSLShaderCompiler::compile(const ShaderCompileInfo& compile_info, std::vector<uint8_t>& output) {
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
		//	return false;
		//}

		includer->reset();

		if (!shader.parse(GetDefaultResources(), k_client_version, false, messages, *includer)) {
			EDGE_LOGE("\nParsing error: In shader \"{}\" \n{}\n{}\n", compile_info.path, shader.getInfoLog(), shader.getInfoDebugLog());
			return false;
		}

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages) || !program.mapIO()) {
			EDGE_LOGE("\nLinking error: In shader \"{}\" \n{}\n{}\n", compile_info.path, program.getInfoLog(), program.getInfoDebugLog());
			return false;
		}

		auto stage = shader.getStage();
		glslang::TIntermediate* intermediate = program.getIntermediate(stage);

		// Failed to translate to spirv
		if (!intermediate) {
			EDGE_LOGE("Failed translating to spirv.");
			return false;
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


		spv::SpvBuildLogger logger;

		std::vector<uint32_t> spirv_code{};
		GlslangToSpv(*intermediate, spirv_code, &logger, &spvOptions);

		std::string error{};
		logger.error(error);
		if (!error.empty()) {
			EDGE_LOGE("\nSPIRV generation error: In shader \"{}\" \n{}", compile_info.path, error);
			return false;
		}

		std::string warning;
		logger.warning(warning);
		if (!warning.empty()) {
			EDGE_LOGW("SPIRV generation warnings: In shader \"{}\" \n{}\n", compile_info.path, warning);
		}

		output.resize(spirv_code.size() * sizeof(uint32_t));

		uint8_t* begin = reinterpret_cast<uint8_t*>(&spirv_code[0]);
		uint8_t* end = begin + output.size();

		memcpy(output.data(), begin, output.size());
		//std::copy(begin, end, output.begin());

		return true;
	}
}