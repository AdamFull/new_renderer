#include <slang.h>
#include <slang-com-ptr.h>

#include <spdlog/spdlog.h>
#include <argparse/argparse.hpp>

#include <span>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <filesystem>

#include <atomic>

std::string_view GetFileName(std::string_view path)
{
	size_t pos = path.find_last_of("/\\");
	if (pos == std::string_view::npos)
		return path;

	return path.substr(pos + 1);
}

std::string_view GetFileExtension(std::string_view path)
{
	size_t dotPos = path.rfind('.');
	if (dotPos == std::string_view::npos)
		return {};

	return path.substr(dotPos + 1);
}

bool ReadFile(std::string_view path, std::string& output)
{
	FILE* file = fopen(path.data(), "r");
	if (!file)
		return false;

	fseek(file, 0, SEEK_END);
	output.resize(ftell(file));
	fseek(file, 0, SEEK_SET);

	if (!fread(output.data(), 1ull, output.size(), file))
	{
		fclose(file);
		return false;
	}

	fclose(file);

	return true;
}

bool WriteFileBin(std::string_view path, std::span<const uint8_t> data)
{
	FILE* file = fopen(path.data(), "wb");
	if (!file)
		return false;

	if (!fwrite(data.data(), 1ull, data.size(), file))
	{
		fclose(file);
		return false;
	}

	fclose(file);

	return true;
}

bool WriteFileHex(std::string_view path, std::span<const uint8_t> data)
{
	FILE* file = fopen(path.data(), "wb");
	if (!file)
		return false;

	const int32_t WORDS_PER_LINE = 8;
	for (int32_t i = 0; i < data.size(); i += WORDS_PER_LINE)
	{
		fwrite("\t", 1ull, 1ull, file);
		for (int32_t j = 0; j < WORDS_PER_LINE && i + j < data.size(); ++j)
		{
			const uint32_t word = data[i + j];

			std::stringstream ss;
			ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << word;
			auto hex_byte = ss.str();

			fwrite(hex_byte.c_str(), 1ull, hex_byte.size(), file);

			if (i + j + 1 < data.size())
				fwrite(",", 1ull, 1ull, file);
		}
		fwrite("\n", 1ull, 1ull, file);
		fflush(file);
	}

	fclose(file);

	return true;
}

SlangSourceLanguage DetectSourceLanguage(std::string_view path)
{
	auto ext = GetFileExtension(path);

	if (ext == "slang")
		return SLANG_SOURCE_LANGUAGE_SLANG;
	else if (ext == "hlsl")
		return SLANG_SOURCE_LANGUAGE_HLSL;
	else if (ext == "glsl")
		return SLANG_SOURCE_LANGUAGE_GLSL;
	else if (ext == "c")
		return SLANG_SOURCE_LANGUAGE_C;
	else if (ext == "cpp")
		return SLANG_SOURCE_LANGUAGE_CPP;
	else if (ext == "cu")
		return SLANG_SOURCE_LANGUAGE_CUDA;
	else if (ext == "spv")
		return SLANG_SOURCE_LANGUAGE_SPIRV;
	else if (ext == "mtl")
		return SLANG_SOURCE_LANGUAGE_METAL;

	return SLANG_SOURCE_LANGUAGE_UNKNOWN;
}

SlangCompileTarget GetCompileCodegenTarget(std::string_view str)
{
	if (str == "glsl")
		return SLANG_GLSL;
	else if (str == "hlsl")
		return SLANG_HLSL;
	else if (str == "spirv")
		return SLANG_SPIRV;
	else if (str == "spirv_asm")
		return SLANG_SPIRV_ASM;
	else if (str == "dxbc")
		return SLANG_DXBC;
	else if (str == "dxbc_asm")
		return SLANG_DXBC_ASM;
	else if (str == "dxil")
		return SLANG_DXIL;
	else if (str == "dxil_asm")
		return SLANG_DXIL_ASM;

	return SLANG_TARGET_UNKNOWN;
}

