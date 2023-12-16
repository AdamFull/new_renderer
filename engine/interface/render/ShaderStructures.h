#pragma once

#include <string>
#include <unordered_map>
#include "VertexStructures.h"

namespace engine
{
	namespace render
	{
		enum EShaderResourcesUpdateFlagBits
		{
			SHADER_RESOURCES_UPDATE_UNIFORM_BLOCKS_FLAG_BIT = 1 << 0,
			SHADER_RESOURCES_UPDATE_STORAGE_BLOCKS_FLAG_BIT = 1 << 1,
			SHADER_RESOURCES_UPDATE_TEXTURES_FLAG_BIT = 1 << 2,
			SHADER_RESOURCES_UPDATE_ALL_FLAG_BIT =
			SHADER_RESOURCES_UPDATE_UNIFORM_BLOCKS_FLAG_BIT |
			SHADER_RESOURCES_UPDATE_STORAGE_BLOCKS_FLAG_BIT |
			SHADER_RESOURCES_UPDATE_TEXTURES_FLAG_BIT
		};

		enum EShaderPrimitiveTopology : uint32_t
		{
			SHADER_PRIMITIVE_TOPOLOGY_POINT_LIST,
			SHADER_PRIMITIVE_TOPOLOGY_LINE_LIST,
			SHADER_PRIMITIVE_TOPOLOGY_LINE_STRIP,
			SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
			SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		};

		enum EShaderCullMode : uint32_t
		{
			SHADER_CULL_MODE_NONE = 1 << 0,
			SHADER_CULL_MODE_FRONT_BIT = 1 << 1,
			SHADER_CULL_MODE_BACK_BIT = 1 << 2,
			SHADER_CULL_MODE_FRONT_AND_BACK = SHADER_CULL_MODE_FRONT_BIT | SHADER_CULL_MODE_BACK_BIT
		};

		enum EShaderFrontFace : uint32_t
		{
			SHADER_FRONT_FACE_CCW,
			SHADER_FRONT_FACE_CW
		};

		struct FReflectionUniform
		{
			uint32_t set{ 0u };
			uint32_t binding{ 0u };
			size_t offset{ 0ull };
			size_t size{ 0ull };
		};

		template<class _Ty>
		struct FReflectionUniformBlock
		{
			uint32_t set{ 0u };
			uint32_t binding{ 0u };
			size_t size{ 0ull };
			std::unordered_map<size_t, _Ty> uniforms{};
		};

		template<class _Ty>
		struct FReflectionPushConstantBlock
		{
			size_t size{ 0ull };
			std::unordered_map<size_t, _Ty> uniforms{};
		};

		struct FShaderReflectionData
		{
			std::unordered_map<size_t, FReflectionUniform> uniforms{};
			std::unordered_map<size_t, FReflectionUniformBlock<FReflectionUniform>> uniformBlocks{};
			std::unordered_map<size_t, FReflectionPushConstantBlock<FReflectionUniform>> pushBlocks{};

			std::array<uint32_t, 3ull> localSizes;
		};

		struct FShaderUploadInfo
		{
			int64_t libraryId{ ~0ll };
			int64_t shaderId{ ~0ll };
			std::string originalName{};
		};

		struct FShaderCreationBlockInfo
		{
			uint32_t stage{ 0u };
			std::vector<uint32_t>* code{ nullptr };
		};

		struct FGraphicsShaderCreateInfo
		{
			// EShaderPrimitiveTopology
			uint32_t primitiveTopology{};
			// EShaderCullMode
			uint32_t cullMode{};
			// EShaderFrontFace
			uint32_t frontFace{};

			uint32_t usageCount{ 1u };

			std::vector<size_t> colorAttachments{};
			size_t depthAttachment{ ~0ull };
			size_t stencilAttachment{ ~0ull };

			FVertexDeclarationInfo vertexDeclaration{};
		};

		struct FComputeShaderCreateInfo
		{
			uint32_t usageCount{ 1u };
		};

		struct FUniformDataView
		{
			const void* data{ nullptr };
			size_t size{ 0ull };
		};

		struct FTextureBindInfo
		{
			size_t textureId{ ~0ull };
			uint32_t mipLevel{ 0u };
		};

		struct FShaderResourceCommit
		{
			size_t shaderId{ ~0ull };
			std::unordered_map<size_t, FUniformDataView> uniforms{};
			std::unordered_map<size_t, FTextureBindInfo> textures{};
			std::unordered_map<size_t, size_t> buffers{};
		};
	}
}