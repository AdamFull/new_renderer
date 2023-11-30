#include <easygltf_validator.h>

constexpr const char* validation_error_not_set{ "Validation Error: {} with id \"{}\", have not set required attribure \"{}\"" };
constexpr const char* validation_error_invalid_value{ "Validation Error: {} with id \"{}\", has invalid required attribute \"{}\". Value: {}" };

// Accessor validation
bool easygltf::validateSparseAccessorIndices(size_t id, const SparseAccessorIndices& sparseAccessorIndices, const std::unique_ptr<IDebugMessenger>& messenger)
{
	auto isValueValid =
		sparseAccessorIndices.componentType == base::enumerators::component_type::byte_type ||
		sparseAccessorIndices.componentType == base::enumerators::component_type::ubyte_type ||
		sparseAccessorIndices.componentType == base::enumerators::component_type::short_type ||
		sparseAccessorIndices.componentType == base::enumerators::component_type::ushort_type ||
		sparseAccessorIndices.componentType == base::enumerators::component_type::int_type ||
		sparseAccessorIndices.componentType == base::enumerators::component_type::uint_type ||
		sparseAccessorIndices.componentType == base::enumerators::component_type::float_type ||
		sparseAccessorIndices.componentType == base::enumerators::component_type::double_type;

	if (sparseAccessorIndices.bufferView == base::constants::not_set)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "SparseAccessorIndices", id, "bufferView"));
		return false;
	}

	if (!isValueValid)
	{
		if(sparseAccessorIndices.componentType == base::constants::not_set)
			messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "SparseAccessorIndices", id, "componentType"));
		else
			messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "SparseAccessorIndices", id, "componentType", sparseAccessorIndices.componentType));

		return false;
	}

	return true;
}

bool easygltf::validateSparseAccessorValues(size_t id, const SparseAccessorValues& sparseAccessorValues, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (sparseAccessorValues.bufferView == base::constants::not_set)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "SparseAccessorValues", id, "bufferView"));
		return false;
	}

	return true;
}

bool easygltf::validateSparseAccessor(size_t id, const SparseAccessor& sparseAccessor, const std::unique_ptr<IDebugMessenger>& messenger)
{
	bool result{ true };

	if (sparseAccessor.count == base::constants::not_set)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "SparseAccessor", id, "count"));
		result = false;
	}

	result = result && validateSparseAccessorIndices(id, sparseAccessor.indices, messenger);
	result = result && validateSparseAccessorValues(id, sparseAccessor.values, messenger);

	return result;
}

bool easygltf::validateAccessor(size_t id, const Accessor& accessor, const std::unique_ptr<IDebugMessenger>& messenger)
{
	auto isValueValid =
		accessor.componentType == base::enumerators::component_type::byte_type ||
		accessor.componentType == base::enumerators::component_type::ubyte_type ||
		accessor.componentType == base::enumerators::component_type::short_type ||
		accessor.componentType == base::enumerators::component_type::ushort_type ||
		accessor.componentType == base::enumerators::component_type::int_type ||
		accessor.componentType == base::enumerators::component_type::uint_type ||
		accessor.componentType == base::enumerators::component_type::float_type ||
		accessor.componentType == base::enumerators::component_type::double_type;

	// Check accessor count
	if (accessor.count == 0ull)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "Accessor", id, "count"));
		return false;
	}

	// Check component type
	if (!isValueValid)
	{
		if(accessor.componentType == base::constants::not_set)
			messenger->message(DebugMessageType::error, std::format(validation_error_not_set , "Accessor", id, "componentType"));
		else
			messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "Accessor", id, "componentType", accessor.componentType));

		return false;
	}

	if (accessor.type.empty())
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "Accessor", id, "type"));
		return false;
	}

	if (accessor.sparse != SparseAccessor{} && !validateSparseAccessor(id, accessor.sparse, messenger))
		return false;

	return true;
}

// Animation validation
bool easygltf::validateAnimationTarget(size_t id, const AnimationTarget& animationTarget, const std::unique_ptr<IDebugMessenger>& messenger)
{
	auto isValueValid =
		animationTarget.path == base::enumerators::animation_path::translation ||
		animationTarget.path == base::enumerators::animation_path::rotation ||
		animationTarget.path == base::enumerators::animation_path::scale ||
		animationTarget.path == base::enumerators::animation_path::weights;

	if (!isValueValid)
	{
		if(animationTarget.path.empty())
			messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "AnimationTarget", id, "path"));
		else
			messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "AnimationTarget", id, "path", animationTarget.path));

		return false;
	}

	return true;
}

bool easygltf::validateAnimationChannel(size_t id, const AnimationChannel& animationChannel, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (animationChannel.sampler == base::constants::not_set)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "AnimationChannel", id, "sampler"));
		return false;
	}

	return validateAnimationTarget(id, animationChannel.target, messenger);
}

bool easygltf::validateAnimationSampler(size_t id, const AnimationSampler& animationSampler, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (animationSampler.input == base::constants::not_set)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "AnimationSampler", id, "input"));
		return false;
	}

	if (animationSampler.output == base::constants::not_set)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "AnimationSampler", id, "output"));
		return false;
	}

	return true;
}