SlangStage GetShaderStage(std::string_view stage)
{
	if (stage == "vertex")
		return SLANG_STAGE_VERTEX;
	else if (stage == "hull")
		return SLANG_STAGE_HULL;
	else if (stage == "domain")
		return SLANG_STAGE_DOMAIN;
	else if (stage == "geometry")
		return SLANG_STAGE_GEOMETRY;
	else if (stage == "fragment")
		return SLANG_STAGE_FRAGMENT;
	else if (stage == "compute")
		return SLANG_STAGE_COMPUTE;
	else if (stage == "raygen")
		return SLANG_STAGE_RAY_GENERATION;
	else if (stage == "intersection")
		return SLANG_STAGE_INTERSECTION;
	else if (stage == "anyhit")
		return SLANG_STAGE_ANY_HIT;
	else if (stage == "closesthit")
		return SLANG_STAGE_CLOSEST_HIT;
	else if (stage == "miss")
		return SLANG_STAGE_MISS;
	else if (stage == "callable")
		return SLANG_STAGE_CALLABLE;
	else if (stage == "mesh")
		return SLANG_STAGE_MESH;
	else if (stage == "amplification")
		return SLANG_STAGE_AMPLIFICATION;

	return SLANG_STAGE_NONE;
}

class FilesystemWrapper : public ISlangFileSystem
{
public:
	SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) override
	{
		if (uuid == ISlangUnknown::getTypeGuid() || uuid == ISlangFileSystem::getTypeGuid())
		{
			*outObject = static_cast<ISlangFileSystem*>(this);
			addRef();
			return SLANG_OK;
		}
		*outObject = nullptr;
		return SLANG_E_NO_INTERFACE;
	}

	SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override
	{
		return ++ref_counter;
	}

	SLANG_NO_THROW uint32_t SLANG_MCALL release() override
	{
		uint32_t c = --ref_counter;
		if (c == 0) delete this;
		return c;
	}

	// Implement ISlangFileSystem::loadFile()
	SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(
		const char* path,
		ISlangBlob** outBlob) override
	{
		// Track the file path
		files_included.insert(path);

		if (!std::filesystem::exists(path))
		{
			return SLANG_E_NOT_FOUND;
		}

		std::string content;
		if (!ReadFile(path, content))
			return SLANG_E_CANNOT_OPEN;

		// Create an ISlangBlob to hold the file data
		Slang::ComPtr<ISlangBlob> blob;
		//SlangResult res = spCreateBlob(content.data(), content.size(), blob.writeRef());
		//if (SLANG_FAILED(res))
		//{
		//	return res;
		//}

		// Return the blob
		*outBlob = blob.detach();
		return SLANG_OK;
	}

	// A method to retrieve all accessed files
	std::vector<std::string> getAccessedFiles() const
	{
		return std::vector<std::string>(files_included.begin(), files_included.end());
	}

private:
	std::unordered_set<std::string> files_included;
	std::atomic<uint32_t> ref_counter{ 1u };
};

