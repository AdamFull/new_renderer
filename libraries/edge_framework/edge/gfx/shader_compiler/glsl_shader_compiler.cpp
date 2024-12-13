#include "glsl_shader_compiler.h"

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/build_info.h>

#include <edge/util/fs_helper.h>
#include <edge/util/Logger.h>

#include <hash.hpp>

#include <execution>
#include <format>

namespace edge
{
	namespace gfx
	{
		namespace shc
		{
			// Just copy of https://github.com/KhronosGroup/glslang/blob/main/StandAlone/DirStackFileIncluder.h
			class shader_includer : public glslang::TShader::Includer
			{
			public:
				shader_includer(FilesystemWrapper& fs, std::string_view base_path) :
					fs{ fs },
					base_path{ base_path }
				{

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
						delete static_cast<std::vector<uint8_t>*>(result->userData);
						delete result;
					}
				}

				virtual std::set<std::string> getIncludedFiles()
				{
					return includedFiles;
				}

				virtual ~shader_includer() override { }
			protected:
				std::vector<std::string> directoryStack;
				int externalLocalDirectoryCount;
				std::set<std::string> includedFiles;
				FilesystemWrapper& fs;
				std::string base_path;

				// Search for a valid "local" path based on combining the stack of include
				// directories and the nominal name of the header.
				virtual IncludeResult* readLocalPath(const char* headerName, const char* includerName, int depth)
				{
					// Discard popped include directories, and
					// initialize when at parse-time first level.
					directoryStack.resize(depth + externalLocalDirectoryCount);
					if (depth == 1)
						directoryStack.back() = getDirectory(includerName);

					std::array<char, 1024ull> buffer{};

					// Find a directory that works, using a reverse search of the include stack.
					for (auto it = directoryStack.rbegin(); it != directoryStack.rend(); ++it)
					{
						std::string path = *it + '/' + headerName;
						std::replace(path.begin(), path.end(), '\\', '/');

						std::vector<uint8_t>* file_data = new std::vector<uint8_t>;
						if (fs.Exists(path) && fs.ReadFile(path, *file_data))
						{
							directoryStack.emplace_back(getDirectory(path));
							includedFiles.insert(path);

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


			EShLanguage GetShaderLanguage(ShaderStageFlag type)
			{
				switch (type)
				{
				case ShaderStageFlag::eVertex: return EShLangVertex; break;
				case ShaderStageFlag::eTessellationControl: return EShLangTessControl; break;
				case ShaderStageFlag::eTessellationEvaluation: return EShLangTessEvaluation; break;
				case ShaderStageFlag::eGeometry: return EShLangGeometry; break;
				case ShaderStageFlag::eFragment: return EShLangFragment; break;
				case ShaderStageFlag::eCompute: return EShLangCompute; break;
				//case ShaderStageFlag::eRayGen: return EShLangRayGen; break;
				//case ShaderStageFlag::eIntersection: return EShLangIntersect; break;
				//case ShaderStageFlag::eAnyHit: return EShLangAnyHit; break;
				//case ShaderStageFlag::eClosestHit: return EShLangClosestHit; break;
				//case ShaderStageFlag::eMiss: return EShLangMiss; break;
				//case ShaderStageFlag::eCallable: return EShLangCallable; break;
				//case ShaderStageFlag::eTask: return EShLangTaskNV; break;
				//case ShaderStageFlag::eMesh: return EShLangMeshNV; break;
				}

				return EShLangCount;
			}

			TBuiltInResource GetResources()
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

			glslang::EShTargetClientVersion k_client_version{ glslang::EShTargetVulkan_1_1 };
			glslang::EShClient k_client{ glslang::EShClientVulkan };
			glslang::EShTargetLanguageVersion k_language_version{ glslang::EShTargetSpv_1_4 };
			EShMessages k_messages{ EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault | EShMsgCascadingErrors };

			std::string GeneratePreamble(const CompileShaderInfo& info)
			{
				std::stringstream ss;

				// Include extention is required by default
				ss << "#extension GL_GOOGLE_include_directive : require" << std::endl;

				// Apply base definitions
				for (auto& definition : info.base_definitions)
					ss << std::format("#define {}", definition) << std::endl;

				// Apply definitions (TODO: add definition values)
				for (auto& definition : info.definitions)
					ss << std::format("#define {}", definition) << std::endl;

				return ss.str();
			}

			glslang::TShader CreateShader(const CompileShaderInfo& compile_info, EShMessages& messages)
			{
				auto language = GetShaderLanguage(compile_info.stage);

				glslang::TShader shader(language);

				bool building_release = compile_info.build_type == ShaderCompilerBuildType::eRelease;
				if (!building_release)
				{
					messages = utl::cast<EShMessages>(messages | EShMsgDebugInfo);
					//shader.setDebugInfo(true);
				}

				shader.setOverrideVersion(460);
				shader.setEnvInput(glslang::EShSourceGlsl, language, k_client, 110);
				shader.setEnvClient(k_client, k_client_version);
				shader.setEnvTarget(glslang::EShTargetSpv, k_language_version);

				return shader;
			}

			GLSLShaderCompiler::GLSLShaderCompiler(FilesystemWrapper& fs) :
				fs{ fs }
			{
				if (!glslang::InitializeProcess())
				{
					EDGE_LOGE("Failed to initialize glslang process.");
				}
			}

			GLSLShaderCompiler::~GLSLShaderCompiler()
			{
				glslang::FinalizeProcess();
			}

			bool GLSLShaderCompiler::Preprocess(const CompileShaderInfo& compile_info, std::string_view source_code, std::string& preprocessed_code, uint32_t& hash)
			{
				EShMessages messages{ k_messages };

				auto shader = CreateShader(compile_info, messages);

				auto shader_source = source_code.data();
				auto shader_name = compile_info.name.data();
				shader.setStringsWithLengthsAndNames(&shader_source, nullptr, &shader_name, 1);

				std::string preamble = GeneratePreamble(compile_info);
				shader.setPreamble(preamble.c_str());

				// Create includer and push include dirs to it
				shader_includer includer{ fs, compile_info.base_path };
				for (auto& dir : compile_info.include_directories)
					includer.pushExternalLocalDirectory(dir);

				auto resources = GetResources();
				if (!shader.preprocess(&resources, k_client_version, ENoProfile, false, false, messages, &preprocessed_code, includer))
				{
					EDGE_LOGE("\Preprocess error: In shader \"{}\" \n{}\n{}\n", compile_info.name, shader.getInfoLog(), shader.getInfoDebugLog());
					return false;
				}

				hash = utl::murmur_hash(preprocessed_code);

				return true;
			}

			bool GLSLShaderCompiler::Compile(const CompileShaderInfo& compile_info, std::string_view source_code, std::vector<uint8_t>& code)
			{
				EShMessages messages{ k_messages };

				auto shader = CreateShader(compile_info, messages);

				auto shader_source = source_code.data();
				auto shader_name = compile_info.name.data();
				shader.setStringsWithLengthsAndNames(&shader_source, nullptr, &shader_name, 1);

				std::string preamble = GeneratePreamble(compile_info);
				shader.setPreamble(preamble.c_str());

				// Create includer and push include dirs to it
				shader_includer includer{ fs, compile_info.base_path };
				for (auto& dir : compile_info.include_directories)
					includer.pushExternalLocalDirectory(dir);

				auto resources = GetResources();
				if (!shader.parse(&resources, k_client_version, false, messages))
				{
					EDGE_LOGE("\nParsing error: In shader \"{}\" \n{}\n{}\n", compile_info.name, shader.getInfoLog(), shader.getInfoDebugLog());
					return false;
				}

				glslang::TProgram program;
				program.addShader(&shader);

				if (!program.link(messages) || !program.mapIO())
				{
					EDGE_LOGE("\nLinking error: In shader \"{}\" \n{}\n{}\n", compile_info.name, program.getInfoLog(), program.getInfoDebugLog());
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

				bool building_release = compile_info.build_type == ShaderCompilerBuildType::eRelease;

				glslang::SpvOptions spvOptions;
				spvOptions.generateDebugInfo = !building_release;
				spvOptions.disableOptimizer = !building_release;
				//spvOptions.emitNonSemanticShaderDebugInfo = !building_release;
				//spvOptions.emitNonSemanticShaderDebugSource = !building_release;
				spvOptions.optimizeSize = building_release;

				spv::SpvBuildLogger logger;

				std::vector<uint32_t> spirv_code{};
				GlslangToSpv(*intermediate, spirv_code, &logger, &spvOptions);

				std::string error{};
				logger.error(error);
				if (!error.empty())
				{
					EDGE_LOGE("\nSPIRV generation error: In shader \"{}\" \n{}", compile_info.name, error);
					return false;
				}

				std::string warning;
				logger.warning(warning);
				if (!warning.empty())
				{
					EDGE_LOGW("SPIRV generation warnings: In shader \"{}\" \n{}\n", compile_info.name, warning);
				}

				code.resize(spirv_code.size() * sizeof(uint32_t));

				uint8_t* begin = reinterpret_cast<uint8_t*>(&spirv_code[0]);
				uint8_t* end = begin + code.size();

				std::copy(std::execution::par_unseq, begin, end, code.begin());

				return true;
			}
		}
	}
}