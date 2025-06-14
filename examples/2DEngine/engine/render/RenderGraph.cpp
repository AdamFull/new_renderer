#include "RenderGraph.h"

namespace e2d {
    // PassDescriptor
    PassDescriptor::PassDescriptor(std::string_view name) : name_{ name } {

    }

	class RGResourcePool {
    public:
        RGResourcePool() = default;
        ~RGResourcePool() = default;

        // Non-copyable, movable
        RGResourcePool(const RGResourcePool&) = delete;
        RGResourcePool& operator=(const RGResourcePool&) = delete;
        RGResourcePool(RGResourcePool&&) = default;
        RGResourcePool& operator=(RGResourcePool&&) = default;

        // Resource creation
        auto create_buffer(const edge::gfx::BufferCreateInfo& create_info, std::string_view name = "") -> RGResourceHandle {
            RGResourceId id = allocate_resource_id();

            // Create resource
            RGResource resource;
            resource.id = id;
            resource.name = name;
            resource.type = edge::gfx::ResourceType::eBuffer;
            resource.size_bytes = create_info.size * create_info.count;
            resource.needs_allocation = true;

            // Create buffer resource
            RGBufferResource buffer(create_info);

            resources_[id] = std::move(resource);
            buffers_[id] = std::move(buffer);

            return RGResourceHandle(id);
        }

        auto create_texture(const edge::gfx::TextureCreateInfo& create_info, std::string_view name = "") -> RGResourceHandle {
            RGResourceId id = allocate_resource_id();

            // Create resource
            RGResource resource;
            resource.id = id;
            resource.name = name;
            resource.type = edge::gfx::ResourceType::eTexture;
            // TODO: Estimate texture size
            resource.size_bytes = static_cast<size_t>(create_info.width) * create_info.height * create_info.depth * 4;
            resource.needs_allocation = true;

            // Create texture resource
            RGTextureResource texture(create_info);

            resources_[id] = std::move(resource);
            textures_[id] = std::move(texture);

            return RGResourceHandle(id);
        }

        // External resource import
        auto import_buffer(edge::gfx::IBuffer* buffer, std::string name = "") -> RGResourceHandle {
            RGResourceId id = allocate_resource_id();

            // Create resource
            RGResource resource;
            resource.id = id;
            resource.name = std::move(name);
            resource.type = edge::gfx::ResourceType::eBuffer;
            resource.size_bytes = buffer->get_size();
            resource.needs_allocation = false;
            resource.is_external = true;

            // Create buffer resource
            RGBufferResource buffer_resource{};
            buffer_resource.buffer = buffer;

            resources_[id] = std::move(resource);
            buffers_[id] = std::move(buffer_resource);

            return RGResourceHandle(id);
        }

        auto import_texture(edge::gfx::ITexture* texture, std::string name = "") -> RGResourceHandle {
            RGResourceId id = allocate_resource_id();

            // Create resource
            RGResource resource;
            resource.id = id;
            resource.name = name;
            resource.type = edge::gfx::ResourceType::eTexture;
            // TODO: Estimate texture size
            resource.size_bytes = texture->get_width() * texture->get_height() * texture->get_depth() * 4;
            resource.needs_allocation = false;
            resource.is_external = true;

            // Create texture resource
            RGTextureResource texture_resource{};
            texture_resource.texture = texture;

            resources_[id] = std::move(resource);
            textures_[id] = std::move(texture_resource);

            return RGResourceHandle(id);
        }

        // Resource access
        inline auto get_resource(RGResourceId id) -> RGResource* {
            auto it = resources_.find(id);
            return (it != resources_.end()) ? &it->second : nullptr;
        }

        inline auto get_resource(RGResourceId id) const -> RGResource const* {
            auto it = resources_.find(id);
            return (it != resources_.end()) ? &it->second : nullptr;
        }

        inline auto get_buffer(RGResourceHandle handle) const -> edge::gfx::IBuffer* {
            auto it = buffers_.find(handle.id());
            return (it != buffers_.end()) ? it->second.buffer : nullptr;
        }

        inline auto get_texture(RGResourceHandle handle) const -> edge::gfx::ITexture* {
            auto it = textures_.find(handle.id());
            return (it != textures_.end()) ? it->second.texture : nullptr;
        }

        inline auto set_resource_state(RGResourceId id, edge::gfx::ResourceStateFlags state) -> void {
            if (auto* res = get_resource(id)) {
                res->current_state = state;
            }
        }

        inline auto get_resource_state(RGResourceId id) const -> edge::gfx::ResourceStateFlags {
            if (const auto* res = get_resource(id)) {
                return res->current_state;
            }
            return edge::gfx::ResourceStateFlag::eUndefined;
        }

        inline auto mark_resource_usage(RGResourceId id, uint32_t pass_index) -> void {
            if (auto* res = get_resource(id)) {
                res->first_use_pass = std::min(res->first_use_pass, pass_index);
                res->last_use_pass = std::max(res->last_use_pass, pass_index);
            }
        }

        // Memory management
        void allocate_resources() {

        }

        void deallocate_expired_resources(uint32_t current_pass) {

        }

        size_t get_total_memory_usage() const {

        }

    private:
        RGResourceId next_resource_id_ = 1;

        // ECS-style component storage
        std::unordered_map<RGResourceId, RGResource> resources_;
        std::unordered_map<RGResourceId, RGBufferResource> buffers_;
        std::unordered_map<RGResourceId, RGTextureResource> textures_;

        RGResourceId allocate_resource_id() { return next_resource_id_++; }
	};
}