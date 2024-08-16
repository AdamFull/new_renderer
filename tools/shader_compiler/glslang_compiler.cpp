#include "glslang_compiler.h"

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>

#include <logger/logger.h>

#include <stack>

#include <glslang/build_info.h>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>

#include "compiler_util.h"

using namespace shader_compiler;

// Just copy of https://github.com/KhronosGroup/glslang/blob/main/StandAlone/DirStackFileIncluder.h
class shader_includer : public glslang::TShader::Includer
{
public:
	shader_includer() = default;

	virtual IncludeResult* includeLocal(const char* headerName,
		const char* includerName,
		size_t inclusionDepth) override
	{
		return readLocalPath(headerName, includerName, (int)inclusionDepth);
	}

	virtual IncludeResult* includeSystem(const char* headerName,
		const char* /*includerName*/,
		size_t /*inclusionDepth*/) override
	{
		return readSystemPath(headerName);
	}

	// Externally set directories. E.g., from a command-line -I<dir>.
	//  - Most-recently pushed are checked first.
	//  - All these are checked after the parse-time stack of local directories
	//    is checked.
	//  - This only applies to the "local" form of #include.
	//  - Makes its own copy of the path.
	virtual void pushExternalLocalDirectory(const std::string& dir)
	{
		directoryStack.emplace_back(dir);
		externalLocalDirectoryCount = (int)directoryStack.size();
	}

	virtual void releaseInclude(IncludeResult* result) override
	{
		if (result != nullptr) {
			delete[] static_cast<tUserDataElement*>(result->userData);
			delete result;
		}
	}

	virtual std::set<std::string> getIncludedFiles()
	{
		return includedFiles;
	}

	virtual ~shader_includer() override { }
protected:
	typedef char tUserDataElement;
	std::vector<std::string> directoryStack;
	int externalLocalDirectoryCount;
	std::set<std::string> includedFiles;
	std::array<const char*, 2ull> includeVariants{".glsl", ".h"};

	// Search for a valid "local" path based on combining the stack of include
	// directories and the nominal name of the header.
	virtual IncludeResult* readLocalPath(const char* headerName, const char* includerName, int depth)
	{
		// Discard popped include directories, and
		// initialize when at parse-time first level.
		directoryStack.resize(depth + externalLocalDirectoryCount);
		if (depth == 1)
			directoryStack.back() = getDirectory(includerName);

		// Find a directory that works, using a reverse search of the include stack.
		for (auto it = directoryStack.rbegin(); it != directoryStack.rend(); ++it) 
		{
			for (auto& include_ext : includeVariants)
			{
				std::string path = *it + '/' + headerName + include_ext;
				std::replace(path.begin(), path.end(), '\\', '/');
				std::ifstream file(path, std::ios_base::binary | std::ios_base::ate);
				if (file)
				{
					directoryStack.emplace_back(getDirectory(path));
					includedFiles.insert(path);
					return newIncludeResult(path, file, (int)file.tellg());
				}
			}
		}

		return nullptr;
	}

	// Search for a valid <system> path.
	// Not implemented yet; returning nullptr signals failure to find.
	virtual IncludeResult* readSystemPath(const char* /*headerName*/) const
	{
		return nullptr;
	}

	// Do actual reading of the file, filling in a new include result.
	virtual IncludeResult* newIncludeResult(const std::string& path, std::ifstream& file, int length) const
	{
		char* content = new tUserDataElement[length];
		file.seekg(0, file.beg);
		file.read(content, length);
		return new IncludeResult(path, content, length, content);
	}

	// If no path markers, return current working directory.
	// Otherwise, strip file name and return path leading up to it.
	virtual std::string getDirectory(const std::string path) const
	{
		size_t last = path.find_last_of("/\\");
		return last == std::string::npos ? "." : path.substr(0, last);
	}
};

std::string remove_spaces_around_dot(const std::string& line)
{
	std::string result;
	bool is_last_was_dot{ false };

	for (char c : line)
	{
		if (c == ' ' && is_last_was_dot)
			continue;

		is_last_was_dot = (c == '.');
		result.push_back(c);
	}

	u64 pos = result.find(" .");
	while (pos != std::string::npos)
	{
		result.erase(pos, 1);
		pos = result.find(" .", pos);
	}

	return result;
}

