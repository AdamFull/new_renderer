#include "easygltf_validator.h"

constexpr const char* validation_error_not_set{ "Validation Error: {} with id \"{}\", have not set required attribure \"{}\"" };
constexpr const char* validation_error_invalid_value{ "Validation Error: {} with id \"{}\", has invalid required attribute \"{}\". Value: {}" };

// Accessor validation
bool easygltf::validateSparseAccessorIndices(size_t id, const SparseAccessorIndices& sparseAccessorIndices, std::stringstream& errors, std::stringstream& warnings)
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
		errors << std::format(validation_error_not_set, "SparseAccessorIndices", id, "bufferView");
		return false;
	}

	if (!isValueValid)
	{
		if(sparseAccessorIndices.componentType == base::constants::not_set)
			errors << std::format(validation_error_not_set, "SparseAccessorIndices", id, "componentType");
		else
			errors << std::format(validation_error_invalid_value, "SparseAccessorIndices", id, "componentType", sparseAccessorIndices.componentType);

		return false;
	}

	return true;
}

bool easygltf::validateSparseAccessorValues(size_t id, const SparseAccessorValues& sparseAccessorValues, std::stringstream& errors, std::stringstream& warnings)
{
	if (sparseAccessorValues.bufferView == base::constants::not_set)
	{
		errors << std::format(validation_error_not_set, "SparseAccessorValues", id, "bufferView");
		return false;
	}

	return true;
}

bool easygltf::validateSparseAccessor(size_t id, const SparseAccessor& sparseAccessor, std::stringstream& errors, std::stringstream& warnings)
{
	bool result{ true };

	if (sparseAccessor.count == base::constants::not_set)
	{
		errors << std::format(validation_error_not_set, "SparseAccessor", id, "count");
		result = false;
	}

	result = result && validateSparseAccessorIndices(id, sparseAccessor.indices, errors, warnings);
	result = result && validateSparseAccessorValues(id, sparseAccessor.values, errors, warnings);

	return result;
}

bool easygltf::validateAccessor(size_t id, const Accessor& accessor, std::stringstream& errors, std::stringstream& warnings)
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
		errors << std::format(validation_error_not_set, "Accessor", id, "count");
		return false;
	}

	// Check component type
	if (!isValueValid)
	{
		if(accessor.componentType == base::constants::not_set)
			errors << std::format(validation_error_not_set , "Accessor", id, "componentType");
		else
			errors << std::format(validation_error_invalid_value, "Accessor", id, "componentType", accessor.componentType);

		return false;
	}

	if (accessor.type.empty())
	{
		errors << std::format(validation_error_not_set, "Accessor", id, "type");
		return false;
	}

	if (accessor.sparse != SparseAccessor{} && !validateSparseAccessor(id, accessor.sparse, errors, warnings))
		return false;

	return true;
}

// Animation validation
bool easygltf::validateAnimationTarget(size_t id, const AnimationTarget& animationTarget, std::stringstream& errors, std::stringstream& warnings)
{
	auto isValueValid =
		animationTarget.path == base::enumerators::animation_path::translation ||
		animationTarget.path == base::enumerators::animation_path::rotation ||
		animationTarget.path == base::enumerators::animation_path::scale ||
		animationTarget.path == base::enumerators::animation_path::weights;

	if (!isValueValid)
	{
		if(animationTarget.path.empty())
			errors << std::format(validation_error_not_set, "AnimationTarget", id, "path");
		else
			errors << std::format(validation_error_invalid_value, "AnimationTarget", id, "path", animationTarget.path);

		return false;
	}

	return true;
}

bool easygltf::validateAnimationChannel(size_t id, const AnimationChannel& animationChannel, std::stringstream& errors, std::stringstream& warnings)
{
	if (animationChannel.sampler == base::constants::not_set)
	{
		errors << std::format(validation_error_not_set, "AnimationChannel", id, "sampler");
		return false;
	}

	return validateAnimationTarget(id, animationChannel.target, errors, warnings);
}

