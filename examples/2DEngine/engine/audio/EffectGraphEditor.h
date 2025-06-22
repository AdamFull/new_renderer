#pragma once 

#include "EffectGraph.h"
#include "EffectNodes.h"

#include <imgui.h>
#include <imnodes.h>

namespace e2d::audio {
	class EffectGraphEditor {
    public:
        EffectGraphEditor();
        ~EffectGraphEditor();

        auto initialize() -> bool;
        auto shutdown() -> void;

        auto render(EffectGraph& graph) -> void;
        auto is_open() const -> bool { return is_open_; }
        auto set_open(bool open) -> void { is_open_ = open; }

        // File operations
        auto load_graph(EffectGraph& graph, const std::string& filepath) -> bool;
        auto save_graph(const EffectGraph& graph, const std::string& filepath) -> bool;
    private:
        struct NodeUIData {
            ImVec2 position{ 0, 0 };
            bool selected{ false };
        };

        auto render_menu_bar(EffectGraph& graph) -> void;
        auto render_node_creation_popup(EffectGraph& graph) -> void;
        auto render_node(EffectGraph& graph, NodeID node_id, EffectGraphNode* node) -> void;
        auto render_connections(const EffectGraph& graph) -> void;
        auto handle_node_creation(EffectGraph& graph, const std::string& node_type) -> void;
        auto handle_link_creation(EffectGraph& graph) -> void;
        auto handle_link_deletion(EffectGraph& graph) -> void;
        auto handle_node_deletion(EffectGraph& graph) -> void;

        // Helper functions
        auto get_socket_position(NodeID node_id, SocketID socket_id, bool is_output) -> ImVec2;
        auto is_output_socket(int imnodes_id) const -> bool;
        auto node_id_to_imnodes_id(NodeID node_id) -> int;
        auto socket_to_imnodes_id(NodeID node_id, SocketID socket_id, bool is_output) -> int;
        auto imnodes_id_to_node_socket(int imnodes_id) -> std::pair<NodeID, SocketID>;

        bool is_open_{ true };
        bool show_create_node_popup_{ false };

        std::unordered_map<NodeID, NodeUIData> node_ui_data_;
        NodeID selected_node_id_{ 0 };

        // For file dialogs
        std::string current_filepath_;
        char filepath_buffer_[256];

        // For node creation
        char file_path_buffer_[256];

        // ImNodes context
        ImNodesContext* imnodes_context_{ nullptr };

        // Link creation state
        int link_id_counter_{ 1 };
        std::unordered_map<std::pair<NodeID, NodeID>, int> connection_to_link_id_;
        std::unordered_map<int, std::pair<NodeID, NodeID>> link_id_to_connection_;
	};
}