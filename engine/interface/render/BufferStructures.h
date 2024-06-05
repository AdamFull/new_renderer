#pragma once

#include <standart_library.h>
#include <container/flags.h>
#include <utils/hash.h>

namespace engine
{
	namespace render
	{
		enum class EBufferUsageFlagBits
		{
			eTransfer = 1 << 0,
			eStorage = 1 << 1,
			eVertex = 1 << 2,
			eIndex = 1 << 3,
			eUniform = 1 << 4,
			eIndirect = 1 << 5,
			eGpuOnly = 1 << 6,
			eCpuToGpu = 1 << 7,

			eHasDeviceAddress = eVertex | eIndex | eStorage | eIndirect,

			eStagingBuffer = eTransfer,
			eStorageBufferCPU = eTransfer | eStorage | eCpuToGpu,
			eStorageBuffer = eTransfer | eStorage | eGpuOnly,
			eVertexBuffer = eTransfer | eVertex | eGpuOnly,
			eIndexBuffer = eTransfer | eIndex | eGpuOnly,
			eUniformBuffer = eTransfer | eUniform | eCpuToGpu,
			eIndirectBuffer = eTransfer | eStorage | eIndirect | eGpuOnly
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EBufferUsageFlags, EBufferUsageFlagBits)

		struct FBufferUploadRegion
		{
			u64 count{ 1ull };
			u64 size{ 0ull };

			u64 srcOffset{ 0ull };
			u64 dstOffset{ 0ull };
		};

		struct FBufferUploadInfo
		{
			u64 bufferId{ ~0ull };
			utl::vector<u8> data{};
			utl::vector<FBufferUploadRegion> regions{};
		};

		struct FBufferCreateInfo
		{
			// EBufferTypeFlagBits
			EBufferUsageFlags flags{};
			u64 count{ 1ull };
			u64 size{ 0ull };
		};

		struct FBufferView
		{
			u64 alignedSize{ 0ull };
			u64 elementCount{ 0ull };
			uint64_t address{ 0ull };
		};
	}
}