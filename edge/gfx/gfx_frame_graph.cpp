#include "gfx_frame_graph.h"

#include "gfx_context.h"

#include "../fs/fs_helper.h"
#include "../util/Logger.h"

#include "../fs/fs_virtual_filesystem.h"

namespace edge::gfx {
	class TextureResource final : public IFGResource {
	public:
		~TextureResource() override = default;

		static auto consturct(const TextureCreateInfo& texture_ci) -> std::unique_ptr<IFGResource> {
			std::unique_ptr<TextureResource> self{};

			return self;
		}

		auto evaluate(IFGResource* parent) -> bool override {
			TextureCreateInfo new_ci{ last_ci };
			uint32_t new_hash = last_hash;

			if (parent) {
				auto* parent_resource = parent->get_resource();
				auto* texture = parent_resource->as<ITexture*>();

				if (dependency_flags & FGResourceDependencyFlag::eUsage) {
					//new_ci.usage = texture->get_usage();
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

			if ((last_hash != new_hash && !external) || !resource) {
				if (resource) {
					destroy(resource);
				}

				last_hash = new_hash;
				last_ci = new_ci;

				resource = create_texture(new_ci);
			}

			for (auto* child : children) {
				child->evaluate(this);
			}
		}
	private:
		TextureCreateInfo last_ci{};
		uint32_t last_hash{ ~0u };
		FGResourceDependencyFlags dependency_flags{};
		float relative_scale{ 1.0f };
	};

	class BufferResource final : public IFGResource {
	public:
		~BufferResource() override = default;

	private:
		BufferCreateInfo create_info{};

	};

	class FrameGraph final : public IFrameGraph {
	public:
		~FrameGraph() override {
		}

		auto add_resource(const FGTextureResourceCreateInfo& create_info) -> uint32_t override {
			auto handle_id = static_cast<uint32_t>(resources.size());
			// TODO:
			return handle_id;
		}

		auto add_resource(const FGBufferResourceCreateInfo& create_info) -> uint32_t override {
			auto handle_id = static_cast<uint32_t>(resources.size());
			// TODO:
			return handle_id;
		}

		void update_resource(uint32_t handle, IGFXResource* resource) override {
			// TODO: 
		}

		auto get_resource(uint32_t handle) -> IFGResource& override {
			return *resources[handle];
		}

		auto build() -> bool override {
			// Update resources
			for (auto& resource : resources) {
				// TODO: evaluate resources without parent
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