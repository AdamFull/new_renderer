#include "EffectGraphEditor.h"

#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <edge/fs/fs_virtual_filesystem.h>
#include <edge/util/Logger.h>

#include <fstream>

#include <nfd.h>

namespace e2d::audio {
    namespace ui_helpers {
        auto knob_float(const char* label, float* value, float min_val, float max_val, float default_val = 0.0f) -> bool {
            // Enhanced knob implementation with better visual feedback
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return false;

            const ImGuiID id = window->GetID(label);
            const float radius = 20.0f;
            const ImVec2 pos = window->DC.CursorPos;
            const ImRect bb(pos, ImVec2(pos.x + radius * 2, pos.y + radius * 2 + ImGui::GetTextLineHeight()));

            ImGui::ItemSize(bb);
            if (!ImGui::ItemAdd(bb, id)) return false;

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_None);

            if (held) {
                ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
                float delta = (-mouse_delta.y * 0.01f);
                *value = ImClamp(*value + delta * (max_val - min_val), min_val, max_val);
            }

            // Draw knob
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const ImVec2 center = ImVec2(bb.Min.x + radius, bb.Min.y + radius);

            // Background circle
            ImU32 bg_color = hovered ? IM_COL32(70, 70, 70, 255) : IM_COL32(50, 50, 50, 255);
            draw_list->AddCircleFilled(center, radius, bg_color);

            // Value arc
            float angle = -2.5f + ((*value - min_val) / (max_val - min_val)) * 5.0f;
            float cos_a = cosf(angle), sin_a = sinf(angle);
            ImVec2 pointer_end = ImVec2(center.x + cos_a * (radius - 5), center.y + sin_a * (radius - 5));

            draw_list->AddLine(center, pointer_end, IM_COL32(255, 255, 255, 255), 2.0f);
            draw_list->AddCircle(center, radius, IM_COL32(150, 150, 150, 255), 32, 1.0f);

            // Label
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
            ImGui::Text("%s: %.2f", label, *value);

            return held;
        }

        auto frequency_knob(const char* label, float* frequency, float min_freq = 20.0f, float max_freq = 20000.0f) -> bool {
            return knob_float(label, frequency, min_freq, max_freq, 1000.0f);
        }

        auto gain_knob(const char* label, float* gain_db, float min_db = -60.0f, float max_db = 20.0f) -> bool {
            return knob_float(label, gain_db, min_db, max_db, 0.0f);
        }

        auto q_knob(const char* label, float* q, float min_q = 0.1f, float max_q = 10.0f) -> bool {
            return knob_float(label, q, min_q, max_q, 0.707f);
        }

        auto volume_knob(const char* label, float* volume, float min_vol = 0.0f, float max_vol = 2.0f) -> bool {
            return knob_float(label, volume, min_vol, max_vol, 1.0f);
        }

        // Wave type selector
        auto wave_type_selector(const char* label, WaveType* wave_type) -> bool {
            const char* items[] = { "Sine", "Square", "Saw", "Triangle" };
            int current_item = static_cast<int>(*wave_type);

            if (ImGui::Combo(label, &current_item, items, IM_ARRAYSIZE(items))) {
                *wave_type = static_cast<WaveType>(current_item);
                return true;
            }
            return false;
        }

        // Filter type selector
        auto filter_mode_selector(const char* label, BiquadFilter::Mode* mode) -> bool {
            const char* items[] = {
                "Low Pass", "Band Pass", "High Pass", "All Pass",
                "Notch", "Bell", "Low Shelf", "High Shelf"
            };
            int current_item = static_cast<int>(*mode);

            if (ImGui::Combo(label, &current_item, items, IM_ARRAYSIZE(items))) {
                *mode = static_cast<BiquadFilter::Mode>(current_item);
                return true;
            }
            return false;
        }

        // Color schemes for different node types
        auto get_node_color(std::string_view node_type) -> ImU32 {
            if (node_type == "OscillatorNode") return IM_COL32(100, 200, 100, 255);
            if (node_type == "FileInputNode") return IM_COL32(200, 100, 100, 255);
            if (node_type == "MixerNode") return IM_COL32(100, 100, 200, 255);
            if (node_type == "BiquadFilterNode") return IM_COL32(200, 200, 100, 255);
            if (node_type == "GainNode") return IM_COL32(150, 150, 150, 255);
            if (node_type == "DelayNode") return IM_COL32(250, 160, 255, 255);
            if (node_type == "EqualizerNode") return IM_COL32(100, 200, 200, 255);
            return IM_COL32(128, 128, 128, 255);
        }