std::string clean_code(const std::string& input)
{
	std::istringstream stream(input);
	std::string cleaned_code, line;
	std::stack<char> block_stack;
	bool was_empty_line = false;

	while (std::getline(stream, line))
	{
		u64 first = line.find_first_not_of(" \t");
		if (first == std::string::npos)
		{
			was_empty_line = true;
			continue;
		}

		u64 last = line.find_last_not_of(" \t");
		line = line.substr(first, (last - first + 1));

		line = remove_spaces_around_dot(line);

		if ((line.find('}') != std::string::npos) && !block_stack.empty())
			block_stack.pop();

		line = std::string(block_stack.size(), '\t') + line;

		if (line.find('{') != std::string::npos)
			block_stack.push('{');

		if (was_empty_line && !cleaned_code.empty())
			cleaned_code += '\n';
		cleaned_code += line + '\n';

		was_empty_line = false;
	}

	return cleaned_code;
}

EShLanguage get_sh_language(EShaderType type)
{
	switch (type)
	{
	case EShaderType::eVertex: return EShLangVertex; break;
	case EShaderType::eControl: return EShLangTessControl; break;
	case EShaderType::eEvaluation: return EShLangTessEvaluation; break;
	case EShaderType::eGeometry: return EShLangGeometry; break;
	case EShaderType::eFragment: return EShLangFragment; break;
	case EShaderType::eCompute: return EShLangCompute; break;
	case EShaderType::eRayGen: return EShLangRayGen; break;
	case EShaderType::eIntersection: return EShLangIntersect; break;
	case EShaderType::eAnyHit: return EShLangAnyHit; break;
	case EShaderType::eClosestHit: return EShLangClosestHit; break;
	case EShaderType::eMiss: return EShLangMiss; break;
	case EShaderType::eCallable: return EShLangCallable; break;
	case EShaderType::eTask: return EShLangTaskNV; break;
	case EShaderType::eMesh: return EShLangMeshNV; break;
	}

	return EShLangCount;
}

