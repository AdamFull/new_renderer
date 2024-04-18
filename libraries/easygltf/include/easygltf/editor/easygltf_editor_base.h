#pragma once

#include <include/easygltf/editor/easygltf_editor_fwd.h>
#include <include/easygltf/easygltf_base.h>
#include <glm/glm.hpp>

namespace easygltf
{
	namespace editor
	{
		class IGLTFObject
		{
		public:
			virtual ~IGLTFObject()
			{
				for (auto& [name, extension] : extensions)
				{
					if (extension)
					{
						delete extension;
						extension = nullptr;
					}
				}
			}

			template<class _Ty>
			_Ty* getExtension()
			{
				auto found = extensions.find(_Ty::name);
				if (found != extensions.end())
					return found->second;

				return nullptr;
			}

			virtual nlohmann::json serialize() = 0;
		protected:
			std::string name{};
			std::unordered_map<std::string, IGLTFObject*> extensions{};
			// TODO: store extras
		};

		enum class EGLTFComponentType
		{
			eByte,
			eUByte,
			eShort,
			eUShort,
			eInt,
			eUInt,
			eFloat,
			eDouble
		};

		enum class EGLTFElementType
		{
			eVec2,
			eVec3,
			eVec4,
			eMat2,
			eMat3,
			eMat4
		};

		enum class EGLTFVertexAttributeType
		{
			ePOSITION,
			eNORMAL,
			eTEXCOORD_0,
			eTEXCOORD_1,
			eCOLOR_0,
			eCOLOR_1,
			eTANGENT,
			eJOINTS_0,
			eWEOGHTS_0,
			eCUSTOM
		};

		enum class EGLTFPrimitiveTopology
		{
			ePoints,
			eLines,
			eLineLoop,
			eLineStrip,
			eTriangles,
			eTriangleStrip,
			eTriangleFan
		};

		struct GLTFAnimation : public IGLTFObject
		{
			~GLTFAnimation() = default;

			nlohmann::json serialize() override;
		};

		struct GLTFCamera : public IGLTFObject
		{
			~GLTFCamera() = default;

			nlohmann::json serialize() override;
		};

		struct GLTFImage : public IGLTFObject
		{
			~GLTFImage() = default;

			nlohmann::json serialize() override;
		};

		struct GLTFMaterial : public IGLTFObject
		{
			~GLTFMaterial() = default;

			nlohmann::json serialize() override;
		};

		struct GLTFStream
		{
			GLTFStream() = default;

			EGLTFComponentType componentType{};
			EGLTFElementType elementType{};

			utl::vector<uint8_t> data{};
			size_t count{ 0ull };
		};

		struct GLTFVertexStream : public GLTFStream
		{
			EGLTFVertexAttributeType type{};
		};


		struct GLTFPrimitive : public IGLTFObject
		{
			~GLTFPrimitive() = default;

			nlohmann::json serialize() override;

			GLTFMaterial* material{ nullptr };

			utl::vector<GLTFVertexStream> attributes{};
			utl::vector<utl::vector<GLTFVertexStream>> targets{};

			utl::vector<uint32_t> indices{};

			EGLTFPrimitiveTopology mode{};
		};

		struct GLTFMesh : public IGLTFObject
		{
			~GLTFMesh() = default;

			nlohmann::json serialize() override;
		};

		struct GLTFNode : public IGLTFObject
		{
			~GLTFNode() = default;

			nlohmann::json serialize() override;

			GLTFCamera* camera{ nullptr };
			GLTFSkin* skin{ nullptr };
			GLTFMesh* mesh{ nullptr };

			glm::vec3 translation{};
			glm::vec4 rotation{};
			glm::vec3 scale{};
			glm::mat4 matrix{};

			utl::vector<double> weights{};

			utl::vector<GLTFNode*> children{};
		};

		struct GLTFSkin : public IGLTFObject
		{
			~GLTFSkin() = default;

			nlohmann::json serialize() override;
		};

		struct GLTFScene : public IGLTFObject
		{
			~GLTFScene() = default;

			nlohmann::json serialize() override;

			utl::vector<GLTFNode*> nodes{};
		};

		struct GLTFSampler : public IGLTFObject
		{
			~GLTFSampler() = default;

			nlohmann::json serialize() override;
		};

		struct GLTFTexture : public IGLTFObject
		{
			~GLTFTexture() = default;

			nlohmann::json serialize() override;
		};
	}
}