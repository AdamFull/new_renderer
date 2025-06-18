#include "RenderGraph.h"

#include <edge/util/Logger.h>
#include <edge/gfx/gfx_context.h>
#include <edge/gfx/technique_system/gfx_technique_system.h>

#include <edge/util/utl_profiler.h>
#include <edge/gfx/gfx_profiler.h>

namespace e2d {
    class BasePass : public IRGPass {
    public:
        BasePass(IRenderGraph& graph) : 
            graph_{ graph } {

        }

        ~BasePass() override = default;

        static auto construct(IRenderGraph& graph, RGPassSetupFn&& setup_fn, RGPassExecuteFn&& exec_fn, edge::gfx::TechniqueDatabase::Technique& technique) -> std::unique_ptr<IRGPass> {
            auto self = std::make_unique<BasePass>(graph);

            self->setup_func_ = std::move(setup_fn);
            self->execute_func_ = std::move(exec_fn);

            self->name_ = technique.name;
            self->shader_ = technique.shader;
            self->pipeline_ = technique.pipeline;
            self->root_sign_ = technique.root_signature;
            self->descriptor_layouts_ = technique.descriptor_layouts;

            return self;
        }

        auto add_srv(RGResourceHandle handle) -> void override {
            add_resource_barrier(handle, edge::gfx::ResourceStateFlag::eShaderResource);
        }

        auto add_uav(RGResourceHandle handle, uint32_t subresource_index) -> void override {
            edge::gfx::ResourceStateFlags stage{ edge::gfx::ResourceStateFlag::eUnorderedAccess };

            if (pipeline_) {
                auto pipeline_type = pipeline_->get_type();
                if (pipeline_type == edge::gfx::PipelineType::eCompute) {
                    stage |= edge::gfx::ResourceStateFlag::eNonGraphicsShader;
                }
                else if (pipeline_type == edge::gfx::PipelineType::eGraphics) {
                    stage |= edge::gfx::ResourceStateFlag::eGraphicsShader;
                }
            }

            add_resource_barrier(handle, stage, subresource_index);
        }

        auto add_vertex_buffer(RGResourceHandle handle) -> void override {
            add_resource_barrier(handle, edge::gfx::ResourceStateFlag::eVertexRead);
        }

        auto add_index_buffer(RGResourceHandle handle) -> void override {
            add_resource_barrier(handle, edge::gfx::ResourceStateFlag::eIndexRead);
        }

        auto add_indirect_buffer(RGResourceHandle handle) -> void override {
            add_resource_barrier(handle, edge::gfx::ResourceStateFlag::eIndirectArgument);
        }

        auto add_color_attachment(RGResourceHandle handle, edge::gfx::LoadOp load_op, edge::gfx::ClearColor clear_color) -> void override {
            add_resource_barrier(handle, edge::gfx::ResourceStateFlag::eRenderTarget);

            auto* graph_resource = graph_.get_resource(handle);
            auto* rhi_resource = graph_resource->get_resource();

            auto& new_attachnemt = rendering_info.attachments.emplace_back();
            new_attachnemt.texture = rhi_resource->as<edge::gfx::ITexture*>();
            new_attachnemt.texture_state = graph_resource->get_state();
            new_attachnemt.load_op = load_op;
            new_attachnemt.store_op = edge::gfx::StoreOp::eNone;
            new_attachnemt.clear_color = clear_color;
        }

        auto add_depth_attachment(RGResourceHandle handle, edge::gfx::LoadOp load_op, edge::gfx::ClearColor clear_color) -> void override {
            add_resource_barrier(handle, edge::gfx::ResourceStateFlag::eDepthWrite | edge::gfx::ResourceStateFlag::eDepthRead);

            auto* graph_resource = graph_.get_resource(handle);
            auto* rhi_resource = graph_resource->get_resource();

            rendering_info.depth_attachment.texture = rhi_resource->as<edge::gfx::ITexture*>();
            rendering_info.depth_attachment.texture_state = graph_resource->get_state();
            rendering_info.depth_attachment.load_op = load_op;
            rendering_info.depth_attachment.store_op = edge::gfx::StoreOp::eNone;
            rendering_info.depth_attachment.clear_color = clear_color;
        }