bool easygltf::validateAnimationSampler(size_t id, const AnimationSampler& animationSampler, std::stringstream& errors, std::stringstream& warnings)
{
	if (animationSampler.input == base::constants::not_set)
	{
		errors << std::format(validation_error_not_set, "AnimationSampler", id, "input");
		return false;
	}

	if (animationSampler.output == base::constants::not_set)
	{
		errors << std::format(validation_error_not_set, "AnimationSampler", id, "output");
		return false;
	}

	return true;
}

bool easygltf::validateAnimation(size_t id, const Animation& animation, std::stringstream& errors, std::stringstream& warnings)
{
	bool result{ true };

	for (size_t sid = 0ull; sid < animation.samplers.size(); ++sid)
		result = result && validateAnimationSampler(sid, animation.samplers.at(sid), errors, warnings);

	for (size_t cid = 0ull; cid < animation.channels.size(); ++cid)
		result = result && validateAnimationChannel(cid, animation.channels.at(cid), errors, warnings);

	return result;
}

// Validate asset
bool easygltf::validateAsset(const Asset& asset, std::stringstream& errors, std::stringstream& warnings)
{
	if (asset.version.empty())
	{
		errors << std::format(validation_error_not_set, "Asset", 0, "version");
		return false;
	}

	return true;
}

// Validate Buffer
bool easygltf::validateBuffer(size_t id, const Buffer& buffer, std::stringstream& errors, std::stringstream& warnings)
{
	if (buffer.byteLength == 0ull)
	{
		errors << std::format(validation_error_not_set, "Buffer", id, "byteLength");
		return false;
	}

	return true;
}

// Validate BufferView
bool easygltf::validateBufferView(size_t id, const BufferView& bufferView, std::stringstream& errors, std::stringstream& warnings)
{
	if (bufferView.buffer == base::constants::not_set)
	{
		errors << std::format(validation_error_not_set, "BufferView", id, "buffer");
		return false;
	}

	if (bufferView.byteLength == 0ull)
	{
		errors << std::format(validation_error_not_set, "BufferView", id, "byteLength");
		return false;
	}

	return true;
}

// Camera validation
bool easygltf::validatePerspectiveCamera(size_t id, const PerspectiveCamera& camera, std::stringstream& errors, std::stringstream& warnings)
{
	if (camera.aspectRatio < 0.0)
	{
		errors << std::format(validation_error_invalid_value, "PerspectiveCamera", id, "aspectRatio", camera.aspectRatio);
		return false;
	}

	if (camera.yfov < 0.0)
	{
		errors << std::format(validation_error_invalid_value, "PerspectiveCamera", id, "yfov", camera.yfov);
		return false;
	}

	if (camera.zfar < 0.0)
	{
		errors << std::format(validation_error_invalid_value, "PerspectiveCamera", id, "zfar", camera.zfar);
		return false;
	}

	if (camera.znear < 0.0)
	{
		errors << std::format(validation_error_invalid_value, "PerspectiveCamera", id, "znear", camera.znear);
		return false;
	}

	return true;
}

bool easygltf::validateOrthographicCamera(size_t id, const OrthographicCamera& camera, std::stringstream& errors, std::stringstream& warnings)
{
	if (camera.zfar < 0.0)
	{
		errors << std::format(validation_error_invalid_value, "OrthographicCamera", id, "zfar", camera.zfar);
		return false;
	}

	if (camera.znear < 0.0)
	{
		errors << std::format(validation_error_invalid_value, "OrthographicCamera", id, "znear", camera.znear);
		return false;
	}

	return true;
}

