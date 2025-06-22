#include "EffectGraph.h"

#include <unordered_set>
#include <stack>

#include <edge/util/Logger.h>

namespace e2d::audio {
    std::unordered_map<std::string, NodeFactory::CreateFunction> NodeFactory::creators_;

    auto NodeFactory::register_node_type(const std::string& type_name, CreateFunction create_func) -> void {
        creators_[type_name] = create_func;
    }

    auto NodeFactory::create_node(const std::string& type_name) -> std::unique_ptr<EffectGraphNode> {
        auto it = creators_.find(type_name);
        if (it != creators_.end()) {
            return it->second();
        }
        return nullptr;
    }

    auto NodeFactory::get_registered_types() -> std::vector<std::string> {
        std::vector<std::string> types;
        for (const auto& [type, _] : creators_) {
            types.push_back(type);
        }
        return types;
    }



    auto EffectGraph::add_node(std::unique_ptr<EffectGraphNode>&& node) -> NodeID {
        NodeID id = next_node_id_++;
        node->set_id(id);
        nodes_[id] = std::move(node);
        execution_order_dirty_ = true;
        return id;
    }

    auto EffectGraph::remove_node(NodeID id) -> bool {
        auto it = nodes_.find(id);
        if (it == nodes_.end()) {
            return false;
        }

        // Remove all connections involving this node
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                [id](const NodeConnection& conn) {
                    return conn.input.node_id == id || conn.output.node_id == id;
                }),
            connections_.end()
        );

        nodes_.erase(it);
        execution_order_dirty_ = true;

        // Remove buffer
        temp_buffers_.erase(id);

        return true;
    }

    auto EffectGraph::get_node(NodeID id) -> EffectGraphNode* {
        auto it = nodes_.find(id);
        return (it != nodes_.end()) ? it->second.get() : nullptr;
    }

    auto EffectGraph::get_node(NodeID id) const -> const EffectGraphNode* {
        auto it = nodes_.find(id);
        return (it != nodes_.end()) ? it->second.get() : nullptr;
    }

    auto EffectGraph::get_all_nodes() const -> const std::unordered_map<NodeID, std::unique_ptr<EffectGraphNode>>& {
        return nodes_;
    }

    auto EffectGraph::connect(const NodeSocket& output, const NodeSocket& input, float gain) -> bool {
        // Validate sockets
        if (output.type != NodeSocket::Type::eOutput || input.type != NodeSocket::Type::eInput) {
            EDGE_LOGE("[EffectGraph] Invalid socket types for connection");
            return false;
        }

        auto output_node = get_node(output.node_id);
        auto input_node = get_node(input.node_id);

        if (!output_node || !input_node) {
            EDGE_LOGE("[EffectGraph] Invalid nodes for connection");
            return false;
        }

        // Check if connection already exists
        for (const auto& conn : connections_) {
            if (conn.output.node_id == output.node_id &&
                conn.output.socket_id == output.socket_id &&
                conn.input.node_id == input.node_id &&
                conn.input.socket_id == input.socket_id) {
                EDGE_LOGW("[EffectGraph] Connection already exists");
                return false;
            }
        }

        connections_.emplace_back(output, input, gain);
        execution_order_dirty_ = true;

        return true;
    }

    auto EffectGraph::disconnect(const NodeSocket& output, const NodeSocket& input) -> bool {
        auto it = std::find_if(connections_.begin(), connections_.end(),
            [&](const NodeConnection& conn) {
                return conn.output.node_id == output.node_id &&
                    conn.output.socket_id == output.socket_id &&
                    conn.input.node_id == input.node_id &&
                    conn.input.socket_id == input.socket_id;
            });

        if (it != connections_.end()) {
            connections_.erase(it);
            execution_order_dirty_ = true;
            return true;
        }

        return false;
    }

    auto EffectGraph::get_connections() const -> const std::vector<NodeConnection>& {
        return connections_;
    }

    auto EffectGraph::get_node_inputs(NodeID id) const -> std::vector<NodeConnection> {
        std::vector<NodeConnection> inputs;
        for (const auto& conn : connections_) {
            if (conn.input.node_id == id) {
                inputs.push_back(conn);
            }
        }
        return inputs;
    }

    auto EffectGraph::get_node_outputs(NodeID id) const -> std::vector<NodeConnection> {
        std::vector<NodeConnection> outputs;
        for (const auto& conn : connections_) {
            if (conn.output.node_id == id) {
                outputs.push_back(conn);
            }
        }
        return outputs;
    }

    auto EffectGraph::process(std::span<uint8_t> output, const AudioFormat& format) -> size_t {
        if (output_node_id_ == 0 || nodes_.find(output_node_id_) == nodes_.end()) {
            std::fill(output.begin(), output.end(), 0);
            return 0;
        }

        // Rebuild execution order if needed
        if (execution_order_dirty_) {
            execution_order_ = build_execution_order();
            execution_order_dirty_ = false;
        }

        processed_nodes_.clear();

        process_node_recursive(output_node_id_, output, format);

        //// Ensure all nodes have buffers allocated
        //for (NodeID id : execution_order_) {
        //    auto& buffer = node_buffers_[id];
        //    if (buffer.size() < output.size()) {
        //        buffer.resize(output.size());
        //    }
        //    std::fill(buffer.begin(), buffer.end(), 0);
        //}
        //
        //// Process nodes in execution order
        //for (NodeID id : execution_order_) {
        //    auto node = get_node(id);
        //    if (!node) continue;
        //
        //    auto& node_buffer = node_buffers_[id];
        //
        //    // Collect inputs
        //    auto input_connections = get_node_inputs(id);
        //    std::vector<std::span<const uint8_t>> inputs;
        //    inputs.reserve(input_connections.size());
        //
        //    for (const auto& conn : input_connections) {
        //        auto& input_buffer = node_buffers_[conn.output.node_id];
        //        inputs.emplace_back(input_buffer.data(), input_buffer.size());
        //    }
        //
        //    // Process the node
        //    auto processed_bytes = node->process(
        //        std::span<uint8_t>(node_buffer.data(), output.size()),
        //        format,
        //        inputs
        //    );
        //}
        //
        //// Copy output from the output node
        //auto& output_buffer = node_buffers_[output_node_id_];
        //std::copy_n(output_buffer.begin(), std::min(output.size(), output_buffer.size()), output.begin());

        return output.size();
    }

    auto EffectGraph::build_execution_order() -> std::vector<NodeID> {
        if (is_cyclic()) {
            EDGE_LOGE("[EffectGraph] Cyclic dependency detected in graph");
            return {};
        }
        return topological_sort();
    }

    auto EffectGraph::process_node_recursive(NodeID node_id, std::span<uint8_t> output, const AudioFormat& format) -> void {
        // Avoid processing the same node multiple times
        if (processed_nodes_.count(node_id)) {
            return;
        }

        auto node = get_node(node_id);
        if (!node) {
            std::fill(output.begin(), output.end(), 0);
            return;
        }

        // Get input connections
        auto input_connections = get_node_inputs(node_id);

        if (input_connections.empty()) {
            // No inputs - this is a source node (oscillator, file input, noise, etc.)
            node->process(output, format, {});
            processed_nodes_.insert(node_id);
            return;
        }

        // Group connections by socket_id and process input nodes
        std::map<SocketID, NodeConnection> socket_connections;
        for (const auto& conn : input_connections) {
            socket_connections[conn.input.socket_id] = conn;
        }

        // Process input nodes and prepare input spans in socket order
        std::vector<std::span<const uint8_t>> input_spans;
        auto input_sockets = node->get_input_sockets();
        input_spans.resize(input_sockets.size());

        for (size_t i = 0; i < input_sockets.size(); ++i) {
            SocketID socket_id = input_sockets[i].socket_id;

            auto it = socket_connections.find(socket_id);
            if (it != socket_connections.end()) {
                const auto& conn = it->second;

                // Get or create temporary buffer for this input
                auto& temp_buffer = temp_buffers_[conn.output.node_id];
                if (temp_buffer.size() < output.size()) {
                    temp_buffer.resize(output.size());
                }

                // Process the input node into its temporary buffer
                std::span<uint8_t> input_buffer_span(temp_buffer.data(), output.size());
                process_node_recursive(conn.output.node_id, input_buffer_span, format);

                // Apply gain if needed
                if (conn.gain != 1.0f) {
                    auto float_span = std::span<float>((float*)temp_buffer.data(), temp_buffer.size() / sizeof(float));
                    for (auto& sample : float_span) {
                        sample *= conn.gain;
                    }
                }

                input_spans[i] = std::span<const uint8_t>(temp_buffer.data(), temp_buffer.size());
            }
            else {
                // No connection for this socket - provide empty buffer
                static std::vector<uint8_t> empty_buffer(4096, 0); // Static to avoid reallocation
                if (empty_buffer.size() < output.size()) {
                    empty_buffer.resize(output.size(), 0);
                }
                else {
                    std::fill(empty_buffer.begin(), empty_buffer.begin() + output.size(), 0);
                }
                input_spans[i] = std::span<const uint8_t>(empty_buffer.data(), output.size());
            }
        }

        // Process current node with all inputs
        node->process(output, format, input_spans);
        processed_nodes_.insert(node_id);
    }

    auto EffectGraph::is_cyclic() const -> bool {
        std::unordered_set<NodeID> white, gray, black;

        for (const auto& [id, _] : nodes_) {
            white.insert(id);
        }

        std::function<bool(NodeID)> visit = [&](NodeID node) -> bool {
            if (black.count(node)) return false;
            if (gray.count(node)) return true; // Back edge found, cycle detected

            white.erase(node);
            gray.insert(node);

            auto outputs = get_node_outputs(node);
            for (const auto& conn : outputs) {
                if (visit(conn.input.node_id)) {
                    return true;
                }
            }

            gray.erase(node);
            black.insert(node);
            return false;
            };

        while (!white.empty()) {
            if (visit(*white.begin())) {
                return true;
            }
        }

        return false;
    }

    auto EffectGraph::topological_sort() -> std::vector<NodeID> {
        std::vector<NodeID> result;
        std::unordered_set<NodeID> visited;
        std::stack<NodeID> stack;

        std::function<void(NodeID)> visit = [&](NodeID node) {
            if (visited.count(node)) return;
            visited.insert(node);

            auto outputs = get_node_outputs(node);
            for (const auto& conn : outputs) {
                visit(conn.input.node_id);
            }

            stack.push(node);
            };

        for (const auto& [id, _] : nodes_) {
            visit(id);
        }

        while (!stack.empty()) {
            result.push_back(stack.top());
            stack.pop();
        }

        return result;
    }

    auto EffectGraph::to_json() const -> nlohmann::json {
        nlohmann::json j;

        // Serialize nodes
        nlohmann::json nodes_json = nlohmann::json::array();
        for (const auto& [id, node] : nodes_) {
            nlohmann::json node_json;
            node_json["id"] = id;
            node_json["type"] = node->get_type_name();
            node_json["position"] = { node->get_position().first, node->get_position().second };
            node_json["data"] = node->to_json();
            nodes_json.push_back(node_json);
        }
        j["nodes"] = nodes_json;

        // Serialize connections
        nlohmann::json connections_json = nlohmann::json::array();
        for (const auto& conn : connections_) {
            nlohmann::json conn_json;
            conn_json["output"] = {
                {"node_id", conn.output.node_id},
                {"socket_id", conn.output.socket_id},
                {"name", conn.output.name}
            };
            conn_json["input"] = {
                {"node_id", conn.input.node_id},
                {"socket_id", conn.input.socket_id},
                {"name", conn.input.name}
            };
            conn_json["gain"] = conn.gain;
            connections_json.push_back(conn_json);
        }
        j["connections"] = connections_json;

        j["output_node"] = output_node_id_;
        j["next_node_id"] = next_node_id_;

        return j;
    }

    auto EffectGraph::from_json(const nlohmann::json& json) -> bool {
        try {
            clear();

            // Load nodes
            if (json.contains("nodes")) {
                for (const auto& node_json : json["nodes"]) {
                    std::string type = node_json["type"];
                    auto node = NodeFactory::create_node(type);
                    if (!node) {
                        EDGE_LOGE("[EffectGraph] Unknown node type: {}", type);
                        continue;
                    }

                    NodeID id = node_json["id"];
                    node->set_id(id);

                    if (node_json.contains("position")) {
                        float x = node_json["position"][0];
                        float y = node_json["position"][1];
                        node->set_position(x, y);
                    }

                    if (node_json.contains("data")) {
                        node->from_json(node_json["data"]);
                    }

                    nodes_[id] = std::move(node);
                    next_node_id_ = std::max(next_node_id_, id + 1);
                }
            }

            // Load connections
            if (json.contains("connections")) {
                for (const auto& conn_json : json["connections"]) {
                    NodeSocket output(
                        conn_json["output"]["node_id"],
                        conn_json["output"]["socket_id"],
                        NodeSocket::Type::eOutput,
                        conn_json["output"]["name"]
                    );

                    NodeSocket input(
                        conn_json["input"]["node_id"],
                        conn_json["input"]["socket_id"],
                        NodeSocket::Type::eInput,
                        conn_json["input"]["name"]
                    );

                    float gain = conn_json.value("gain", 1.0f);
                    connect(output, input, gain);
                }
            }

            if (json.contains("output_node")) {
                output_node_id_ = json["output_node"];
            }

            if (json.contains("next_node_id")) {
                next_node_id_ = json["next_node_id"];
            }

            execution_order_dirty_ = true;
            return true;

        }
        catch (const std::exception& e) {
            EDGE_LOGE("[EffectGraph] Failed to load from JSON: {}", e.what());
            return false;
        }
    }

    auto EffectGraph::clear() -> void {
        nodes_.clear();
        connections_.clear();
        temp_buffers_.clear();
        processed_nodes_.clear();
        next_node_id_ = 1;
        output_node_id_ = 0;
        execution_order_dirty_ = true;
    }
}