        auto add_stencil_attachment(RGResourceHandle handle, edge::gfx::LoadOp load_op, edge::gfx::ClearColor clear_color) -> void override {
            add_resource_barrier(handle, edge::gfx::ResourceStateFlag::eStencilWrite | edge::gfx::ResourceStateFlag::eStencilRead);

            auto* graph_resource = graph_.get_resource(handle);
            auto* rhi_resource = graph_resource->get_resource();

            rendering_info.stencil_attachment.texture = rhi_resource->as<edge::gfx::ITexture*>();
            rendering_info.stencil_attachment.texture_state = graph_resource->get_state();
            rendering_info.stencil_attachment.load_op = load_op;
            rendering_info.stencil_attachment.store_op = edge::gfx::StoreOp::eNone;
            rendering_info.stencil_attachment.clear_color = clear_color;
        }

        auto set_offset(int32_t x, int32_t y) -> void override {
            rendering_info.offset_x = x;
            rendering_info.offset_y = y;
        }

        auto set_size(uint32_t width, int32_t height) -> void override {
            rendering_info.width = width;
            rendering_info.height = height;
        }

        auto set_layers(uint32_t layer_count) -> void override {
            rendering_info.layer_count = layer_count;
        }

        auto get_graph() -> IRenderGraph& override {
            return graph_;
        }

        auto get_descriptor_layouts() noexcept -> std::span<edge::gfx::IDescriptorLayout*> override {
            return descriptor_layouts_;
        }

        auto bind_descriptors(edge::gfx::ICommandList& cmd_list, std::span<edge::gfx::IBindingLayout*> binding_layouts) -> void override {
            edge::gfx::BindDescriptorsInfo bind_info{};
            bind_info.first_set = 0;
            bind_info.binding_layouts = binding_layouts;
            bind_info.pipeline_type = pipeline_->get_type();
            bind_info.root_signature = root_sign_;
            cmd_list.bind_descriptors(bind_info);
        }

        auto push_constants(edge::gfx::ICommandList& cmd_list, std::span<const char> data) -> void override {
            cmd_list.push_constants(shader_, root_sign_, data);
        }

        auto execute(edge::gfx::ICommandList& cmd_list, float delta_time) -> void override {
            EDGE_PROFILE_ZONE_SCOPED;

            EDGE_GFX_PROFILER_BEGIN_ZONE_NC(cmd_list, name_, 0xFFFF11FF);

            // Request resource usage
            if (setup_func_) {
                setup_func_(*this);
            }

            edge::gfx::ResourceBarrierInfo barrier{
                .buffer_barriers = buffer_barriers_,
                .texture_barriers = texture_barriers_
            };
            cmd_list.resource_barrier(barrier);
            cmd_list.bind_pipeline(pipeline_);

            bool is_graphics_pipeline = pipeline_->get_type() == edge::gfx::PipelineType::eGraphics;
            if (is_graphics_pipeline) {
                cmd_list.begin_rendering(rendering_info);
            }

            // Collect rendering commands
            if (execute_func_) {
                execute_func_(*this, cmd_list, delta_time);
            }

            if (is_graphics_pipeline) {
                cmd_list.end_rendering();
            }

            EDGE_GFX_PROFILER_END_ZONE(cmd_list);
        }
    protected:
        auto add_resource_barrier(RGResourceHandle handle, edge::gfx::ResourceStateFlags required_state, uint32_t subresource_index = ~0u) -> bool {
            auto* graph_resource = graph_.get_resource(handle);
            if (!graph_resource) {
                return false;
            }

            auto* rhi_resource = graph_resource->get_resource();
            if (!rhi_resource) {
                return false;
            }

            // Already in expected state
            auto current_state = graph_resource->get_state();
            if (current_state == required_state) {
                return false;
            }

            auto resource_type = rhi_resource->get_resource_type();
            switch (resource_type)
            {
            case edge::gfx::ResourceType::eBuffer: {
                auto& barrier = buffer_barriers_.emplace_back();
                barrier.buffer = rhi_resource->as<edge::gfx::IBuffer*>();
                barrier.src_state = current_state;
                barrier.dst_state = required_state;
                barrier.range.size = barrier.buffer->get_size();
                break;
            }
            case edge::gfx::ResourceType::eTexture: {
                auto& barrier = texture_barriers_.emplace_back();
                barrier.texture = rhi_resource->as<edge::gfx::ITexture*>();
                barrier.subresource_index = subresource_index;
                barrier.src_state = current_state;
                barrier.dst_state = required_state;
                break;
            }
            default: {
                EDGE_LOGE("[RenderGraph] Unsupported resource type.");
                break;
            }
            }

            graph_resource->set_state(required_state);
            return true;
        }

