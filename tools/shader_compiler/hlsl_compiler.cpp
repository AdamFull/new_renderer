#include "hlsl_compiler.h"

#include <locale>
#include <codecvt>
#include <iostream>

#include <windows.h>
#include <dxcapi.h>

#include <logger.hpp>

namespace shader_compiler
{
	class IncludeHandler : public IDxcIncludeHandler
	{
	public:
		explicit IncludeHandler(std::span<std::filesystem::path> include_directories)
			: refCount(1), include_dirs(include_dirs) {}

		HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override
		{
			if (!pFilename || !ppIncludeSource)
				return E_POINTER;

			// Convert the filename from LPCWSTR to std::wstring
			std::wstring filename(pFilename);

			// Attempt to resolve the file using the provided include directories
			for (const auto& dir : include_dirs)
			{
				std::filesystem::path full_path = dir / filename;

				if (std::filesystem::exists(full_path))
				{
					// Read the file contents
					std::ifstream file_stream(full_path, std::ios::binary | std::ios::ate);
					if (!file_stream.is_open())
					{
						return E_FAIL;
					}

					std::streamsize size = file_stream.tellg();
					file_stream.seekg(0, std::ios::beg);

					std::vector<char> buffer(static_cast<size_t>(size));
					if (!file_stream.read(buffer.data(), size))
					{
						return E_FAIL;
					}

					// Use DXC's library to create a blob encoding
					CComPtr<IDxcLibrary> library;
					if (FAILED(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library))))
					{
						return E_FAIL;
					}

					CComPtr<IDxcBlobEncoding> encodingBlob;
					if (FAILED(library->CreateBlobWithEncodingOnHeapCopy(buffer.data(), static_cast<UINT32>(buffer.size()), CP_UTF8, &encodingBlob)))
					{
						return E_FAIL;
					}

					// Return the blob encoding
					*ppIncludeSource = encodingBlob.Detach();
					return S_OK;
				}
			}

