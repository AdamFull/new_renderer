#include <easygltf_loader.h>

#include <fstream>
#include <iostream>

#include <easygltf_validator.h>
#include <easygltf_parse.h>
#include <easygltf_helpers.h>

using namespace easygltf;

namespace path_helper
{
	std::string get_ext(const std::string& path)
	{
		if (auto fnd = path.find_last_of("."))
		{
			if (fnd != std::string::npos)
				return path.substr(fnd);
		}

		return "";
	}

	std::string get_filename(const std::string& path)
	{
		return path.substr(path.find_last_of("/\\") + 1);
	}

	std::string get_stem(const std::string& path)
	{
		auto filename = get_filename(path);
		auto ext = get_ext(filename);
		return filename.erase(filename.size() - ext.size(), filename.size());
	}

	std::string path_append(const std::string& base_path, const std::string& relative_path)
	{
		if (base_path.empty())
			return relative_path;

		if (relative_path.empty())
			return base_path;

		std::string result_path = base_path;
		if (base_path.back() != '/')
		{
			if (relative_path.front() != '/')
				result_path += '/';
		}

		result_path += relative_path;
		return result_path;
	}

	std::string parent_path(const std::string& path)
	{
		const size_t separator_pos = path.find_last_of("/\\");
		return (separator_pos == std::string::npos) ? "" : path.substr(0, separator_pos);
	}
}

class DefaultFilesystemWrapper : public IFilesystemWrapper
{
public:
	~DefaultFilesystemWrapper() override = default;

	bool exists(const std::string& path) override
	{
		return std::filesystem::exists(path);
	}

	std::string expandFilePath(const std::string& path) override
	{
		return std::filesystem::weakly_canonical(path).string();
	}

	bool readFile(const std::string& path, std::vector<uint8_t>& data) override
	{
		std::ifstream instream(path, std::ios::in | std::ios::binary);
		std::copy(std::istreambuf_iterator<char>(instream), std::istreambuf_iterator<char>(), std::back_inserter(data));

		return instream.is_open() && !data.empty();
	}

	bool writeFile(const std::string& path, const std::vector<uint8_t>& data) override
	{
		std::ofstream outstream(path, std::ios::out | std::ios::binary);
		outstream.write(reinterpret_cast<const char*>(data.data()), data.size());
		return outstream.is_open() && outstream.good();
	}

	size_t fileSize(const std::string& path) override
	{
		return std::filesystem::file_size(path);
	}
};

class DefaultDebugMessenger : public IDebugMessenger
{
public:
	~DefaultDebugMessenger() override = default;

	void message(DebugMessageType type, const std::string& data) override
	{
		std::string severity{};
		switch (type)
		{
		case easygltf::DebugMessageType::info: severity = "info"; break;
		case easygltf::DebugMessageType::debug: severity = "debug"; break;
		case easygltf::DebugMessageType::warning: severity = "warning"; break;
		case easygltf::DebugMessageType::error: severity = "error";  break;
		}

		std::cout << std::format("[GLTFProject][{}]: {}", severity, data) << std::endl;
	}
};

void CGLTFLoader::useDefaultFilesystemWrapper()
{
	m_pFS = std::make_unique<DefaultFilesystemWrapper>();
}

void CGLTFLoader::setFilesystemWrapper(std::unique_ptr<IFilesystemWrapper>&& wrapper)
{
	m_pFS = std::move(wrapper);
}

void CGLTFLoader::useDefaultDebugMessenger()
{
	m_pMessenger = std::make_unique<DefaultDebugMessenger>();
}

void CGLTFLoader::setDebugMessenger(std::unique_ptr<IDebugMessenger>&& messenger)
{
	m_pMessenger = std::move(messenger);
}

bool CGLTFLoader::load(const std::string& fsPath, FGLTFRoot& root)
{
	if (!m_pFS)
		useDefaultFilesystemWrapper();

	if (!m_pMessenger)
		useDefaultDebugMessenger();

	root.rootDirectory = path_helper::parent_path(fsPath);

	return _load(fsPath, root);
}

bool CGLTFLoader::_load(const std::string& fsPath, FGLTFRoot& root)
{
	if (!m_pFS->exists(fsPath))
	{
		m_pMessenger->message(DebugMessageType::error, std::format("Failed to open file {}", fsPath));
		return false;
	}

	std::vector<uint8_t> rdata{};
	if (!m_pFS->readFile(fsPath, rdata))
	{
		m_pMessenger->message(DebugMessageType::error, std::format("Failed to read file {}", fsPath));
		return false;
	}

	if (rdata.empty())
	{
		m_pMessenger->message(DebugMessageType::error, "File data is empty.");
		return false;
	}

	auto ext = path_helper::get_ext(fsPath);

	if (ext == ".glb")
		return _loadGLBFromMemory(rdata, root);

	return _loadGLTFFromMemory(rdata, root);
}

bool CGLTFLoader::_loadGLTFFromMemory(const std::vector<uint8_t>& data, FGLTFRoot& root)
{
	auto json = nlohmann::json::parse(data);
	json.get_to(root);

	// Load buffers
	for (size_t buffer_id = 0ull; buffer_id < root.buffers.size(); ++buffer_id)
	{
		auto& buffer = root.buffers.at(buffer_id);

		auto buffer_path = path_helper::path_append(root.rootDirectory, url_decode(buffer.uri));
		if (!m_pFS->readFile(buffer_path, buffer.data))
		{
			m_pMessenger->message(DebugMessageType::error, std::format("Failed to read file {}", buffer_path));
			return false;
		}
	}

	return validate(root, m_pMessenger);
}

bool CGLTFLoader::_loadGLBFromMemory(const std::vector<uint8_t>& data, FGLTFRoot& root)
{
	return false;
}