        IRenderGraph& graph_;

        std::string name_{};

        edge::gfx::IShader* shader_{ nullptr };
        edge::gfx::IPipeline* pipeline_{ nullptr };
        edge::gfx::IRootSignature* root_sign_{ nullptr };
        std::span<edge::gfx::IDescriptorLayout*> descriptor_layouts_;
        
        edge::gfx::RenderingInfo rendering_info{};

        std::vector<edge::gfx::BufferBarrierDesc> buffer_barriers_{};
        std::vector<edge::gfx::TextureBarrierDesc> texture_barriers_{};

        RGPassSetupFn setup_func_{};
        RGPassExecuteFn execute_func_{};
    };


    class BufferResource final : public IRGResource {
    public:
        friend class IRenderGraph;

        BufferResource(const edge::gfx::BufferCreateInfo& create_info) :
            create_info_{ create_info } {
        }

        BufferResource(edge::gfx::IGFXResource* buffer) {
            resource_ = buffer;
        }

        ~BufferResource() override {
            if (resource_ && !is_imported_) {
                edge::gfx::destroy(resource_);
            }
        }

        static auto construct(const edge::gfx::BufferCreateInfo& create_info) -> std::unique_ptr<IRGResource> {
            std::unique_ptr<BufferResource> self{};

            self = std::make_unique<BufferResource>(create_info);
            self->is_persistent_ = true;

            return self;
        }

        static auto construct(edge::gfx::IGFXResource* resource) -> std::unique_ptr<IRGResource> {
            std::unique_ptr<BufferResource> self{};

            self = std::make_unique<BufferResource>(resource);
            self->is_imported_ = true;

            return self;
        }

        auto evaluate(IRenderGraph& graph) -> void override {
            // TODO: Implement resource attribute inherition
            if (!resource_ && !is_imported_) {
                resource_ = edge::gfx::create_buffer(create_info_);
            }
        }

        auto get_size() -> size_t override {
            if (resource_) {
                auto* buffer = resource_->as<edge::gfx::IBuffer*>();
                return buffer->get_size();
            }

            return 0ull;
        }
    private:
        edge::gfx::BufferCreateInfo create_info_{};
    };

    class TextureResource final : public IRGResource {
    public:
        friend class IRenderGraph;

        TextureResource(const edge::gfx::TextureCreateInfo& create_info) :
            create_info_{ create_info } {
        }

        TextureResource(edge::gfx::IGFXResource* texture) {
            resource_ = texture;
        }

        ~TextureResource() override {
            if (resource_ && !is_imported_) {
                edge::gfx::destroy(resource_);
            }
        }

        static auto construct(const edge::gfx::TextureCreateInfo& create_info, const std::array<float, 3>& scale) -> std::unique_ptr<IRGResource> {
            std::unique_ptr<TextureResource> self{};

            self = std::make_unique<TextureResource>(create_info);
            self->is_persistent_ = true;
            self->scale_ = scale;

            return self;
        }

        static auto construct(edge::gfx::IGFXResource* ext_resource) -> std::unique_ptr<IRGResource> {
            std::unique_ptr<TextureResource> self{};

            self = std::make_unique<TextureResource>(ext_resource);
            self->is_imported_ = true;

            return self;
        }

