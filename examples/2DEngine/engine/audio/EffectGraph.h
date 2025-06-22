#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <span>
#include <functional>
#include <memory>

#include "Common.h"

#include <nlohmann/json.hpp>

namespace e2d::audio {
	using NodeID = uint32_t;
	using SocketID = uint32_t;

    struct NodeSocket {
        enum class Type {
            eInput,
            eOutput
        };

        NodeID node_id{};
        SocketID socket_id{};
        Type type{};
        std::string name{};
        bool is_connected{ false };

        NodeSocket() = default;
        NodeSocket(NodeID node, SocketID socket, Type t, std::string_view n)
            : node_id(node), socket_id(socket), type(t), name(n) {}
    };

    struct NodeConnection {
        NodeSocket output{};
        NodeSocket input{};
        float gain{ 1.0f };

        NodeConnection() = default;
        NodeConnection(const NodeSocket& out, const NodeSocket& in, float g = 1.0f)
            : output(out), input(in), gain(g) {}
    };

    class EffectGraphNode {
    public:
        virtual ~EffectGraphNode() = default;

        virtual auto get_type_name() const -> std::string_view = 0;
        auto get_input_sockets() const -> const std::vector<NodeSocket>& { return input_sockets_; }
        auto get_output_sockets() const -> const std::vector<NodeSocket>& { return output_sockets_; }

        virtual auto process(std::span<uint8_t> out, const AudioFormat& format,
            const std::vector<std::span<const uint8_t>>& inputs) -> size_t = 0;

        virtual auto render() -> bool = 0;

        virtual auto to_json() const->nlohmann::json = 0;
        virtual auto from_json(const nlohmann::json& json) -> void = 0;

        auto get_id() const -> NodeID { return id_; }
        auto set_id(NodeID id) -> void { id_ = id; }

        auto get_position() const -> std::pair<float, float> { return { pos_x_, pos_y_ }; }
        auto set_position(float x, float y) -> void { pos_x_ = x; pos_y_ = y; }

    protected:
        NodeID id_{ 0 };
        float pos_x_{ 0.0f }, pos_y_{ 0.0f };

        std::vector<NodeSocket> input_sockets_;
        std::vector<NodeSocket> output_sockets_;
    };

    class EffectGraph {
    public:
        EffectGraph() = default;
        ~EffectGraph() = default;

        // Node management
        auto add_node(std::unique_ptr<EffectGraphNode>&& node) -> NodeID;
        auto remove_node(NodeID id) -> bool;
        auto get_node(NodeID id) -> EffectGraphNode*;
        auto get_node(NodeID id) const -> const EffectGraphNode*;
        auto get_all_nodes() const -> const std::unordered_map<NodeID, std::unique_ptr<EffectGraphNode>>&;

        // Connection management
        auto connect(const NodeSocket& output, const NodeSocket& input, float gain = 1.0f) -> bool;
        auto disconnect(const NodeSocket& output, const NodeSocket& input) -> bool;
        auto get_connections() const -> const std::vector<NodeConnection>&;
        auto get_node_inputs(NodeID id) const -> std::vector<NodeConnection>;
        auto get_node_outputs(NodeID id) const -> std::vector<NodeConnection>;

        // Graph execution
        auto process(std::span<uint8_t> output, const AudioFormat& format) -> size_t;
        auto set_output_node(NodeID id) -> void { output_node_id_ = id; }
        auto get_output_node() const -> NodeID { return output_node_id_; }

        // Serialization
        auto to_json() const -> nlohmann::json;
        auto from_json(const nlohmann::json& json) -> bool;

        auto clear() -> void;

    private:
        auto build_execution_order() -> std::vector<NodeID>;
        auto process_node_recursive(NodeID node_id, std::span<uint8_t> output, const AudioFormat& format) -> void;
        auto is_cyclic() const -> bool;
        auto topological_sort() -> std::vector<NodeID>;

        std::unordered_map<NodeID, std::unique_ptr<EffectGraphNode>> nodes_;
        std::vector<NodeConnection> connections_;
        NodeID next_node_id_{ 1 };
        NodeID output_node_id_{ 0 };

        // Cached execution order
        std::vector<NodeID> execution_order_;
        bool execution_order_dirty_{ true };

        // Audio processing buffers
        std::unordered_map<NodeID, std::vector<uint8_t>> temp_buffers_;
        std::unordered_set<NodeID> processed_nodes_;
    };

    class NodeFactory {
    public:
        using CreateFunction = std::function<std::unique_ptr<EffectGraphNode>()>;

        static auto register_node_type(const std::string& type_name, CreateFunction create_func) -> void;
        static auto create_node(const std::string& type_name) -> std::unique_ptr<EffectGraphNode>;
        static auto get_registered_types() -> std::vector<std::string>;

    private:
        static std::unordered_map<std::string, CreateFunction> creators_;
    };

    // Macro for easy node registration
#define REGISTER_AUDIO_NODE(ClassName) \
        namespace { \
            struct ClassName##_Registrar { \
                ClassName##_Registrar() { \
                    NodeFactory::register_node_type(#ClassName, []() -> std::unique_ptr<EffectGraphNode> { \
                        return std::make_unique<ClassName>(); \
                    }); \
                } \
            }; \
            static ClassName##_Registrar ClassName##_registrar_instance; \
        }
}

namespace std {
    template<>
    struct hash<std::pair<e2d::audio::NodeID, e2d::audio::SocketID>> {
        size_t operator()(const std::pair<e2d::audio::NodeID, e2d::audio::SocketID>& p) const noexcept {
            size_t h1 = std::hash<e2d::audio::NodeID>{}(p.first);
            size_t h2 = std::hash<e2d::audio::SocketID>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };
}