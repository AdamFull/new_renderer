#include "gfx_slang_shader_compiler.h"

#include "../../util/utl_hash.hpp"
#include "../../util/Logger.h"
#include "../../fs/fs_helper.h"
#include "../../fs/fs_virtual_filesystem.h"

#include "../gfx_structures.h"

#include <atomic>

#include <slang.h>

namespace edge::gfx {
	class StringBlob : public ISlangBlob {
	public:
		StringBlob(std::string&& data)
			: ref_cnt(1), data(std::move(data)) {}

		// IUnknown methods:
		virtual SlangResult SLANG_MCALL queryInterface(const SlangUUID& guid, void** outObject) override {
			if (!outObject) return SLANG_FAIL;
			*outObject = nullptr;
			if (guid == ISlangBlob::getTypeGuid() || guid == ISlangUnknown::getTypeGuid()) {
				*outObject = static_cast<ISlangBlob*>(this);
				addRef();
				return SLANG_OK;
			}
			return SLANG_E_NO_INTERFACE;
		}
		virtual uint32_t SLANG_MCALL addRef() override {
			return ++ref_cnt;
		}
		virtual uint32_t SLANG_MCALL release() override {
			uint32_t count = --ref_cnt;
			if (count == 0) {
				delete this;
			}
			return count;
		}

		// ISlangBlob methods:
		virtual void const* SLANG_MCALL getBufferPointer() override {
			return data.data();
		}
		virtual size_t SLANG_MCALL getBufferSize() override {
			return data.size();
		}

	private:
		std::atomic<uint32_t> ref_cnt;
		std::string data;
	};

	class EdgeVfsSlangFileSystem final : public ISlangFileSystemExt {
	public:
		EdgeVfsSlangFileSystem() : ref_cnt{ 1 } {
		}

		virtual ~EdgeVfsSlangFileSystem() {}

		virtual SlangResult SLANG_MCALL queryInterface(const SlangUUID& guid, void** outObject) override {
			if (!outObject) return SLANG_FAIL;
			*outObject = nullptr;

			if (guid == ISlangFileSystemExt::getTypeGuid() || guid == ISlangFileSystem::getTypeGuid() ||
				guid == ISlangUnknown::getTypeGuid()) {
				*outObject = static_cast<ISlangFileSystemExt*>(this);
				addRef();
				return SLANG_OK;
			}
			return SLANG_E_NO_INTERFACE;
		}

		virtual uint32_t SLANG_MCALL addRef() override {
			return ++ref_cnt;
		}

		virtual uint32_t SLANG_MCALL release() override {
			uint32_t count = --ref_cnt;
			if (count == 0) {
				delete this;
			}
			return count;
		}

		virtual void* SLANG_MCALL castAs(const SlangUUID& guid) override {
			if (guid == ISlangFileSystemExt::getTypeGuid() || guid == ISlangFileSystem::getTypeGuid() ||
				guid == ISlangUnknown::getTypeGuid()) {
				return static_cast<ISlangFileSystemExt*>(this);
			}
			return nullptr;
		}

		SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(char const* path, ISlangBlob** out_blob) SLANG_OVERRIDE {
			if (!path || !out_blob) {
				return SLANG_E_INVALID_ARG;
			}
			*out_blob = nullptr;

			if (!fs::exists(path)) {
				return SLANG_E_NOT_FOUND;
			}

			auto found_in_cache = file_cache.find(path);
			if (found_in_cache != file_cache.end()) {
				found_in_cache->second->addRef();
				*out_blob = found_in_cache->second;
				return SLANG_OK;
			}

			std::string file_data{};
			if (!fs::read_file(path, file_data)) {
				return SLANG_E_NOT_FOUND;
			}

			if (collect_hash) {
				hash ^= murmur_hash(file_data.data(), file_data.size(), 0);
			}

			auto* new_blob = new StringBlob(std::move(file_data));
			file_cache[path] = new_blob;
			new_blob->addRef();

			*out_blob = new_blob;

			return SLANG_OK;
		}

		virtual SlangResult SLANG_MCALL getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity) override {
			if (!path || !outUniqueIdentity) {
				return SLANG_E_INVALID_ARG;
			}

			auto normalized = edge::fs::normalize_path(path);
			*outUniqueIdentity = new StringBlob(std::string(normalized.begin(), normalized.end()));
			return SLANG_OK;
		}

		virtual SlangResult SLANG_MCALL calcCombinedPath(SlangPathType fromPathType, const char* from_path, const char* path, ISlangBlob** outCombinedPath) override {
			if (!path || !outCombinedPath) {
				return SLANG_E_INVALID_ARG;
			}
			*outCombinedPath = nullptr;

			auto current_path = fs::parent_path(from_path);

			std::array<char, 512> buffer{};
			auto appended_path = fs::path_append(current_path, path, buffer);

			auto normalized_path = fs::normalize_path(appended_path);
			*outCombinedPath = new StringBlob(std::string(normalized_path.begin(), normalized_path.end()));
		}

