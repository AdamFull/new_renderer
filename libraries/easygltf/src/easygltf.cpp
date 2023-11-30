#include <easygltf.h>

#include <iostream>

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

namespace helpers
{
	void add_when_set(int32_t& target, int32_t offset)
	{
		if (target != base::constants::not_set) target += offset;
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

size_t get_accessor_element_size(const Accessor& accessor)
{
	size_t size{ 0ull };

	switch (accessor.componentType)
	{
	case easygltf::base::enumerators::component_type::byte_type:
	case easygltf::base::enumerators::component_type::ubyte_type: size = 1ull; break;
	case easygltf::base::enumerators::component_type::short_type:
	case easygltf::base::enumerators::component_type::ushort_type: size = 2ull; break;
	case easygltf::base::enumerators::component_type::uint_type:
	case easygltf::base::enumerators::component_type::float_type: size = 4ull; break;
	}

	if (accessor.type == easygltf::base::enumerators::accessor_type::vec2_type)
		size *= 2ull;
	else if (accessor.type == easygltf::base::enumerators::accessor_type::vec3_type)
		size *= 3ull;
	else if (accessor.type == easygltf::base::enumerators::accessor_type::vec4_type)
		size *= 4ull;
	else if (accessor.type == easygltf::base::enumerators::accessor_type::mat2_type)
		size *= 4ull;
	else if (accessor.type == easygltf::base::enumerators::accessor_type::mat3_type)
		size *= 9ull;
	else if (accessor.type == easygltf::base::enumerators::accessor_type::mat4_type)
		size *= 16ull;

	return size;
}

// CGLTFProject

// Create default filesystem wrapper
void CGLTFProject::useDefaultFilesystemWrapper()
{
	m_pFS = std::make_unique<DefaultFilesystemWrapper>();
}

void CGLTFProject::setFilesystemWrapper(std::unique_ptr<IFilesystemWrapper>&& wrapper)
{
	m_pFS = std::move(wrapper);
}

void CGLTFProject::useDefaultDebugMessenger()
{
	m_pMessenger = std::make_unique<DefaultDebugMessenger>();
}

void CGLTFProject::setDebugMessenger(std::unique_ptr<IDebugMessenger>&& messenger)
{
	m_pMessenger = std::move(messenger);
}

bool CGLTFProject::load(const std::string& fsPath)
{
	if (!m_pFS)
		useDefaultFilesystemWrapper();

	if (!m_pMessenger)
		useDefaultDebugMessenger();

	m_srOpennedFilePath = fsPath;
	m_srParentPath = path_helper::parent_path(fsPath);

	return _load(fsPath, m_root);
}

bool CGLTFProject::save(const std::string& fsPath)
{
	if (bInBinaryMode)
		return _saveAsGlb(fsPath.empty() ? m_srOpennedFilePath : fsPath);

	return _saveAsGltf(fsPath.empty() ? m_srOpennedFilePath : fsPath);
}

// TODO: finish this part
bool CGLTFProject::import(const std::string& fsPath, const FImportGltfInfo& importInfo)
{
	bool res{ false };
	FGLTFRoot other{};

	res = _load(fsPath, other);
	if (!res)
		return false;

	auto accessor_offset = static_cast<int32_t>(m_root.accessors.size());
	auto buffer_view_offset = static_cast<int32_t>(m_root.bufferViews.size());
	auto buffer_offset = static_cast<int32_t>(m_root.buffers.size());
	auto node_offset = static_cast<int32_t>(m_root.nodes.size());
	auto camera_offset = static_cast<int32_t>(m_root.cameras.size());
	auto skin_offset = static_cast<int32_t>(m_root.skins.size());
	auto mesh_offset = static_cast<int32_t>(m_root.meshes.size());
	auto image_offset = static_cast<int32_t>(m_root.images.size());
	auto texture_offset = static_cast<int32_t>(m_root.textures.size());
	auto sampler_offset = static_cast<int32_t>(m_root.samplers.size());
	auto material_offset = static_cast<int32_t>(m_root.materials.size());
	
	// From extensions
	auto audio_emitter_offset = static_cast<int32_t>(m_root.khrAudio.emitters.size());
	auto audio_source_offset = static_cast<int32_t>(m_root.khrAudio.sources.size());
	auto light_offset = static_cast<int32_t>(m_root.khrLightsPunctual.lights.size());
	auto material_variant_offset = static_cast<int32_t>(m_root.khrMaterialVariants.variants.size());

	// Importing extensions
	for (auto& [name, extension] : other.extensions)
		m_root.extensions.emplace(name, extension);

	// Merge KHR_audio extension
	if (other.hasExtension<extensions::KHR_audio>())
	{
		auto& okhr_audio = other.khrAudio;
		auto& tkhr_audio = m_root.khrAudio;

		for (auto& emitter : okhr_audio.emitters)
			tkhr_audio.emitters.emplace_back(emitter);

		for (auto& source : okhr_audio.sources)
			tkhr_audio.sources.emplace_back(source);
	}

	// Merge KHR_lights_punctual extension
	if (other.hasExtension<extensions::KHR_lights_punctual>())
	{
		auto& okhr_lights_punctual = other.khrLightsPunctual;
		auto& tkhr_lights_punctual = m_root.khrLightsPunctual;

		for (auto& light : okhr_lights_punctual.lights)
			tkhr_lights_punctual.lights.emplace_back(light);
	}
	
	// Merge KHR_materials_variants extension
	if (other.hasExtension<extensions::KHR_materials_variants>())
	{
		auto& okhr_materials_variants = other.khrMaterialVariants;
		auto& tkhr_materials_variants = m_root.khrMaterialVariants;

		for (auto& variant : okhr_materials_variants.variants)
			tkhr_materials_variants.variants.emplace_back(variant);
	}

	// Merge buffers
	{
		for (auto& buffer : other.buffers)
			m_root.buffers.emplace_back(buffer);

		for (int32_t buffer_id = buffer_offset; buffer_id < m_root.buffers.size(); ++buffer_id)
			_loadBuffer(buffer_id);
	}

	// Merge buffer views
	{
		for (auto& buffer_view : other.bufferViews)
		{
			auto& nbuffer_view = m_root.bufferViews.emplace_back(buffer_view);
			nbuffer_view.buffer += buffer_offset;
		}
	}

	// Merge accessors
	{
		for (auto& accessor : other.accessors)
		{
			auto& naccessor = m_root.accessors.emplace_back(accessor);
			naccessor.bufferView += buffer_view_offset;
		}
	}

	// Merge scenes
	{
		auto& oscene = other.scenes[other.scene];
		auto& tscene = m_root.scenes[m_root.scene];

		for (int32_t node : oscene.nodes)
			tscene.nodes.emplace_back(node_offset + node);

		// Add extensions
		for (auto& [name, extension] : oscene.extensions)
			tscene.extensions.emplace(name, extension);

		if (oscene.hasExtension<extensions::KHR_audio>())
		{
			auto& okhr_audio = oscene.khrAudio;
			auto& tkhr_audio = tscene.khrAudio;

			for (int32_t emitter : okhr_audio.emitters)
				tkhr_audio.emitters.emplace_back(audio_emitter_offset + emitter);
		}

		// Add extras
		for (auto& [name, extra] : oscene.extras)
			tscene.extras.emplace(name, extra);
	}

	// Merge nodes
	{
		for (auto& node : other.nodes)
		{
			auto& nnode = m_root.nodes.emplace_back(node);

			helpers::add_when_set(nnode.camera, camera_offset);
			helpers::add_when_set(nnode.skin,  skin_offset);
			helpers::add_when_set(nnode.mesh,  mesh_offset);

			if (nnode.hasExtension<extensions::KHR_audio>())
				helpers::add_when_set(nnode.khrAudio.emitter, audio_emitter_offset);

			if (nnode.hasExtension<extensions::KHR_lights_punctual>())
				helpers::add_when_set(nnode.khrLightsPunctual.light, light_offset);

			for (auto& child : nnode.children)
				child += node_offset;
		}
	}

	// Merge cameras
	{
		for (auto& camera : other.cameras)
			m_root.cameras.emplace_back(camera);
	}

	// Merge skins
	{
		for (auto& skin : other.skins)
		{
			auto nskin = m_root.skins.emplace_back(skin);

			for (auto& joint : nskin.joints)
				joint += node_offset;

			nskin.skeleton += node_offset;
		}
	}

	// Merge images
	{
		for (auto& image : other.images)
		{
			auto& nimage = m_root.images.emplace_back(image);
			helpers::add_when_set(nimage.bufferView, buffer_view_offset);
		}
	}

	// Merge samplers
	{
		for (auto& sampler : other.samplers)
			m_root.samplers.emplace_back(sampler);
	}

	// Merge textures
	{
		for (auto& texture : other.textures)
		{
			auto& ntexture = m_root.textures.emplace_back(texture);	
			helpers::add_when_set(ntexture.sampler, sampler_offset);
		}
	}

	// Merge meshes
	{
		for (auto& mesh : other.meshes)
		{
			auto& nmesh = m_root.meshes.emplace_back(mesh);
			for (auto& primitive : nmesh.primitives)
			{
				for (auto& [name, accessor] : primitive.attributes)
					accessor += accessor_offset;

				helpers::add_when_set(primitive.indices, accessor_offset);
				helpers::add_when_set(primitive.material, material_offset);

				if (primitive.hasExtension<extensions::KHR_draco_mesh_compression>())
				{
					auto& ext = primitive.khrDracoMeshCompression;
					ext.bufferView += buffer_view_offset;

					for (auto& [name, accessor] : ext.attributes)
						accessor += accessor_offset;
				}

				if (primitive.hasExtension<extensions::KHR_materials_variants>())
				{
					auto& ext = primitive.khrMaterialVariants;

					for (auto& mapping : ext.mappings)
					{
						mapping.material += material_offset;

						for (auto& variant : mapping.variants)
							variant += material_variant_offset;
					}
				}
			}
		}
	}

	// Merge materials
	{
		for (auto& material : other.materials)
		{
			auto& nmaterial = m_root.materials.emplace_back(material);

			helpers::add_when_set(nmaterial.pbrMetallicRoughness.baseColorTexture.index, texture_offset);
			helpers::add_when_set(nmaterial.pbrMetallicRoughness.metallicRoughnessTexture.index, texture_offset);
			helpers::add_when_set(nmaterial.normalTexture.index, texture_offset);
			helpers::add_when_set(nmaterial.occlusionTexture.index, texture_offset);
			helpers::add_when_set(nmaterial.emissiveTexture.index, texture_offset);

			if (nmaterial.hasExtension<extensions::EXT_materials_displacement>())
			{
				auto& ext = nmaterial.extMaterialDisplacement;
				helpers::add_when_set(ext.displacementTexture.index, texture_offset);
			}

			if (nmaterial.hasExtension<extensions::KHR_materials_anisotropy>())
			{
				auto& ext = nmaterial.khrMaterialAnisotropy;
				helpers::add_when_set(ext.anisotropyTexture.index, texture_offset);
			}

			if (nmaterial.hasExtension<extensions::KHR_materials_clearcoat>())
			{
				auto& ext = nmaterial.khrMaterialClearcoat;
				helpers::add_when_set(ext.clearcoatTexture.index, texture_offset);
				helpers::add_when_set(ext.clearcoatRoughnessTexture.index, texture_offset);
				helpers::add_when_set(ext.clearcoatNormalTexture.index, texture_offset);
			}

			if (nmaterial.hasExtension<extensions::KHR_materials_iridescence>())
			{
				auto& ext = nmaterial.khrMaterialIridescence;
				helpers::add_when_set(ext.iridescenceTexture.index, texture_offset);
				helpers::add_when_set(ext.iridescenceThicknessTexture.index, texture_offset);
			}

			if (nmaterial.hasExtension<extensions::KHR_materials_sheen>())
			{
				auto& ext = nmaterial.khrMaterialSheen;
				helpers::add_when_set(ext.sheenColorTexture.index, texture_offset);
				helpers::add_when_set(ext.sheenRoughnessTexture.index, texture_offset);
			}

			if (nmaterial.hasExtension<extensions::KHR_materials_specular>())
			{
				auto& ext = nmaterial.khrMaterialSpecular;
				helpers::add_when_set(ext.specularTexture.index, texture_offset);
				helpers::add_when_set(ext.specularColorTexture.index, texture_offset);
			}

			if (nmaterial.hasExtension<extensions::KHR_materials_transmission>())
			{
				auto& ext = nmaterial.khrMaterialTransmission;
				helpers::add_when_set(ext.transmissionTexture.index, texture_offset);
			}

			if (nmaterial.hasExtension<extensions::KHR_materials_volume>())
			{
				auto& ext = nmaterial.khrMaterialVolume;
				helpers::add_when_set(ext.thicknessTexture.index, texture_offset);
			}
		}
	}

	return validate(m_root, m_pMessenger);
}

const FGLTFRoot& CGLTFProject::getGltf() const
{
	return m_root;
}

std::string CGLTFProject::getResourcePath(const std::string& path)
{
	return path_helper::path_append(m_srParentPath, path_helper::url_decode(path));
}

Stream CGLTFProject::getStream(const Accessor& accessor)
{
	Stream stream{};

	const BufferView& buffer_view = m_root.bufferViews[accessor.bufferView];
	auto& buffer = m_root.buffers[buffer_view.buffer];

	stream.stride = get_accessor_element_size(accessor);
	stream.count = accessor.count;
	stream.data = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];

	return stream;
}

Stream CGLTFProject::getStream(int32_t accessor_id)
{
	const Accessor& accessor = m_root.accessors[accessor_id];
	return getStream(accessor);
}

int32_t CGLTFProject::createAccessor(const std::string& name)
{
	Accessor accessor{};
	accessor.name = name;
	m_root.accessors.emplace_back(accessor);

	return static_cast<int32_t>(m_root.accessors.size() - 1ull);
}

int32_t CGLTFProject::createAnimation(const std::string& name)
{
	Animation animation{};
	animation.name = name;
	m_root.animations.emplace_back(animation);

	return static_cast<int32_t>(m_root.animations.size() - 1ull);
}

int32_t CGLTFProject::createBuffer(const std::string& name)
{
	Buffer buffer{};
	buffer.name = name;
	m_root.buffers.emplace_back(buffer);

	return static_cast<int32_t>(m_root.buffers.size() - 1ull);
}

int32_t CGLTFProject::createBufferView(const std::string& name)
{
	BufferView bufferView{};
	bufferView.name = name;
	m_root.bufferViews.emplace_back(bufferView);

	return static_cast<int32_t>(m_root.bufferViews.size() - 1ull);
}

int32_t CGLTFProject::createCamera(const std::string& name)
{
	Camera camera{};
	camera.name = name;
	m_root.cameras.emplace_back(camera);

	return static_cast<int32_t>(m_root.cameras.size() - 1ull);
}

int32_t CGLTFProject::createImage(const std::string& name)
{
	Image image{};
	image.name = name;
	m_root.images.emplace_back(image);

	return static_cast<int32_t>(m_root.images.size() - 1ull);
}

int32_t CGLTFProject::createMaterial(const std::string& name)
{
	Material material{};
	material.name = name;
	m_root.materials.emplace_back(material);

	return static_cast<int32_t>(m_root.materials.size() - 1ull);
}

int32_t CGLTFProject::createMesh(const std::string& name)
{
	Mesh mesh{};
	mesh.name = name;
	m_root.meshes.emplace_back(mesh);

	return static_cast<int32_t>(m_root.meshes.size() - 1ull);
}

int32_t CGLTFProject::createNode(const std::string& name)
{
	Node node{};
	node.name = name;
	m_root.nodes.emplace_back(node);

	return static_cast<int32_t>(m_root.nodes.size() - 1ull);
}

int32_t CGLTFProject::createSampler(const std::string& name)
{
	Sampler sampler{};
	sampler.name = name;
	m_root.samplers.emplace_back(sampler);

	return static_cast<int32_t>(m_root.samplers.size() - 1ull);
}

int32_t CGLTFProject::createScene(const std::string& name)
{
	Scene scene{};
	scene.name = name;
	m_root.scenes.emplace_back(scene);

	return static_cast<int32_t>(m_root.scenes.size() - 1ull);
}

int32_t CGLTFProject::createSkin(const std::string& name)
{
	Skin skin{};
	skin.name = name;
	m_root.skins.emplace_back(skin);

	return static_cast<int32_t>(m_root.skins.size() - 1ull);
}

int32_t CGLTFProject::createTexture(const std::string& name)
{
	Texture texture{};
	texture.name = name;
	m_root.textures.emplace_back(texture);

	return static_cast<int32_t>(m_root.textures.size() - 1ull);
}

size_t CGLTFProject::removeAccessor(int32_t index)
{
	m_root.accessors.erase(std::next(m_root.accessors.begin(), index));

	// Update data for animations
	for (auto& animation : m_root.animations)
	{
		for (auto& sampler : animation.samplers)
		{
			updateDataIndex(sampler.input, index);
			updateDataIndex(sampler.output, index);
		}
	}

	// Update primitives
	for (auto& mesh : m_root.meshes)
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
	for (auto& skin : m_root.skins)
		updateDataIndex(skin.inverseBindMatrices, index);

	return m_root.accessors.size();
}

size_t CGLTFProject::removeAnimation(int32_t index)
{
	m_root.animations.erase(std::next(m_root.animations.begin(), index));
	return m_root.animations.size();
}

size_t CGLTFProject::removeBuffer(int32_t index)
{
	m_root.buffers.erase(std::next(m_root.buffers.begin(), index));
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_root.bufferViews, index, [](BufferView& view) -> int32_t& { return view.buffer; });
	return m_root.buffers.size();
}

