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

		using RenderGraphPassResourceAccess = std::array<std::vector<uint32_t>, 3ull>;

		struct RenderGraphResource
		{
			RenderResourceTypeFlag resource_type{};
			// Id of real resource
			uint32_t resource_id{ ~0u };

			std::array<std::unordered_set<uint32_t>, 3ull> usages{};
		};

		// Describes resource binding. For example binding of texture with concrete mip level, or writable/readable buffer.
		struct RenderResourceBinding
		{
			// In this case it's resource name hash from reflection
			uint32_t bind_point{};

			// Index of render resource
			uint32_t render_resource_id{ ~0u };

			// Subresource binding if needed
			uint32_t subresource_index{ ~0u };
		};

		// Describes out attachment like color or depth/stencil attachment
		struct RenderResourceAttachment
		{
			// Index of render resource
			uint32_t render_resource_id{ ~0u };

			// Render resource clear color
			gfx::ClearColor clear_color{};
		};

		struct ResourceBindings
		{
			std::array<std::vector<RenderResourceBinding>, 3ull> bindings{};
			std::array<std::vector<RenderResourceAttachment>, 3ull> attachments{};

			RenderResourceAttachment depth_attachment{};
			RenderResourceAttachment stencil_attachment{};
		};

		struct ShaderPassCreateInfo
		{
			std::string shader_name{};

			// Some explanation 3 is means 3 variant of access: read(0), write(1), both(2).
			std::array<std::vector<RenderResourceBinding>, 3ull> resource_bindings{};

			// This one is for attachments
			std::array<std::vector<RenderResourceAttachment>, 3ull> resource_attachments{};

			void BindWriteResource(uint32_t binding_point, uint32_t render_resource_id) { resource_bindings[utl::enum_cast(RenderGraphResourceAccess::eWrite)].emplace_back(binding_point, render_resource_id, ~0u); }
			void BindWriteResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource) { resource_bindings[utl::enum_cast(RenderGraphResourceAccess::eWrite)].emplace_back(binding_point, render_resource_id, subresource); }
			
			void BindReadResource(uint32_t binding_point, uint32_t render_resource_id) { resource_bindings[utl::enum_cast(RenderGraphResourceAccess::eRead)].emplace_back(binding_point, render_resource_id, ~0u); }
			void BindReadResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource) { resource_bindings[utl::enum_cast(RenderGraphResourceAccess::eRead)].emplace_back(binding_point, render_resource_id, subresource); }

			void AddWriteResource(uint32_t render_resource_id, std::array<float, 4ull> clear_color = {}) { resource_attachments[utl::enum_cast(RenderGraphResourceAccess::eWrite)].emplace_back(render_resource_id, gfx::ClearColor(std::move(clear_color))); }
			void AddReadResource(uint32_t render_resource_id) { resource_attachments[utl::enum_cast(RenderGraphResourceAccess::eRead)].emplace_back(render_resource_id, gfx::ClearColor{}); }
		};


		struct ComputeShaderPassCreateInfo : ShaderPassCreateInfo
		{
			
		};

		struct RenderShaderPassCreateInfo : ShaderPassCreateInfo
		{
			RenderResourceAttachment depth_attachment{};
			RenderResourceAttachment stencil_attachment{};

			void SetDepthAttahment(uint32_t render_resource_id, float depth) { depth_attachment = { render_resource_id, gfx::ClearColor(depth) }; }
			void SetDepthAttahment(uint32_t render_resource_id, uint32_t stencil) { stencil_attachment = { render_resource_id, gfx::ClearColor(stencil) }; }
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