TBuiltInResource get_resources()
{
	//auto props = UDevice->GetPhysical().getProperties();
	TBuiltInResource resources = {};
	resources.maxLights = 32;
	resources.maxClipPlanes = 6;
	resources.maxTextureUnits = 32;
	resources.maxTextureCoords = 32;
	resources.maxVertexAttribs = 64;
	resources.maxVertexUniformComponents = 4096;
	resources.maxVaryingFloats = 64;
	resources.maxVertexTextureImageUnits = 32;
	resources.maxCombinedTextureImageUnits = 80;
	resources.maxTextureImageUnits = 32;
	resources.maxFragmentUniformComponents = 4096;
	resources.maxDrawBuffers = 32;
	resources.maxVertexUniformVectors = 128;
	resources.maxVaryingVectors = 8;
	resources.maxFragmentUniformVectors = 16;
	resources.maxVertexOutputVectors = 16;
	resources.maxFragmentInputVectors = 15;
	resources.minProgramTexelOffset = -8;
	resources.maxProgramTexelOffset = 7;
	resources.maxClipDistances = 8;
	resources.maxComputeWorkGroupCountX = 65535;
	resources.maxComputeWorkGroupCountY = 65535;
	resources.maxComputeWorkGroupCountZ = 65535;
	resources.maxComputeWorkGroupSizeX = 1024;
	resources.maxComputeWorkGroupSizeY = 1024;
	resources.maxComputeWorkGroupSizeZ = 64;
	resources.maxComputeUniformComponents = 1024;
	resources.maxComputeTextureImageUnits = 16;
	resources.maxComputeImageUniforms = 8;
	resources.maxComputeAtomicCounters = 8;
	resources.maxComputeAtomicCounterBuffers = 1;
	resources.maxVaryingComponents = 60;
	resources.maxVertexOutputComponents = 64;
	resources.maxGeometryInputComponents = 64;
	resources.maxGeometryOutputComponents = 128;
	resources.maxFragmentInputComponents = 128;
	resources.maxImageUnits = 8;
	resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	resources.maxCombinedShaderOutputResources = 8;
	resources.maxImageSamples = 0;
	resources.maxVertexImageUniforms = 0;
	resources.maxTessControlImageUniforms = 0;
	resources.maxTessEvaluationImageUniforms = 0;
	resources.maxGeometryImageUniforms = 0;
	resources.maxFragmentImageUniforms = 8;
	resources.maxCombinedImageUniforms = 8;
	resources.maxGeometryTextureImageUnits = 16;
	resources.maxGeometryOutputVertices = 256;
	resources.maxGeometryTotalOutputComponents = 1024;
	resources.maxGeometryUniformComponents = 1024;
	resources.maxGeometryVaryingComponents = 64;
	resources.maxTessControlInputComponents = 128;
	resources.maxTessControlOutputComponents = 128;
	resources.maxTessControlTextureImageUnits = 16;
	resources.maxTessControlUniformComponents = 1024;
	resources.maxTessControlTotalOutputComponents = 4096;
	resources.maxTessEvaluationInputComponents = 128;
	resources.maxTessEvaluationOutputComponents = 128;
	resources.maxTessEvaluationTextureImageUnits = 16;
	resources.maxTessEvaluationUniformComponents = 1024;
	resources.maxTessPatchComponents = 120;
	resources.maxPatchVertices = 32;
	resources.maxTessGenLevel = 64;
	resources.maxViewports = 16;
	resources.maxVertexAtomicCounters = 0;
	resources.maxTessControlAtomicCounters = 0;
	resources.maxTessEvaluationAtomicCounters = 0;
	resources.maxGeometryAtomicCounters = 0;
	resources.maxFragmentAtomicCounters = 8;
	resources.maxCombinedAtomicCounters = 8;
	resources.maxAtomicCounterBindings = 1;
	resources.maxVertexAtomicCounterBuffers = 0;
	resources.maxTessControlAtomicCounterBuffers = 0;
	resources.maxTessEvaluationAtomicCounterBuffers = 0;
	resources.maxGeometryAtomicCounterBuffers = 0;
	resources.maxFragmentAtomicCounterBuffers = 1;
	resources.maxCombinedAtomicCounterBuffers = 1;
	resources.maxAtomicCounterBufferSize = 16384;
	resources.maxTransformFeedbackBuffers = 4;
	resources.maxTransformFeedbackInterleavedComponents = 64;
	resources.maxCullDistances = 8;
	resources.maxCombinedClipAndCullDistances = 8;
	resources.maxSamples = 4;
	resources.maxMeshOutputVerticesEXT = resources.maxMeshOutputVerticesNV = 256;
	resources.maxMeshOutputPrimitivesEXT = resources.maxMeshOutputPrimitivesNV = 512;
	resources.maxMeshWorkGroupSizeX_EXT = resources.maxMeshWorkGroupSizeX_NV = 32;
	resources.maxMeshWorkGroupSizeY_EXT = resources.maxMeshWorkGroupSizeY_NV = 1;
	resources.maxMeshWorkGroupSizeZ_EXT = resources.maxMeshWorkGroupSizeZ_NV = 1;
	resources.maxTaskWorkGroupSizeX_EXT = resources.maxTaskWorkGroupSizeX_NV = 32;
	resources.maxTaskWorkGroupSizeY_EXT = resources.maxTaskWorkGroupSizeY_NV = 1;
	resources.maxTaskWorkGroupSizeZ_EXT = resources.maxTaskWorkGroupSizeZ_NV = 1;
	resources.maxMeshViewCountEXT = resources.maxMeshViewCountNV = 4;

	resources.limits.nonInductiveForLoops = true;
	resources.limits.whileLoops = true;
	resources.limits.doWhileLoops = true;
	resources.limits.generalUniformIndexing = true;
	resources.limits.generalAttributeMatrixVectorIndexing = true;
	resources.limits.generalVaryingIndexing = true;
	resources.limits.generalSamplerIndexing = true;
	resources.limits.generalVariableIndexing = true;
	resources.limits.generalConstantMatrixVectorIndexing = true;
	return resources;
}

