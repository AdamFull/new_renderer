#pragma once 

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "TextureStructures.h"

namespace engine
{
	namespace render
	{
		enum class ERenderResourceTypeFlagBits : uint32_t
		{
			eRenderTarget = 1 << 0,
			eStorageTarget = 1 << 1,
			eStaticImage = 1 << 2,
			eAllImage = eRenderTarget | eStorageTarget | eStaticImage,
			eBuffer = 1 << 3
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ERenderResourceTypeFlags, ERenderResourceTypeFlagBits)

		enum class ERenderResourceAccessFlagBits : uint16_t
		{
			eRead = 1 << 0,
			eWrite = 1 << 1
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ERenderResourceAccessFlags, ERenderResourceAccessFlagBits)

		enum class ERenderResourceGroupFlagBits : uint16_t
		{
			eColor = 1 << 0,
			eResolve = 1 << 1,
			eStorageTexture = 1 << 2,
			eBlitTexture = 1 << 3,
			eStorage = 1 << 4
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ERenderResourceGroupFlags, ERenderResourceGroupFlagBits)

		enum class ERenderResourceGroupType : uint32_t
		{
			eColorRead					= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eColor)),
			eColorWrite					= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eColor)),
			eResolveWrite				= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eResolve)),
			eStorageTextureRead			= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eStorageTexture)),
			eStorageTextureWrite		= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eStorageTexture)),
			eBlitTextureRead			= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eBlitTexture)),
			eBlitTextureWrite			= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eBlitTexture)),
			eStorageRead				= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eStorage)),
			eStorageWrite				= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eStorage)),
		};

		enum ERenderGraphSizePolicy
		{
			RENDER_GRAPH_SIZE_POLICY_MANUAL,
			RENDER_GRAPH_SIZE_POLICY_FROM_TARGET,
			RENDER_GRAPH_SIZE_POLICY_FROM_BUFFER_SIZE
		};

		enum ERenderResourceDependencyBarrierType : uint32_t
		{
			eReadAfterWrite				= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceAccessFlagBits::eRead)),
			eWriteAfterNothing			= packUint16x2(utl::cast<u16>(0u), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite)),
			eWriteAfterRead				= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite)),
			eWriteAfterWrite			= packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite))
		};
		using ERenderResourceDependencyBarrierTypeMask = uint32_t;

		inline constexpr ERenderResourceGroupType packResourceGroup(ERenderResourceAccessFlagBits access_flag, ERenderResourceGroupFlagBits group_flag)
		{
			return utl::cast<ERenderResourceGroupType>(packUint16x2(utl::enum_cast(access_flag), utl::enum_cast(group_flag)));
		}

		inline void unpackResourceGroup(ERenderResourceGroupType resource_group, ERenderResourceAccessFlagBits& access_flag, ERenderResourceGroupFlagBits& group_flag)
		{
			u16 access_mask{}, group_mask{};
			unpackUint16x2(utl::enum_cast(resource_group), access_mask, group_mask);

			access_flag = utl::cast<ERenderResourceAccessFlagBits>(access_mask);
			group_flag = utl::cast<ERenderResourceGroupFlagBits>(group_mask);
		}

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

		struct FDispatch2Group
		{
			uint32_t x{ 1u }, y{ 1u }, z{ 1u };
		};

		struct FDispatch2Info
		{
			size_t shaderHandle{};
			bool divideByLocals{ true };
			FDispatch2Group globalSizes{};
		};

		struct FDispatchIndirectInfo
		{
			size_t shaderHandle{};
			bool divideByLocals{ true };
			std::vector<FDispatch2Group> groups{};
		};

		/**
		* @struct FRenderResource
		* @brief Represents a rendering resource in the graphics engine.
		*
		* This structure is used to manage and track the usage of various rendering resources such as textures, buffers, etc.
		*/
		struct FRenderResource
		{
			ERenderResourceTypeFlags type{ 0u }; ///< Type of the render resource, defined by specific constants.
			uint32_t queuesUsed{ 0u }; ///< Bitfield representing the queues where this resource is used.
			size_t handle{ ~0ull }; ///< Handle to the actual resource, typically a GPU resource handle.

			/// Maps each resources group type to a set of resources.
			std::unordered_map<ERenderResourceAccessFlagBits, std::unordered_set<size_t>> usages{};
		};

		struct FRenderConfigPassCreateInfo
		{
			uint32_t type{ 0u };

			uint32_t queue{ 0u };
			std::string shader{};
			bool oneTime{ false };

			std::string sizeTarget{};

			std::unordered_map<ERenderResourceAccessFlagBits, std::unordered_map<std::string, std::string>> resourceBindings{};
			std::unordered_map<ERenderResourceAccessFlagBits, std::vector<std::string>> otherResources{};

			void bindWriteResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eWrite].emplace(binding_point, resource_name); }
			void bindReadResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eRead].emplace(binding_point, resource_name); }
			void addWriteResource(const std::string& resource_name) { otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); }
			void addReadResource(const std::string& resource_name) { otherResources[ERenderResourceAccessFlagBits::eRead].emplace_back(resource_name); }
		};

		struct FRenderConfigRenderCreateInfo : public FRenderConfigPassCreateInfo
		{
			bool flipViewport{ false };
			std::string depthResource{};
			std::string stencilResource{};

			void setDepthTarget(const std::string& resource_name) { depthResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); }
			void setStencilTarget(const std::string& resource_name) { stencilResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); }
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
			std::unordered_map<ERenderResourceGroupType, std::vector<size_t>> resources{};
			std::unordered_map<std::string, size_t> bindings{};
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

			virtual void create() = 0;

			virtual size_t addStaticResource(const std::string& name, const std::string& path) = 0;
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