size_t CGLTFProject::removeBufferView(int32_t index)
{
	m_root.bufferViews.erase(std::next(m_root.bufferViews.begin(), index));

	// At first remove it for images
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_root.images, index, [](Image& image) -> int32_t& { return image.bufferView; });

	// Remove for accessors
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_root.accessors, index, [](Accessor& accessor) -> int32_t& { return accessor.bufferView; });

	// Remove for sparse accessors
	for (auto& accessor : m_root.accessors)
	{
		auto& sparse = accessor.sparse;
		updateDataIndex(sparse.indices.bufferView, index);
		updateDataIndex(sparse.values.bufferView, index);
	}

	return m_root.bufferViews.size();
}

size_t CGLTFProject::removeCamera(int32_t index)
{
	m_root.cameras.erase(std::next(m_root.cameras.begin(), index));
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_root.nodes, index, [](Node& node) -> int32_t& { return node.camera; });
	return m_root.cameras.size();
}

size_t CGLTFProject::removeImage(int32_t index)
{
	m_root.images.erase(std::next(m_root.images.begin(), index));

	// Find all image usages and resort them
	for (auto& texture : m_root.textures)
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

	return m_root.images.size();
}

size_t CGLTFProject::removeMaterial(int32_t index)
{
	m_root.materials.erase(std::next(m_root.materials.begin(), index));

	// Remove material from primitives
	for (auto& mesh : m_root.meshes)
	{
		for (auto& primitive : mesh.primitives)
			updateDataIndex(primitive.material, index);
	}

	return m_root.materials.size();
}

