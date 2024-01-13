#pragma once 

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "TextureStructures.h"

namespace engine
{
	namespace render
	{
		constexpr const uint32_t make_render_group_flag(uint16_t access_type, uint16_t group_type)
		{
			return (access_type << 16) | (group_type & 0xFFFF);
		}

		enum ERenderResourceType : uint32_t
		{
			RENDER_RESOURCE_TYPE_RENDER_TARGET = 1 << 0,
			RENDER_RESOURCE_TYPE_STORAGE_TARGET = 1 << 1,
			RENDER_RESOURCE_TYPE_STATIC_IMAGE = 1 << 2,
			RENDER_RESOURCE_TYPE_ALL_IMAGE = RENDER_RESOURCE_TYPE_RENDER_TARGET | RENDER_RESOURCE_TYPE_STORAGE_TARGET | RENDER_RESOURCE_TYPE_STATIC_IMAGE,
			RENDER_RESOURCE_TYPE_BUFFER = 1 << 3
		};

		enum ERenderResourceAccessTypeBits : uint16_t
		{
			RENDER_RESOURCE_ACCESS_READ_BIT = 1 << 0,
			RENDER_RESOURCE_ACCESS_WRITE_BIT = 1 << 1
		};

		enum ERenderResourceGroupTypeBits : uint16_t
		{
			RENDER_RESOURCE_GROUP_COLOR_BIT = 1 << 0,
			RENDER_RESOURCE_GROUP_RESOLVE_BIT = 1 << 1,
			RENDER_RESOURCE_GROUP_STORAGE_TEXTURE_BIT = 1 << 2,
			RENDER_RESOURCE_GROUP_BLIT_TEXTURE_BIT = 1 << 3,
			RENDER_RESOURCE_GROUP_STORAGE_BIT = 1 << 4,
		};

		enum ERenderResourceGroupType : uint32_t
		{
			RENDER_RESOURCE_GROUP_COLOR_READ					= make_render_group_flag(RENDER_RESOURCE_ACCESS_READ_BIT, RENDER_RESOURCE_GROUP_COLOR_BIT),
			RENDER_RESOURCE_GROUP_COLOR_WRITE					= make_render_group_flag(RENDER_RESOURCE_ACCESS_WRITE_BIT, RENDER_RESOURCE_GROUP_COLOR_BIT),
			RENDER_RESOURCE_GROUP_RESOLVE_WRITE					= make_render_group_flag(RENDER_RESOURCE_ACCESS_WRITE_BIT, RENDER_RESOURCE_GROUP_RESOLVE_BIT),
			RENDER_RESOURCE_GROUP_STORAGE_TEXTURE_READ			= make_render_group_flag(RENDER_RESOURCE_ACCESS_READ_BIT, RENDER_RESOURCE_GROUP_STORAGE_TEXTURE_BIT),
			RENDER_RESOURCE_GROUP_STORAGE_TEXTURE_WRITE			= make_render_group_flag(RENDER_RESOURCE_ACCESS_WRITE_BIT, RENDER_RESOURCE_GROUP_STORAGE_TEXTURE_BIT),
			RENDER_RESOURCE_GROUP_BLIT_TEXTURE_READ				= make_render_group_flag(RENDER_RESOURCE_ACCESS_READ_BIT, RENDER_RESOURCE_GROUP_BLIT_TEXTURE_BIT),
			RENDER_RESOURCE_GROUP_BLIT_TEXTURE_WRITE			= make_render_group_flag(RENDER_RESOURCE_ACCESS_WRITE_BIT, RENDER_RESOURCE_GROUP_BLIT_TEXTURE_BIT),
			RENDER_RESOURCE_GROUP_STORAGE_READ					= make_render_group_flag(RENDER_RESOURCE_ACCESS_READ_BIT, RENDER_RESOURCE_GROUP_STORAGE_BIT),
			RENDER_RESOURCE_GROUP_STORAGE_WRITE					= make_render_group_flag(RENDER_RESOURCE_ACCESS_WRITE_BIT, RENDER_RESOURCE_GROUP_STORAGE_BIT),
		};

		enum ERenderGraphSizePolicy
		{
			RENDER_GRAPH_SIZE_POLICY_MANUAL,
			RENDER_GRAPH_SIZE_POLICY_FROM_TARGET,
			RENDER_GRAPH_SIZE_POLICY_FROM_BUFFER_SIZE,
		};

		struct FRenderGraphAttachmentInfo
		{
			size_t handle{ ~0ull };
			uint32_t loadOp{ ~0u };
			uint32_t storeOp{ ~0u };
		};

		struct FRenderGraphAttachmentOperationInfo
		{
			uint32_t loadOp{ ~0u };
			uint32_t storeOp{ ~0u };
		};

		struct FBeginRendering2Info
		{
			uint32_t queue{ 0u };
			uint32_t layerCount{ 1u };
			FRect2D<float> renderArea{};
			bool flipViewport{ false };

			std::vector<FRenderGraphAttachmentInfo> colorAttachments{};

			FRenderGraphAttachmentInfo depthAttachment{};
			FRenderGraphAttachmentInfo stencilAttachment{};

#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string name{};
#endif
		};

