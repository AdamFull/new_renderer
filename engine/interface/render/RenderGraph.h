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
			EQueueTypeFlagBits queue{};
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

		struct FRenderResource
		{
			ERenderResourceTypeFlags type{ 0u };
			EQueueTypeFlags queuesUsed{ 0u };
			size_t handle{ ~0ull };

			std::unordered_map<ERenderResourceAccessFlagBits, std::unordered_set<size_t>> usages{};
		};

		struct FRenderConfigPassCreateInfo
		{
			EQueueTypeFlagBits queue{};
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
			ERenderingType type{};
			bool flipViewport{ false };
			std::string depthResource{};
			std::string stencilResource{};

			void setDepthTarget(const std::string& resource_name) { depthResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); }
			void setStencilTarget(const std::string& resource_name) { stencilResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); }
		};

		struct FRenderConfigComputeCreateInfo : public FRenderConfigPassCreateInfo
		{
			EComputeType type{};
		};

		struct FPassCreateInfo
		{
			EQueueTypeFlagBits queue{};
			std::string shader{};
			bool oneTime{ false };

			ERenderingSizeType sizePolicy{};
			size_t sizeTarget{ ~0ull };

			std::unordered_map<ERenderResourceGroupType, std::vector<size_t>> resources{};
			std::unordered_map<std::string, size_t> bindings{};
		};

		struct FRenderPassCreateInfo : public FPassCreateInfo
		{
			bool flipViewport{ false };

			size_t depthTarget{ ~0ull };
			size_t stencilTarget{ ~0ull };
		};

		struct FComputePassCreateInfo : public FPassCreateInfo
		{
		};

		class IRenderGraph
		{
		public:
			virtual ~IRenderGraph() = default;

			virtual void create() = 0;

			virtual size_t addStaticResource(const std::string& name, const std::string& path) = 0;
			virtual size_t addRenderResource(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual size_t addRenderResource(const std::string& name, const FBufferCreateInfo& buffer_ci) = 0;

			virtual const FRenderResource& getRenderResource(const std::string& name) const = 0;
			virtual const FRenderResource& getRenderResource(size_t resource_id) const = 0;

			virtual size_t addRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) = 0;
			virtual size_t addRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) = 0;

			virtual void bake() = 0;

			virtual void execute(float dt) = 0;
		};
	}
}