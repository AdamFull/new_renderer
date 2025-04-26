#include "gfx_slang_shader_compiler.h"

#include <slang.h>
#include <slang-com-ptr.h>

#include "../../util/utl_hash.hpp"
#include "../../util/Logger.h"
#include "../../fs/fs_helper.h"
#include "../../fs/fs_virtual_filesystem.h"

#include <atomic>

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

	slang::IGlobalSession* g_session{ nullptr };

	SlangShaderCompiler::SlangShaderCompiler(const ShaderCompilerCreateInfo& create_info) {
		static slang::IGlobalSession* g_slang_session{ nullptr };
		if (!g_slang_session) {
			// Create a global Slang session (load Slang core module). 
			// Using default options: enable HLSL/Slang by default.
			Slang::ComPtr<slang::IGlobalSession> new_session;
			SlangGlobalSessionDesc session_desc = {};  // default (GLSL not enabled unless needed)
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

	}

	auto SlangShaderCompiler::collect_dependency_hash(const ShaderCompileInfo& compile_info) -> uint32_t {
		assert(g_session && "Slang global session not initialized");

		filesystem->reset(true);

		return 0;
	}

	auto SlangShaderCompiler::compile(const ShaderCompileInfo& compile_info, std::vector<uint8_t>& output) -> bool {
		assert(g_session && "Slang global session not initialized");

		filesystem->reset(false);

		slang::ICompileRequest* compile_request{ nullptr };
		if (SLANG_FAILED(g_session->createCompileRequest(&compile_request))) {
			EDGE_LOGE("Failed to create Slang compile request.");
			return false;
		}

		compile_request->setFileSystem(filesystem);

		int target_index = compile_request->addCodeGenTarget(SLANG_SPIRV);
		compile_request->setTargetProfile(target_index, g_session->findProfile("spirv_1_4"));

		// TODO: setup optimization and debug
		compile_request->setOptimizationLevel(SLANG_OPTIMIZATION_LEVEL_DEFAULT);
		compile_request->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_STANDARD);

		// TODO: need to setup additional include paths
		//compile_request->addSearchPath();

		SlangSourceLanguage source_lang = SLANG_SOURCE_LANGUAGE_SLANG;
		int translation_unit_index = compile_request->addTranslationUnit(source_lang, nullptr);
		compile_request->addTranslationUnitSourceString(translation_unit_index, compile_info.path.data(), compile_info.source.data());
		//compile_request->addTranslationUnitSourceFile(translation_unit_index, sourcePath.c_str());

		//compile_request->getEntryPoint(translation_unit_index, 0);
		//compile_request->addEntryPoint(translation_unit_index, entry_point, stage);

		SlangResult compile_result = compile_request->compile();
		bool success = SLANG_SUCCEEDED(compile_result);

		const char* diag_str = compile_request->getDiagnosticOutput();
		if (!success && diag_str && diag_str[0] != '\0') {
			EDGE_LOGE("[Slang Shader Compiler] {}", diag_str);
		}
		else if (diag_str && diag_str[0] != '\0') {
			EDGE_LOGW("[Slang Shader Compiler] {}", diag_str);
		}

		if (success) {
			// Retrieve compiled SPIR-V code (as a blob or raw bytes)
			Slang::ComPtr<ISlangBlob> spirv_blob;
			if (SLANG_SUCCEEDED(compile_request->getEntryPointCodeBlob(0, target_index, spirv_blob.writeRef()))) {
				output.resize(spirv_blob->getBufferSize());
				memcpy(output.data(), spirv_blob->getBufferPointer(), spirv_blob->getBufferSize());
			}
			else {
				success = false;
				EDGE_LOGE("[Slang Shader Compiler] Failed to retrieve SPIR-V code.");
			}
		}

		compile_request->release();

		return true;
	}
}