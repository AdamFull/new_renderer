#pragma once 

#include <container/hash_map.h>
#include "TextureStructures.h"

namespace engine
{
	namespace render
	{
		enum class ERenderResourceTypeFlagBits : u32
		{
			eRenderTarget = 1 << 0,
			eStorageTarget = 1 << 1,
			eStaticImage = 1 << 2,
			eBackbuffer = 1 << 3,
			eAllImage = eRenderTarget | eStorageTarget | eStaticImage | eBackbuffer,
			eBuffer = 1 << 4,
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ERenderResourceTypeFlags, ERenderResourceTypeFlagBits)

		enum class ERenderResourceAccessFlagBits : u16
		{
			eNone,
			eRead = 1 << 0,
			eWrite = 1 << 1
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ERenderResourceAccessFlags, ERenderResourceAccessFlagBits)

		enum class ERenderResourceGroupFlagBits : u16
		{
			eColor = 1 << 0,
			eResolve = 1 << 1,
			eStorageTexture = 1 << 2,
			eBlitTexture = 1 << 3,
			eStorage = 1 << 4,
			eBackbuffer = 1 << 5
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ERenderResourceGroupFlags, ERenderResourceGroupFlagBits)

		enum class ERenderResourceGroupType : u32
		{
			eColorRead					= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eColor)),
			eColorWrite					= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eColor)),
			eResolveWrite				= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eResolve)),
			eStorageTextureRead			= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eStorageTexture)),
			eStorageTextureWrite		= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eStorageTexture)),
			eBlitTextureRead			= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eBlitTexture)),
			eBlitTextureWrite			= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eBlitTexture)),
			eStorageRead				= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eStorage)),
			eStorageWrite				= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eStorage)),
			eBackbufferRead				= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead), utl::cast<u16>(ERenderResourceGroupFlagBits::eBackbuffer)),
			eBackbufferWrite			= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eBackbuffer)),
		};

		enum class ERenderResourceDependencyBarrierType : u32
		{
			eReadAfterNone				= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eNone), utl::cast<u16>(ERenderResourceAccessFlagBits::eRead)),
			eReadAfterWrite				= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceAccessFlagBits::eRead)),
			eWriteAfterNone				= utl::pack_u32_16x2(utl::cast<u16>(0u), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite)),
			eWriteAfterRead				= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite)),
			eWriteAfterWrite			= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite))
		};

		enum class ERenderResourceBarrierType
		{
			eMemory,
			eIntermediate,
			eFinal
		};

		enum class ERenderResourceLoadOp : u32
		{
			eDontCare,
			eClear,
			eLoad
		};

		enum class ERenderResourceStoreOp : u32
		{
			eDontCare,
			eStore
		};

		enum class EPassType
		{
			eNone,
			eCompute,
			eRender,
			eDebug
		};


		inline constexpr ERenderResourceGroupType packResourceGroup(ERenderResourceAccessFlagBits access_flag, ERenderResourceGroupFlagBits group_flag)
		{
			return utl::cast<ERenderResourceGroupType>(utl::pack_u32_16x2(utl::enum_cast(access_flag), utl::enum_cast(group_flag)));
		}

		inline void unpackResourceGroup(ERenderResourceGroupType resource_group, ERenderResourceAccessFlagBits& access_flag, ERenderResourceGroupFlagBits& group_flag)
		{
			u16 access_mask{}, group_mask{};
			utl::unpack_u32_16x2(utl::enum_cast(resource_group), access_mask, group_mask);
			access_flag = utl::cast<ERenderResourceAccessFlagBits>(access_mask);
			group_flag = utl::cast<ERenderResourceGroupFlagBits>(group_mask);
		}

		inline constexpr ERenderResourceDependencyBarrierType packDependencyBarrier(ERenderResourceAccessFlagBits src, ERenderResourceAccessFlagBits dst)
		{
			return utl::cast<ERenderResourceDependencyBarrierType>(utl::pack_u32_16x2(utl::enum_cast(src), utl::enum_cast(dst)));
		}

		inline void unpackDependencyBarrier(ERenderResourceDependencyBarrierType dependency_barrier, ERenderResourceAccessFlagBits& src, ERenderResourceAccessFlagBits& dst)
		{
			u16 src_mask{}, dst_mask{};
			utl::unpack_u32_16x2(utl::enum_cast(dependency_barrier), src_mask, dst_mask);
			src = utl::cast<ERenderResourceAccessFlagBits>(src_mask);
			dst = utl::cast<ERenderResourceAccessFlagBits>(dst_mask);
		}

		struct FClearValues
		{
			FClearValues(const glm::vec4& c) : color(c) {}
			FClearValues(f32 d) : depth(d) {}
			FClearValues(u32 s) : stencil(s) {}

			glm::vec4 color{ 0.f, 0.f, 0.f, 1.f };
			f32 depth{ 1.f };
			u32 stencil{ 0u };
		};

		struct FRenderResourceView
		{
			ERenderResourceTypeFlagBits type{};
			FRHIHandle handle{ INVALID_HANDLE };
		};

		struct FResourceBarrierInfo
		{
			ERenderResourceBarrierType barrierType{};
			ERenderResourceDependencyBarrierType dependencyType{};
			FRenderResourceView resource{};
			EPassType srcPassType{};
			EPassType dstPassType{};
			u32 subresourceIndex{ ~0u };
		};

		struct FRenderGraphAttachmentInfo
		{
			FAPIHandle handle{ INVALID_HANDLE };
			u32 mipLevel{ ~0u };
			ERenderResourceLoadOp loadOp{};
			ERenderResourceStoreOp storeOp{};
			bool backbuffer{ false };
		};

		struct FRenderGraphAttachmentOperationInfo
		{
			ERenderResourceLoadOp loadOp{};
			ERenderResourceStoreOp storeOp{};
		};

		struct FPassBuildInfo
		{
			utl::hash_map<FGraphHandle, FRenderGraphAttachmentOperationInfo> attachments{};

			const FRenderGraphAttachmentOperationInfo& getAttachmentInfo(FGraphHandle resource_index) const
			{
				auto found = attachments.find(resource_index);
				if (found != attachments.end())
					return found->second;

				return {};
			}
		};

		struct FBeginRendering2Info
		{
			EQueueTypeFlagBits queue{};
			u32 layerCount{ 1u };
			FRect2D<f32> renderArea{};
			utl::hash_map<FAPIHandle, FClearValues> clearValues;

			utl::vector<FRenderGraphAttachmentInfo>* colorAttachments{ nullptr };

			FRenderGraphAttachmentInfo* depthAttachment{ nullptr };
			FRenderGraphAttachmentInfo* stencilAttachment{ nullptr };

#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string name{};
#endif
		};

		struct FBeginComputeInfo
		{
			EQueueTypeFlagBits queue{};

#ifndef BUILD_WITHOUT_DEBUGGIND
			std::string name{};
#endif
		};

		struct FDispatch2Group
		{
			u32 x{ 1u }, y{ 1u }, z{ 1u };
		};

		struct FDispatch2Info
		{
			FAPIHandle shaderHandle{};
			bool divideByLocals{ true };
			FDispatch2Group globalSizes{};
		};

		struct FDispatchIndirectInfo
		{
			FAPIHandle shaderHandle{};
			bool divideByLocals{ true };
			utl::vector<FDispatch2Group> groups{};
		};

		struct FRenderResource
		{
			ERenderResourceTypeFlagBits type{};
			EQueueTypeFlags queuesUsed{ 0u };
			FRHIHandle handle{ INVALID_HANDLE };

			utl::hash_map<ERenderResourceAccessFlagBits, std::unordered_set<u64>> usages{};
		};

		struct FRenderConfigResourceBindingDescription
		{
			std::string name{};
			u32 subresource{ ~0u };
		};

		struct FRenderConfigPassCreateInfo
		{
			EQueueTypeFlagBits queue{};
			std::string shader{};
			bool oneTime{ false };

			std::string sizeTarget{};

			utl::hash_map<ERenderResourceAccessFlagBits, utl::hash_map<std::string, FRenderConfigResourceBindingDescription>> resourceBindings{};
			utl::hash_map<ERenderResourceAccessFlagBits, utl::vector<std::pair<std::string, FClearValues>>> otherResources{};

			virtual void ClearResource() 
			{ 
				resourceBindings.clear(); 
				otherResources.clear();
			}

			void BindWriteResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eWrite].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, ~0u }); }
			void BindWriteResource(const std::string& binding_point, const std::string& resource_name, u32 subresource) { resourceBindings[ERenderResourceAccessFlagBits::eWrite].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, subresource }); }
			void BindReadResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eRead].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, ~0u }); }
			void BindReadResource(const std::string& binding_point, const std::string& resource_name, u32 subresource) { resourceBindings[ERenderResourceAccessFlagBits::eRead].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, subresource }); }
			void AddWriteResource(const std::string& resource_name, const glm::vec4 clear_color = glm::vec4(0.f, 0.f, 0.f, 1.f)) { otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(std::make_pair(resource_name, clear_color)); }
			void AddReadResource(const std::string& resource_name) { otherResources[ERenderResourceAccessFlagBits::eRead].emplace_back(resource_name, glm::vec4(0.f, 0.f, 0.f, 1.f)); }
		};

		struct FRenderConfigRenderCreateInfo : public FRenderConfigPassCreateInfo
		{
			ERenderingType type{};
			std::string depthResource{};
			std::string stencilResource{};
			bool flipViewportState{ false };


			void FlipFiewport() { flipViewportState = true; }
			void SetDepthTarget(const std::string& resource_name, f32 clear_color = 1.f) 
			{ 
				if (stencilResource == resource_name)
				{
					depthResource = resource_name;
					return;
				}

				depthResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(std::make_pair(resource_name, clear_color)).first;
			}
			void SetStencilTarget(const std::string& resource_name, u32 clear_color = 0u) 
			{ 
				if (depthResource == resource_name)
				{
					stencilResource = resource_name;
					return;
				}

				stencilResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(std::make_pair(resource_name, clear_color)).first;
			}
		};

		struct FRenderConfigComputeDispatchGroupInfo
		{
			ERenderingSizeType type{};
			bool divideByLocals{ true };
			std::string target{};
			u32 width{ 1u }, height{ 1u }, depth{ 1u };
			u32 subresource{ ~0u };
		};

		struct FComputeDispatchGroupInfo
		{
			ERenderingSizeType type{};
			bool divideByLocals{ true };
			FGraphHandle target{ INVALID_HANDLE };
			u32 width{ 1u }, height{ 1u }, depth{ 1u };
			u32 subresource{ ~0u };
		};

		struct FRenderConfigComputeCreateInfo : public FRenderConfigPassCreateInfo
		{
			EComputeType type{};
			utl::vector<FRenderConfigComputeDispatchGroupInfo> groupInfos{};

			void ClearResource() override
			{
				FRenderConfigPassCreateInfo::ClearResource();
				groupInfos.clear();
			}

			void AddWorkgroupSizeByBuffer(const std::string& target, bool divide_by_locals)
			{
				auto& group_info = groupInfos.emplace_back();
				group_info.type = ERenderingSizeType::eFromBuffer;
				group_info.target = target;
				group_info.divideByLocals = divide_by_locals;
			}

			void AddWorkgroupSizeByRT(const std::string& target, u32 subresource, bool divide_by_locals)
			{
				auto& group_info = groupInfos.emplace_back();
				group_info.type = ERenderingSizeType::eFromRenderTarget;
				group_info.target = target;
				group_info.divideByLocals = divide_by_locals;
				group_info.subresource = subresource;
			}

			void AddWorkgroupSizeByRT(const std::string& target, bool divide_by_locals) { AddWorkgroupSizeByRT(target, ~0u, divide_by_locals); }

			void AddWorkgroupSizeManual(u32 width, u32 height, u32 depth, bool divide_by_locals)
			{
				auto& group_info = groupInfos.emplace_back();
				group_info.type = ERenderingSizeType::eManual;
				group_info.width = width;
				group_info.height = height;
				group_info.depth = depth;
				group_info.divideByLocals = divide_by_locals;
			}
		};

		struct FRenderResourceBinding
		{
			FGraphHandle resource{ INVALID_HANDLE };
			u32 subresource{ ~0u };

			bool operator==(const FRenderResourceBinding& other) const
			{ return (resource == other.resource && subresource == other.subresource); }
		};

		struct FPassCreateInfo
		{
			EQueueTypeFlagBits queue{};
			std::string shader{};
			bool oneTime{ false };

			utl::hash_map<FRHIHandle, FClearValues> clearValues;
			utl::hash_map<ERenderResourceGroupType, utl::vector<FRenderResourceBinding>> resources{};
			utl::hash_map<std::string, FRenderResourceBinding> bindings{};
		};

		struct FRenderPassCreateInfo : public FPassCreateInfo
		{
			bool flipViewport{ false };

			FGraphHandle depthTarget{ INVALID_HANDLE };
			FGraphHandle stencilTarget{ INVALID_HANDLE };
		};

		struct FComputePassCreateInfo : public FPassCreateInfo
		{
			utl::vector<FComputeDispatchGroupInfo> groupInfos{};
		};

		class IRenderGraph
		{
		public:
			virtual ~IRenderGraph() = default;

			virtual void Create(const utl::vector<u8>& config, const utl::vector<u8>& schema) = 0;
			virtual void PostLoad() = 0;

			virtual void AddRenderResource(const std::string& name, const FRenderResource& resource_desc) = 0;
			virtual void SetBackbufferResourceName(const std::string& name) = 0;

			virtual void PushDebugMarker(const std::string& name, u32 color, EQueueTypeFlagBits queue) = 0;
			virtual void PopDebugMarker(const std::string& name, EQueueTypeFlagBits queue) = 0;

			virtual const FRenderResource& GetRenderResource(const std::string& name) const = 0;
			virtual const FRenderResource& GetRenderResource(FGraphHandle resource_id) const = 0;

			virtual u64 AddRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) = 0;
			virtual u64 AddRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) = 0;

			virtual void Build() = 0;
			virtual void Update(f32 dt) = 0;
			virtual void Execute(f32 dt) = 0;
		};
	}
}

template <>
struct std::hash<engine::render::FRenderResourceBinding>
{
	std::size_t operator()(const engine::render::FRenderResourceBinding& k) const
	{
		return ((std::hash<engine::render::FGraphHandle>()(k.resource) ^ (std::hash<u32>()(k.subresource) << 1)) >> 1);
	}
};