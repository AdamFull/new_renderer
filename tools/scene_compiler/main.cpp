#include <easygltf/easygltf.h>

#include <mesh_processor.h>

using namespace mesh_compiler;

class CArgumentParser
{
public:
	CArgumentParser(int& argc, char** argv)
	{
		for (int i = 1; i < argc; ++i)
			this->tokens.push_back(std::string(argv[i]));
	}

	template<class _Ty>
	_Ty try_get(const std::string& option, _Ty default_value)
	{
		if (!exists(option))
			return default_value;

		auto svalue = get(option).value();

		if constexpr (std::is_same_v<_Ty, std::string>)
			return svalue;

		_Ty value;
		std::stringstream ss;
		ss << svalue;
		ss >> value;

		return value;
	}

	std::optional<std::string> get(const std::string& option) const
	{
		std::vector<std::string>::const_iterator itr;
		itr = std::find(this->tokens.begin(), this->tokens.end(), option);
		if (itr != this->tokens.end() && ++itr != this->tokens.end())
			return *itr;
		return std::nullopt;
	}

	bool exists(const std::string& option) const
	{
		return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
	}
private:
	std::vector<std::string> tokens;
};

int main(int argc, char** argv)
{
	CArgumentParser argparse(argc, argv);

	easygltf::CGLTFProject project{};
	CMeshPorcessor mesh_processor{};
	
	bool loaded = project.load("D:\\GitHub\\Incenerate-Project\\assets\\models\\glTF-Sample-Models\\2.0\\DamagedHelmet\\glTF\\DamagedHelmet.gltf");

	mesh_processor.prepare(&project);
	mesh_processor.process();

	return 0;
}