bool easygltf::validateAnimation(size_t id, const Animation& animation, const std::unique_ptr<IDebugMessenger>& messenger)
{
	bool result{ true };

	for (size_t sid = 0ull; sid < animation.samplers.size(); ++sid)
		result = result && validateAnimationSampler(sid, animation.samplers.at(sid), messenger);

	for (size_t cid = 0ull; cid < animation.channels.size(); ++cid)
		result = result && validateAnimationChannel(cid, animation.channels.at(cid), messenger);

	return result;
}

// Validate asset
bool easygltf::validateAsset(const Asset& asset, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (asset.version.empty())
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "Asset", 0, "version"));
		return false;
	}

	return true;
}

// Validate Buffer
bool easygltf::validateBuffer(size_t id, const Buffer& buffer, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (buffer.byteLength == 0ull)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "Buffer", id, "byteLength"));
		return false;
	}

	return true;
}

// Validate BufferView
bool easygltf::validateBufferView(size_t id, const BufferView& bufferView, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (bufferView.buffer == base::constants::not_set)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "BufferView", id, "buffer"));
		return false;
	}

	if (bufferView.byteLength == 0ull)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "BufferView", id, "byteLength"));
		return false;
	}

	return true;
}

// Camera validation
bool easygltf::validatePerspectiveCamera(size_t id, const PerspectiveCamera& camera, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (camera.aspectRatio < 0.0)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "PerspectiveCamera", id, "aspectRatio", camera.aspectRatio));
		return false;
	}

	if (camera.yfov < 0.0)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "PerspectiveCamera", id, "yfov", camera.yfov));
		return false;
	}

	if (camera.zfar < 0.0)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "PerspectiveCamera", id, "zfar", camera.zfar));
		return false;
	}

	if (camera.znear < 0.0)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "PerspectiveCamera", id, "znear", camera.znear));
		return false;
	}

	return true;
}

bool easygltf::validateOrthographicCamera(size_t id, const OrthographicCamera& camera, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (camera.zfar < 0.0)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "OrthographicCamera", id, "zfar", camera.zfar));
		return false;
	}

	if (camera.znear < 0.0)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "OrthographicCamera", id, "znear", camera.znear));
		return false;
	}

	return true;
}

bool easygltf::validateCamera(size_t id, const Camera& camera, const std::unique_ptr<IDebugMessenger>& messenger)
{
	auto isValueValid =
		camera.type == base::enumerators::camera_type::perspective ||
		camera.type == base::enumerators::camera_type::orthographic;

	if (!isValueValid)
	{
		if(camera.type.empty())
			messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "Camera", id, "type"));
		else
			messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "Camera", id, "type", camera.type));

		return false;
	}

	if (camera.type == base::enumerators::camera_type::perspective)
		return validatePerspectiveCamera(id, camera.perspective, messenger);

	return validateOrthographicCamera(id, camera.orthographic, messenger);
}

// Validate Image
bool easygltf::validateImage(size_t id, const Image& image, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (image.uri.empty())
	{
		if (image.bufferView == base::constants::not_set || image.mimeType.empty())
		{
			messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "Image", id, "uri"));
			return false;
		}
	}

	return true;
}

// Maretial validation
bool easygltf::validateOcclusionTexture(size_t id, const OcclusionTexture& occlusionTexture, const std::unique_ptr<IDebugMessenger>& messenger)
{
	auto* baseClass = static_cast<const TextureInfo*>(&occlusionTexture);
	return validateTextureInfo(id, *baseClass, messenger);
}

bool easygltf::validateNormalTexture(size_t id, const NormalTexture& normalTexture, const std::unique_ptr<IDebugMessenger>& messenger)
{
	auto* baseClass = static_cast<const TextureInfo*>(&normalTexture);
	return validateTextureInfo(id, *baseClass, messenger);
}

bool easygltf::validateTextureInfo(size_t id, const TextureInfo& textureInfo, const std::unique_ptr<IDebugMessenger>& messenger)
{
	//if (textureInfo.index == base::constants::not_set)
	//{
	//	errors << std::format(validation_error_not_set, "TextureInfo", id, "index");
	//	return false;
	//}

	return true;
}

bool easygltf::validatePbrMetallicRoughness(size_t id, const PbrMetallicRoughness& pbrMetallicRoughness, const std::unique_ptr<IDebugMessenger>& messenger)
{
	bool result{ true };

	result = result && validateTextureInfo(id, pbrMetallicRoughness.baseColorTexture, messenger);
	result = result && validateTextureInfo(id, pbrMetallicRoughness.metallicRoughnessTexture, messenger);

	return result;
}

bool easygltf::validateMaterial(size_t id, const Material& material, const std::unique_ptr<IDebugMessenger>& messenger)
{
	bool result{ true };

	result = result && validatePbrMetallicRoughness(id, material.pbrMetallicRoughness, messenger);
	result = result && validateNormalTexture(id, material.normalTexture, messenger);
	result = result && validateOcclusionTexture(id, material.occlusionTexture, messenger);
	result = result && validateTextureInfo(id, material.emissiveTexture, messenger);

	return result;
}

