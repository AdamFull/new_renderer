#pragma once

#include <edge/gfx/gfx_base.h>

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
			eStorageBuffer = 1 << 4,
			eIndirectBuffer = 1 << 5,
			eAllBuffer = eStorageBuffer | eIndirectBuffer
		};
		EDGE_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(RenderResourceTypeFlags, RenderResourceTypeFlag);

		struct RenderResource
		{
			RenderResourceTypeFlag resource_type{};
			// Id of real resource
			uint32_t resource_id{ ~0u };

			edge::gfx::ResourceStateFlags current_state{};

			bool finalized{ false };

			//std::unordered_set<ResourceAccess> usages{};

			edge::gfx::AccessTypeFlags GetAccess() const
			{
				if (edge::eq_(current_state, edge::gfx::ResourceStateFlag::eRead))
					return edge::gfx::AccessTypeFlag::eRead;
				else if (edge::eq_(current_state, edge::gfx::ResourceStateFlag::eWrite))
					return edge::gfx::AccessTypeFlag::eWrite;
				else if (edge::eq_(current_state, edge::gfx::ResourceStateFlag::eRead | edge::gfx::ResourceStateFlag::eWrite))
					return edge::gfx::AccessTypeFlag::eRead | edge::gfx::AccessTypeFlag::eWrite;

				return {};
			}

			edge::gfx::ResourceStateFlags GetState() const
			{
				return current_state;
			}

			void SetState(edge::gfx::ResourceStateFlags new_state)
			{
				current_state = new_state;
			}
		};

		enum class ResourceAttachmentTypeFlag
		{
			eColor = 1 << 0,
			eDepth = 1 << 1,
			eStencil = 1 << 2,
			eDepthStencil = eDepth | eStencil
		};
		EDGE_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ResourceAttachmentTypeFlags, ResourceAttachmentTypeFlag);

		// Describes out attachment like color or depth/stencil attachment
		struct RenderResourceAttachment
		{
			edge::gfx::AccessTypeFlags access{};

			ResourceAttachmentTypeFlags flags{};

			// Index of render resource
			uint32_t render_resource_id{ ~0u };

			// Render resource clear color
			edge::gfx::ClearColor clear_color{};
		};

		struct ShaderPassCreateInfo
		{
			// Shader name in shader cache
			std::string_view technique_name{};

			std::function<void(ShaderPass&, edge::gfx::ICommandList&, float)> draw_callback{};
			std::function<bool()> enabled_callback{};

			// Means that this pass will be executed only one time
			bool one_time{ false };
		};

		struct ComputeShaderPassCreateInfo : ShaderPassCreateInfo
		{

		};

		struct RenderShaderPassCreateInfo : ShaderPassCreateInfo
		{
			std::vector<RenderResourceAttachment> resource_attachments{};

			void AddWriteAttachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}, edge::gfx::ClearColor clear_color = {}) { resource_attachments.emplace_back(edge::gfx::AccessTypeFlag::eWrite, flags, render_resource_id, clear_color); }
			void AddReadAttachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}) { resource_attachments.emplace_back(edge::gfx::AccessTypeFlag::eRead, flags, render_resource_id, edge::gfx::ClearColor{}); }
		};

		struct TextureResourceCreateInfo
		{
			edge::gfx::TextureUsageFlags usage{};

			uint32_t width{ 1u };
			uint32_t height{ 1u };
			uint32_t depth{ 1u };

			uint32_t layers{ 1u };
			uint32_t levels{ 1u };

			bool resizable{ false };
			float scale{ 1.f };
			uint32_t scale_parent_resource{ ~0u };

			edge::gfx::Format format{ edge::gfx::Format::eUndefined };
		};

		struct TextureObject
		{
			edge::gfx::ITexture* texture{ nullptr };
			edge::gfx::ISampler* sampler{ nullptr };

			edge::gfx::TextureUsageFlags usage{};
			bool resizable{ false };
			float scale{ 1.f };
			uint32_t scale_parent_resource{ ~0u };
		};

		struct BufferObject
		{
			edge::gfx::IBuffer* buffer{ nullptr };
		};

		class ExecutionNode
		{
		public:
			virtual ~ExecutionNode() = default;

			virtual void Execute(edge::gfx::ICommandList& command_list, float delta_time) = 0;
		};

		class ExecutionContext
		{
		public:
			virtual ~ExecutionContext() = default;

			virtual uint32_t AddResource() = 0;
			virtual uint32_t AddResourceSync(std::string_view path, edge::gfx::ISampler* sampler = nullptr) = 0;
			virtual uint32_t AddResource(std::string_view path, edge::gfx::ISampler* sampler = nullptr) = 0;
			virtual uint32_t AddResource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data) = 0;
			virtual uint32_t AddResource(const TextureResourceCreateInfo& create_info, edge::gfx::ISampler* sampler = nullptr) = 0;
			virtual uint32_t AddResource(const edge::gfx::BufferCreateInfo& create_info) = 0;

			virtual void set_resource_name(uint32_t resource_index, std::string_view name) = 0;

			virtual RenderResource& GetResource(uint32_t resource_index) = 0;
			virtual RenderResource const& GetResource(uint32_t resource_index) const = 0;

			virtual std::unique_ptr<ExecutionNode> CreateNode(std::string_view group_name, uint32_t group_color = 0xFFFFFFFF) = 0;
			virtual std::unique_ptr<ExecutionNode> CreateNode(const RenderShaderPassCreateInfo& create_info) = 0;
			virtual std::unique_ptr<ExecutionNode> CreateNode(const ComputeShaderPassCreateInfo& create_info) = 0;

			virtual void AddNode(std::unique_ptr<ExecutionNode>&& node) = 0;
			virtual void Execute(float delta_time) = 0;

			virtual void UpdateGlobalBinding(uint32_t binding, std::span<const uint8_t> data) = 0;

			virtual void Begin() = 0;
			virtual void End() = 0;

			virtual edge::gfx::ITexture* GetTexture(uint32_t resource_index) = 0;
			virtual edge::gfx::ITexture* GetTexture(const RenderResource& render_resource) = 0;

			virtual edge::gfx::ISampler* GetSampler(uint32_t resource_index) = 0;
			virtual edge::gfx::ISampler* GetSampler(const RenderResource& render_resource) = 0;

			virtual edge::gfx::IBuffer* GetBuffer(uint32_t resource_index) = 0;
			virtual edge::gfx::IBuffer* GetBuffer(const RenderResource& render_resource) = 0;

			virtual edge::gfx::BufferSuballocation RequestUniformBlock(uint64_t block_size) = 0;
			virtual edge::gfx::ICommandList& GetCurrentCommandList() = 0;
			virtual edge::gfx::IDescriptorAllocator& GetCurrentDescriptorAllocator() = 0;
		};
	}
}