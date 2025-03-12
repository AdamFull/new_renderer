#include "gfx_glsl_shader_compiler.h"

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/build_info.h>

#include "../../../../util/Hash.hpp"
#include "../../../../util/Logger.h"
#include "../../../../fs/fs_helper.h"
#include "../../../../fs/fs_virtual_filesystem.h"

#include <set>

namespace edge
{
	namespace gfx
	{
		glslang::EShTargetClientVersion k_client_version{ glslang::EShTargetVulkan_1_1 };
		glslang::EShClient k_client{ glslang::EShClientVulkan };
		glslang::EShTargetLanguageVersion k_language_version{ glslang::EShTargetSpv_1_4 };
		EShMessages k_messages{ EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault | EShMsgCascadingErrors };

		// Just copy of https://github.com/KhronosGroup/glslang/blob/main/StandAlone/DirStackFileIncluder.h
		class shader_includer : public glslang::TShader::Includer
		{
		public:
			shader_includer(fs::VirtualFilesystem* fs, std::string_view base_path) :
				fs{ fs },
				base_path{ base_path }
			{
				directoryStack.reserve(16);
			}

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
				directoryStack.emplace_back(base_path + '/' + dir);
				externalLocalDirectoryCount = (int)directoryStack.size();
			}

			virtual void releaseInclude(IncludeResult* result) override
			{
				if (result != nullptr)
				{
					delete static_cast<std::string*>(result->userData);
					delete result;
				}
			}

			uint32_t getDependencyHash()
			{
				return hash;
			}

			void reset(bool collect_hash = false)
			{
				this->collect_hash = collect_hash;
				hash = 0;
			}

			virtual ~shader_includer() override { }
		protected:
			std::vector<std::string> directoryStack;
			int externalLocalDirectoryCount;
			fs::VirtualFilesystem* fs;
			std::string base_path;

			uint32_t hash{ 0u };
			bool collect_hash{ false };

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
					std::string path = *it + '/' + headerName;
					std::replace(path.begin(), path.end(), '\\', '/');

					std::string* file_data = new std::string;
					if (fs->Exists(path) && fs->ReadFile(path, *file_data))
					{
						directoryStack.emplace_back(getDirectory(path));

						if (collect_hash)
						{
							hash ^= murmur_hash(file_data->c_str(), file_data->size(), 0);
						}

						return new IncludeResult(path, reinterpret_cast<const char*>(file_data->data()), file_data->size(), file_data);
					}

					delete file_data;
				}

				return nullptr;
			}

			// Search for a valid <system> path.
			// Not implemented yet; returning nullptr signals failure to find.
			virtual IncludeResult* readSystemPath(const char* /*headerName*/) const
			{
				return nullptr;
			}

			// If no path markers, return current working directory.
			// Otherwise, strip file name and return path leading up to it.
			virtual std::string getDirectory(const std::string path) const
			{
				size_t last = path.find_last_of("/\\");
				return last == std::string::npos ? base_path : path.substr(0, last);
			}
		};
		
		static TBuiltInResource GetResources()
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

		static uint32_t MakeHashName(std::string_view input)
		{
			uint32_t hash{ 0u };

			// Current substring start index
			size_t start = 0;
			while (start < input.size())
			{
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

		static EShLanguage GetLanguage(std::string_view path)
		{
			EShLanguage language{};

			auto ext = fs::GetExt(path);
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

		static glslang::TShader CreateShader(const ShaderCompileInfo& compile_info, EShMessages& messages)
		{
			EShLanguage language = GetLanguage(compile_info.path);

			glslang::TShader shader(language);
			shader.setOverrideVersion(460);
			shader.setEnvInput(glslang::EShSourceGlsl, language, k_client, 110);
			shader.setEnvClient(k_client, k_client_version);
			shader.setEnvTarget(glslang::EShTargetSpv, k_language_version);

			messages = k_messages;
#ifndef NDEBUG
			messages = static_cast<EShMessages>(messages | EShMsgDebugInfo);
			//shader.setDebugInfo(true);
#endif

			return shader;
		}

		GLSLShaderCompiler::GLSLShaderCompiler(const ShaderCompilerCreateInfo& create_info) :
			filesystem{ create_info.filesystem }
		{
			if (!glslang::InitializeProcess())
			{
				EDGE_LOGE("Failed to initialize glslang process.");
			}

			includer = std::make_unique<shader_includer>(filesystem, create_info.base_path);

			for(std::string_view include_dir : create_info.additional_include_dirs)
				includer->pushExternalLocalDirectory(include_dir.data());

		}

		GLSLShaderCompiler::~GLSLShaderCompiler()
		{
			glslang::FinalizeProcess();
		}

		auto GLSLShaderCompiler::CollectDependencyHash(const ShaderCompileInfo& compile_info) -> uint32_t
		{
			EShMessages messages{};
			auto shader = CreateShader(compile_info, messages);

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

			auto resources = GetResources();
			if (!shader.parse(&resources, k_client_version, false, messages, *includer))
			{
				EDGE_LOGE("\nParsing error: In shader \"{}\" \n{}\n{}\n", compile_info.path, shader.getInfoLog(), shader.getInfoDebugLog());
				return {};
			}

			return includer->getDependencyHash();
		}

		bool GLSLShaderCompiler::Compile(const ShaderCompileInfo& compile_info, std::vector<uint8_t>& output)
		{
			EShMessages messages{};
			auto shader = CreateShader(compile_info, messages);

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

			auto resources = GetResources();
			if (!shader.parse(&resources, k_client_version, false, messages, *includer))
			{
				EDGE_LOGE("\nParsing error: In shader \"{}\" \n{}\n{}\n", compile_info.path, shader.getInfoLog(), shader.getInfoDebugLog());
				return false;
			}

			glslang::TProgram program;
			program.addShader(&shader);

			if (!program.link(messages) || !program.mapIO())
			{
				EDGE_LOGE("\nLinking error: In shader \"{}\" \n{}\n{}\n", compile_info.path, program.getInfoLog(), program.getInfoDebugLog());
				return false;
			}

			auto stage = shader.getStage();
			glslang::TIntermediate* intermediate = program.getIntermediate(stage);

			// Failed to translate to spirv
			if (!intermediate)
			{
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
			if (!error.empty())
			{
				EDGE_LOGE("\nSPIRV generation error: In shader \"{}\" \n{}", compile_info.path, error);
				return false;
			}

			std::string warning;
			logger.warning(warning);
			if (!warning.empty())
			{
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
}