		virtual SlangResult SLANG_MCALL getPathType(const char* path, SlangPathType* outPathType) override {
			if (!path || !outPathType) {
				return SLANG_E_INVALID_ARG;
			}

			if (!fs::exists(path)) {
				return SLANG_E_NOT_FOUND;
			}

			*outPathType = fs::is_directory(path) ? SLANG_PATH_TYPE_DIRECTORY : SLANG_PATH_TYPE_FILE;
			return SLANG_OK;
		}

		virtual SlangResult SLANG_MCALL getPath(PathKind /*pathKind*/, const char* path, ISlangBlob** outPath) override {
			if (!path || !outPath) {
				return SLANG_E_INVALID_ARG;
			}

			*outPath = new StringBlob(std::string(path, path + strlen(path)));
			return SLANG_OK;
		}

		virtual void SLANG_MCALL clearCache() override {
			file_cache.clear();
		}

		virtual SlangResult SLANG_MCALL enumeratePathContents(const char* path, FileSystemContentsCallBack callback, void* userData) override {
			if (!path || !callback) {
				return SLANG_E_INVALID_ARG;
			}

			if (!fs::is_directory(path)) {
				return SLANG_E_NOT_FOUND;
			}

			for (auto entry : fs::walk(path)) {
				SlangPathType type = fs::is_directory(entry) ? SLANG_PATH_TYPE_DIRECTORY : SLANG_PATH_TYPE_FILE;
				callback(type, entry.data(), userData);
			}

			return SLANG_OK;
		}

		virtual OSPathKind SLANG_MCALL getOSPathKind() override {
			return OSPathKind::Direct;
		}

		uint32_t get_dependency_hash() {
			return hash;
		}

		void reset(bool collect_hash = false) {
			this->collect_hash = collect_hash;
			hash = 0;
		}
	private:
		std::atomic<uint32_t> ref_cnt;

		std::string base_path;
		std::vector<std::string> search_paths{};
		std::unordered_map<std::string, StringBlob*> file_cache{};