bool easygltf::validateCamera(size_t id, const Camera& camera, std::stringstream& errors, std::stringstream& warnings)
{
	auto isValueValid =
		camera.type == base::enumerators::camera_type::perspective ||
		camera.type == base::enumerators::camera_type::orthographic;

	if (!isValueValid)
	{
		if(camera.type.empty())
			errors << std::format(validation_error_not_set, "Camera", id, "type");
		else
			errors << std::format(validation_error_invalid_value, "Camera", id, "type", camera.type);

		return false;
	}

	if (camera.type == base::enumerators::camera_type::perspective)
		return validatePerspectiveCamera(id, camera.perspective, errors, warnings);

	return validateOrthographicCamera(id, camera.orthographic, errors, warnings);
}

// Validate Image
bool easygltf::validateImage(size_t id, const Image& image, std::stringstream& errors, std::stringstream& warnings)
{
	if (image.uri.empty())
	{
		if (image.bufferView == base::constants::not_set || image.mimeType.empty())
		{
			errors << std::format(validation_error_not_set, "Image", id, "uri");
			return false;
		}
	}

	return true;
}

// Maretial validation
bool easygltf::validateOcclusionTexture(size_t id, const OcclusionTexture& occlusionTexture, std::stringstream& errors, std::stringstream& warnings)
{
	auto* baseClass = static_cast<const TextureInfo*>(&occlusionTexture);
	return validateTextureInfo(id, *baseClass, errors, warnings);
}

bool easygltf::validateNormalTexture(size_t id, const NormalTexture& normalTexture, std::stringstream& errors, std::stringstream& warnings)
{
	auto* baseClass = static_cast<const TextureInfo*>(&normalTexture);
	return validateTextureInfo(id, *baseClass, errors, warnings);
}

bool easygltf::validateTextureInfo(size_t id, const TextureInfo& textureInfo, std::stringstream& errors, std::stringstream& warnings)
{
	//if (textureInfo.index == base::constants::not_set)
	//{
	//	errors << std::format(validation_error_not_set, "TextureInfo", id, "index");
	//	return false;
	//}

	return true;
}

bool easygltf::validatePbrMetallicRoughness(size_t id, const PbrMetallicRoughness& pbrMetallicRoughness, std::stringstream& errors, std::stringstream& warnings)
{
	bool result{ true };

	result = result && validateTextureInfo(id, pbrMetallicRoughness.baseColorTexture, errors, warnings);
	result = result && validateTextureInfo(id, pbrMetallicRoughness.metallicRoughnessTexture, errors, warnings);

	return result;
}

bool easygltf::validateMaterial(size_t id, const Material& material, std::stringstream& errors, std::stringstream& warnings)
{
	bool result{ true };

	result = result && validatePbrMetallicRoughness(id, material.pbrMetallicRoughness, errors, warnings);
	result = result && validateNormalTexture(id, material.normalTexture, errors, warnings);
	result = result && validateOcclusionTexture(id, material.occlusionTexture, errors, warnings);
	result = result && validateTextureInfo(id, material.emissiveTexture, errors, warnings);

	return result;
}

// Validate mesh
bool easygltf::validatePrimitive(size_t id, const Primitive& primitive, std::stringstream& errors, std::stringstream& warnings)
{
	if (primitive.attributes.empty())
	{
		errors << std::format(validation_error_not_set, "Primitive", id, "attributes");
		return false;
	}

	return true;
}

bool easygltf::validateMesh(size_t id, const Mesh& mesh, std::stringstream& errors, std::stringstream& warnings)
{
	if (mesh.primitives.empty())
	{
		errors << std::format(validation_error_not_set, "Mesh", id, "primitives");
		return false;
	}

	return true;
}