// Validate mesh
bool easygltf::validatePrimitive(size_t id, const Primitive& primitive, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (primitive.attributes.empty())
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "Primitive", id, "attributes"));
		return false;
	}

	return true;
}

bool easygltf::validateMesh(size_t id, const Mesh& mesh, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (mesh.primitives.empty())
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "Mesh", id, "primitives"));
		return false;
	}

	return true;
}

// Validate Node
bool easygltf::validateNode(size_t id, const Node& node, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (!compare_dmat4(node.matrix, base::constants::dmat4identity))
	{
		return
			compare_dvec3(node.translation, base::constants::dvec3zeros) ||
			compare_dvec4(node.rotation, base::constants::dvec4quat_default) ||
			compare_dvec3(node.scale, base::constants::dvec3ones);
	}

	return true;
}

// Validate sampler
bool easygltf::validateSampler(size_t id, const Sampler& sampler, const std::unique_ptr<IDebugMessenger>& messenger)
{
	bool isValidWrapS =
		sampler.wrapS == base::enumerators::texture_wrap::repeat ||
		sampler.wrapS == base::enumerators::texture_wrap::clamp_to_edge ||
		sampler.wrapS == base::enumerators::texture_wrap::mirrored_repeat;

	if (!isValidWrapS)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "Sampler", id, "wrapS", sampler.wrapS));
		return false;
	}

	bool isValidWrapT =
		sampler.wrapT == base::enumerators::texture_wrap::repeat ||
		sampler.wrapT == base::enumerators::texture_wrap::clamp_to_edge ||
		sampler.wrapT == base::enumerators::texture_wrap::mirrored_repeat;

	if (!isValidWrapT)
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_invalid_value, "Sampler", id, "wrapT", sampler.wrapT));
		return false;
	}

	return true;
}

// Validate Scene
bool easygltf::validateScene(size_t id, const Scene& scene, const std::unique_ptr<IDebugMessenger>& messenger)
{
	// Validate node indices
	return true;
}

// Validate Skin
bool easygltf::validateSkin(size_t id, const Skin& skin, const std::unique_ptr<IDebugMessenger>& messenger)
{
	if (skin.joints.empty())
	{
		messenger->message(DebugMessageType::error, std::format(validation_error_not_set, "Skin", id, "joints"));
		return false;
	}

	return true;
}

// Validate Texture
bool easygltf::validateTexture(size_t id, const Texture& texture, const std::unique_ptr<IDebugMessenger>& messenger)
{
	return true;
}

// All validation
bool easygltf::validate(const FGLTFRoot& gltf, const std::unique_ptr<IDebugMessenger>& messenger)
{
	bool result{ true };

	// Validate accessors
	for (size_t id = 0ull; id < gltf.accessors.size(); ++id)
		result = result && validateAccessor(id, gltf.accessors.at(id), messenger);

	// Validate animations
	for (size_t id = 0ull; id < gltf.animations.size(); ++id)
		result = result && validateAnimation(id, gltf.animations.at(id), messenger);

	// Validate buffers
	for (size_t id = 0ull; id < gltf.buffers.size(); ++id)
		result = result && validateBuffer(id, gltf.buffers.at(id), messenger);

	// Validate buffer views
	for (size_t id = 0ull; id < gltf.bufferViews.size(); ++id)
		result = result && validateBufferView(id, gltf.bufferViews.at(id), messenger);

	// Validate cameras
	for (size_t id = 0ull; id < gltf.cameras.size(); ++id)
		result = result && validateCamera(id, gltf.cameras.at(id), messenger);

	// Validate images
	for (size_t id = 0ull; id < gltf.images.size(); ++id)
		result = result && validateImage(id, gltf.images.at(id), messenger);

	// Validate materials
	for (size_t id = 0ull; id < gltf.materials.size(); ++id)
		result = result && validateMaterial(id, gltf.materials.at(id), messenger);

	// Validate meshes
	for (size_t id = 0ull; id < gltf.meshes.size(); ++id)
		result = result && validateMesh(id, gltf.meshes.at(id), messenger);

	// Validate nodes
	for (size_t id = 0ull; id < gltf.nodes.size(); ++id)
		result = result && validateNode(id, gltf.nodes.at(id), messenger);

	// Validate samplers
	for (size_t id = 0ull; id < gltf.samplers.size(); ++id)
		result = result && validateSampler(id, gltf.samplers.at(id), messenger);

	// Validate scenes
	for (size_t id = 0ull; id < gltf.scenes.size(); ++id)
		result = result && validateScene(id, gltf.scenes.at(id), messenger);

	// Validate skins
	for (size_t id = 0ull; id < gltf.skins.size(); ++id)
		result = result && validateSkin(id, gltf.skins.at(id), messenger);

	// Validate textures
	for (size_t id = 0ull; id < gltf.textures.size(); ++id)
		result = result && validateTexture(id, gltf.textures.at(id), messenger);

	// Validate asset
	result = result && validateAsset(gltf.asset, messenger);

	return result;
}