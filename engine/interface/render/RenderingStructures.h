#pragma once

#include <unordered_map>

#include <interface/render/BaseStructures.h>

namespace engine
{
	namespace render
	{
		enum ERenderingStepType
		{
			RENDERING_STEP_SYNCHRONIZATION_TYPE,
			RENDERING_STEP_RENDERING_TYPE
		};

		enum ERenderingType
		{
			RENDERING_FULLSCREEN_TYPE,
			RENDERING_VISBUFFER_TYPE,
			RENDERING_RENDER_SCRIPT_TYPE
		};

		enum ESynchronizationType
		{
			SYNCHRONIZATION_WAIT_TYPE,
			SYNCHRONIZATION_SIGNAL_TYPE
		};

		struct FRenderingStepSynchronizationInfo
		{
			// ESynchronizationType
			uint32_t type{};

			uint32_t queueFrom{};
			uint32_t queueTo{};
		};

		struct FRenderingStepRenderingInfo
		{
			// ERenderingType
			uint32_t type{ ~0u };

			uint32_t queue{ 0u };

			std::string renderScript{}; // Optional

			std::string shaderName{}; // Optional

			bool oneTime{ false }; // Destroy after execution
			bool flipViewport{ false }; // Flip viewport horizontally
			bool materialPass{ false };

			std::unordered_map<std::string, size_t> shaderInputs{};
			std::vector<size_t> renderTargets{};

			size_t depthTarget{ ~0ull };
			size_t stencilTarget{ ~0ull };
		};

		struct FRenderingStepCreateInfo
		{
			// ERenderingStepType
			uint32_t type{};

			FRenderingStepSynchronizationInfo synchronization{};
			FRenderingStepRenderingInfo rendering{};
		};

		struct FBeginRenderingInfo
		{
			uint32_t queue{ 0u };
			FRect2D<float> renderArea{};
			bool flipViewport{ false };

			std::vector<size_t> colorAttachments{};
			size_t depthAttachment{ ~0ull };
			size_t stencilAttachment{ ~0ull };
#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string name{};
#endif
		};
	}
}