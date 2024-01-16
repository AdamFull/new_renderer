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
			uint32_t type{};

			size_t value{};

			uint32_t srcQueue{};
			uint32_t dstQueue{};
		};

		struct FRenderingStepBarrier
		{
			EBarrierType type{};
			EQueueTypeFlagBits queue{};
			std::vector<size_t> targets;
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
			size_t target{ ~0ull };

			uint32_t sizeX{ 1u };
			uint32_t sizeY{ 1u };
			uint32_t sizeZ{ 1u };
		};

		struct FRenderingStepRenderingInfo
		{
			ERenderingType type{};

			EQueueTypeFlagBits queue{};

			std::string renderScript{}; // Optional

			std::string shaderName{}; // Optional

			bool flipViewport{ false }; // Flip viewport horizontally
			bool materialPass{ false };

			std::unordered_map<std::string, size_t> shaderInputs{};
			std::vector<size_t> renderTargets{};

			FRenderTargetSizeInfo sizeInfo{};

			size_t depthTarget{ ~0ull };
			size_t stencilTarget{ ~0ull };
		};

		struct FRenderingStepComputeInfo
		{
			EComputeType type{};
			EQueueTypeFlagBits queue{};

			std::string renderScript{};

			std::string shaderName{};

			std::unordered_map<std::string, size_t> shaderInputs{};
			std::unordered_map<std::string, size_t> writeTargets{};
			std::unordered_map<std::string, size_t> writeBuffers{};

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
			FRect2D<float> renderArea{};
			bool flipViewport{ false };
			uint32_t layerCount{ 1u };

			std::vector<size_t> colorAttachments{};
			size_t depthAttachment{ ~0ull };
			size_t stencilAttachment{ ~0ull };
#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string name{};
#endif
		};

		struct FBeginComputeInfo
		{
			EQueueTypeFlagBits queue{};

			std::vector<size_t> writeAttachments{};
			std::vector<size_t> writeBuffers{};

			FPoint3D<uint32_t> globalSizes{};
#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string name{};
#endif
		};
	}
}