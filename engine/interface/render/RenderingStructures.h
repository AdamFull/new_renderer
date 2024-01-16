#pragma once

#include <unordered_map>

#include "BaseStructures.h"

namespace engine
{
	namespace render
	{
		enum class ERenderingStepType
		{
			eSynchronization,
			eRendering,
			eCompute
		};

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

		enum class ESynchronizationType
		{
			eSemaphore,
			eBarrier
		};

		enum class ESemaphoreMode
		{
			eSignal,
			eWait
		};

		enum class EBarrierType
		{
			eG2C,
			eG2G,
			eC2C,
			eC2G,
		};

		enum class ERenderingSizeType
		{
			eFromRenderTarget = 1,
			eFromBuffer = 2,
			eManual = 3
		};

		struct FRenderingStepSemaphore
		{
			u32 type{};

			u64 value{};

			u32 srcQueue{};
			u32 dstQueue{};
		};

		struct FRenderingStepBarrier
		{
			EBarrierType type{};
			EQueueTypeFlagBits queue{};
			std::vector<u64> targets;
		};

		struct FRenderingStepSynchronizationInfo
		{
			ESynchronizationType type{};
			FRenderingStepSemaphore semaphore{};
			FRenderingStepBarrier barrier{};
		};

		struct FRenderTargetSizeInfo
		{
			ERenderingSizeType type{ ERenderingSizeType::eManual };
			u64 target{ ~0ull };

			u32 sizeX{ 1u };
			u32 sizeY{ 1u };
			u32 sizeZ{ 1u };
		};

		struct FRenderingStepRenderingInfo
		{
			ERenderingType type{};

			EQueueTypeFlagBits queue{};

			std::string renderScript{}; // Optional

			std::string shaderName{}; // Optional

			bool flipViewport{ false }; // Flip viewport horizontally
			bool materialPass{ false };

			std::unordered_map<std::string, u64> shaderInputs{};
			std::vector<u64> renderTargets{};

			FRenderTargetSizeInfo sizeInfo{};

			u64 depthTarget{ ~0ull };
			u64 stencilTarget{ ~0ull };
		};

		struct FRenderingStepComputeInfo
		{
			EComputeType type{};
			EQueueTypeFlagBits queue{};

			std::string renderScript{};

			std::string shaderName{};

			std::unordered_map<std::string, u64> shaderInputs{};
			std::unordered_map<std::string, u64> writeTargets{};
			std::unordered_map<std::string, u64> writeBuffers{};

			FRenderTargetSizeInfo dispatchInfo{};
			bool divideByLocals{ true };
		};

		struct FRenderingStepCreateInfo
		{
			// ERenderingStepType
			ERenderingStepType type{};
			bool oneTime{ false };

			FRenderingStepSynchronizationInfo synchronization{};
			FRenderingStepRenderingInfo rendering{};
			FRenderingStepComputeInfo compute{};
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