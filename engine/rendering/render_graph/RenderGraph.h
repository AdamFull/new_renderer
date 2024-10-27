#pragma once

#include <engine/rendering/gfx/Structures.h>
#include <memory.hpp>

namespace engine
{
	namespace render
	{
		struct ComputeShaderPassCreateInfo
		{
			std::string shader_name{};
		};

		struct RenderShaderPassCreateInfo
		{
			std::string shader_name{};

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