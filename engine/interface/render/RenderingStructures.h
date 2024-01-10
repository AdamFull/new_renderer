#pragma once

#include <unordered_map>

#include "BaseStructures.h"

namespace engine
{
	namespace render
	{
		enum ERenderingStepType
		{
			RENDERING_STEP_SYNCHRONIZATION_TYPE,
			RENDERING_STEP_RENDERING_TYPE,
			RENDERING_STEP_COMPUTE_TYPE
		};

		enum ERenderingType
		{
			RENDERING_FULLSCREEN_TYPE,
			RENDERING_GEOMETRY_TYPE,
			RENDERING_CSM_TYPE,
			RENDERING_RENDER_SCRIPT_TYPE
		};

		enum EComputeType
		{
			COMPUTE_DEFAULT_TYPE,
			COMPUTE_GEOMETRY_TYPE,
			COMPUTE_CUBEMAP_TYPE,
			COMPUTE_CULLING_TYPE
		};

		enum ESynchronizationType
		{
			SYNCHRONIZATION_SEMAPHORE_TYPE,
			SYNCHRONIZATION_BARRIER_TYPE
		};

		enum ESemaphoreMode
		{
			SEMAPHORE_SIGNAL_MODE,
			SEMAPHORE_WAIT_MODE
		};

		enum EBarrierType
		{
			BARRIER_FROM_GRAPHICS_TO_COMPUTE,
			BARRIER_FROM_GRAPHICS_TO_GRAPHICS,
			BARRIER_FROM_COMPUTE_TO_COMPUTE,
			BARRIER_FROM_COMPUTE_TO_GRAPHICS,
		};

		enum ERenderingSizeType
		{
			RENDER_SIZE_TYPE_FROM_RT = 1,
			RENDER_SIZE_TYPE_FROM_BUFFER = 2,
			RENDER_SIZE_TYPE_MANUAL = 3
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
			uint32_t type{};
			uint32_t queue{};
			std::vector<size_t> targets;
		};

		struct FRenderingStepSynchronizationInfo
		{
			uint32_t type{};
			FRenderingStepSemaphore semaphore{};
			FRenderingStepBarrier barrier{};
		};

		struct FRenderTargetSizeInfo
		{
			uint32_t type{ RENDER_SIZE_TYPE_MANUAL };
			size_t target{ ~0ull };

			uint32_t sizeX{ 1u };
			uint32_t sizeY{ 1u };
			uint32_t sizeZ{ 1u };
		};

		struct FRenderingStepRenderingInfo
		{
			// ERenderingType
			uint32_t type{ 0u };

			uint32_t queue{ 0u };

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
			uint32_t type{ 0u };
			uint32_t queue{ ~0u };

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
			uint32_t type{};
			bool oneTime{ false };

			FRenderingStepSynchronizationInfo synchronization{};
			FRenderingStepRenderingInfo rendering{};
			FRenderingStepComputeInfo compute{};
		};

		struct FBeginRenderingInfo
		{
			uint32_t queue{ 0u };
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
			uint32_t queue{ 0u };

			std::vector<size_t> writeAttachments{};
			std::vector<size_t> writeBuffers{};

			FPoint3D<uint32_t> globalSizes{};
#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string name{};
#endif
		};
	}
}