		uint32_t hash{ 0u };
		bool collect_hash{ false };
	};

	auto get_descriptor_type(slang::BindingType binding_type) -> DescriptorType {
		switch (binding_type)
		{
		case slang::BindingType::Sampler:
			return DescriptorType::eSampler;
		case slang::BindingType::Texture:
			return DescriptorType::eSampledImage;
		case slang::BindingType::ConstantBuffer:
		case slang::BindingType::ParameterBlock:
		case slang::BindingType::InlineUniformData:
			return DescriptorType::eUniformBuffer;
		case slang::BindingType::TypedBuffer:
		case slang::BindingType::RawBuffer:
		case slang::BindingType::MutableTypedBuffer:
		case slang::BindingType::MutableRawBuffer:
			return DescriptorType::eStorageBuffer;
		case slang::BindingType::CombinedTextureSampler:
			return DescriptorType::eCombinedImageSampler;
		case slang::BindingType::MutableTexture:
			return DescriptorType::eStorageImage;
		default:
			return DescriptorType::eUnknown;
		}
	}

	auto get_shader_stage(SlangStage stage) -> ShaderStageFlag {
		switch (stage)
		{
		case SLANG_STAGE_VERTEX: return ShaderStageFlag::eVertex;
		case SLANG_STAGE_HULL: return ShaderStageFlag::eTessellationControl;
		case SLANG_STAGE_DOMAIN: return ShaderStageFlag::eTessellationEvaluation;
		case SLANG_STAGE_GEOMETRY: return ShaderStageFlag::eGeometry;
		case SLANG_STAGE_FRAGMENT: return ShaderStageFlag::eFragment;
		case SLANG_STAGE_COMPUTE: return ShaderStageFlag::eCompute;
		default: return (ShaderStageFlag)0;
		}
	}

	auto get_size_from_slang_type(slang::TypeLayoutReflection* type_layout) -> uint32_t {
		if (!type_layout) {
			return 0;
		}

		auto type_layout_kind = type_layout->getKind();
		switch (type_layout_kind) {
		case slang::TypeReflection::Kind::Array: {
			auto element_type = type_layout->getElementTypeLayout();
			auto element_count = type_layout->getElementCount();
			return element_type->getSize() * element_count;
		}

		case slang::TypeReflection::Kind::Struct: {
			return type_layout->getSize();
		}

		default:
			return type_layout->getSize(type_layout->getParameterCategory());
		}
	}

	SlangCompilationResult::SlangCompilationResult(Slang::ComPtr<slang::ISession> session, Slang::ComPtr<slang::IComponentType> program) :
		session{session},
		linked_program{ program } {
		session->addRef();
		linked_program->addRef();
	}

	SlangCompilationResult::~SlangCompilationResult() {
		linked_program->release();
	}

	auto SlangCompilationResult::collect_result(std::vector<uint8_t>& output) -> bool {
		Slang::ComPtr<slang::IBlob> diagnostics;
		Slang::ComPtr<slang::IBlob> spirv_blob;
		if (SLANG_FAILED(linked_program->getEntryPointCode(0, 0, spirv_blob.writeRef(), diagnostics.writeRef()))) {
			EDGE_LOGE("[Slang Shader Compiled] Failed to get compiled code.");
			if (diagnostics) {
				EDGE_LOGI("[Slang Shader Compiled] Diagnostics:\n{}", (const char*)diagnostics->getBufferPointer());
			}
			return false;
		}

		// Dump compiled spirv code
		output.resize(spirv_blob->getBufferSize());
		memcpy(output.data(), spirv_blob->getBufferPointer(), spirv_blob->getBufferSize());

		return true;
	}

	auto SlangCompilationResult::collect_reflection(ShaderReflection& refl) -> bool {
		slang::ProgramLayout* program_layout = linked_program->getLayout();

		ShaderStageFlag stage{};
		if (program_layout->getEntryPointCount()) {
			auto* entry_point = program_layout->getEntryPointByIndex(0);
			stage = get_shader_stage(entry_point->getStage());

			if (stage == ShaderStageFlag::eCompute) {
				SlangUInt group_sizes[3]{};
				entry_point->getComputeThreadGroupSize(3, group_sizes);

				for (int32_t i = 0; i < 3; ++i) {
					refl.local_sizes[i] = static_cast<uint32_t>(group_sizes[i]);
				}
			}
		}

		for (int32_t i = 0; i < program_layout->getParameterCount(); i++) {
			auto* param = program_layout->getParameterByIndex(i);

			if (!param) {
				continue;
			}

			auto* param_type = param->getTypeLayout();
			if (!param_type) {
				continue;
			}

			auto kind = param_type->getKind();
			auto category = param->getCategory();
			const char* name = param->getName();

			auto binding_index = param->getBindingIndex();
			auto binding_space = param->getBindingSpace();

			if (category == slang::ParameterCategory::PushConstantBuffer) {
				auto& resource = refl.constants;
				resource.stages |= stage;

				resource.set_name(name);

				if (kind == slang::TypeReflection::Kind::Struct ||
					kind == slang::TypeReflection::Kind::ConstantBuffer) {

					auto* content_type = param_type->getElementTypeLayout();
					resource.size = get_size_from_slang_type(content_type);
					resource.range_begin = refl.shader_variables.count;

					for (int32_t f_idx = 0; f_idx < content_type->getFieldCount(); ++f_idx) {
						auto* field = content_type->getFieldByIndex(f_idx);
						if (!field) {
							continue;
						}

						ShaderVariable& variable = refl.add_shader_variable(field->getName());
						variable.offset = field->getOffset();
						variable.size = get_size_from_slang_type(field->getTypeLayout());
					}
					resource.variable_count = refl.shader_variables.count - resource.range_begin;
				}
			}
			else {
				auto [found, resource] = refl.lookup_shader_resource(name);
				if (found) {
					resource.stages |= stage;
					continue;
				}

				resource.stages |= stage;

				resource.binding = binding_index;
				resource.set = binding_space;

				if (param_type->isArray()) {
					resource.count = param_type->getElementCount();
				}

				if (param_type->getBindingRangeCount()) {
					auto binding_type = param_type->getBindingRangeType(0);
					resource.type = get_descriptor_type(binding_type);
				}

				if (kind == slang::TypeReflection::Kind::Struct ||
					kind == slang::TypeReflection::Kind::ConstantBuffer ||
					kind == slang::TypeReflection::Kind::ShaderStorageBuffer) {

					auto* content_type = param_type->getElementTypeLayout();
					resource.size = get_size_from_slang_type(content_type);
					resource.range_begin = refl.shader_variables.count;

					for (int32_t f_idx = 0; f_idx < content_type->getFieldCount(); ++f_idx) {
						auto* field = content_type->getFieldByIndex(f_idx);
						if (!field) {
							continue;
						}

						ShaderVariable& variable = refl.add_shader_variable(field->getName());
						variable.offset = field->getOffset();
						variable.size = get_size_from_slang_type(field->getTypeLayout());
					}
					resource.variable_count = refl.shader_variables.count - resource.range_begin;
				}
			}
		}

		return true;
	}

	slang::IGlobalSession* g_session{ nullptr };

	SlangShaderCompiler::SlangShaderCompiler(const ShaderCompilerCreateInfo& create_info) {
		std::array<slang::CompilerOptionEntry, 3> compiler_options{};

		static slang::IGlobalSession* g_slang_session{ nullptr };
		if (!g_slang_session) {
			// Create a global Slang session (load Slang core module). 
			// Using default options: enable HLSL/Slang by default.
			Slang::ComPtr<slang::IGlobalSession> new_session;
			SlangGlobalSessionDesc session_desc = {};
			if (SLANG_SUCCEEDED(slang::createGlobalSession(&session_desc, new_session.writeRef()))) {
				g_slang_session = new_session.detach();  // keep it alive for reuse
			}
			else {
				g_slang_session = nullptr;
			}
		}
		g_session = g_slang_session;
		filesystem = new EdgeVfsSlangFileSystem();
	}

	SlangShaderCompiler::~SlangShaderCompiler() {
		delete filesystem;
		filesystem = nullptr;
	}

	auto SlangShaderCompiler::collect_dependency_hash(const ShaderCompileInfo& compile_info) -> uint32_t {
		assert(g_session && "Slang global session not initialized");

		filesystem->reset(true);

		return 0;
	}

	auto SlangShaderCompiler::compile(const ShaderCompileInfo& compile_info) -> ICompilationResult* {
		assert(g_session && "Slang global session not initialized");

		filesystem->reset(false);

		std::vector<slang::PreprocessorMacroDesc> preprocessor_input{};
		for (auto& definition : compile_info.definitions) {
			preprocessor_input.emplace_back(definition.c_str(), "");
		}

		slang::CompilerOptionEntry compiler_options[] = {
			{ slang::CompilerOptionName::Optimization, {.intValue0 = SLANG_OPTIMIZATION_LEVEL_NONE } },
			{ slang::CompilerOptionName::DebugInformation, {.intValue0 = SLANG_DEBUG_INFO_LEVEL_MAXIMAL } },
			{ slang::CompilerOptionName::EmitSpirvMethod, {.intValue0 = SLANG_EMIT_SPIRV_DIRECTLY } }
		};

		slang::TargetDesc target_desc{};
		target_desc.format = SLANG_SPIRV;
		target_desc.profile = g_session->findProfile("spirv_1_5");
		target_desc.floatingPointMode = SLANG_FLOATING_POINT_MODE_PRECISE;
		//target_desc.forceGLSLScalarBufferLayout = true;
		target_desc.compilerOptionEntries = compiler_options;
		target_desc.compilerOptionEntryCount = static_cast<SlangInt>(std::size(compiler_options));

		slang::SessionDesc session_desc{};
		session_desc.targets = &target_desc;
		session_desc.targetCount = 1;
		session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
		// TODO: Add additional search paths
		session_desc.preprocessorMacros = preprocessor_input.data();
		session_desc.preprocessorMacroCount = static_cast<SlangInt>(preprocessor_input.size());
		session_desc.fileSystem = filesystem;
		session_desc.compilerOptionEntries = compiler_options;
		session_desc.compilerOptionEntryCount = static_cast<SlangInt>(std::size(compiler_options));

		Slang::ComPtr<slang::ISession> session{};
		if (SLANG_FAILED(g_session->createSession(session_desc, session.writeRef()))) {
			EDGE_LOGE("[Slang Shader Compiled] Failed to create compile session.");
			return nullptr;
		}

		auto module_name = fs::get_stem(compile_info.path);

		Slang::ComPtr<slang::IBlob> diagnostics;
		slang::IModule* slang_module = session->loadModuleFromSourceString(module_name.data(),
			compile_info.path.data(), compile_info.source.data(), diagnostics.writeRef());

		if (!slang_module) {
			EDGE_LOGE("[Slang Shader Compiled] Failed to open slang module.");
			if (diagnostics) {
				EDGE_LOGI("[Slang Shader Compiled] Diagnostics:\n{}", (const char*)diagnostics->getBufferPointer());
			}
			return nullptr;
		}

		Slang::ComPtr<slang::IEntryPoint> entry_point;
		if (SLANG_FAILED(slang_module->getDefinedEntryPoint(0, entry_point.writeRef()))) {
			EDGE_LOGE("[Slang Shader Compiled] Failed to get shader entry point.");
			return nullptr;
		}

		Slang::ComPtr<slang::IComponentType> linked_program;
		if (SLANG_FAILED(entry_point->link(linked_program.writeRef(), diagnostics.writeRef()))) {
			EDGE_LOGE("[Slang Shader Compiled] Failed to link slang module.");
			if (diagnostics) {
				EDGE_LOGI("[Slang Shader Compiled] Diagnostics:\n{}", (const char*)diagnostics->getBufferPointer());
			}
			return nullptr;
		}

		return new SlangCompilationResult(session, linked_program);
	}
}