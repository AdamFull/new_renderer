#include <easygltf.h>

#include <fstream>
#include <filesystem>
#include <format>

#include <easygltf_parse.h>
#include <easygltf_validator.h>

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

	std::string url_decode(const std::string& str)
	{
		std::string ret;
		char ch;
		int i, ii, len = str.length();

		for (i = 0; i < len; i++)
		{
			if (str[i] != '%')
			{
				if (str[i] == '+')
					ret += ' ';
				else
					ret += str[i];
			}
			else
			{
				sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
				ch = static_cast<char>(ii);
				ret += ch;
				i = i + 2;
			}
		}
		return ret;
	}
}

void removeIdFromVectorAndDecrementIdWithHigherIndex(std::vector<int32_t>& container, int32_t index)
{
	auto found = std::remove(container.begin(), container.end(), index);
	if (found != container.end())
	{
		container.erase(found);

		// Decreasing id indices
		for (auto& id : container)
		{
			if (id > index)
				--id;
		}
	}
}

void updateDataIndex(int32_t& target, int32_t index)
{
	if (target > index)
		--target;
	else if (target == index)
		target = base::constants::not_set;
}

template<class _Ty, class _Pred>
void removeIdFromVectorAndDecrementIdWithHigherIndexPred(std::vector<_Ty>& container, int32_t index, _Pred&& pred)
{
	for (auto& id : container)
		updateDataIndex(pred(id), index);
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

// CGltfProject

// Create default filesystem wrapper
void CGltfProject::useDefaultFilesystemWrapper()
{
	m_pFS = std::make_unique<DefaultFilesystemWrapper>();
}

void CGltfProject::setFilesystemWrapper(std::unique_ptr<IFilesystemWrapper>&& wrapper)
{
	m_pFS = std::move(wrapper);
}

bool CGltfProject::load(const std::string& fsPath)
{
	if (!m_pFS)
		useDefaultFilesystemWrapper();

	m_srParentPath = path_helper::parent_path(fsPath);

	return _load(fsPath, m_scene);
}

bool CGltfProject::save(const std::string& fsPath)
{
	return false;
}

// TODO: finish this part
bool CGltfProject::import(const std::string& fsPath, const FImportGltfInfo& importInfo)
{
	bool res{ false };
	FGltf n_scene{};

	res = _load(fsPath, n_scene);
	if (!res)
		return false;

	// Importing extensions


	return false;
}

const FGltf& CGltfProject::getGltf() const
{
	return m_scene;
}

std::string CGltfProject::getResourcePath(const std::string& path)
{
	return path_helper::path_append(m_srParentPath, path_helper::url_decode(path));
}

uint8_t* CGltfProject::getBufferPointer(const Accessor& accessor)
{
	const BufferView& buffer_view = m_scene.bufferViews[accessor.bufferView];
	auto& buffer = m_mBufferData[buffer_view.buffer];
	return &buffer[accessor.byteOffset + buffer_view.byteOffset];
}

uint8_t* CGltfProject::getBufferPointer(int32_t accessor_id)
{
	const Accessor& accessor = m_scene.accessors[accessor_id];
	return getBufferPointer(accessor);
}

int32_t CGltfProject::createAccessor(const std::string& name)
{
	Accessor accessor{};
	accessor.name = name;
	m_scene.accessors.emplace_back(accessor);

	return static_cast<int32_t>(m_scene.accessors.size() - 1ull);
}

int32_t CGltfProject::createAnimation(const std::string& name)
{
	Animation animation{};
	animation.name = name;
	m_scene.animations.emplace_back(animation);

	return static_cast<int32_t>(m_scene.animations.size() - 1ull);
}

int32_t CGltfProject::createBuffer(const std::string& name)
{
	Buffer buffer{};
	buffer.name = name;
	m_scene.buffers.emplace_back(buffer);

	return static_cast<int32_t>(m_scene.buffers.size() - 1ull);
}

int32_t CGltfProject::createBufferView(const std::string& name)
{
	BufferView bufferView{};
	bufferView.name = name;
	m_scene.bufferViews.emplace_back(bufferView);

	return static_cast<int32_t>(m_scene.bufferViews.size() - 1ull);
}

int32_t CGltfProject::createCamera(const std::string& name)
{
	Camera camera{};
	camera.name = name;
	m_scene.cameras.emplace_back(camera);

	return static_cast<int32_t>(m_scene.cameras.size() - 1ull);
}

int32_t CGltfProject::createImage(const std::string& name)
{
	Image image{};
	image.name = name;
	m_scene.images.emplace_back(image);

	return static_cast<int32_t>(m_scene.images.size() - 1ull);
}

int32_t CGltfProject::createMaterial(const std::string& name)
{
	Material material{};
	material.name = name;
	m_scene.materials.emplace_back(material);

	return static_cast<int32_t>(m_scene.materials.size() - 1ull);
}

int32_t CGltfProject::createMesh(const std::string& name)
{
	Mesh mesh{};
	mesh.name = name;
	m_scene.meshes.emplace_back(mesh);

	return static_cast<int32_t>(m_scene.meshes.size() - 1ull);
}

int32_t CGltfProject::createNode(const std::string& name)
{
	Node node{};
	node.name = name;
	m_scene.nodes.emplace_back(node);

	return static_cast<int32_t>(m_scene.nodes.size() - 1ull);
}

int32_t CGltfProject::createSampler(const std::string& name)
{
	Sampler sampler{};
	sampler.name = name;
	m_scene.samplers.emplace_back(sampler);

	return static_cast<int32_t>(m_scene.samplers.size() - 1ull);
}

int32_t CGltfProject::createScene(const std::string& name)
{
	Scene scene{};
	scene.name = name;
	m_scene.scenes.emplace_back(scene);

	return static_cast<int32_t>(m_scene.scenes.size() - 1ull);
}

int32_t CGltfProject::createSkin(const std::string& name)
{
	Skin skin{};
	skin.name = name;
	m_scene.skins.emplace_back(skin);

	return static_cast<int32_t>(m_scene.skins.size() - 1ull);
}

int32_t CGltfProject::createTexture(const std::string& name)
{
	Texture texture{};
	texture.name = name;
	m_scene.textures.emplace_back(texture);

	return static_cast<int32_t>(m_scene.textures.size() - 1ull);
}

size_t CGltfProject::removeAccessor(int32_t index)
{
	m_scene.accessors.erase(std::next(m_scene.accessors.begin(), index));

	// Update data for animations
	for (auto& animation : m_scene.animations)
	{
		for (auto& sampler : animation.samplers)
		{
			updateDataIndex(sampler.input, index);
			updateDataIndex(sampler.output, index);
		}
	}

	// Update primitives
	for (auto& mesh : m_scene.meshes)
	{
		for (auto& primitive : mesh.primitives)
		{
			// Update attributes
			for (auto& [name, attrib] : primitive.attributes)
				updateDataIndex(attrib, index);

			updateDataIndex(primitive.indices, index);
		}
	}

	// Update skins
	for (auto& skin : m_scene.skins)
		updateDataIndex(skin.inverseBindMatrices, index);

	return m_scene.accessors.size();
}

size_t CGltfProject::removeAnimation(int32_t index)
{
	m_scene.animations.erase(std::next(m_scene.animations.begin(), index));
	return m_scene.animations.size();
}

size_t CGltfProject::removeBuffer(int32_t index)
{
	m_scene.buffers.erase(std::next(m_scene.buffers.begin(), index));
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_scene.bufferViews, index, [](BufferView& view) -> int32_t& { return view.buffer; });
	return m_scene.buffers.size();
}

