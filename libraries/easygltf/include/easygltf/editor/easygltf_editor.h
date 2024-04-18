#pragma once

#include <include/easygltf/editor/easygltf_editor_base.h>

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
			utl::vector<GLTFAnimation*> animations{};
			utl::vector<GLTFCamera*> cameras{};
			utl::vector<GLTFImage*> images{};
			utl::vector<GLTFMaterial*> materials{};
			utl::vector<GLTFMesh*> meshes{};
			utl::vector<GLTFNode*> nodes{};
			utl::vector<GLTFSkin*> skins{};
			utl::vector<GLTFScene*> scenes{};
			utl::vector<GLTFSampler*> samplers{};
			utl::vector<GLTFTexture*> textures{};

			GLTFScene* currentScene{ nullptr };
		};
	}
}