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
	}
}