size_t CGltfProject::removeBufferView(int32_t index)
{
	m_scene.bufferViews.erase(std::next(m_scene.bufferViews.begin(), index));

	// At first remove it for images
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_scene.images, index, [](Image& image) -> int32_t& { return image.bufferView; });

	// Remove for accessors
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_scene.accessors, index, [](Accessor& accessor) -> int32_t& { return accessor.bufferView; });

	// Remove for sparse accessors
	for (auto& accessor : m_scene.accessors)
	{
		auto& sparse = accessor.sparse;
		updateDataIndex(sparse.indices.bufferView, index);
		updateDataIndex(sparse.values.bufferView, index);
	}

	return m_scene.bufferViews.size();
}

size_t CGltfProject::removeCamera(int32_t index)
{
	m_scene.cameras.erase(std::next(m_scene.cameras.begin(), index));
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_scene.nodes, index, [](Node& node) -> int32_t& { return node.camera; });
	return m_scene.cameras.size();
}

size_t CGltfProject::removeImage(int32_t index)
{
	m_scene.images.erase(std::next(m_scene.images.begin(), index));

	// Find all image usages and resort them
	for (auto& texture : m_scene.textures)
	{
		//auto basisu_support = texture.extensions.find("KHR_texture_basisu");
		//if (basisu_support != texture.extensions.end())
		//{
		//	auto& extension = basisu_support->second;
		//	auto source = extension.Get("source");
		//	auto image = source.GetNumberAsInt();
		//
		//	if (image > index)
		//	{
		//		--image;
		//		extension = tinygltf::Value({ {"source", tinygltf::Value(image)} });
		//	}
		//	else if (image == index)
		//		texture.extensions.erase(basisu_support);
		//
		//	continue;
		//}
		//else
		//{
		//	if (texture.source > index)
		//		--texture.source;
		//	else if (texture.source == index)
		//		texture.source = -1;
		//}
	}

	return m_scene.images.size();
}

size_t CGltfProject::removeMaterial(int32_t index)
{
	m_scene.materials.erase(std::next(m_scene.materials.begin(), index));

	// Remove material from primitives
	for (auto& mesh : m_scene.meshes)
	{
		for (auto& primitive : mesh.primitives)
			updateDataIndex(primitive.material, index);
	}

	return m_scene.materials.size();
}

