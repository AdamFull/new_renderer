#pragma once

#include <easygltf_base.h>

namespace easygltf
{
	// Accessor validation
	bool validateSparseAccessorIndices(size_t id, const SparseAccessorIndices& sparseAccessorIndices, std::stringstream& errors, std::stringstream& warnings);
	bool validateSparseAccessorValues(size_t id, const SparseAccessorValues& sparseAccessorValues, std::stringstream& errors, std::stringstream& warnings);
	bool validateSparseAccessor(size_t id, const SparseAccessor& sparseAccessor, std::stringstream& errors, std::stringstream& warnings);
	bool validateAccessor(size_t id, const Accessor& accessor, std::stringstream& errors, std::stringstream& warnings);

	// Animation validation
	bool validateAnimationTarget(size_t id, const AnimationTarget& animationTarget, std::stringstream& errors, std::stringstream& warnings);
	bool validateAnimationChannel(size_t id, const AnimationChannel& animationChannel, std::stringstream& errors, std::stringstream& warnings);
	bool validateAnimationSampler(size_t id, const AnimationSampler& animationSampler, std::stringstream& errors, std::stringstream& warnings);
	bool validateAnimation(size_t id, const Animation& animation, std::stringstream& errors, std::stringstream& warnings);

	// Asset validation
	bool validateAsset(const Asset& asset, std::stringstream& errors, std::stringstream& warnings);

	// Buffer validation
	bool validateBuffer(size_t id, const Buffer& buffer, std::stringstream& errors, std::stringstream& warnings);

	// BufferView validation
	bool validateBufferView(size_t id, const BufferView& bufferView, std::stringstream& errors, std::stringstream& warnings);

	// Camera validation
	bool validatePerspectiveCamera(size_t id, const PerspectiveCamera& camera, std::stringstream& errors, std::stringstream& warnings);
	bool validateOrthographicCamera(size_t id, const OrthographicCamera& camera, std::stringstream& errors, std::stringstream& warnings);
	bool validateCamera(size_t id, const Camera& camera, std::stringstream& errors, std::stringstream& warnings);

	// Image validation
	bool validateImage(size_t id, const Image& image, std::stringstream& errors, std::stringstream& warnings);

	// Maretial validation
	bool validateOcclusionTexture(size_t id, const OcclusionTexture& occlusionTexture, std::stringstream& errors, std::stringstream& warnings);
	bool validateNormalTexture(size_t id, const NormalTexture& normalTexture, std::stringstream& errors, std::stringstream& warnings);
	bool validateTextureInfo(size_t id, const TextureInfo& textureInfo, std::stringstream& errors, std::stringstream& warnings);
	bool validatePbrMetallicRoughness(size_t id, const PbrMetallicRoughness& pbrMetallicRoughness, std::stringstream& errors, std::stringstream& warnings);
	bool validateMaterial(size_t id, const Material& material, std::stringstream& errors, std::stringstream& warnings);

	// Mesh valudation
	bool validatePrimitive(size_t id, const Primitive& primitive, std::stringstream& errors, std::stringstream& warnings);
	bool validateMesh(size_t id, const Mesh& mesh, std::stringstream& errors, std::stringstream& warnings);

	// Node validation
	 bool validateNode(size_t id, const Node& node, std::stringstream& errors, std::stringstream& warnings);

	// Sampler validation
	bool validateSampler(size_t id, const Sampler& sampler, std::stringstream& errors, std::stringstream& warnings);

	// Scene validation
	bool validateScene(size_t id, const Scene& scene, std::stringstream& errors, std::stringstream& warnings);

	// Skin validation
	bool validateSkin(size_t id, const Skin& skin, std::stringstream& errors, std::stringstream& warnings);

	// Texture validation
	bool validateTexture(size_t id, const Texture& texture, std::stringstream& errors, std::stringstream& warnings);

	bool validate(const FGltf& gltf, std::stringstream& errors, std::stringstream& warnings);
}