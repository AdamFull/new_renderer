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
	namespace gfx
	{
		class Context;
	}

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

		struct ResourceAccess
		{
			gfx::AccessTypeFlags access_flags{};
			uint32_t pass_index{};
		};

		struct RenderGraphResourceState
		{
			gfx::ResourceStateFlags flags{ gfx::ResourceStateFlag::eUndefined };
			gfx::AccessTypeFlags access{ 0 };

			void Reset()
			{
				flags = gfx::ResourceStateFlag::eUndefined;
				access = 0;
			}
		};

		struct RenderGraphResource
		{
			RenderResourceTypeFlag resource_type{};
			// Id of real resource
			uint32_t resource_id{ ~0u };

			RenderGraphResourceState current_state{};
			std::array<RenderGraphResourceState, 16ull> subresource_states{};

			gfx::ResourceStateFlags intermediate_state{ gfx::ResourceStateFlag::eUndefined };

			gfx::ResourceStateFlags final_state_comp{ gfx::ResourceStateFlag::eUndefined };
			gfx::ResourceStateFlags final_state_other{ gfx::ResourceStateFlag::eUndefined };

			//std::unordered_set<ResourceAccess> usages{};

			gfx::AccessTypeFlags GetAccess(uint32_t subresource_index = ~0u) const
			{
				if (subresource_index == ~0u)
				{
					return current_state.access;
				}

				return subresource_states[subresource_index].access;
			}

			void SetAccess(gfx::AccessTypeFlags access, uint32_t subresource_index = ~0u)
			{
				if (subresource_index == ~0u)
				{
					current_state.access = access;

					for (auto& subresource : subresource_states)
					{
						subresource.access = access;
					}
				}
				else
				{
					subresource_states[subresource_index].access = access;
				}
			}

			gfx::ResourceStateFlags GetState(uint32_t subresource_index = ~0u) const
			{
				if ((resource_type & RenderResourceTypeFlag::eBuffer) || subresource_index == ~0u)
				{
					return current_state.flags;
				}
				else
				{
					return subresource_states[subresource_index].flags;
				}
			}

			void SetState(gfx::ResourceStateFlags new_state, uint32_t subresource_index = ~0u)
			{
				if (resource_type & RenderResourceTypeFlag::eBuffer)
				{
					current_state.flags = new_state;
				}
				else
				{
					if (subresource_index == ~0u)
					{
						current_state.flags = new_state;
						for (auto& state : subresource_states)
						{
							state.flags = new_state;
						}
					}
					else
					{
						subresource_states[subresource_index].flags = new_state;
					}
				}
			}
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
			gfx::AccessTypeFlags access{};

			ResourceAttachmentTypeFlags flags{};

			// Index of render resource
			uint32_t render_resource_id{ ~0u };

			// Render resource clear color
			gfx::ClearColor clear_color{};
		};

		struct ShaderPassCreateInfo
		{
			// Shader name in shader cache
			std::string shader_name{};

			std::function<void(ShaderPass&, gfx::CommandList&, float)> draw_callback{};

			// Means that this pass will be executed only one time
			bool one_time{ false };
		};

		struct ComputeShaderPassCreateInfo : ShaderPassCreateInfo
		{

		};

		struct RenderShaderPassCreateInfo : ShaderPassCreateInfo
		{
			std::vector<RenderResourceAttachment> resource_attachments{};

			void AddWriteAttachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}, gfx::ClearColor clear_color = {}) { resource_attachments.emplace_back(gfx::AccessTypeFlag::eWrite, flags, render_resource_id, clear_color); }
			void AddReadAttachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}) { resource_attachments.emplace_back(gfx::AccessTypeFlag::eRead, flags, render_resource_id, gfx::ClearColor{}); }
		};

		struct TextureObject
		{
			gfx::Texture* texture{ nullptr };
			gfx::Sampler* sampler{ nullptr };

			gfx::TextureUsageFlags usage{};
			bool resizable{ false };
		};

		struct BufferObject
		{
			gfx::Buffer* buffer{ nullptr };
		};

		class IGraphNode
		{
		public:
			virtual ~IGraphNode() = default;

			virtual void Evaluate(gfx::CommandList& command_list, float delta_time) = 0;
		};

		class IRenderGraph
		{
		public:
			virtual ~IRenderGraph() = default;

			virtual uint32_t AddResource() = 0;
			virtual uint32_t AddResource(const std::string& path, gfx::Sampler* sampler = nullptr) = 0;
			virtual uint32_t AddResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr) = 0;
			virtual uint32_t AddResource(const gfx::BufferCreateInfo& create_info) = 0;

			virtual RenderGraphResource& GetResource(uint32_t resource_index) = 0;
			virtual RenderGraphResource const& GetResource(uint32_t resource_index) const = 0;

			virtual std::unique_ptr<IGraphNode> CreateNode(const std::string& group_name, uint32_t group_color = 0xFFFFFFFF) = 0;
			virtual std::unique_ptr<IGraphNode> CreateNode(const RenderShaderPassCreateInfo& create_info) = 0;
			virtual std::unique_ptr<IGraphNode> CreateNode(const ComputeShaderPassCreateInfo& create_info) = 0;

			virtual void AddNode(std::unique_ptr<IGraphNode>&& node) = 0;
			virtual void Evaluate(float delta_time) = 0;

			virtual void BeginFrame() = 0;
			virtual void EndFrame() = 0;

			virtual gfx::Context& GetContext() = 0;
			virtual gfx::Context const& GetContext() const = 0;

			virtual gfx::Texture* GetTexture(uint32_t resource_index) = 0;
			virtual gfx::Texture* GetTexture(const RenderGraphResource& render_resource) = 0;

			virtual gfx::Sampler* GetSampler(uint32_t resource_index) = 0;
			virtual gfx::Sampler* GetSampler(const RenderGraphResource& render_resource) = 0;

			virtual gfx::Buffer* GetBuffer(uint32_t resource_index) = 0;
			virtual gfx::Buffer* GetBuffer(const RenderGraphResource& render_resource) = 0;
		};
	}
}