#pragma once

#include <string>
#include <unordered_map>
#include "VertexStructures.h"

namespace engine
{
	namespace render
	{
		enum class EShaderResourcesUpdateFlagBits
		{
			eUniformBlocks = 1 << 0,
			eStorageBlocks = 1 << 1,
			eTextures = 1 << 2,
			eAll = eUniformBlocks | eStorageBlocks | eTextures
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EShaderResourcesUpdateFlags, EShaderResourcesUpdateFlagBits)

		enum class EVertexPrimitiveTopology : uint32_t
		{
			ePointList,
			eLineList,
			eLineStrip,
			eTriangleList,
			eTriangleStrip,
			eTriangleFan
		};

		enum class EShaderCullModeFlagBits : uint32_t
		{
			eNone = 1 << 0,
			eFront = 1 << 1,
			eBack = 1 << 2,
			eFrontAndBack = eFront | eBack
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EShaderCullModeFlags, EShaderCullModeFlagBits)

		enum class EShaderFrontFace : uint32_t
		{
			eCCW,
			eCW
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
			EVertexPrimitiveTopology primitiveTopology{};
			// EShaderCullMode
			EShaderCullModeFlags cullMode{};
			// EShaderFrontFace
			EShaderFrontFace frontFace{};

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
			uint32_t binding{ 0u };
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