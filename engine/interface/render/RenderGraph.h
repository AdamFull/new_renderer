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
			eReadAfterWrite				= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceAccessFlagBits::eRead)),
			eWriteAfterNothing			= utl::pack_u32_16x2(utl::cast<u16>(0u), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite)),
			eWriteAfterRead				= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite)),
			eWriteAfterWrite			= utl::pack_u32_16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite))
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

		enum class EViewportDependencyType
		{
			eManual,
			eOnViewport,
			eOnRenderTarget
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

		struct FViewportCreateInfo
		{
			void setFixedSize2D(u32 size)
			{
				width = height = size;
				dependencyType = EViewportDependencyType::eManual;
			}

			void setFixedSize2D(u32 w, u32 h)
			{
				width = w;
				height = h;
				dependencyType = EViewportDependencyType::eManual;
			}

			void dependOnTarget(const std::string& name)
			{
				dependsOn = name;
				dependencyType = EViewportDependencyType::eOnRenderTarget;
			}

			void dependOnViewport(const std::string& name)
			{
				dependsOn = name;
				dependencyType = EViewportDependencyType::eOnViewport;
			}

			void setScale(float new_scale) { scale = new_scale; }

			std::string dependsOn;
			u32 width{ 0u }, height{ 0u }, depth{ 1u };
			EViewportDependencyType dependencyType;
			float scale{ 1.f };
		};

		struct FViewportView
		{
			u64 dependencyResource{ INVALID_HANDLE };
			u32 width{ 0u }, height{ 0u }, depth{ 1u };
			EViewportDependencyType dependencyType;
			float scale{ 1.f };
		};

		struct FRenderResourceView
		{
			ERenderResourceTypeFlagBits type{};
			u64 handle{ INVALID_HANDLE };
		};

		struct FResourceBarrierInfo
		{
			ERenderResourceDependencyBarrierType type{};
			FRenderResourceView resource{};
			EPassType srcPassType{};
			EPassType dstPassType{};
			u32 subresourceIndex{ ~0u };
			bool translationBarrier{ false };
		};

		struct FRenderGraphAttachmentInfo
		{
			u64 handle{ INVALID_HANDLE };
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
			utl::hash_map<u64, FRenderGraphAttachmentOperationInfo> attachments{};

			const FRenderGraphAttachmentOperationInfo& getAttachmentInfo(u64 resource_index) const
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
			u64 shaderHandle{};
			bool divideByLocals{ true };
			FDispatch2Group globalSizes{};
		};

		struct FDispatchIndirectInfo
		{
			u64 shaderHandle{};
			bool divideByLocals{ true };
			utl::vector<FDispatch2Group> groups{};
		};

		struct FRenderResource
		{
			ERenderResourceTypeFlagBits type{};
			EQueueTypeFlags queuesUsed{ 0u };
			u64 handle{ INVALID_HANDLE };

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
			utl::hash_map<ERenderResourceAccessFlagBits, utl::vector<std::string>> otherResources{};

			virtual void clearResource() 
			{ 
				resourceBindings.clear(); 
				otherResources.clear();
			}

			void bindWriteResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eWrite].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, 0u }); }
			void bindWriteResource(const std::string& binding_point, const std::string& resource_name, u32 subresource) { resourceBindings[ERenderResourceAccessFlagBits::eWrite].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, subresource }); }
			void bindReadResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eRead].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, 0u }); }
			void bindReadResource(const std::string& binding_point, const std::string& resource_name, u32 subresource) { resourceBindings[ERenderResourceAccessFlagBits::eRead].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, subresource }); }
			void addWriteResource(const std::string& resource_name) { otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); }
			void addReadResource(const std::string& resource_name) { otherResources[ERenderResourceAccessFlagBits::eRead].emplace_back(resource_name); }
		};

		struct FRenderConfigRenderCreateInfo : public FRenderConfigPassCreateInfo
		{
			ERenderingType type{};
			std::string viewport{};
			std::string depthResource{};
			std::string stencilResource{};
			bool flipViewportState{ false };


			void flipFiewport() { flipViewportState = true; }
			void setViewport(const std::string& name) { viewport = name; }
			void setDepthTarget(const std::string& resource_name) 
			{ 
				if (stencilResource == resource_name)
				{
					depthResource = resource_name;
					return;
				}

				depthResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); 
			}
			void setStencilTarget(const std::string& resource_name) 
			{ 
				if (depthResource == resource_name)
				{
					stencilResource = resource_name;
					return;
				}

				stencilResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); 
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
			u64 target{ INVALID_HANDLE };
			u32 width{ 1u }, height{ 1u }, depth{ 1u };
			u32 subresource{ ~0u };
		};

		struct FRenderConfigComputeCreateInfo : public FRenderConfigPassCreateInfo
		{
			EComputeType type{};
			utl::vector<FRenderConfigComputeDispatchGroupInfo> groupInfos{};

			void clearResource() override
			{
				FRenderConfigPassCreateInfo::clearResource();
				groupInfos.clear();
			}

			void addDispatchGroupInfo(const std::string& target, u32 subresource, bool divide_by_locals)
			{
				auto& group_info = groupInfos.emplace_back();
				group_info.type = ERenderingSizeType::eFromRenderTarget;
				group_info.target = target;
				group_info.divideByLocals = divide_by_locals;
				group_info.subresource = subresource;
			}

			void addDispatchGroupInfo(const std::string& target, bool divide_by_locals) { addDispatchGroupInfo(target, 0u, divide_by_locals); }

			void addDispatchGroupInfo(u32 width, u32 height, u32 depth, u32 subresource, bool divide_by_locals)
			{
				auto& group_info = groupInfos.emplace_back();
				group_info.type = ERenderingSizeType::eManual;
				group_info.width = width;
				group_info.height = height;
				group_info.depth = depth;
				group_info.divideByLocals = divide_by_locals;
				group_info.subresource = subresource;
			}

			void addDispatchGroupInfo(u32 width, u32 height, u32 depth, bool divide_by_locals) { addDispatchGroupInfo(width, height, depth, 0u, divide_by_locals); }
		};

		struct FRenderResourceBinding
		{
			u64 resource{ INVALID_HANDLE };
			u32 subresource{ ~0u };

			bool operator==(const FRenderResourceBinding& other) const
			{ return (resource == other.resource && subresource == other.subresource); }
		};

		struct FRenderResourceBindingKey
		{
			FRenderResourceBinding binding;
			ERenderResourceDependencyBarrierType type;

			bool operator==(const FRenderResourceBindingKey& other) const
			{ return (binding == other.binding && type == other.type); }
		};

		struct FPassCreateInfo
		{
			EQueueTypeFlagBits queue{};
			std::string shader{};
			bool oneTime{ false };

			utl::hash_map<ERenderResourceGroupType, utl::vector<FRenderResourceBinding>> resources{};
			utl::hash_map<std::string, FRenderResourceBinding> bindings{};
		};

		struct FRenderPassCreateInfo : public FPassCreateInfo
		{
			bool flipViewport{ false };

			u64 viewportIndex{ INVALID_HANDLE };
			u64 depthTarget{ INVALID_HANDLE };
			u64 stencilTarget{ INVALID_HANDLE };
		};

		struct FComputePassCreateInfo : public FPassCreateInfo
		{
			utl::vector<FComputeDispatchGroupInfo> groupInfos{};
		};

		class IRenderGraph
		{
		public:
			virtual ~IRenderGraph() = default;

			virtual void create() = 0;

			virtual i64 addViewport(const std::string& name, const FViewportCreateInfo& create_info) = 0;
			virtual void setDefaultViewport(const std::string& name) = 0;
			virtual const FViewportView& getViewportView(const std::string& name) const = 0;
			virtual const FViewportView& getViewportView(u64 idx) const = 0;
			virtual const FViewportView& getDefaultViewportView() const = 0;
			virtual const FViewport& getViewport(const std::string& name) const = 0;
			virtual const FViewport& getViewport(u64 idx) const = 0;
			virtual const FViewport& getDefaultViewport() const = 0;
			virtual u64 getDefaultViewportIndex() const = 0;

			virtual u64 addStaticResource(const std::string& name, const std::string& path) = 0;
			virtual u64 addRenderResource(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual u64 addRenderResource(const std::string& name, const FBufferCreateInfo& buffer_ci) = 0;
			virtual void setBackbufferResourceName(const std::string& name) = 0;

			virtual void pushDebugMarker(const std::string& name, u32 color, EQueueTypeFlagBits queue) = 0;
			virtual void popDebugMarker(const std::string& name, EQueueTypeFlagBits queue) = 0;

			virtual const FRenderResource& getRenderResource(const std::string& name) const = 0;
			virtual const FRenderResource& getRenderResource(u64 resource_id) const = 0;

			virtual u64 addRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) = 0;
			virtual u64 addRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) = 0;

			virtual void build() = 0;
			virtual void update(f32 dt) = 0;
			virtual void execute(f32 dt) = 0;
		};
	}
}

template <>
struct std::hash<engine::render::FRenderResourceBinding>
{
	std::size_t operator()(const engine::render::FRenderResourceBinding& k) const
	{
		return ((std::hash<u64>()(k.resource) ^ (std::hash<u32>()(k.subresource) << 1)) >> 1);
	}
};

template <>
struct std::hash<engine::render::FRenderResourceBindingKey>
{
	std::size_t operator()(const engine::render::FRenderResourceBindingKey& k) const
	{
		return ((std::hash<engine::render::FRenderResourceBinding>()(k.binding) ^ (std::hash<u32>()(utl::enum_cast(k.type)) << 1)) >> 1);
	}
};