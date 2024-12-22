#pragma once

#include "easygltf_editor_base.h"

namespace easygltf
{
	namespace editor
	{
		class CGLTFEditor : public IGLTFObject
		{
		public:
			void load(const FGLTFRoot& root);

			nlohmann::json serialize() override;
		private:
			std::vector<GLTFAnimation*> animations{};
			std::vector<GLTFCamera*> cameras{};
			std::vector<GLTFImage*> images{};
			std::vector<GLTFMaterial*> materials{};
			std::vector<GLTFMesh*> meshes{};
			std::vector<GLTFNode*> nodes{};
			std::vector<GLTFSkin*> skins{};
			std::vector<GLTFScene*> scenes{};
			std::vector<GLTFSampler*> samplers{};
			std::vector<GLTFTexture*> textures{};

			GLTFScene* currentScene{ nullptr };
		};
	}
}