			// If the file isn't found in the include directories, return failure
			return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		}

		// IUnknown methods
		ULONG STDMETHODCALLTYPE AddRef() override
		{
			return InterlockedIncrement(&refCount);
		}

		ULONG STDMETHODCALLTYPE Release() override
		{
			ULONG newCount = InterlockedDecrement(&refCount);
			if (newCount == 0)
			{
				delete this;
			}
			return newCount;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
		{
			if (riid == IID_IUnknown)
			{
				*ppvObject = static_cast<IDxcIncludeHandler*>(this);
				AddRef();
				return S_OK;
			}
			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}
	private:
		ULONG refCount;
		std::vector<std::filesystem::path> include_dirs;
	};

	std::string get_sh_language(EShaderType type)
	{
		switch (type)
		{
		case EShaderType::eVertex: return "vs"; break;
		case EShaderType::eControl: return "hs"; break;
		case EShaderType::eEvaluation: return "ds"; break;
		case EShaderType::eGeometry: return "gs"; break;
		case EShaderType::eFragment: return "ps"; break;
		case EShaderType::eCompute: return "cs"; break;
		case EShaderType::eTask: return "as"; break;
		case EShaderType::eMesh: return "ms"; break;
		}

		return "LIB";
	}

	// Utility function to determine SPIR-V target environment
	std::wstring get_spirv_target_env(ETargetAPIType api)
	{
		switch (api)
		{
		case ETargetAPIType::eVK10: return L"-fspv-target-env=vulkan1.0";
		case ETargetAPIType::eVK11: return L"-fspv-target-env=vulkan1.1";
		case ETargetAPIType::eVK12: return L"-fspv-target-env=vulkan1.2";
		case ETargetAPIType::eVK13: return L"-fspv-target-env=vulkan1.3";
		default: return L"-fspv-target-env=vulkan1.1"; // Default to Vulkan 1.1
		}
	}

	HLSLCompiler::HLSLCompiler(ETargetAPIType target_api, ETargetBuildType build_type)
	{
		this->target_api = target_api;
		this->build_type = build_type;

		CoUninitialize();
	}

	HLSLCompiler::~HLSLCompiler()
	{

	}

	bool HLSLCompiler::initialize()
	{
		HRESULT hr = CoInitialize(nullptr);
		if (FAILED(hr))
		{
			log_error("Failed to initialize COM Library");
			return false;
		}

		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_pCompiler));
		if (FAILED(hr))
		{
			log_error("Failed to create DXC Compiler instance");
			CoUninitialize();
			return false;
		}

		hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_pLibrary));
		if (FAILED(hr))
		{
			log_error("Failed to create DXC Library instance");
			CoUninitialize();
			return false;
		}

		return true;
	}

	bool HLSLCompiler::preprocess(const std::string& shader_code, const FCompileInfo& info, std::string& preprocessed_code, u32& preprocessed_hash)
	{
		CComPtr<IDxcBlobEncoding> pSourceBlob;
		HRESULT hr = m_pLibrary->CreateBlobWithEncodingFromPinned(shader_code.data(), shader_code.size(), CP_UTF8, &pSourceBlob);
		if (FAILED(hr))
		{
			log_error("Failed to create source blob");
			return false;
		}

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring shader_name = converter.from_bytes(info.name);

		CComPtr<IncludeHandler> include_handler = new IncludeHandler(info.include_directories);

		CComPtr<IDxcOperationResult> pResult;
		hr = m_pCompiler->Preprocess(
			pSourceBlob,			// pSource
			shader_name.c_str(),	// pSourceName
			nullptr,				// pArguments
			0, 						// argCount
			nullptr,                // pDefines
			0,                      // defineCount
			include_handler,		// pIncludeHandler
			&pResult);				// ppResult

		if (SUCCEEDED(hr))
			pResult->GetStatus(&hr);

		if (FAILED(hr))
		{
			IDxcBlobEncoding* pError;
			hr = pResult->GetErrorBuffer(&pError);

			if (SUCCEEDED(hr) && pError)
			{
				log_error("\nShader preprocessing failed: \n{}", (const char*)pError->GetBufferPointer());
				pError->Release();
				return false;
			}

			log_error("Shader preprocessing failed.");

			return false;
		}

		IDxcBlob* pBlob;
		hr = pResult->GetResult(&pBlob);
		if (SUCCEEDED(hr))
		{
			char* begin = (char*)pBlob->GetBufferPointer();
			char* end = begin + pBlob->GetBufferSize();
			preprocessed_code.insert(preprocessed_code.end(), begin, end);
		}
		pBlob->Release();

		preprocessed_hash = utl::murmur_hash(preprocessed_code);

		return true;
	}

	utl::vector<uint32_t> HLSLCompiler::compile(const std::string& shader_code, const FCompileInfo& info)
	{
		CComPtr<IDxcBlobEncoding> pSourceBlob;
		HRESULT hr = m_pLibrary->CreateBlobWithEncodingFromPinned(shader_code.data(), shader_code.size(), CP_UTF8, &pSourceBlob);
		if (FAILED(hr))
		{
			log_error("Failed to create source blob");
			return {};
		}

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring shader_name = converter.from_bytes(info.name);

		// Get the SPIR-V target environment based on the API type
		std::wstring spirv_env = get_spirv_target_env(target_api);

		std::vector<LPCWSTR> arguments = { L"-spirv", spirv_env.c_str() };

		auto shader_lang = get_sh_language(info.type);
		shader_lang = shader_lang + "_6_0";

		auto target_profile = converter.from_bytes(shader_lang);

		CComPtr<IncludeHandler> include_handler = new IncludeHandler(info.include_directories);

		CComPtr<IDxcOperationResult> pResult;
		hr = m_pCompiler->Compile(
			pSourceBlob,            // pSource
			shader_name.c_str(),    // pSourceName
			L"main",              // pEntryPoint
			target_profile.c_str(),              // pTargetProfile
			arguments.data(),       // pArguments
			arguments.size(),       // argCount
			nullptr,                // pDefines
			0,                      // defineCount
			include_handler,		// pIncludeHandler
			&pResult);              // ppResult

		if (SUCCEEDED(hr))
			pResult->GetStatus(&hr);

		if (FAILED(hr))
		{
			if (pResult)
			{
				IDxcBlobEncoding* pError;
				hr = pResult->GetErrorBuffer(&pError);
				if (SUCCEEDED(hr) && pError)
				{
					log_error("\nShader compilation failed: \n{}", (const char*)pError->GetBufferPointer());
					pError->Release();
					return {};
				}
			}

			log_error("Shader compilation failed.");
			return {};
		}

		utl::vector<uint32_t> result;

		IDxcBlob* pBlob;
		hr = pResult->GetResult(&pBlob);
		if (SUCCEEDED(hr))
		{
			if (pBlob->GetBufferSize() % sizeof(uint32_t) == 0)
			{
				const uint32_t* pData = reinterpret_cast<const uint32_t*>(pBlob->GetBufferPointer());
				size_t numElements = pBlob->GetBufferSize() / sizeof(uint32_t);

				result = utl::vector<uint32_t>(pData, pData + numElements);
			}
		}
		pBlob->Release();

		return result;
	}
}