#pragma once

#include <easygltf_base.h>

namespace easygltf
{
	void to_json(nlohmann::json& json, const SparseAccessorIndices& type);
	void from_json(const nlohmann::json& json, SparseAccessorIndices& type);

	void to_json(nlohmann::json& json, const SparseAccessorValues& type);
	void from_json(const nlohmann::json& json, SparseAccessorValues& type);

	void to_json(nlohmann::json& json, const SparseAccessor& type);
	void from_json(const nlohmann::json& json, SparseAccessor& type);

	void to_json(nlohmann::json& json, const Accessor& type);
	void from_json(const nlohmann::json& json, Accessor& type);

	void to_json(nlohmann::json& json, const AnimationTarget& type);
	void from_json(const nlohmann::json& json, AnimationTarget& type);

	void to_json(nlohmann::json& json, const AnimationChannel& type);
	void from_json(const nlohmann::json& json, AnimationChannel& type);

	void to_json(nlohmann::json& json, const AnimationSampler& type);
	void from_json(const nlohmann::json& json, AnimationSampler& type);

	void to_json(nlohmann::json& json, const Animation& type);
	void from_json(const nlohmann::json& json, Animation& type);

	void to_json(nlohmann::json& json, const Asset& type);
	void from_json(const nlohmann::json& json, Asset& type);

	void to_json(nlohmann::json& json, const Buffer& type);
	void from_json(const nlohmann::json& json, Buffer& type);

	void to_json(nlohmann::json& json, const BufferView& type);
	void from_json(const nlohmann::json& json, BufferView& type);

	void to_json(nlohmann::json& json, const PerspectiveCamera& type);
	void from_json(const nlohmann::json& json, PerspectiveCamera& type);

	void to_json(nlohmann::json& json, const OrthographicCamera& type);
	void from_json(const nlohmann::json& json, OrthographicCamera& type);

	void to_json(nlohmann::json& json, const Camera& type);
	void from_json(const nlohmann::json& json, Camera& type);

	void to_json(nlohmann::json& json, const Image& type);
	void from_json(const nlohmann::json& json, Image& type);

	void to_json(nlohmann::json& json, const TextureInfo& type);
	void from_json(const nlohmann::json& json, TextureInfo& type);

	void to_json(nlohmann::json& json, const PbrMetallicRoughness& type);
	void from_json(const nlohmann::json& json, PbrMetallicRoughness& type);

	void to_json(nlohmann::json& json, const NormalTexture& type);
	void from_json(const nlohmann::json& json, NormalTexture& type);

	void to_json(nlohmann::json& json, const OcclusionTexture& type);
	void from_json(const nlohmann::json& json, OcclusionTexture& type);

	void to_json(nlohmann::json& json, const Material& type);
	void from_json(const nlohmann::json& json, Material& type);

	void to_json(nlohmann::json& json, const Primitive& type);
	void from_json(const nlohmann::json& json, Primitive& type);

	void to_json(nlohmann::json& json, const Mesh& type);
	void from_json(const nlohmann::json& json, Mesh& type);

	void to_json(nlohmann::json& json, const Node& type);
	void from_json(const nlohmann::json& json, Node& type);

	void to_json(nlohmann::json& json, const Sampler& type);
	void from_json(const nlohmann::json& json, Sampler& type);

	void to_json(nlohmann::json& json, const Scene& type);
	void from_json(const nlohmann::json& json, Scene& type);

	void to_json(nlohmann::json& json, const Skin& type);
	void from_json(const nlohmann::json& json, Skin& type);

	void to_json(nlohmann::json& json, const Texture& type);
	void from_json(const nlohmann::json& json, Texture& type);

	void to_json(nlohmann::json& json, const FGLTFRoot& type);
	void from_json(const nlohmann::json& json, FGLTFRoot& type);
}