glslang::EShTargetClientVersion get_client_version(ETargetAPIType eAPI)
{
	switch (eAPI)
	{
	case ETargetAPIType::eVK10: return glslang::EShTargetVulkan_1_0; break;
	case ETargetAPIType::eVK11: return glslang::EShTargetVulkan_1_1; break;
	case ETargetAPIType::eVK12: return glslang::EShTargetVulkan_1_2; break;
	case ETargetAPIType::eVK13: return glslang::EShTargetVulkan_1_3; break;
	case ETargetAPIType::eGL: return glslang::EShTargetOpenGL_450; break;
	default:
		break;
	}
}

glslang::EShClient get_compile_client(ETargetAPIType eAPI)
{
	switch (eAPI)
	{
	case ETargetAPIType::eVK10:
	case ETargetAPIType::eVK11:
	case ETargetAPIType::eVK12:
	case ETargetAPIType::eVK13: return glslang::EShClientVulkan; break;
	case ETargetAPIType::eGL: return glslang::EShClientOpenGL; break;
	default: return glslang::EShClientNone;
	}
}

glslang::EShTargetLanguageVersion get_SPIR_version(ETargetAPIType eAPI)
{
	switch (eAPI)
	{
	case ETargetAPIType::eVK10: return glslang::EShTargetSpv_1_0; break;
	case ETargetAPIType::eVK11: return glslang::EShTargetSpv_1_4; break;
	case ETargetAPIType::eVK12: return glslang::EShTargetSpv_1_5; break;
	case ETargetAPIType::eVK13: return glslang::EShTargetSpv_1_6; break;
	default:
		return glslang::EShTargetSpv_1_0;
	}
}

CGLSLANGCompiler::CGLSLANGCompiler(ETargetAPIType target_api, ETargetBuildType build_type)
{
	this->target_api = target_api;
	this->build_type = build_type;
}

CGLSLANGCompiler::~CGLSLANGCompiler()
{
	glslang::FinalizeProcess();
}

bool CGLSLANGCompiler::initialize()
{
	if (!glslang::InitializeProcess())
	{
		log_error("Failed to initialize glslang processor.");
		return false;
	}

	return true;
}

bool CGLSLANGCompiler::preprocess(std::span<uint8_t> shader_code, const FCompileInfo& info, std::string& preprocessed_code, u32& preprocessed_hash)
{
	auto language = get_sh_language(info.type);
	glslang::TShader shader(language);

	auto resources = get_resources();

	auto messages = utl::cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault);

	bool building_release = build_type == ETargetBuildType::eRelease;
	if (!building_release)
	{
		messages = utl::cast<EShMessages>(messages | EShMsgDebugInfo);
		shader.setDebugInfo(true);
	}

	auto shader_source = reinterpret_cast<const char*>(shader_code.data());
	auto shader_name = info.name.c_str();
	shader.setStringsWithLengthsAndNames(&shader_source, nullptr, &shader_name, 1);
	shader.setPreamble(info.preamble.c_str());
	shader.setOverrideVersion(info.version);

	auto client_version = get_client_version(target_api);
	auto compile_client = get_compile_client(target_api);
	auto spir_version = get_SPIR_version(target_api);
	shader.setEnvInput(glslang::EShSourceGlsl, language, compile_client, 110);
	shader.setEnvClient(compile_client, client_version);
	shader.setEnvTarget(glslang::EShTargetSpv, spir_version);

	// Create includer and push include dirs to it
	shader_includer includer;
	for (auto& dir : info.include_directories)
		includer.pushExternalLocalDirectory(dir.string());

	if (!shader.preprocess(&resources, client_version, ENoProfile, false, false, messages, &preprocessed_code, includer))
	{
		std::lock_guard<std::mutex> lock(m_terminationLock);
		make_shader_compiler_fatal_error(info.name, shader_code, std::format("Preprocessing error: In shader \"{}\" \n{}\n{}\n", info.name, shader.getInfoLog(), shader.getInfoDebugLog()));
		return false;
	}

	preprocessed_code = clean_code(preprocessed_code);
	preprocessed_hash = utl::murmur_hash(preprocessed_code);

	return true;
}