size_t CGLTFProject::removeMesh(int32_t index)
{
	m_root.meshes.erase(std::next(m_root.meshes.begin(), index));
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_root.nodes, index, [](Node& node) -> int32_t& { return node.mesh; });
	return m_root.meshes.size();
}

size_t CGLTFProject::removeNode(int32_t index)
{
	m_root.nodes.erase(std::next(m_root.nodes.begin(), index));

	// Remove node from scenes
	for (auto& scene : m_root.scenes)
		removeIdFromVectorAndDecrementIdWithHigherIndex(scene.nodes, index);

	// Remove node from children
	for (auto& node : m_root.nodes)
		removeIdFromVectorAndDecrementIdWithHigherIndex(node.children, index);

	// Remove node from animations
	for (auto& animation : m_root.animations)
		removeIdFromVectorAndDecrementIdWithHigherIndexPred(animation.channels, index, [](AnimationChannel& cmannel) -> int32_t& { return cmannel.target.node; });

	// Remove node from skin
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_root.skins, index, [](Skin& node) -> int32_t& { return node.skeleton; });

	return m_root.nodes.size();
}

size_t CGLTFProject::removeSampler(int32_t index)
{
	m_root.samplers.erase(std::next(m_root.samplers.begin(), index));
	removeIdFromVectorAndDecrementIdWithHigherIndexPred(m_root.textures, index, [](Texture& texture) -> int32_t& { return texture.sampler; });
	return m_root.samplers.size();
}

