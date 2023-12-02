#include <editor/easygltf_editor.h>

using namespace easygltf::editor;


nlohmann::json GLTFAnimation::serialize()
{
	return {};
}

nlohmann::json GLTFCamera::serialize()
{
	return {};
}

nlohmann::json GLTFImage::serialize()
{
	return {};
}

nlohmann::json GLTFMaterial::serialize()
{
	return {};
}

nlohmann::json GLTFPrimitive::serialize()
{
	return {};
}

nlohmann::json GLTFMesh::serialize()
{
	return {};
}

nlohmann::json GLTFNode::serialize()
{
	return {};
}

nlohmann::json GLTFSkin::serialize()
{
	return {};
}

nlohmann::json GLTFScene::serialize()
{
	return {};
}

nlohmann::json GLTFSampler::serialize()
{
	return {};
}

nlohmann::json GLTFTexture::serialize()
{
	return {};
}


void CGLTFEditor::load(const FGLTFRoot& root)
{
	// Load samplers
	for (const auto& sampler : root.samplers)
	{
		auto& nsampler = samplers.emplace_back();
		nsampler = new GLTFSampler();
	}

	// Load images
	for (const auto& image : root.images)
	{
		auto& nimage = images.emplace_back();
		nimage = new GLTFImage();
	}
}

nlohmann::json CGLTFEditor::serialize()
{
	nlohmann::json json{};

	for (auto& node : nodes)
	{

	}

	return {};
}