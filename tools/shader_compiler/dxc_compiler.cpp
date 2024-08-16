#include "dxc_compiler.h"

#include <locale>
#include <codecvt>
#include <iostream>

#include <windows.h>

#include <logger/logger.h>

using namespace shader_compiler;

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

CDXCCompiler::CDXCCompiler(ETargetAPIType target_api, ETargetBuildType build_type)
{
	
}

CDXCCompiler::~CDXCCompiler()
{
	CoUninitialize();
}

bool CDXCCompiler::initialize()
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

bool CDXCCompiler::preprocess(std::span<uint8_t> shader_code, const FCompileInfo& info, std::string& preprocessed_code, u32& preprocessed_hash)
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

	CComPtr<IDxcOperationResult> pResult;
	hr = m_pCompiler->Preprocess(pSourceBlob, shader_name.c_str(), nullptr, 0, nullptr, 0, nullptr, &pResult);

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

utl::vector<uint32_t> CDXCCompiler::compile(std::span<char> shader_code, const FCompileInfo& info)
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

	std::vector<LPCWSTR> arguments = { L"-spirv" };

	auto shader_lang = get_sh_language(info.type);
	shader_lang = shader_lang + "_6_0";

	auto target_profile = converter.from_bytes(shader_lang);

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
		nullptr,                // pIncludeHandler
		&pResult);              // ppResult

	if(SUCCEEDED(hr))
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