utl::vector<u32> CGLSLANGCompiler::compile(std::span<char> shader_code, const FCompileInfo& info)
{
	std::vector<u32> spirv{};

	auto language = get_sh_language(info.type);

	glslang::TProgram program;
	glslang::TShader shader(language);

	auto resources = get_resources();

	auto messages = utl::cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault);

	bool building_release = build_type == ETargetBuildType::eRelease;
	if (!building_release)
	{
		messages = utl::cast<EShMessages>(messages | EShMsgDebugInfo);
		//shader.setDebugInfo(true);
	}

	auto shader_source = shader_code.data();
	auto shader_name = info.name.c_str();
	shader.setStringsWithLengthsAndNames(&shader_source, nullptr, &shader_name, 1);

	shader.setAutoMapBindings(true);
	shader.setAutoMapLocations(true);

	auto client_version = get_client_version(target_api);
	auto compile_client = get_compile_client(target_api);
	auto spir_version = get_SPIR_version(target_api);
	shader.setEnvInput(glslang::EShSourceGlsl, language, compile_client, 100);
	shader.setEnvClient(compile_client, client_version);
	shader.setEnvTarget(glslang::EShTargetSpv, spir_version);
	shader.setOverrideVersion(info.version);

	if (!shader.parse(&resources, client_version, false, messages))
	{
		std::lock_guard<std::mutex> lock(m_terminationLock);
		log_error("\nParsing error: In shader \"{}\" \n{}\n{}\n", info.name, shader.getInfoLog(), shader.getInfoDebugLog());
		return {};
	} 

	program.addShader(&shader);

	if (!program.link(messages) || !program.mapIO())
	{
		std::lock_guard<std::mutex> lock(m_terminationLock);
		//make_shader_compiler_fatal_error(info.name, shader_code, std::format("Linking error: In shader \"{}\" \n{}\n{}\n", info.name, shader.getInfoLog(), shader.getInfoDebugLog()));
		return {};
	}

	glslang::SpvOptions spvOptions;
	spvOptions.generateDebugInfo = !building_release;
	spvOptions.disableOptimizer = !building_release;
	//spvOptions.emitNonSemanticShaderDebugInfo = !building_release;
	//spvOptions.emitNonSemanticShaderDebugSource = !building_release;
	spvOptions.optimizeSize = building_release;

	spv::SpvBuildLogger logger;
	GlslangToSpv(*program.getIntermediate(language), spirv, &logger, &spvOptions);

	std::string error{};
	logger.error(error);
	if (!error.empty())
	{
		std::lock_guard<std::mutex> lock(m_terminationLock);
		//make_shader_compiler_fatal_error(info.name, shader_code, std::format("SPIRV generation error: In shader \"{}\" \n{}\n", info.name, error));
		return {};
	}

	std::string warning;
	logger.warning(warning);
	if (!warning.empty())
	{
		log_warning("SPIRV generation warnings: In shader \"{}\" \n{}\n", info.name, warning);
	}

	// TODO: i can cross compile spirv inside apis
	//// Cross compilation
	//std::string cross_compiled{};
	//if (target_api == ETargetAPIType::eGL)
	//{
	//	spirv_cross::CompilerGLSL::Options ops;
	//	ops.version = 450;
	//	ops.es = false;
	//
	//	spirv_cross::CompilerGLSL glsl(spirv);
	//	glsl.set_common_options(ops);
	//
	//	cross_compiled = glsl.compile();
	//	
	//}
	//else if (target_api == ETargetAPIType::eDX11 || target_api == ETargetAPIType::eDX12)
	//{
	//	spirv_cross::CompilerHLSL::Options hlslOptions;
	//	hlslOptions.shader_model = target_api == ETargetAPIType::eDX11 ? 500 : 600;
	//
	//	spirv_cross::CompilerHLSL hlsl(spirv);
	//	hlsl.set_hlsl_options(hlslOptions);
	//
	//	cross_compiled = hlsl.compile();
	//}
	//else if (target_api == ETargetAPIType::eMTL)
	//{
	//	spirv_cross::CompilerMSL msl(spirv);
	//
	//	cross_compiled = msl.compile();
	//}
	//
	//// Copy cross compiled data
	//if (!cross_compiled.empty())
	//{
	//	auto out_size = cross_compiled.size() / sizeof(u32);
	//
	//	spirv.resize(out_size);
	//	std::memcpy(spirv.data(), cross_compiled.data(), cross_compiled.size());
	//}

	return utl::vector<u32>(spirv.begin(), spirv.end());
}