#pragma once

#include "easygltf_base.h"

namespace easygltf
{
	// Accessor validation
	bool validateSparseAccessorIndices(size_t id, const SparseAccessorIndices& sparseAccessorIndices, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateSparseAccessorValues(size_t id, const SparseAccessorValues& sparseAccessorValues, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateSparseAccessor(size_t id, const SparseAccessor& sparseAccessor, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateAccessor(size_t id, const Accessor& accessor, const std::unique_ptr<IDebugMessenger>& messenger);

	// Animation validation
	bool validateAnimationTarget(size_t id, const AnimationTarget& animationTarget, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateAnimationChannel(size_t id, const AnimationChannel& animationChannel, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateAnimationSampler(size_t id, const AnimationSampler& animationSampler, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateAnimation(size_t id, const Animation& animation, const std::unique_ptr<IDebugMessenger>& messenger);

	// Asset validation
	bool validateAsset(const Asset& asset, const std::unique_ptr<IDebugMessenger>& messenger);

	// Buffer validation
	bool validateBuffer(size_t id, const Buffer& buffer, const std::unique_ptr<IDebugMessenger>& messenger);

	// BufferView validation
	bool validateBufferView(size_t id, const BufferView& bufferView, const std::unique_ptr<IDebugMessenger>& messenger);

	// Camera validation
	bool validatePerspectiveCamera(size_t id, const PerspectiveCamera& camera, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateOrthographicCamera(size_t id, const OrthographicCamera& camera, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateCamera(size_t id, const Camera& camera, const std::unique_ptr<IDebugMessenger>& messenger);

	// Image validation
	bool validateImage(size_t id, const Image& image, const std::unique_ptr<IDebugMessenger>& messenger);

	// Maretial validation
	bool validateOcclusionTexture(size_t id, const OcclusionTexture& occlusionTexture, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateNormalTexture(size_t id, const NormalTexture& normalTexture, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateTextureInfo(size_t id, const TextureInfo& textureInfo, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validatePbrMetallicRoughness(size_t id, const PbrMetallicRoughness& pbrMetallicRoughness, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateMaterial(size_t id, const Material& material, const std::unique_ptr<IDebugMessenger>& messenger);

	// Mesh valudation
	bool validatePrimitive(size_t id, const Primitive& primitive, const std::unique_ptr<IDebugMessenger>& messenger);
	bool validateMesh(size_t id, const Mesh& mesh, const std::unique_ptr<IDebugMessenger>& messenger);

	// Node validation
	 bool validateNode(size_t id, const Node& node, const std::unique_ptr<IDebugMessenger>& messenger);

	// Sampler validation
	bool validateSampler(size_t id, const Sampler& sampler, const std::unique_ptr<IDebugMessenger>& messenger);

	// Scene validation
	bool validateScene(size_t id, const Scene& scene, const std::unique_ptr<IDebugMessenger>& messenger);

	// Skin validation
	bool validateSkin(size_t id, const Skin& skin, const std::unique_ptr<IDebugMessenger>& messenger);

	// Texture validation
	bool validateTexture(size_t id, const Texture& texture, const std::unique_ptr<IDebugMessenger>& messenger);

	bool validate(const FGLTFRoot& gltf, const std::unique_ptr<IDebugMessenger>& messenger);
}