size_t CGLTFProject::removeScene(int32_t index)
{
	m_root.scenes.erase(std::next(m_root.scenes.begin(), index));

	if (index == m_root.scene)
		m_root.scene = base::constants::not_set;

	return m_root.scenes.size();
}

size_t CGLTFProject::removeSkin(int32_t index)
{
	// Remove bones and also vertex data
	m_root.skins.erase(std::next(m_root.skins.begin(), index));
	return m_root.skins.size();
}

size_t CGLTFProject::removeTexture(int32_t index)
{
	m_root.textures.erase(std::next(m_root.textures.begin(), index));

	for (auto& material : m_root.materials)
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

	return m_root.textures.size();
}

bool CGLTFProject::_load(const std::string& fsPath, FGLTFRoot& gltf_scene)
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
		return _loadGlbFromMemory(rdata, gltf_scene);

	return _loadGltfFromMemory(rdata, gltf_scene);
}

bool CGLTFProject::_loadGltfFromMemory(const std::vector<uint8_t>& data, FGLTFRoot& gltf_scene)
{
	auto json = nlohmann::json::parse(data);
	json.get_to(gltf_scene);

	// Load buffers
	for (size_t buffer_id = 0ull; buffer_id < gltf_scene.buffers.size(); ++buffer_id)
	{
		if (!_loadBuffer(buffer_id))
			return false;
	}

	return validate(gltf_scene, m_pMessenger);
}

bool CGLTFProject::_loadGlbFromMemory(const std::vector<uint8_t>& data, FGLTFRoot& gltf_scene)
{
	bInBinaryMode = true;

	return validate(gltf_scene, m_pMessenger);
}

bool CGLTFProject::_saveAsGltf(const std::string& path)
{
	auto json = nlohmann::json(m_root);
	auto dumped = json.dump(4);
	
	return m_pFS->writeFile(path, std::vector<uint8_t>(dumped.begin(), dumped.end()));
}

bool CGLTFProject::_saveAsGlb(const std::string& path)
{
	return false;
}

bool CGLTFProject::_loadBuffer(int32_t buffer_id)
{
	auto& buffer = m_root.buffers.at(buffer_id);

	auto buffer_path = path_helper::path_append(m_srParentPath, path_helper::url_decode(buffer.uri));
	if (!m_pFS->readFile(buffer_path, buffer.data))
	{
		m_pMessenger->message(DebugMessageType::error, std::format("Failed to read file {}", buffer_path));
		return false;
	}

	return false;
}