        auto get_socket_color(bool is_output) -> ImU32 {
            return is_output ? IM_COL32(255, 100, 100, 255) : IM_COL32(100, 255, 100, 255);
        }
    }


    EffectGraphEditor::EffectGraphEditor() {
        std::memset(filepath_buffer_, 0, sizeof(filepath_buffer_));
        std::memset(file_path_buffer_, 0, sizeof(file_path_buffer_));
    }

    EffectGraphEditor::~EffectGraphEditor() {
        shutdown();
    }

    auto EffectGraphEditor::initialize() -> bool {
        imnodes_context_ = ImNodes::CreateContext();
        if (!imnodes_context_) {
            EDGE_LOGE("[EffectGraphEditor] Failed to create ImNodes context");
            return false;
        }

        ImNodes::SetCurrentContext(imnodes_context_);

        // Setup ImNodes style
        auto& style = ImNodes::GetStyle(); 
        style.Flags |= ImNodesStyleFlags_GridLines | ImNodesStyleFlags_GridLinesPrimary;
        style.GridSpacing = 32.0f;

        return true;
    }

    auto EffectGraphEditor::shutdown() -> void {
        if (imnodes_context_) {
            ImNodes::DestroyContext(imnodes_context_);
            imnodes_context_ = nullptr;
        }
    }

    auto EffectGraphEditor::render(EffectGraph& graph) -> void {
        if (!is_open_) return;

        ImGui::SetNextWindowSize(ImVec2(1200, 800), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Effect Editor", &is_open_)) {
            render_menu_bar(graph);

            // Main editor area
            ImGui::BeginChild("GraphEditor", ImVec2(0, 0), true);

            ImNodes::BeginNodeEditor();

            // Render all nodes
            for (const auto& [node_id, node] : graph.get_all_nodes()) {
                render_node(graph, node_id, node.get());
            }

            // Render connections
            render_connections(graph);

            ImNodes::EndNodeEditor();

            // Handle interactions
            handle_link_creation(graph);
            handle_link_deletion(graph);
            handle_node_deletion(graph);

            ImGui::EndChild();
        }
        ImGui::End();

        // Right-click context menu
            // ImGui::IsWindowHovered() && 
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("CreateNodePopup");
            show_create_node_popup_ = true;
        }

        // Render popups
        render_node_creation_popup(graph);
    }

    auto EffectGraphEditor::render_menu_bar(EffectGraph& graph) -> void {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    graph.clear();
                    node_ui_data_.clear();
                    selected_node_id_ = 0;
                }

                if (ImGui::MenuItem("Open...")) {
                    nfdchar_t* out_path;
                    if (NFD_OpenDialog("json", NULL, &out_path) == NFD_OKAY) {
                        load_graph(graph, out_path);
                        free(out_path);
                    }
                }

                if (ImGui::MenuItem("Save As...")) {
                    nfdchar_t* out_path;
                    if (NFD_SaveDialog("json", "untitled.json", &out_path) == NFD_OKAY) {
                        save_graph(graph, out_path);
                        free(out_path);
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Create")) {
                for (const auto& node_type : NodeFactory::get_registered_types()) {
                    if (ImGui::MenuItem(node_type.c_str())) {
                        handle_node_creation(graph, node_type);
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    auto EffectGraphEditor::render_node_creation_popup(EffectGraph& graph) -> void {
        if (ImGui::BeginPopup("CreateNodePopup")) {
            ImGui::Text("Create Node");
            ImGui::Separator();

            for (const auto& node_type : NodeFactory::get_registered_types()) {
                if (ImGui::Selectable(node_type.c_str())) {
                    handle_node_creation(graph, node_type);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }

    auto EffectGraphEditor::render_node(EffectGraph& graph, NodeID node_id, EffectGraphNode* node) -> void {
        if (!node) return;

        int imnodes_id = node_id_to_imnodes_id(node_id);

        // Set node color based on type
        ImNodes::PushColorStyle(ImNodesCol_TitleBar, ui_helpers::get_node_color(node->get_type_name()));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, ui_helpers::get_node_color(node->get_type_name()));

        ImNodes::BeginNode(imnodes_id);

        // Title bar
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(node->get_type_name().data());
        ImNodes::EndNodeTitleBar();

        // Input sockets
        auto input_sockets = node->get_input_sockets();
        for (const auto& socket : input_sockets) {
            int socket_id = socket_to_imnodes_id(node_id, socket.socket_id, false);
            ImNodes::BeginInputAttribute(socket_id);
            ImGui::Text("%s", socket.name.c_str());
            ImNodes::EndInputAttribute();
        }

        // Node content (parameters)
        ImGui::PushItemWidth(120);
        bool node_changed = node->render();
        ImGui::PopItemWidth();

        // Output sockets
        auto output_sockets = node->get_output_sockets();
        for (const auto& socket : output_sockets) {
            int socket_id = socket_to_imnodes_id(node_id, socket.socket_id, true);
            ImNodes::BeginOutputAttribute(socket_id);
            ImGui::Indent(100);
            ImGui::Text("%s", socket.name.c_str());
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();

        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();

        // Track selection
        if (ImNodes::IsNodeSelected(imnodes_id)) {
            selected_node_id_ = node_id;
        }
    }

    auto EffectGraphEditor::render_connections(const EffectGraph& graph) -> void {
        const auto& connections = graph.get_connections();

        for (const auto& conn : connections) {
            int output_attr = socket_to_imnodes_id(conn.output.node_id, conn.output.socket_id, true);
            int input_attr = socket_to_imnodes_id(conn.input.node_id, conn.input.socket_id, false);

            // Find or create link ID
            auto conn_key = std::make_pair(conn.output.node_id, conn.input.node_id);
            int link_id;

            auto it = connection_to_link_id_.find(conn_key);
            if (it != connection_to_link_id_.end()) {
                link_id = it->second;
            }
            else {
                link_id = link_id_counter_++;
                connection_to_link_id_[conn_key] = link_id;
                link_id_to_connection_[link_id] = conn_key;
            }

            ImNodes::Link(link_id, output_attr, input_attr);
        }
    }

    auto EffectGraphEditor::handle_node_creation(EffectGraph& graph, const std::string& node_type) -> void {
        auto node = NodeFactory::create_node(node_type);
        if (!node) {
            EDGE_LOGE("[EffectGraphEditor] Failed to create node of type: {}", node_type);
            return;
        }

        // Set position based on mouse
        ImVec2 mouse_pos = ImGui::GetMousePos();
        //ImVec2 canvas_pos = ImNodes::CanvasPositionToLocalPosition(mouse_pos);
        ImVec2 canvas_pos = mouse_pos;
        node->set_position(canvas_pos.x, canvas_pos.y);

        auto node_id = graph.add_node(std::move(node));
        selected_node_id_ = node_id;
    }

    auto EffectGraphEditor::handle_link_creation(EffectGraph& graph) -> void {
        int start_attr, end_attr;
        if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
            auto [start_node, start_socket] = imnodes_id_to_node_socket(start_attr);
            auto [end_node, end_socket] = imnodes_id_to_node_socket(end_attr);

            bool start_is_output = is_output_socket(start_attr);
            bool end_is_output = is_output_socket(end_attr);

            if (start_is_output == end_is_output) {
                EDGE_LOGE("[AudioGraphEditor] Invalid connection: both sockets are same type");
                return;
            }

            NodeSocket output_socket, input_socket;
            if (start_is_output) {
                output_socket = NodeSocket(start_node, start_socket, NodeSocket::Type::eOutput, "");
                input_socket = NodeSocket(end_node, end_socket, NodeSocket::Type::eInput, "");
            }
            else {
                output_socket = NodeSocket(end_node, end_socket, NodeSocket::Type::eOutput, "");
                input_socket = NodeSocket(start_node, start_socket, NodeSocket::Type::eInput, "");
            }

            // Additional validation
            auto output_node = graph.get_node(output_socket.node_id);
            auto input_node = graph.get_node(input_socket.node_id);

            if (!output_node || !input_node) {
                EDGE_LOGE("[AudioGraphEditor] Invalid nodes for connection");
                return;
            }

            // Check that socket exists
            auto& output_sockets = output_node->get_output_sockets();
            auto& input_sockets = input_node->get_input_sockets();

            bool output_valid = false, input_valid = false;

            for (const auto& socket : output_sockets) {
                if (socket.socket_id == output_socket.socket_id) {
                    output_socket.name = socket.name;
                    output_valid = true;
                    break;
                }
            }

            for (const auto& socket : input_sockets) {
                if (socket.socket_id == input_socket.socket_id) {
                    input_socket.name = socket.name;
                    input_valid = true;
                    break;
                }
            }

            if (!output_valid || !input_valid) {
                EDGE_LOGE("[AudioGraphEditor] Invalid socket IDs for connection");
                return;
            }

            // Create connection
            if (!graph.connect(output_socket, input_socket)) {
                EDGE_LOGE("[AudioGraphEditor] Failed to create connection in graph");
            }
        }
    }

    auto EffectGraphEditor::is_output_socket(int imnodes_id) const -> bool {
        return (imnodes_id & 0x80000000) != 0;
    }

    auto EffectGraphEditor::handle_link_deletion(EffectGraph& graph) -> void {
        int link_id;
        if (ImNodes::IsLinkDestroyed(&link_id)) {
            auto it = link_id_to_connection_.find(link_id);
            if (it != link_id_to_connection_.end()) {
                auto [output_node, input_node] = it->second;

                // Find the actual connection to remove
                const auto& connections = graph.get_connections();
                for (const auto& conn : connections) {
                    if (conn.output.node_id == output_node && conn.input.node_id == input_node) {
                        graph.disconnect(conn.output, conn.input);
                        break;
                    }
                }

                connection_to_link_id_.erase(std::make_pair(output_node, input_node));
                link_id_to_connection_.erase(link_id);
            }
        }
    }

    auto EffectGraphEditor::handle_node_deletion(EffectGraph& graph) -> void {
        const int num_selected = ImNodes::NumSelectedNodes();
        if (num_selected > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            std::vector<int> selected_nodes(num_selected);
            ImNodes::GetSelectedNodes(selected_nodes.data());

            for (int imnodes_id : selected_nodes) {
                NodeID node_id = static_cast<NodeID>(imnodes_id);

                // Remove connections first
                auto input_connections = graph.get_node_inputs(node_id);
                auto output_connections = graph.get_node_outputs(node_id);

                for (const auto& conn : input_connections) {
                    graph.disconnect(conn.output, conn.input);
                }
                for (const auto& conn : output_connections) {
                    graph.disconnect(conn.output, conn.input);
                }

                // Remove the node
                graph.remove_node(node_id);

                if (selected_node_id_ == node_id) {
                    selected_node_id_ = 0;
                }
            }
        }
    }

    auto EffectGraphEditor::node_id_to_imnodes_id(NodeID node_id) -> int {
        return static_cast<int>(node_id);
    }

    auto EffectGraphEditor::socket_to_imnodes_id(NodeID node_id, SocketID socket_id, bool is_output) -> int {
        // Encode node_id, socket_id, and type into a unique int
        // Format: [is_output:1][node_id:15][socket_id:16]
        return (is_output ? 0x80000000 : 0) | ((node_id & 0x7FFF) << 16) | (socket_id & 0xFFFF);
    }

    auto EffectGraphEditor::imnodes_id_to_node_socket(int imnodes_id) -> std::pair<NodeID, SocketID> {
        NodeID node_id = (imnodes_id >> 16) & 0x7FFF;
        SocketID socket_id = imnodes_id & 0xFFFF;
        return std::make_pair(node_id, socket_id);
    }

    auto EffectGraphEditor::load_graph(EffectGraph& graph, const std::string& filepath) -> bool {
        if (!edge::fs::exists(filepath)) {
            EDGE_LOGE("[EffectGraphEditor] Graph file not found: {}", filepath);
            return false;
        }

        std::vector<uint8_t> file_data;
        if (!edge::fs::read_file(filepath, file_data)) {
            EDGE_LOGE("[EffectGraphEditor] Failed to read graph file: {}", filepath);
            return false;
        }

        try {
            std::string json_str(file_data.begin(), file_data.end());
            auto json = nlohmann::json::parse(json_str);

            if (graph.from_json(json)) {
                current_filepath_ = filepath;

                // Clear UI state
                node_ui_data_.clear();
                selected_node_id_ = 0;
                connection_to_link_id_.clear();
                link_id_to_connection_.clear();
                link_id_counter_ = 1;

                return true;
            }
        }
        catch (const std::exception& e) {
            EDGE_LOGE("[EffectGraphEditor] Failed to parse graph JSON: {}", e.what());
        }

        return false;
    }

    auto EffectGraphEditor::save_graph(const EffectGraph& graph, const std::string& filepath) -> bool {
        try {
            auto json = graph.to_json();
            std::string json_str = json.dump(4);

            std::ofstream file(filepath);
            if (!file.is_open()) {
                EDGE_LOGE("[EffectGraphEditor] Failed to open file for writing: {}", filepath);
                return false;
            }

            file << json_str;
            current_filepath_ = filepath;
            return file.good();
        }
        catch (const std::exception& e) {
            EDGE_LOGE("[EffectGraphEditor] Failed to save graph: {}", e.what());
            return false;
        }
    }
}