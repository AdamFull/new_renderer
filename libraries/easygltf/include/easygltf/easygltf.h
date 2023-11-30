#pragma once

#include <easygltf_base.h>

namespace easygltf
{
	struct FImportGltfInfo
	{
		bool mergeBuffers{ false };
		bool importAnimations{ true }; // Import animations and skeletons
		bool importCameras{ true };
		bool importMeshes{ true }; // Includes materials, textures, samplers, images
		bool importScenes{ true };
		// Extensions

		int32_t importSceneId{ -1 };
	};



	class CGLTFProject
	{
	public:
		void useDefaultFilesystemWrapper();
		void setFilesystemWrapper(std::unique_ptr<IFilesystemWrapper>&& wrapper);

		void useDefaultDebugMessenger();
		void setDebugMessenger(std::unique_ptr<IDebugMessenger>&& messenger);

		bool load(const std::string& fsPath);
		bool save(const std::string& fsPath = "");
		bool import(const std::string& fsPath, const FImportGltfInfo& importInfo);

		const FGLTFRoot& getGltf() const;

		std::string getResourcePath(const std::string& path);

		Stream getStream(const Accessor& accessor);
		Stream getStream(int32_t accessor_id);

		// Getters
		//const Accessor& getAccessor(int32_t idx) const;
		//const Animation& getAnimation(int32_t idx) const;
		//const Buffer& getBuffer(int32_t idx) const;
		//const BufferView& getBufferView(int32_t idx) const;
		//const Camera& getCamera(int32_t idx) const;
		//const Image& getImage(int32_t idx) const;
		//const Material& getMaterial(int32_t idx) const;
		//const Mesh& getMesh(int32_t idx) const;
		//const Node& getNode(int32_t idx) const;
		//const Sampler& getSampler(int32_t idx) const;
		//const Scene& getScene(int32_t idx) const;
		//const Skin& getSkin(int32_t idx) const;
		//const Texture& getTexture(int32_t idx) const;

		// Editing
		int32_t createAccessor(const std::string& name);
		int32_t createAnimation(const std::string& name);
		int32_t createBuffer(const std::string& name);
		int32_t createBufferView(const std::string& name);
		int32_t createCamera(const std::string& name);
		int32_t createImage(const std::string& name);
		int32_t createMaterial(const std::string& name);
		int32_t createMesh(const std::string& name);
		int32_t createNode(const std::string& name);
		int32_t createSampler(const std::string& name);
		int32_t createScene(const std::string& name);
		int32_t createSkin(const std::string& name);
		int32_t createTexture(const std::string& name);

		size_t removeAccessor(int32_t index);
		size_t removeAnimation(int32_t index);
		size_t removeBuffer(int32_t index);
		size_t removeBufferView(int32_t index);
		size_t removeCamera(int32_t index);
		size_t removeImage(int32_t index);
		size_t removeMaterial(int32_t index);
		size_t removeMesh(int32_t index);
		size_t removeNode(int32_t index);
		size_t removeSampler(int32_t index);
		size_t removeScene(int32_t index);
		size_t removeSkin(int32_t index);
		size_t removeTexture(int32_t index);
	private:
		bool _load(const std::string& fsPath, FGLTFRoot& gltf_scene);
		bool _loadGltfFromMemory(const std::vector<uint8_t>& data, FGLTFRoot& gltf_scene);
		bool _loadGlbFromMemory(const std::vector<uint8_t>& data, FGLTFRoot& gltf_scene);
		bool _saveAsGltf(const std::string& path);
		bool _saveAsGlb(const std::string& path);

		bool _loadBuffer(int32_t buffer_id);
	private:
		std::string m_srOpennedFilePath{};
		std::string m_srParentPath{};
		std::unique_ptr<IFilesystemWrapper> m_pFS{};
		std::unique_ptr<IDebugMessenger> m_pMessenger{};

		FGLTFRoot m_root{};
		bool bInBinaryMode{ false };
	};
}