int main(int argc, char* argv[])
{
	argparse::ArgumentParser argparser("shader_compiler");

	argparser.add_argument("input")
		.required()
		.metavar("<file>")
		.help("input shader to compile");

	argparser.add_argument("-o")
		.required()
		.metavar("<file>")
		.default_value(std::string("out.spv"))
		.help("save compiled data in to <file>");

	argparser.add_argument("-I")
		.append()
		.metavar("<dir>")
		.default_value(std::vector<std::string>{})
		.help("add dir to the include search path; includer's directory\n is searched first, followed by left-to-right order of -I\n");

	argparser.add_argument("-D")
		.append()
		.metavar("<name[=def]>")
		.default_value(std::vector<std::string>{})
		.help("define a pre-processor macro");

	argparser.add_argument("-x")
		.help("save binary output as text-based 32-bit hexadecimal numbers")
		.flag();

	argparser.add_argument("-t", "--target")
		.required()
		.metavar("<target>")
		.help("specify output shader targret: glsl, hlsl, spirv, spirv_asm, dxbc, dxbc_asm, dxil, dxil_adm");

	argparser.add_argument("-p", "--profile")
		.required()
		.metavar("<profile>")
		.help("specify shader language profile: vs|fs|gs|ts|cs_{major}_{minor}");

	argparser.add_argument("-e", "--entry")
		.metavar("<name>")
		.help("specify <name> as the entry-point function name");

	argparser.add_argument("--stage")
		.metavar("<stage>")
		.help("specify <stage> shader stage type: vertex, hull, domain, geometry, fragment, compute, raygen, intersection, anyhit, closesthit, miss, callable, mesh, amplification");

	argparser.add_argument("-d", "--depfile")
		.metavar("<file>")
		.help("writes depfile for build systems");

	try
	{
		argparser.parse_args(argc, argv);
	}
	catch (const std::exception& err)
	{
		std::cerr << err.what() << std::endl;
		std::cerr << argparser;
		return 1;
	}

	std::string shader_source_code{};

	auto input_file_path = argparser.get<std::string>("input");
	if (!ReadFile(input_file_path, shader_source_code))
	{
		std::cerr << "Shader source file was not found!" << std::endl;
		return -1;
	}

	SlangSession* slangSession = spCreateSession(nullptr);
	if (!slangSession)
	{
		std::cerr << "Failed to create Slang session!" << std::endl;
		return -1;
	}

	SlangCompileRequest* compileRequest = spCreateCompileRequest(slangSession);
	if (!compileRequest)
	{
		std::cerr << "Failed to create Slang compile request!" << std::endl;
		spDestroySession(slangSession);
		return -1;
	}

	auto filename = GetFileName(input_file_path);
	auto source_language = DetectSourceLanguage(input_file_path);

	int translationUnitIndex = spAddTranslationUnit(
		compileRequest,
		source_language,
		filename.data()
	);

	// Specify compile definitions
	if (argparser.is_used("-D"))
	{
		auto definitions = argparser.get<std::vector<std::string>>("-D");
		for (const auto& definition : definitions)
		{
			spAddPreprocessorDefine(compileRequest, definition.c_str(), "1");
		}
	}

	// Specify additional include paths
	if (argparser.is_used("-I"))
	{
		auto include_directories = argparser.get<std::vector<std::string>>("-I");
		for (const auto& directory : include_directories)
		{
			spAddSearchPath(compileRequest, directory.c_str());
		}
	}

	spAddTranslationUnitSourceString(
		compileRequest,
		translationUnitIndex,
		filename.data(),
		shader_source_code.c_str()
	);

	auto codegen_target = GetCompileCodegenTarget(argparser.get<std::string>("-t"));
	int target = spAddCodeGenTarget(compileRequest, codegen_target);

	auto language_profile = argparser.get<std::string>("-p");
	spSetTargetProfile(compileRequest, target, spFindProfile(slangSession, language_profile.c_str()));

	std::string entry_point_name{ "main" };
	if (argparser.is_used("-e"))
	{
		entry_point_name = argparser.get<std::string>("-e");
		auto shader_stage = GetShaderStage(argparser.get<std::string>("--stage"));

		spAddEntryPoint(
			compileRequest,
			translationUnitIndex,
			entry_point_name.c_str(),
			shader_stage
		);
	}

	SlangResult result = spCompile(compileRequest);
	if (SLANG_FAILED(result))
	{
		std::cerr << spGetDiagnosticOutput(compileRequest) << std::endl;

		spDestroyCompileRequest(compileRequest);
		spDestroySession(slangSession);
		return -1;
	}

	//SlangReflection* programReflection = spGetReflection(compileRequest);

	// Save compiled code
	size_t size = 0;
	void const* compiled_code = spGetEntryPointCode(compileRequest, 0, &size);
	if (!compiled_code || size == 0)
	{
		std::cerr << "Failed to get SPIR-V code for vsMain!" << std::endl;
		std::cerr << spGetDiagnosticOutput(compileRequest) << std::endl;
	}
	else
	{
		const uint8_t* bytes = static_cast<const uint8_t*>(compiled_code);
		auto output_path = argparser.get<std::string>("-o");

		auto file_data = std::span(bytes, size);
		if (argparser["-x"] == true)
			WriteFileHex(output_path, file_data);
		else
			WriteFileBin(output_path, file_data);
	}

	// Generate depfile
	if (argparser.is_used("-depfile"))
	{
		// TODO: Add to dependency all included files
	}
	
	spDestroyCompileRequest(compileRequest);
	spDestroySession(slangSession);
	//ISlangFileSystem

	return 0;
}