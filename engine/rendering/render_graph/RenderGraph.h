#pragma once

#include <engine/rendering/render_graph/RenderGraphInterface.h>
#include <engine/rendering/RenderResourceContainer.h>
#include <engine/rendering/UniformPool.h>

namespace engine
{
	namespace gfx
	{
		class ResourceLoader;
	}

	namespace render
	{
		class IRenderer;
		class RenderContext;
		class AsyncContext;

		class GroupNode final : public IGraphNode
		{
		public:
			GroupNode(const std::string& name, uint32_t color = 0xFFFFFFFF);
			~GroupNode() = default;

			void AddNode(std::unique_ptr<IGraphNode>&& node);

			void SetGroupEnabledCallback(std::function<bool()>&& callback);

			void Evaluate(gfx::CommandList& command_list, float delta_time) override;
		private:
			std::string group_name{};
			uint32_t group_color{ 0xFFFFFFFF };

			std::function<bool()> enabled_callback{};
			std::vector<std::unique_ptr<IGraphNode>> graph_nodes{};
		};

		class RenderGraph final : public IRenderGraph
		{
		public:
			RenderGraph(IRenderer& renderer, Window* window);
			~RenderGraph();

			uint32_t AddResource() override;
			uint32_t AddResource(const std::string& path, gfx::Sampler* sampler = nullptr) override;
			uint32_t AddResource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data) override;
			uint32_t AddResource(const TextureResourceCreateInfo& create_info, gfx::Sampler* sampler = nullptr) override;
			uint32_t AddResource(const gfx::BufferCreateInfo& create_info) override;

			void SetResourceName(uint32_t resource_index, const std::string& name) override;

			RenderGraphResource& GetResource(uint32_t resource_index) override;
			RenderGraphResource const& GetResource(uint32_t resource_index) const override;

			std::unique_ptr<IGraphNode> CreateNode(const std::string& group_name, uint32_t group_color = 0xFFFFFFFF) override;
			std::unique_ptr<IGraphNode> CreateNode(const RenderShaderPassCreateInfo& create_info) override;
			std::unique_ptr<IGraphNode> CreateNode(const ComputeShaderPassCreateInfo& create_info) override;

			void AddNode(std::unique_ptr<IGraphNode>&& node) override;
			void Evaluate(float delta_time) override;

			void UpdateGlobalBinding(uint32_t binding, std::span<const uint8_t> data);

			void BeginFrame() override;
			void EndFrame() override;

			gfx::Context& GetContext() override;
			gfx::Context const& GetContext() const override;

			gfx::Texture* GetTexture(uint32_t resource_index) override;
			gfx::Texture* GetTexture(const RenderGraphResource& render_resource) override;

			gfx::Sampler* GetSampler(uint32_t resource_index) override;
			gfx::Sampler* GetSampler(const RenderGraphResource& render_resource) override;

			gfx::Buffer* GetBuffer(uint32_t resource_index) override;
			gfx::Buffer* GetBuffer(const RenderGraphResource& render_resource) override;
		private:
			void UpdateRenderTargets();
		private:
			IRenderer& renderer;
			gfx::Context& context;
			gfx::ResourceLoader& resource_loader;

			std::unique_ptr<RenderContext> render_context{};

			std::vector<std::unique_ptr<IGraphNode>> graph_nodes{};
			std::vector<RenderGraphResource> render_resources{};

			gfx::Sampler* default_sampler{ nullptr };

			// Performance counters
			gfx::QueryPool* timestamp_query{ nullptr };
			float timestamp_frequency{ 0.f };
			float gpu_delta_time{ 0.f };

			// Shared descriptors
			gfx::SharedDescriptor* shared_descriptor{ nullptr };
			gfx::ResourceDescriptor* global_resource_descriptor{ nullptr };
			std::unique_ptr<UniformPool> global_uniform_pool{};

			ResourceContainer<TextureObject> static_textures{};
			ResourceContainer<TextureObject> render_targets{};
			ResourceContainer<BufferObject> buffers{};
		};
	}
}