// Validate Node
bool easygltf::validateNode(size_t id, const Node& node, std::stringstream& errors, std::stringstream& warnings)
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
bool easygltf::validateSampler(size_t id, const Sampler& sampler, std::stringstream& errors, std::stringstream& warnings)
{
	bool isValidWrapS =
		sampler.wrapS == base::enumerators::texture_wrap::repeat ||
		sampler.wrapS == base::enumerators::texture_wrap::clamp_to_edge ||
		sampler.wrapS == base::enumerators::texture_wrap::mirrored_repeat;

	if (!isValidWrapS)
	{
		errors << std::format(validation_error_invalid_value, "Sampler", id, "wrapS", sampler.wrapS);
		return false;
	}

	bool isValidWrapT =
		sampler.wrapT == base::enumerators::texture_wrap::repeat ||
		sampler.wrapT == base::enumerators::texture_wrap::clamp_to_edge ||
		sampler.wrapT == base::enumerators::texture_wrap::mirrored_repeat;

	if (!isValidWrapT)
	{
		errors << std::format(validation_error_invalid_value, "Sampler", id, "wrapT", sampler.wrapT);
		return false;
	}

	return true;
}

// Validate Scene
bool easygltf::validateScene(size_t id, const Scene& scene, std::stringstream& errors, std::stringstream& warnings)
{
	// Validate node indices
	return true;
}

// Validate Skin
bool easygltf::validateSkin(size_t id, const Skin& skin, std::stringstream& errors, std::stringstream& warnings)
{
	if (skin.joints.empty())
	{
		errors << std::format(validation_error_not_set, "Skin", id, "joints");
		return false;
	}

	return true;
}

// Validate Texture
bool easygltf::validateTexture(size_t id, const Texture& texture, std::stringstream& errors, std::stringstream& warnings)
{
	return true;
}

// All validation
bool easygltf::validate(const FGltf& gltf, std::stringstream& errors, std::stringstream& warnings)
{
	bool result{ true };

	// Validate accessors
	for (size_t id = 0ull; id < gltf.accessors.size(); ++id)
		result = result && validateAccessor(id, gltf.accessors.at(id), errors, warnings);

	// Validate animations
	for (size_t id = 0ull; id < gltf.animations.size(); ++id)
		result = result && validateAnimation(id, gltf.animations.at(id), errors, warnings);

	// Validate buffers
	for (size_t id = 0ull; id < gltf.buffers.size(); ++id)
		result = result && validateBuffer(id, gltf.buffers.at(id), errors, warnings);

	// Validate buffer views
	for (size_t id = 0ull; id < gltf.bufferViews.size(); ++id)
		result = result && validateBufferView(id, gltf.bufferViews.at(id), errors, warnings);

	// Validate cameras
	for (size_t id = 0ull; id < gltf.cameras.size(); ++id)
		result = result && validateCamera(id, gltf.cameras.at(id), errors, warnings);

	// Validate images
	for (size_t id = 0ull; id < gltf.images.size(); ++id)
		result = result && validateImage(id, gltf.images.at(id), errors, warnings);

	// Validate materials
	for (size_t id = 0ull; id < gltf.materials.size(); ++id)
		result = result && validateMaterial(id, gltf.materials.at(id), errors, warnings);

	// Validate meshes
	for (size_t id = 0ull; id < gltf.meshes.size(); ++id)
		result = result && validateMesh(id, gltf.meshes.at(id), errors, warnings);

	// Validate nodes
	for (size_t id = 0ull; id < gltf.nodes.size(); ++id)
		result = result && validateNode(id, gltf.nodes.at(id), errors, warnings);

	// Validate samplers
	for (size_t id = 0ull; id < gltf.samplers.size(); ++id)
		result = result && validateSampler(id, gltf.samplers.at(id), errors, warnings);

	// Validate scenes
	for (size_t id = 0ull; id < gltf.scenes.size(); ++id)
		result = result && validateScene(id, gltf.scenes.at(id), errors, warnings);

	// Validate skins
	for (size_t id = 0ull; id < gltf.skins.size(); ++id)
		result = result && validateSkin(id, gltf.skins.at(id), errors, warnings);

	// Validate textures
	for (size_t id = 0ull; id < gltf.textures.size(); ++id)
		result = result && validateTexture(id, gltf.textures.at(id), errors, warnings);

	// Validate asset
	result = result && validateAsset(gltf.asset, errors, warnings);

	return result;
}