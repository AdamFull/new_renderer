#pragma once

#include <engine/rendering/gfx/Structures.h>
#include <memory.hpp>

namespace engine
{
	namespace render
	{
		enum class RenderResourceTypeFlag : uint16_t
		{
			eRenderTarget = 1 << 0,
			eStorageTarget = 1 << 1,
			eStaticImage = 1 << 2,
			eBackbuffer = 1 << 3,
			eAllImage = eRenderTarget | eStorageTarget | eStaticImage | eBackbuffer,
			eBuffer = 1 << 4,
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(RenderResourceTypeFlags, RenderResourceTypeFlag);

		enum class RenderResourceGroupFlag : uint16_t
		{
			eColor = 1 << 0,
			eResolve = 1 << 1,
			eStorageTexture = 1 << 2,
			eStorage = 1 << 3,
			eIndirectArgument = 1 << 4,
			eIndirectCount = 1 << 5,
			eBackbuffer = 1 << 6
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(RenderResourceGroupFlags, RenderResourceGroupFlag);
		
		enum class RenderResourceGroupType : uint32_t
		{
			eColorRead,
			eColorWrite,
			eResolveWrite,
			eStorageTextureRead,
			eStorageTextureWrite,
			eStorageRead,
			eStorageWrite,
			eBackbufferRead,
			eBackbufferWrite,
			eIndirectArgumentRead,
			eIndirectArgumentWrite,
			eIndirectCountRead,
			eIndirectCountWrite
		};

		enum class RenderGraphResourceAccess
		{
			eRead,
			eWrite,
			eReadAndWrite
		};

		struct RenderGraphResourceBinding
		{
			RenderResourceGroupFlag resource_group{};
			// Id of render graph resource
			uint32_t render_resource_id{ ~0u };
		};

		using RenderGraphPassResourceAccess = std::array<std::vector<RenderGraphResourceBinding>, 3ull>;

		struct RenderGraphResource
		{
			RenderResourceTypeFlag resource_type{};
			// Id of real resource
			uint32_t resource_id{ ~0u };

			std::array<std::unordered_set<uint32_t>, 3ull> usages{};
		};

		struct ShaderPassCreateInfo
		{
			std::string shader_name{};

			RenderGraphPassResourceAccess shader_bindings{};

			//void BindWriteResource(uint32_t binding_point, RenderGraphResourceBinding resource_binding) {  }
			//
			//void BindWriteResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eWrite].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, ~0u }); }
			//void BindWriteResource(const std::string& binding_point, const std::string& resource_name, u32 subresource) { resourceBindings[ERenderResourceAccessFlagBits::eWrite].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, subresource }); }
			//void BindReadResource(const std::string& binding_point, const std::string& resource_name) { resourceBindings[ERenderResourceAccessFlagBits::eRead].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, ~0u }); }
			//void BindReadResource(const std::string& binding_point, const std::string& resource_name, u32 subresource) { resourceBindings[ERenderResourceAccessFlagBits::eRead].emplace(binding_point, FRenderConfigResourceBindingDescription{ resource_name, subresource }); }
			//void AddWriteResource(const std::string& resource_name, const glm::vec4 clear_color = glm::vec4(0.f, 0.f, 0.f, 1.f)) { otherResources[ERenderResourceAccessFlagBits::eWrite].emplace_back(std::make_pair(resource_name, clear_color)); }
			//void AddReadResource(const std::string& resource_name) { otherResources[ERenderResourceAccessFlagBits::eRead].emplace_back(resource_name, glm::vec4(0.f, 0.f, 0.f, 1.f)); }
			//
			//void BindIndirectArgumentBuffer(const std::string& resource_name, u64 offset) { argumentBuffer = resource_name; argumentOffset = offset; }
			//void BindIndirectCountBuffer(const std::string& resource_name, u64 offset) { countBuffer = resource_name; countOffset = offset; }
		};


		struct ComputeShaderPassCreateInfo : ShaderPassCreateInfo
		{
			
		};

		struct RenderShaderPassCreateInfo : ShaderPassCreateInfo
		{
			std::array<gfx::Attachment, 8ull> color_attachments{};
			uint32_t color_attachment_count{};
		};

		// Base render graph impl

		enum class RenderGraphNodeType
		{
			eStaticCondition,
			eDynamicCondition,
			eGroupNode,
			ePassNode
		};

		// Base render graph node
		struct RenderGraphNodeHeader
		{
			RenderGraphNodeType type;
			uint32_t data_offset;
		};

		struct RenderGraphStaticConditionNode
		{
			RenderGraphNodeHeader* node{ nullptr };
		};

		struct RenderGraphDynamicConditionNode
		{
			std::function<bool()> condition_fn{};
			RenderGraphNodeHeader* pass{ nullptr };
			RenderGraphNodeHeader* fail{ nullptr };
		};

		struct RenderGraphGroupNode
		{
			std::string group_name{};
			uint32_t color{ 0xFFFFFFFF };
			std::vector<RenderGraphNodeHeader*> nodes{};
		};

		struct RenderGraphPassNode
		{
			uint32_t pass_index{ ~0u };
		};

		// Graph node create functions
		template<class _HeaderType>
		inline void RenderGraphNodeAllocateCombinedSpace(RenderGraphNodeType type, void** out, uint64_t size, uint64_t alignment)
		{
			uint64_t aligned_header_size = utl::aligned_size(sizeof(_HeaderType), alignment);
			uint64_t aligned_data_size = utl::aligned_size(size, alignment);
			(*out) = _aligned_malloc(aligned_header_size + aligned_data_size, alignment);

			_HeaderType* header = reinterpret_cast<_HeaderType*>(*out);
			header->type = type;
			header->data_offset = aligned_header_size;

			std::memset(reinterpret_cast<char*>(header) + header->data_offset, 0, aligned_data_size);
		}

		inline void DeallocateRenderGraphNode(RenderGraphNodeHeader** out)
		{
			if ((*out))
			{
				_aligned_free((*out));
				(*out) = nullptr;
			}
		}

		template<class _Ty>
		inline RenderGraphNodeHeader* AllocateRenderGraphNode(RenderGraphNodeType type)
		{
			RenderGraphNodeHeader* out{ nullptr };
			RenderGraphNodeAllocateCombinedSpace<RenderGraphNodeHeader>(type, (void**)&out, sizeof(_Ty), alignof(max_align_t));
			return out;
		}

		template<class _Ty>
		inline _Ty* CastRenderGraphNode(RenderGraphNodeHeader* header)
		{
			auto* data = reinterpret_cast<char*>(header) + header->data_offset;
			return reinterpret_cast<_Ty*>(data);
		}
	}
}