size_t CGltfProject::removeMesh(int32_t index)
{
	m_scene.meshes.erase(std::next(m_scene.meshes.begin(), index));
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_scene.nodes, index, [](Node& node) -> int32_t& { return node.mesh; });
	return m_scene.meshes.size();
}

size_t CGltfProject::removeNode(int32_t index)
{
	m_scene.nodes.erase(std::next(m_scene.nodes.begin(), index));

	// Remove node from scenes
	for (auto& scene : m_scene.scenes)
		removeIdFromVectorAndDecrementIdWithHigherIndex(scene.nodes, index);

	// Remove node from children
	for (auto& node : m_scene.nodes)
		removeIdFromVectorAndDecrementIdWithHigherIndex(node.children, index);

	// Remove node from animations
	for (auto& animation : m_scene.animations)
		removeIdFromVectorAndDecrementIdWithHigherIndexPred(animation.channels, index, [](AnimationChannel& cmannel) -> int32_t& { return cmannel.target.node; });

	// Remove node from skin
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_scene.skins, index, [](Skin& node) -> int32_t& { return node.skeleton; });

	return m_scene.nodes.size();
}

size_t CGltfProject::removeSampler(int32_t index)
{
	m_scene.samplers.erase(std::next(m_scene.samplers.begin(), index));
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_scene.textures, index, [](Texture& texture) -> int32_t& { return texture.sampler; });
	return m_scene.samplers.size();
}

size_t CGltfProject::removeScene(int32_t index)
{
	m_scene.scenes.erase(std::next(m_scene.scenes.begin(), index));

	if (index == m_scene.scene)
		m_scene.scene = base::constants::not_set;

	return m_scene.scenes.size();
}

size_t CGltfProject::removeSkin(int32_t index)
{
	// Remove bones and also vertex data
	m_scene.skins.erase(std::next(m_scene.skins.begin(), index));
	return m_scene.skins.size();
}

size_t CGltfProject::removeTexture(int32_t index)
{
	m_scene.textures.erase(std::next(m_scene.textures.begin(), index));

	for (auto& material : m_scene.materials)
	{
		// For emissive texture
		updateDataIndex(material.emissiveTexture.index, index);

		// For normal texture
		updateDataIndex(material.normalTexture.index, index);

		// For occlusion texture
		updateDataIndex(material.occlusionTexture.index, index);

		// For color texture
		updateDataIndex(material.pbrMetallicRoughness.baseColorTexture.index, index);

		// For metal/rough texture
		updateDataIndex(material.pbrMetallicRoughness.metallicRoughnessTexture.index, index);

		// TODO: For extensions
	}

	return m_scene.textures.size();
}

std::string CGltfProject::getErrors()
{
	return m_ssErrors.str();
}

std::string CGltfProject::getWarnings()
{
	return m_ssWarnings.str();
}

bool CGltfProject::_load(const std::string& fsPath, FGltf& gltf_scene)
{
	if (!m_pFS->exists(fsPath))
	{
		m_ssErrors << std::format("Failed to open file {}\n", fsPath);
		return false;
	}

	std::vector<uint8_t> rdata{};
	if (!m_pFS->readFile(fsPath, rdata))
	{
		m_ssErrors << std::format("Failed to read file {}\n", fsPath);
		return false;
	}

	if (rdata.empty())
	{
		m_ssErrors << "File data is empty.\n";
		return false;
	}

	auto ext = path_helper::get_ext(fsPath);

	if (ext == ".glb")
		return _loadGltfFromMemory(rdata, gltf_scene);

	return _loadGltfFromMemory(rdata, gltf_scene);
}

bool CGltfProject::_loadGltfFromMemory(const std::vector<uint8_t>& data, FGltf& gltf_scene)
{
	auto json = nlohmann::json::parse(data);
	json.get_to(gltf_scene);

	// Load buffers
	for (size_t buffer_id = 0ull; buffer_id < gltf_scene.buffers.size(); ++buffer_id)
	{
		auto& buffer_info = gltf_scene.buffers.at(buffer_id);
		auto& buffer_data = m_mBufferData[buffer_id];
		
		auto buffer_path = path_helper::path_append(m_srParentPath, path_helper::url_decode(buffer_info.uri));
		if (!m_pFS->readFile(buffer_path, buffer_data))
			m_ssErrors << std::format("Failed to read file {}\n", buffer_path);
	}

	return validate(gltf_scene, m_ssErrors, m_ssWarnings);
}

bool CGltfProject::_loadGlbFromMemory(const std::vector<uint8_t>& data, FGltf& gltf_scene)
{
	bInBinaryMode = true;

	return validate(gltf_scene, m_ssErrors, m_ssWarnings);
}

bool CGltfProject::_loadBuffer(int32_t bufferId)
{
	if (bInBinaryMode)
	{

	}

	// If in ascii mode


	return true;
}