        auto evaluate(IRenderGraph& graph) -> void override {
            bool is_size_changed{ false };

            if (parent_handle_.is_valid()) {
                auto* parent_resouece = graph.get_resource(parent_handle_);
                auto* texture_resouece = parent_resouece->get_resource()->as<edge::gfx::ITexture*>();

                auto new_width = texture_resouece->get_width() * scale_[0];
                auto new_height = texture_resouece->get_height() * scale_[1];
                auto new_depth = texture_resouece->get_depth() * scale_[2];

                is_size_changed = create_info_.width != new_width || 
                    create_info_.height != new_height || 
                    create_info_.depth != new_depth;

                create_info_.width = new_width;
                create_info_.height = new_height;
                create_info_.depth = new_depth;
            }

            if ((!resource_ || is_size_changed) && !is_imported_) {
                if (resource_) {
                    edge::gfx::destroy(resource_);
                    resource_ = nullptr;
                }

                resource_ = edge::gfx::create_texture(create_info_);
            }

            for (auto child : child_handles_) {
                auto* child_resource = graph.get_resource(child);
                child_resource->evaluate(graph);
            }
        }

        auto get_size() -> size_t override {
            return 0ull;
        }
    private:
        edge::gfx::TextureCreateInfo create_info_{};
        std::array<float, 3> scale_;
    };




    // Render graph implementation
    RenderGraph::RenderGraph(edge::gfx::TechniqueDatabase& technique_database) : 
        technique_db_{ technique_database } {
    }

    RenderGraph::~RenderGraph() {
    }

    auto RenderGraph::construct(edge::gfx::TechniqueDatabase& technique_database) -> std::unique_ptr<IRenderGraph> {
        auto self = std::make_unique<RenderGraph>(technique_database);
        return self;
    }

    auto RenderGraph::create_resource(const RGResourceCreateInfo& create_info) -> RGResourceHandle {
        auto id = next_resource_id();

        std::unique_ptr<IRGResource> resource{};
        if (create_info.type == edge::gfx::ResourceType::eTexture) {
            if (create_info.external_resource) {
                resource = TextureResource::construct(create_info.external_resource);
            }
            else {
                resource = TextureResource::construct(create_info.texture_create_info, create_info.scale);
            }
        }
        else if (create_info.type == edge::gfx::ResourceType::eBuffer) {
            if (create_info.external_resource) {
                resource = BufferResource::construct(create_info.external_resource);
            }
            else {
                resource = BufferResource::construct(create_info.buffer_create_info);
            }
        }
        else {
            EDGE_LOGE("[RenderGraph] Unknown resource type.")
        }

        auto handle = RGResourceHandle(id);
        if (create_info.parent.is_valid()) {
            // TODO: Check parent and child same type
            resource->set_parent(create_info.parent);

            auto* parent_resource = get_resource(create_info.parent);
            parent_resource->add_child(handle);
        }

        resources_[id] = std::move(resource);
        return handle;
    }

    auto RenderGraph::update_resource(RGResourceHandle handle, edge::gfx::IGFXResource* resource) -> RGResourceHandle {
        if (auto* res = get_resource(handle)) {
            res->set_state(edge::gfx::ResourceStateFlag::eUndefined);
            res->set_resource(resource);
        }

        return handle;
    }

    auto RenderGraph::add_pass(RGPassCreateInfo create_info) -> void {
        auto& found_technique = technique_db_.get_technique(create_info.technique_name);
        passes_.emplace_back(BasePass::construct(*this, std::move(create_info.setup_func), std::move(create_info.execute_func), found_technique));
    }

    auto RenderGraph::get_resource(RGResourceHandle handle) -> IRGResource* {
        auto it = resources_.find(handle.id());
        return (it != resources_.end()) ? it->second.get() : nullptr;
    }

    auto RenderGraph::execute(edge::gfx::ICommandList& cmd_list, float delta_time) -> void {
        for (auto& [id, resource] : resources_) {
            // Evaluate only individual nodes
            if (!resource->get_parent().is_valid()) {
                resource->evaluate(*this);
            }
        }

        for (auto& pass : passes_) {
            pass->execute(cmd_list, delta_time);
        }

        passes_.clear();
    }
}