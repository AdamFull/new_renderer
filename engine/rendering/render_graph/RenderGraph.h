#pragma once

#include <engine/rendering/gfx/Structures.h>
#include <memory.hpp>

struct std_pair_hash
{
	template <class _T1, class _T2>
	std::size_t operator()(const std::pair<_T1, _T2>& pair) const
	{
		std::size_t hash1 = std::hash<_T1>{}(pair.first);
		std::size_t hash2 = std::hash<_T2>{}(pair.second);
		return hash1 ^ hash2;
	}
};

namespace engine
{
	namespace render
	{
		class ShaderPass;

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

			gfx::ResourceStateFlags current_state{ gfx::ResourceStateFlag::eUndefined };
			std::array<gfx::ResourceStateFlags, 16ull> subresource_states{ gfx::ResourceStateFlag::eUndefined };

			gfx::ResourceStateFlags intermediate_state{ gfx::ResourceStateFlag::eUndefined };
			gfx::ResourceStateFlags final_state{ gfx::ResourceStateFlag::eUndefined };

			std::array<std::unordered_set<uint32_t>, 3ull> usages{};

			gfx::ResourceStateFlags GetState(uint32_t subresource_index = ~0u)
			{
				if ((resource_type & RenderResourceTypeFlag::eBuffer) || subresource_index == ~0u)
				{
					return current_state;
				}
				else
				{
					return subresource_states[subresource_index];
				}
			}

			void SetState(gfx::ResourceStateFlags new_state, uint32_t subresource_index = ~0u)
			{
				if (resource_type & RenderResourceTypeFlag::eBuffer)
				{
					current_state = new_state;
				}
				else
				{
					if (subresource_index == ~0u)
					{
						current_state = new_state;
						for (auto& state : subresource_states)
						{
							state = new_state;
						}
					}
					else
					{
						subresource_states[subresource_index] = new_state;
					}
				}
			}
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

		enum class ResourceAttachmentTypeFlag
		{
			eColor = 1 << 0,
			eDepth = 1 << 1,
			eStencil = 1 << 2,
			eDepthStencil = eDepth | eStencil
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ResourceAttachmentTypeFlags, ResourceAttachmentTypeFlag);

		// Describes out attachment like color or depth/stencil attachment
		struct RenderResourceAttachment
		{
			ResourceAttachmentTypeFlags flags{};

			// Index of render resource
			uint32_t render_resource_id{ ~0u };

			// Render resource clear color
			gfx::ClearColor clear_color{};
		};

		struct ResourceBindings
		{
			std::array<std::vector<RenderResourceBinding>, 3ull> bindings{};
			std::array<std::vector<RenderResourceAttachment>, 3ull> attachments{};
		};

		struct ShaderPassCreateInfo
		{
			// Shader name in shader cache
			std::string shader_name{};

			std::function<void(ShaderPass&, gfx::CommandList&, float)> draw_callback{};

			// Means that this pass will be executed only one time
			bool one_time{ false };

			// Some explanation 3 is means 3 variant of access: read(0), write(1), both(2).
			std::array<std::vector<RenderResourceBinding>, 3ull> resource_bindings{};

			// This one is for attachments
			std::array<std::vector<RenderResourceAttachment>, 3ull> resource_attachments{};

			void BindWriteResource(uint32_t binding_point, uint32_t render_resource_id) { resource_bindings[utl::enum_cast(RenderGraphResourceAccess::eWrite)].emplace_back(binding_point, render_resource_id, ~0u); }
			void BindWriteResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource) { resource_bindings[utl::enum_cast(RenderGraphResourceAccess::eWrite)].emplace_back(binding_point, render_resource_id, subresource); }
			
			void BindReadResource(uint32_t binding_point, uint32_t render_resource_id) { resource_bindings[utl::enum_cast(RenderGraphResourceAccess::eRead)].emplace_back(binding_point, render_resource_id, ~0u); }
			void BindReadResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource) { resource_bindings[utl::enum_cast(RenderGraphResourceAccess::eRead)].emplace_back(binding_point, render_resource_id, subresource); }
		};

		struct ComputeShaderPassCreateInfo : ShaderPassCreateInfo
		{
			
		};

		struct RenderShaderPassCreateInfo : ShaderPassCreateInfo
		{
			void AddWriteAttachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}, gfx::ClearColor clear_color = {}) { resource_attachments[utl::enum_cast(RenderGraphResourceAccess::eWrite)].emplace_back(flags, render_resource_id, clear_color); }
			void AddReadAttachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}) { resource_attachments[utl::enum_cast(RenderGraphResourceAccess::eRead)].emplace_back(flags, render_resource_id, gfx::ClearColor{}); }
		};

		struct ConsistentAccessInfo
		{
			gfx::AccessTypeFlags access{};
			uint32_t pass_type{ 0u };
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