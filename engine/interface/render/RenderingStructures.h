#pragma once

#include <unordered_map>

#include "BaseStructures.h"

namespace engine
{
	namespace render
	{
		enum class ERenderingType
		{
			eFullscreen,
			eGeometry,
			eCascadedShadowMap,
			eRenderScript
		};

		enum class EComputeType
		{
			eDefault,
			eGeometry,
			eCubemap,
			eCulling
		};

		enum class ERenderingSizeType
		{
			eFromRenderTarget = 1,
			eFromBuffer = 2,
			eManual = 3
		};

		struct FRenderTargetSizeInfo
		{
			ERenderingSizeType type{ ERenderingSizeType::eManual };
			u64 target{ ~0ull };

			u32 sizeX{ 1u };
			u32 sizeY{ 1u };
			u32 sizeZ{ 1u };
		};

		struct FBeginRenderingInfo
		{
			EQueueTypeFlagBits queue{};
			FRect2D<f32> renderArea{};
			bool flipViewport{ false };
			u32 layerCount{ 1u };

			std::vector<u64> colorAttachments{};
			u64 depthAttachment{ ~0ull };
			u64 stencilAttachment{ ~0ull };
#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string name{};
#endif
		};

		struct FBeginComputeInfo
		{
			EQueueTypeFlagBits queue{};

			std::vector<u64> writeAttachments{};
			std::vector<u64> writeBuffers{};

			FPoint3D<u32> globalSizes{};
#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string name{};
#endif
		};
	}
}