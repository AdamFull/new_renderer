#pragma once 

#include <standart_library.h>
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
			eColorRead					= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eColor)),
			eColorWrite					= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eColor)),
			eResolveWrite				= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eResolve)),
			eStorageTextureRead			= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eStorageTexture)),
			eStorageTextureWrite		= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eStorageTexture)),
			eBlitTextureRead			= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eBlitTexture)),
			eBlitTextureWrite			= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eBlitTexture)),
			eStorageRead				= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead),  utl::cast<u16>(ERenderResourceGroupFlagBits::eStorage)),
			eStorageWrite				= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eStorage)),
			eBackbufferRead				= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead), utl::cast<u16>(ERenderResourceGroupFlagBits::eBackbuffer)),
			eBackbufferWrite			= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceGroupFlagBits::eBackbuffer)),
		};

		enum class ERenderResourceDependencyBarrierType : u32
		{
			eReadAfterWrite				= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceAccessFlagBits::eRead)),
			eWriteAfterNothing			= utl::packUint16x2(utl::cast<u16>(0u), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite)),
			eWriteAfterRead				= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eRead), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite)),
			eWriteAfterWrite			= utl::packUint16x2(utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite), utl::cast<u16>(ERenderResourceAccessFlagBits::eWrite))
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
			eRender
		};

		enum class EViewportDependencyType
		{
			eManual,
			eOnViewport,
			eOnRenderTarget
		};


		inline constexpr ERenderResourceGroupType packResourceGroup(ERenderResourceAccessFlagBits access_flag, ERenderResourceGroupFlagBits group_flag)
		{
			return utl::cast<ERenderResourceGroupType>(utl::packUint16x2(utl::enum_cast(access_flag), utl::enum_cast(group_flag)));
		}

		inline void unpackResourceGroup(ERenderResourceGroupType resource_group, ERenderResourceAccessFlagBits& access_flag, ERenderResourceGroupFlagBits& group_flag)
		{
			u16 access_mask{}, group_mask{};
			utl::unpackUint16x2(utl::enum_cast(resource_group), access_mask, group_mask);
			access_flag = utl::cast<ERenderResourceAccessFlagBits>(access_mask);
			group_flag = utl::cast<ERenderResourceGroupFlagBits>(group_mask);
		}

		inline constexpr ERenderResourceDependencyBarrierType packDependencyBarrier(ERenderResourceAccessFlagBits src, ERenderResourceAccessFlagBits dst)
		{
			return utl::cast<ERenderResourceDependencyBarrierType>(utl::packUint16x2(utl::enum_cast(src), utl::enum_cast(dst)));
		}

		inline void unpackDependencyBarrier(ERenderResourceDependencyBarrierType dependency_barrier, ERenderResourceAccessFlagBits& src, ERenderResourceAccessFlagBits& dst)
		{
			u16 src_mask{}, dst_mask{};
			utl::unpackUint16x2(utl::enum_cast(dependency_barrier), src_mask, dst_mask);
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
			u64 dependencyResource{ ~0ull };
			u32 width{ 0u }, height{ 0u }, depth{ 1u };
			EViewportDependencyType dependencyType;
			float scale{ 1.f };
		};

		struct FRenderResourceView
		{
			ERenderResourceTypeFlagBits type{};
			u64 handle{ ~0ull };
		};

		struct FResourceBarrierInfo
		{
			ERenderResourceDependencyBarrierType type{};
			FRenderResourceView resource{};
			EPassType srcPassType{};
			EPassType dstPassType{};
		};

		struct FRenderGraphAttachmentInfo
		{
			u64 handle{ ~0ull };
			u32 mipLevel{ 0u };
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
			stl::unordered_map<u64, FRenderGraphAttachmentOperationInfo> attachments{};

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

			stl::vector<FRenderGraphAttachmentInfo>* colorAttachments{ nullptr };

			FRenderGraphAttachmentInfo* depthAttachment{ nullptr };
			FRenderGraphAttachmentInfo* stencilAttachment{ nullptr };

#ifndef BUILD_WITHOUT_DEBUGGIND
			stl::string name{};
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
			stl::vector<FDispatch2Group> groups{};
		};

		struct FRenderResource
		{
			ERenderResourceTypeFlagBits type{};
			EQueueTypeFlags queuesUsed{ 0u };
			u64 handle{ ~0ull };

			stl::unordered_map<ERenderResourceAccessFlagBits, stl::unordered_set<u64>> usages{};
		};

		struct FRenderConfigPassCreateInfo
		{
			EQueueTypeFlagBits queue{};
			stl::string shader{};
			bool oneTime{ false };

			stl::string sizeTarget{};

			stl::unordered_map<ERenderResourceAccessFlagBits, stl::unordered_map<stl::string, stl::string>> resourceBindings{};
			stl::unordered_map<ERenderResourceAccessFlagBits, stl::vector<stl::string>> otherResources{};

			void bindWriteResource(const stl::string& binding_point, const stl::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eWrite].emplace(binding_point, resource_name); }
			void bindReadResource(const stl::string& binding_point, const stl::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eRead].emplace(binding_point, resource_name); }
			void addWriteResource(const stl::string& resource_name) { otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); }
			void addReadResource(const stl::string& resource_name) { otherResources[ERenderResourceAccessFlagBits::eRead].emplace_back(resource_name); }
		};

		struct FRenderConfigRenderCreateInfo : public FRenderConfigPassCreateInfo
		{
			ERenderingType type{};
			std::string viewport{};
			stl::string depthResource{};
			stl::string stencilResource{};
			bool flipViewportState{ false };


			void flipFiewport() { flipViewportState = true; }
			void setViewport(const std::string& name) { viewport = name; }
			void setDepthTarget(const stl::string& resource_name) { depthResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); }
			void setStencilTarget(const stl::string& resource_name) { stencilResource = otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(resource_name); }
		};

		struct FRenderConfigComputeCreateInfo : public FRenderConfigPassCreateInfo
		{
			EComputeType type{};
		};

		struct FPassCreateInfo
		{
			EQueueTypeFlagBits queue{};
			stl::string shader{};
			bool oneTime{ false };

			ERenderingSizeType sizePolicy{};
			u64 sizeTarget{ ~0ull };

			stl::unordered_map<ERenderResourceGroupType, stl::vector<u64>> resources{};
			stl::unordered_map<stl::string, u64> bindings{};
		};

		struct FRenderPassCreateInfo : public FPassCreateInfo
		{
			bool flipViewport{ false };

			u64 viewportIndex{ ~0ull };
			u64 depthTarget{ ~0ull };
			u64 stencilTarget{ ~0ull };
		};

		struct FComputePassCreateInfo : public FPassCreateInfo
		{
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

			virtual u64 addStaticResource(const stl::string& name, const stl::string& path) = 0;
			virtual u64 addRenderResource(const stl::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual u64 addRenderResource(const stl::string& name, const FBufferCreateInfo& buffer_ci) = 0;
			virtual void setBackbufferResourceName(const stl::string& name) = 0;

			virtual const FRenderResource& getRenderResource(const stl::string& name) const = 0;
			virtual const FRenderResource& getRenderResource(u64 resource_id) const = 0;

			virtual u64 addRenderPass(const stl::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) = 0;
			virtual u64 addRenderPass(const stl::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) = 0;

			virtual void build() = 0;
			virtual void update(f32 dt) = 0;
			virtual void execute(f32 dt) = 0;
		};
	}
}