#include "gfx_frame_graph.h"

#include "gfx_context.h"

#include "../fs/fs_helper.h"
#include "../util/Logger.h"

#include "../fs/fs_virtual_filesystem.h"

namespace edge::gfx {
	auto IFGResource::create_resources(const FGResourceCreateInfo& texture_ci) -> bool {
		name = texture_ci.name;
		dependency_flags = texture_ci.dependency_flags;
		parent = texture_ci.parent_handle;

		if (texture_ci.external) {
			resource = texture_ci.external;
			imported;
			resource->set_resource_name(name);
		}

		return true;
	}

	class TextureResource final : public IFGResource {
	public:
		~TextureResource() override = default;

		static auto consturct(const FGTextureResourceCreateInfo& texture_ci) -> std::unique_ptr<IFGResource> {
			std::unique_ptr<TextureResource> self = std::make_unique<TextureResource>();
			self->last_ci = texture_ci.create_info;
			self->create_resources(texture_ci);
			return self;
		}

		auto evaluate(IFrameGraph& graph) -> bool override {
			TextureCreateInfo new_ci{ last_ci };
			uint32_t new_hash = last_hash;

			if (parent != ~0u) {
				auto& perent_resource = graph.get_resource(parent);
				auto* texture = perent_resource.get_resource()->as<ITexture*>();

				if (dependency_flags & FGResourceDependencyFlag::eUsage) {
					new_ci.usage = texture->get_usage();
				}

				if (dependency_flags & FGResourceDependencyFlag::eDimensions) {
					new_ci.width = texture->get_width() * relative_scale;
					new_ci.height = texture->get_height() * relative_scale;
					new_ci.depth = texture->get_depth();
				}

				if (dependency_flags & FGResourceDependencyFlag::eLayers) {
					new_ci.layers = texture->get_array_layer_count();
				}

				if (dependency_flags & FGResourceDependencyFlag::eLevels) {
					new_ci.levels = texture->get_mip_level_count();
				}

				if (dependency_flags & FGResourceDependencyFlag::eFormat) {
					new_ci.format = texture->get_format();
				}

				new_hash = gfx::hash(new_ci);
			}

			if ((last_hash != new_hash && !imported) || !resource) {
				if (resource) {
					destroy(resource);
				}

				last_hash = new_hash;
				last_ci = new_ci;

				resource = create_texture(new_ci);
				resource->set_resource_name(name);
				state = ResourceStateFlag::eUndefined;
			}

			for (FGResourceHandle child_handle : children) {
				auto& child = graph.get_resource(child_handle);
				child.evaluate(graph);
			}
		}
	private:
		TextureCreateInfo last_ci{};
		uint32_t last_hash{ ~0u };
		float relative_scale{ 1.0f };
	};

	class BufferResource final : public IFGResource {
	public:
		~BufferResource() override = default;

		static auto consturct(const FGBufferResourceCreateInfo& buffer_ci) -> std::unique_ptr<IFGResource> {
			std::unique_ptr<BufferResource> self = std::make_unique<BufferResource>();
			self->last_ci = buffer_ci.create_info;
			self->create_resources(buffer_ci);
			return self;
		}

		auto evaluate(IFrameGraph& graph) -> bool override {
			BufferCreateInfo new_ci{ last_ci };
			uint32_t new_hash = last_hash;

			if (parent != ~0u) {
				auto& perent_resource = graph.get_resource(parent);
				auto* buffer = perent_resource.get_resource()->as<IBuffer*>();

				if (dependency_flags & FGResourceDependencyFlag::eUsage) {
					new_ci.usage = buffer->get_usage_flags();
				}

				if (dependency_flags & FGResourceDependencyFlag::eDimensions) {
					new_ci.size = buffer->get_size();
				}
			}

			if ((last_hash != new_hash && !imported) || !resource) {
				if (resource) {
					destroy(resource);
				}

				last_hash = new_hash;
				last_ci = new_ci;

				resource = create_buffer(new_ci);
				resource->set_resource_name(name);
				state = ResourceStateFlag::eUndefined;
			}

			for (FGResourceHandle child_handle : children) {
				auto& child = graph.get_resource(child_handle);
				child.evaluate(graph);
			}

			return false;
		}

	private:
		BufferCreateInfo last_ci{};
		uint32_t last_hash{ ~0u };
	};

	class BasePass : public IFGPass {
	public:
		virtual ~BasePass() override = default;

		auto execute(ICommandList& cmd_list, float delta_time) -> void override {
			// TODO: apply barriers and bind resources
		}

		auto get_queue_type() const -> QueueType override { return QueueType::eDirect; }

		auto setup() -> void override {
			if (setup_func) {
				setup_func();
			}

			// TODO: generate barriers
		}

		auto teardown() -> void override {
			if (teardown_func) {
				teardown_func();
			}
		}
	protected:
		IShader* shader{ nullptr };
		IRootSignature* root_sign{ nullptr };
		IPipeline* pipeline{ nullptr };
		// Resource binding layouts
		edge::utl::fixed_vector<edge::gfx::IDescriptorLayout*> descriptor_layouts;

		// Callbacks
		std::function<void(ICommandList&, float)> execute_func;
		std::function<void()> setup_func;
		std::function<void()> teardown_func;

		// Resources
		utl::fixed_vector<IFGResource*, 32> input_resources;
		utl::fixed_vector<IFGResource*, 32> output_resources;
	};

	class RenderPass final : public BasePass {
	public:
		auto get_type() const -> FGPassType override { return FGPassType::eGraphics; }
	private:
	};

	class ComputePass final : public BasePass {
	public:
		auto get_type() const -> FGPassType override { return FGPassType::eCompute; }
	private:
	};

	class FrameGraph final : public IFrameGraph {
	public:
		~FrameGraph() override {
		}

		auto add_resource(const FGTextureResourceCreateInfo& create_info) -> FGResourceHandle override {
			FGResourceHandle resource_handle = static_cast<FGResourceHandle>(resources.size());
			resources.emplace_back(TextureResource::consturct(create_info));
			return resource_handle;
		}

		auto add_resource(const FGBufferResourceCreateInfo& create_info) -> FGResourceHandle override {
			FGResourceHandle resource_handle = static_cast<FGResourceHandle>(resources.size());
			resources.emplace_back(BufferResource::consturct(create_info));
			return resource_handle;
		}

		auto add_pass(const FGPassCreateInfo& create_info) -> FGResourceHandle override {
			return ~0u;
		}

		void update_resource(uint32_t handle, IGFXResource* resource) override {
			// TODO: 
		}

		void remove_resource(FGResourceHandle handle) override {

		}

		void remove_pass(FGPassHandle handle) override {

		}

		auto get_resource(uint32_t handle) -> IFGResource& override {
			return *resources[handle];
		}

		auto get_pass(FGPassHandle handle) -> IFGPass & override {

		}

		auto build() -> bool override {
			// Update resources
			for (auto& resource : resources) {
				if (!resource->has_parent()) {
					resource->evaluate(*this);
				}
			}

			return false;
		}

		auto execute(ICommandList& cmd_list, float delta_time) -> bool override {

			// Clean graph passes after execution

			return false;
		}
	private:
		std::vector<std::unique_ptr<IFGResource>> resources{};
	};

	auto create_frame_graph() -> IFrameGraph* {
		return new FrameGraph();
	}

	auto create_frame_graph_unique() -> std::unique_ptr<IFrameGraph> {
		return std::make_unique<FrameGraph>();
	}

}