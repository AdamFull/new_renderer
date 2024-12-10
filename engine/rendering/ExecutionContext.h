#pragma once

#include "gfx/gfx_base.h"

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
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(RenderResourceTypeFlags, RenderResourceTypeFlag);

		struct RenderResource
		{
			RenderResourceTypeFlag resource_type{};
			// Id of real resource
			uint32_t resource_id{ ~0u };

			gfx::ResourceStateFlags current_state{};

			bool finalized{ false };

			//std::unordered_set<ResourceAccess> usages{};

			gfx::AccessTypeFlags GetAccess() const
			{
				if (utl::eq_(current_state, gfx::ResourceStateFlag::eRead))
					return gfx::AccessTypeFlag::eRead;
				else if (utl::eq_(current_state, gfx::ResourceStateFlag::eWrite))
					return gfx::AccessTypeFlag::eWrite;
				else if (utl::eq_(current_state, gfx::ResourceStateFlag::eRead | gfx::ResourceStateFlag::eWrite))
					return gfx::AccessTypeFlag::eRead | gfx::AccessTypeFlag::eWrite;

				return {};
			}

			gfx::ResourceStateFlags GetState() const
			{
				return current_state;
			}

			void SetState(gfx::ResourceStateFlags new_state)
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

			gfx::PipelineState pipeline_state{};

			void AddWriteAttachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}, gfx::ClearColor clear_color = {}) { resource_attachments.emplace_back(gfx::AccessTypeFlag::eWrite, flags, render_resource_id, clear_color); }
			void AddReadAttachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}) { resource_attachments.emplace_back(gfx::AccessTypeFlag::eRead, flags, render_resource_id, gfx::ClearColor{}); }
		};

		struct TextureResourceCreateInfo
		{
			gfx::TextureUsageFlags usage{};

			uint32_t width{ 1u };
			uint32_t height{ 1u };
			uint32_t depth{ 1u };

			uint32_t layers{ 1u };
			uint32_t levels{ 1u };

			bool resizable{ false };
			float scale{ 1.f };
			uint32_t scale_parent_resource{ ~0u };

			gfx::Format format{ gfx::Format::eUndefined };
		};

		struct TextureObject
		{
			gfx::Texture* texture{ nullptr };
			gfx::Sampler* sampler{ nullptr };

			gfx::TextureUsageFlags usage{};
			bool resizable{ false };
			float scale{ 1.f };
			uint32_t scale_parent_resource{ ~0u };
		};

		struct BufferObject
		{
			gfx::Buffer* buffer{ nullptr };
		};

		class ExecutionNode
		{
		public:
			virtual ~ExecutionNode() = default;

			virtual void Execute(gfx::CommandList& command_list, float delta_time) = 0;
		};

		class ExecutionContext
		{
		public:
			virtual ~ExecutionContext() = default;

			virtual uint32_t AddResource() = 0;
			virtual uint32_t AddResource(std::string_view path, gfx::Sampler* sampler = nullptr) = 0;
			virtual uint32_t AddResource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data) = 0;
			virtual uint32_t AddResource(const TextureResourceCreateInfo& create_info, gfx::Sampler* sampler = nullptr) = 0;
			virtual uint32_t AddResource(const gfx::BufferCreateInfo& create_info) = 0;

			virtual void SetResourceName(uint32_t resource_index, std::string_view name) = 0;

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

			virtual gfx::Context& GetContext() = 0;
			virtual gfx::Context const& GetContext() const = 0;

			virtual gfx::Texture* GetTexture(uint32_t resource_index) = 0;
			virtual gfx::Texture* GetTexture(const RenderResource& render_resource) = 0;

			virtual gfx::Sampler* GetSampler(uint32_t resource_index) = 0;
			virtual gfx::Sampler* GetSampler(const RenderResource& render_resource) = 0;

			virtual gfx::Buffer* GetBuffer(uint32_t resource_index) = 0;
			virtual gfx::Buffer* GetBuffer(const RenderResource& render_resource) = 0;

			virtual gfx::BufferSuballocation RequestUniformBlock(uint64_t block_size) = 0;
			virtual gfx::CommandList& GetCurrentCommandList() = 0;
			virtual gfx::DescriptorAllocator& GetCurrentDescriptorAllocator() = 0;
		};
	}
}