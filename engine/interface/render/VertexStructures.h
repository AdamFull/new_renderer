#pragma once

#include <vector>

namespace engine
{
	namespace render
	{
		enum class EVertexAttributeType : uint32_t
		{
			ePosition,
			eColor,
			eTexCoord,
			eNormal,
			eTangent,
			eJoint,
			eWeight,
			eIndex
		};

		struct FVertexAttributePointer
		{
			EVertexAttributeType type{ EVertexAttributeType::eIndex };
			uint8_t* data{ nullptr };
			size_t stride{ 0ull };
			size_t count{ 0ull };
		};

		struct FVertexBufferBindingInfo
		{
			uint32_t firstBinding{ 0u };
			std::vector<size_t> buffers{};
			std::vector<size_t> offsets{};
		};

		struct FIndexBufferBindingInfo
		{
			size_t bufferId{ ~0ull };
			size_t offset{ 0ull };
			size_t indexSize{ 0ull };
		};

		struct FVertexHandlerAttributeDescription
		{
			size_t size{ 0ull };
			EVertexAttributeType type{ EVertexAttributeType::eIndex };
		};

		struct FVertexHandlerCreateInfo
		{
			std::vector<FVertexHandlerAttributeDescription> descriptions{};
			size_t indexSize{ 1ull };
			std::string name{};
		};

		class IVertexHandler
		{
		public:
			virtual ~IVertexHandler() = default;

			virtual void create(const FVertexHandlerCreateInfo& create_info) = 0;
			virtual void allocate() = 0;

			virtual void bind(uint32_t queue) = 0;

			virtual size_t addVertices(const std::vector<FVertexAttributePointer>& attributes_data) = 0;
			virtual size_t addIndices(const FVertexAttributePointer& indices) = 0;

			virtual size_t getLastVertex() = 0;
			virtual size_t getLastIndex() = 0;

			virtual const FVertexDeclarationInfo& getVertexDeclaration() const = 0;
		};
	}
}