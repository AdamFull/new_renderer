#pragma once

#include "VertexStructures.h"
#include <container/hash_map.h>

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

		enum class EVertexPrimitiveTopology : u32
		{
			ePointList,
			eLineList,
			eLineStrip,
			eTriangleList,
			eTriangleStrip,
			eTriangleFan
		};

		enum class EShaderCullModeFlagBits : u32
		{
			eNone = 1 << 0,
			eFront = 1 << 1,
			eBack = 1 << 2,
			eFrontAndBack = eFront | eBack
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EShaderCullModeFlags, EShaderCullModeFlagBits)

		enum class EShaderFrontFace : u32
		{
			eCCW,
			eCW
		};

		struct FReflectionUniform
		{
			u32 set{ 0u };
			u32 binding{ 0u };
			u64 offset{ 0ull };
			u64 size{ 0ull };
		};

		template<class _Ty>
		struct FReflectionUniformBlock
		{
			u32 set{ 0u };
			u32 binding{ 0u };
			u64 size{ 0ull };
			utl::hash_map<u64, _Ty> uniforms{};
		};

		template<class _Ty>
		struct FReflectionPushConstantBlock
		{
			u64 size{ 0ull };
			utl::hash_map<u64, _Ty> uniforms{};
		};

		struct FShaderReflectionData
		{
			utl::hash_map<u64, FReflectionUniform> uniforms{};
			utl::hash_map<u64, FReflectionUniformBlock<FReflectionUniform>> uniformBlocks{};
			utl::hash_map<u64, FReflectionPushConstantBlock<FReflectionUniform>> pushBlocks{};

			std::array<u32, 3ull> localSizes;
		};

		struct FShaderUploadInfo
		{
			i64 libraryId{ ~0ll };
			i64 shaderId{ ~0ll };
			std::string originalName{};
		};

		struct FShaderCreationBlockInfo
		{
			u32 stage{ 0u };
			utl::vector<u32>* code{ nullptr };
		};

		struct FAttachment
		{
			u64 source{ INVALID_HANDLE };
			bool backbuffer{ false };
		};

		struct FGraphicsShaderCreateInfo
		{
			// EShaderPrimitiveTopology
			EVertexPrimitiveTopology primitiveTopology{};
			// EShaderCullMode
			EShaderCullModeFlags cullMode{};
			// EShaderFrontFace
			EShaderFrontFace frontFace{};

			u32 usageCount{ 1u };

			utl::vector<FAttachment> colorAttachments{};
			FAttachment depthAttachment{};
			FAttachment stencilAttachment{};

			FVertexDeclarationInfo vertexDeclaration{};
		};

		struct FComputeShaderCreateInfo
		{
			u32 usageCount{ 1u };
		};

		struct FReyTracingShaderCreateInfo
		{
			u32 usageCount{ 1u };
		};

		struct FUniformDataView
		{
			const void* data{ nullptr };
			u64 size{ 0ull };
			u32 binding{ 0u };
		};

		struct FTextureBindInfo
		{
			u64 textureId{ INVALID_HANDLE };
			u32 mipLevel{ ~0u };
		};

		struct FShaderResourceCommit
		{
			u64 shaderId{ INVALID_HANDLE };
			utl::hash_map<u64, FUniformDataView> uniforms{};
			utl::hash_map<u64, FTextureBindInfo> textures{};
			utl::hash_map<u64, u64> buffers{};
		};
	}
}