		struct FDispatchGroup
		{
			uint32_t x{ 1u }, y{ 1u }, z{ 1u };
		};

		struct FDispatch2Info
		{
			size_t shaderHandle{};
			bool divideByLocals{ true };
			FDispatchGroup globalSizes{};
		};

		struct FDispatchIndirectInfo
		{
			size_t shaderHandle{};
			bool divideByLocals{ true };
			std::vector<FDispatchGroup> groups{};
		};

		/**
		* @struct FRenderResource
		* @brief Represents a rendering resource in the graphics engine.
		*
		* This structure is used to manage and track the usage of various rendering resources such as textures, buffers, etc.
		*/
		struct FRenderResource
		{
			uint32_t type{ 0u }; ///< Type of the render resource, defined by specific constants.
			uint32_t queuesUsed{ 0u }; ///< Bitfield representing the queues where this resource is used.
			size_t handle{ ~0ull }; ///< Handle to the actual resource, typically a GPU resource handle.

			/// Maps each resources group type to a set of resources.
			std::unordered_map<uint16_t, std::unordered_set<size_t>> usages{};
		};

		struct FRenderConfigPassCreateInfo
		{
			uint32_t type{ 0u };

			uint32_t queue{ 0u };
			std::string shader{};
			bool oneTime{ false };

			std::string sizeTarget{};

			std::unordered_map<uint16_t, std::unordered_map<std::string, std::string>> resourceBindings{};
			std::vector<std::string> renderTargets{};

			void bindWriteResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[RENDER_RESOURCE_ACCESS_WRITE_BIT].emplace(binding_point, resource_name); }
			void bindReadResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[RENDER_RESOURCE_ACCESS_READ_BIT].emplace(binding_point, resource_name); }
			void addRenderTarget(const std::string& resource_name) { renderTargets.emplace_back(resource_name); }
		};

		struct FRenderConfigRenderCreateInfo : public FRenderConfigPassCreateInfo
		{
			bool flipViewport{ false };
			std::string depthResource{};
			std::string stencilResource{};
		};

		struct FRenderConfigComputeCreateInfo : public FRenderConfigPassCreateInfo
		{
			
		};

		/**
		* @struct FPassCreateInfo
		* @brief Contains creation information for a general pass in the rendering pipeline.
		*
		* This structure is used as a base for creating different types of passes in the rendering pipeline, like render passes or compute passes.
		*/
		struct FPassCreateInfo
		{
			uint32_t queue{ 0u }; ///< The queue to which this pass belongs.
			std::string shader{}; ///< Shader used in this pass.
			bool oneTime{ false }; ///< Flag indicating if the pass is a one-time pass.

			uint32_t sizePolicy{ 0u };
			size_t sizeTarget{ ~0ull };

			/// Maps resource types to a vector of resource handles used in this pass.
			std::unordered_map<uint32_t, std::vector<size_t>> resources{};
			std::unordered_map<std::string, size_t> bindings{};

			void bindResource(const std::string& bind_point, uint32_t type, size_t resource_id)
			{
				resources[type].emplace_back(resource_id);
				bindings.emplace(bind_point, resource_id);
			}
		};

		/**
		* @struct FRenderPassCreateInfo
		* @brief Contains creation information specific to render passes.
		*
		* This structure extends FPassCreateInfo to include additional settings and resources specifically needed for render passes.
		*/
		struct FRenderPassCreateInfo : public FPassCreateInfo
		{
			bool flipViewport{ false }; ///< Flag to indicate if the viewport should be flipped.

			size_t depthTarget{ ~0ull }; ///< Handle to the depth target resource.
			size_t stencilTarget{ ~0ull }; ///< Handle to the stencil target resource.
		};

		/**
		 * @struct FComputePassCreateInfo
		 * @brief Contains creation information specific to compute passes.
		 *
		 * This structure extends FPassCreateInfo to include additional settings and resources specifically needed for compute passes.
		 */
		struct FComputePassCreateInfo : public FPassCreateInfo
		{
		};

		/**
		* @class IRenderGraph
		* @brief Interface class for the render graph in the graphics engine.
		*
		* This class provides an interface for managing and executing rendering operations in a structured graph. It defines methods for adding resources and executing the graph.
		*/
		class IRenderGraph
		{
		public:
			/**
			* @brief Virtual destructor for interface class.
			*/
			virtual ~IRenderGraph() = default;

			virtual size_t addStaticResource(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual size_t addRenderResource(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual size_t addRenderResource(const std::string& name, const FBufferCreateInfo& buffer_ci) = 0;

			virtual const FRenderResource& getRenderResource(const std::string& name) const = 0;
			virtual const FRenderResource& getRenderResource(size_t resource_id) const = 0;

			virtual size_t addRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) = 0;
			virtual size_t addRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) = 0;

			/**
			 * @brief Finalizes the graph setup.
			 *
			 * This method should be called after all resources are added and before executing the graph. It performs necessary preparations for rendering.
			 */
			virtual void bake() = 0;

			/**
			 * @brief Executes the render graph.
			 *
			 * @param dt Delta time indicating the time elapsed since the last execution.
			 */
			virtual void execute(float dt) = 0;
		};
	}
}