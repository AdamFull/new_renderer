#include "fs_file_dialog.h"

#include "fs_virtual_filesystem.h"
#include "fs_helper.h"

#include <imgui.h>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace edge::fs::ui {
    struct DialogState {
        std::string id;
        std::string title;
        std::string current_path;
        std::string filename; // For save dialogs
        FileDialogType type;
        FileDialogCallbacks callbacks;
        bool is_open = true;

        // Filter state
        std::vector<FileFilter> filters;
        int current_filter_index = 0; // Index in filters vector, -1 for "All Files"

        // UI state
        std::vector<std::string> current_entries;
        std::vector<bool> is_directory_flags;
        std::vector<std::string> mount_points;
        int selected_index = -1;
        char filename_buffer[256] = {};
        bool needs_refresh = true;
        bool show_mount_points = false;
    };

    // Global state
    static std::unordered_map<std::string, DialogState> active_dialogs;

    // Helper functions
    static auto should_show_file(const std::string& filename, const DialogState& state) -> bool {
        if (state.type != FileDialogType::OpenFile || state.filters.empty()) {
            return true; // Show all files if no filters or not an open file dialog
        }

        if (state.current_filter_index < 0 || state.current_filter_index >= static_cast<int>(state.filters.size())) {
            return true; // "All Files" selected
        }

        const auto& filter = state.filters[state.current_filter_index];
        auto file_ext = fs::get_ext(filename);

        for (const auto& ext : filter.extensions) {
            if (file_ext == ext) {
                return true;
            }
        }

        return false;
    }

    static auto refresh_mount_points(DialogState& state) -> void {
        state.mount_points.clear();
        auto mount_points = fs::get_mount_points();
        for (const auto& mp : mount_points) {
            state.mount_points.emplace_back(mp);
        }

        // Show mount points if we're at root or mount points view
        state.show_mount_points = (state.current_path == "/" || state.current_path.empty() || state.mount_points.empty());
    }

    static auto auto_select_mount_point(DialogState& state) -> void {
        refresh_mount_points(state);

        // If initial path is root and we have mount points, select the first one
        if ((state.current_path == "/" || state.current_path.empty()) && !state.mount_points.empty()) {
            state.current_path = state.mount_points[0];
            state.show_mount_points = false;
            state.needs_refresh = true;
        }
    }

    static auto refresh_directory_listing(DialogState& state) -> void {
        state.current_entries.clear();
        state.is_directory_flags.clear();
        state.selected_index = -1;

        // If showing mount points
        if (state.show_mount_points) {
            for (const auto& mount_point : state.mount_points) {
                state.current_entries.emplace_back(mount_point);
                state.is_directory_flags.emplace_back(true);
            }
            state.needs_refresh = false;
            return;
        }

        // Add parent directory entry if not at root
        if (state.current_path != "/" && !state.current_path.empty()) {
            state.current_entries.emplace_back("..");
            state.is_directory_flags.emplace_back(true);
        }

        // Add mount points entry if we have mount points and not showing them
        if (!state.mount_points.empty()) {
            state.current_entries.emplace_back("[Mount Points]");
            state.is_directory_flags.emplace_back(true);
        }

        // Get mount point for current path to construct correct full paths
        auto mount_point = fs::get_mount_point(state.current_path);

        // Walk the current directory
        auto iterator = fs::walk(state.current_path, false);
        for (auto entry : iterator) {
            std::string entry_str{ entry };

            // The iterator returns entries relative to mount point root, so construct full path correctly
            std::string full_path = std::string{ mount_point };
            if (!full_path.empty() && full_path.back() != '/') {
                full_path += '/';
            }
            full_path += entry_str;

            bool is_dir = fs::is_directory(full_path);

            // Extract just the filename for display and filtering
            std::string display_name = std::string{ fs::get_filename(entry_str) };

            // Apply file filter for files
            if (!is_dir && !should_show_file(display_name, state)) {
                continue;
            }

            state.current_entries.emplace_back(std::move(entry_str));
            state.is_directory_flags.emplace_back(is_dir);
        }

        // Sort: directories first, then files
        std::vector<size_t> indices(state.current_entries.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) -> bool {
            // Special entries always first
            bool a_is_special = (state.current_entries[a] == ".." || state.current_entries[a] == "[Mount Points]");
            bool b_is_special = (state.current_entries[b] == ".." || state.current_entries[b] == "[Mount Points]");

            if (a_is_special && !b_is_special) return true;
            if (!a_is_special && b_is_special) return false;
            if (a_is_special && b_is_special) {
                // ".." comes before "[Mount Points]"
                if (state.current_entries[a] == ".." && state.current_entries[b] == "[Mount Points]") return true;
                if (state.current_entries[a] == "[Mount Points]" && state.current_entries[b] == "..") return false;
                return false;
            }

            // Directories before files
            if (state.is_directory_flags[a] != state.is_directory_flags[b]) {
                return state.is_directory_flags[a];
            }

            // Alphabetical within same type
            return state.current_entries[a] < state.current_entries[b];
            });

        // Reorder based on sorted indices
        std::vector<std::string> sorted_entries;
        std::vector<bool> sorted_flags;

        for (size_t idx : indices) {
            sorted_entries.emplace_back(std::move(state.current_entries[idx]));
            sorted_flags.emplace_back(state.is_directory_flags[idx]);
        }

        state.current_entries = std::move(sorted_entries);
        state.is_directory_flags = std::move(sorted_flags);
        state.needs_refresh = false;
    }

    static auto navigate_to_path(DialogState& state, std::string_view path) -> void {
        std::string new_path{ path };

        // Handle "[Mount Points]" navigation
        if (path == "[Mount Points]") {
            state.show_mount_points = true;
            state.needs_refresh = true;
            return;
        }

        // Handle ".." navigation
        if (path == "..") {
            new_path = std::string{ fs::parent_path(state.current_path) };
            if (new_path.empty()) {
                state.show_mount_points = true;
                state.needs_refresh = true;
                return;
            }
        }
        // Handle mount point selection
        else if (state.show_mount_points) {
            // Direct mount point selection
            auto it = std::find(state.mount_points.begin(), state.mount_points.end(), std::string{ path });
            if (it != state.mount_points.end()) {
                new_path = *it;
                state.show_mount_points = false;
            }
            else {
                return; // Invalid mount point
            }
        }
        // Handle directory entry navigation (entries from iterator include path from mount point)
        else {
            // The entry is relative to mount point root, so construct the full path
            auto mount_point = fs::get_mount_point(state.current_path);
            char buffer[512];
            new_path = std::string{ fs::path_append(mount_point, path, std::span{buffer}) };
        }

        // Ensure path exists and is a directory (skip check for mount points)
        if (std::find(state.mount_points.begin(), state.mount_points.end(), new_path) != state.mount_points.end() ||
            (fs::exists(new_path) && fs::is_directory(new_path))) {
            state.current_path = std::move(new_path);
            state.show_mount_points = false;
            state.needs_refresh = true;
        }
    }

    static auto get_full_selected_path(const DialogState& state) -> std::string {
        if (state.selected_index < 0 || state.selected_index >= static_cast<int>(state.current_entries.size())) {
            return {};
        }

        const auto& selected_entry = state.current_entries[state.selected_index];

        if (selected_entry == "..") {
            return std::string{ fs::parent_path(state.current_path) };
        }

        if (selected_entry == "[Mount Points]") {
            return {}; // Cannot select mount points entry as file
        }

        // If we're showing mount points, return the mount point directly
        if (state.show_mount_points) {
            return selected_entry;
        }

        // The entry is relative to mount point root, so construct the full path correctly
        auto mount_point = fs::get_mount_point(state.current_path);
        char buffer[512];
        return std::string{ fs::path_append(mount_point, selected_entry, std::span{buffer}) };
    }

    static auto render_dialog(DialogState& state) -> void {
        if (!state.is_open) return;

        ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);

        bool open = true;
        if (!ImGui::Begin(state.title.c_str(), &open, ImGuiWindowFlags_NoCollapse)) {
            ImGui::End();
            return;
        }

        if (!open) {
            state.is_open = false;
            if (state.callbacks.on_cancelled) {
                state.callbacks.on_cancelled();
            }
            ImGui::End();
            return;
        }

        // Refresh directory if needed
        if (state.needs_refresh) {
            refresh_directory_listing(state);
        }

        // Location display
        if (state.show_mount_points) {
            ImGui::Text("Location: [Mount Points]");
        }
        else {
            ImGui::Text("Location: %s", state.current_path.c_str());
        }

        // File filter dropdown (only for open file dialogs)
        if (state.type == FileDialogType::OpenFile && !state.filters.empty()) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200);

            std::string current_filter_name = "All Files";
            if (state.current_filter_index >= 0 && state.current_filter_index < static_cast<int>(state.filters.size())) {
                current_filter_name = state.filters[state.current_filter_index].name;
            }

            if (ImGui::BeginCombo("##filter", current_filter_name.c_str())) {
                // "All Files" option
                bool is_selected = (state.current_filter_index < 0);
                if (ImGui::Selectable("All Files", is_selected)) {
                    state.current_filter_index = -1;
                    state.needs_refresh = true;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }

                // Filter options
                for (int i = 0; i < static_cast<int>(state.filters.size()); ++i) {
                    bool is_selected = (state.current_filter_index == i);
                    if (ImGui::Selectable(state.filters[i].name.c_str(), is_selected)) {
                        state.current_filter_index = i;
                        state.needs_refresh = true;
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }

        ImGui::Separator();

        // File/folder listing
        ImGui::BeginChild("FileList", ImVec2(0, -60), true);

        for (int i = 0; i < static_cast<int>(state.current_entries.size()); ++i) {
            const auto& entry = state.current_entries[i];
            const bool is_dir = state.is_directory_flags[i];

            // Icon prefix and display name
            const char* icon;
            std::string display_name;

            if (entry == "..") {
                icon = "[UP] ";
                display_name = icon + entry;
            }
            else if (entry == "[Mount Points]") {
                icon = "[MNT] ";
                display_name = icon + entry;
            }
            else if (state.show_mount_points) {
                // When showing mount points, entry is the mount point name
                icon = "[MNT] ";
                display_name = icon + entry;
            }
            else {
                // For regular entries, show just the filename part
                icon = is_dir ? "[DIR] " : "[FILE] ";
                std::string filename = std::string{ fs::get_filename(entry) };
                display_name = icon + filename;
            }

            if (ImGui::Selectable(display_name.c_str(), state.selected_index == i,
                ImGuiSelectableFlags_AllowDoubleClick)) {
                state.selected_index = i;

                // Handle double-click
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (is_dir || entry == ".." || entry == "[Mount Points]") {
                        // Navigate into directory or special entry
                        navigate_to_path(state, entry);
                    }
                    else if (state.type == FileDialogType::OpenFile) {
                        // Select file and close dialog
                        std::string selected_path = get_full_selected_path(state);
                        if (!selected_path.empty()) {
                            state.is_open = false;
                            if (state.callbacks.on_selected) {
                                state.callbacks.on_selected(selected_path);
                            }
                        }
                    }
                }

                // For save dialog, populate filename with just the filename part
                if (state.type == FileDialogType::SaveFile && !is_dir && entry != ".." && entry != "[Mount Points]") {
                    std::string filename = std::string{ fs::get_filename(entry) };
                    strncpy(state.filename_buffer, filename.c_str(), sizeof(state.filename_buffer) - 1);
                    state.filename_buffer[sizeof(state.filename_buffer) - 1] = '\0';
                }
            }
        }

        ImGui::EndChild();

        // Bottom section with filename input (for save dialog) and buttons
        if (state.type == FileDialogType::SaveFile) {
            ImGui::Text("Filename:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-100);
            ImGui::InputText("##filename", state.filename_buffer, sizeof(state.filename_buffer));
        }

        ImGui::Separator();

        // Buttons
        bool can_confirm = false;
        if (state.type == FileDialogType::OpenFile) {
            can_confirm = (state.selected_index >= 0 &&
                state.selected_index < static_cast<int>(state.current_entries.size()) &&
                !state.is_directory_flags[state.selected_index] &&
                state.current_entries[state.selected_index] != ".." &&
                state.current_entries[state.selected_index] != "[Mount Points]");
        }
        else if (state.type == FileDialogType::OpenFolder) {
            can_confirm = !state.show_mount_points; // Can select current folder if not showing mount points
        }
        else if (state.type == FileDialogType::SaveFile) {
            can_confirm = (strlen(state.filename_buffer) > 0 && !state.show_mount_points);
        }

        if (ImGui::Button("OK") && can_confirm) {
            std::string result_path;

            if (state.type == FileDialogType::OpenFile) {
                result_path = get_full_selected_path(state);
            }
            else if (state.type == FileDialogType::OpenFolder) {
                result_path = state.current_path;
            }
            else if (state.type == FileDialogType::SaveFile) {
                char buffer[512];
                result_path = std::string{ fs::path_append(state.current_path, state.filename_buffer, std::span{buffer}) };
            }

            if (!result_path.empty()) {
                state.is_open = false;
                if (state.callbacks.on_selected) {
                    state.callbacks.on_selected(result_path);
                }
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            state.is_open = false;
            if (state.callbacks.on_cancelled) {
                state.callbacks.on_cancelled();
            }
        }

        ImGui::End();
    }

    // Public API implementation
    auto open_file_dialog(const char* id, const char* title, const char* initial_path,
        const std::vector<FileFilter>& filters, FileDialogCallbacks callbacks) -> bool {
        std::string dialog_id{ id };

        // Check if dialog already exists
        if (active_dialogs.find(dialog_id) != active_dialogs.end()) {
            return false;
        }

        // Create new dialog state
        DialogState state;
        state.id = dialog_id;
        state.title = title;
        state.current_path = initial_path;
        state.type = FileDialogType::OpenFile;
        state.callbacks = callbacks;
        state.filters = filters;
        state.current_filter_index = filters.empty() ? -1 : 0; // Default to first filter or "All Files"
        state.needs_refresh = true;

        // Auto-select mount point if needed
        auto_select_mount_point(state);

        active_dialogs[dialog_id] = std::move(state);
        return true;
    }

    auto open_folder_dialog(const char* id, const char* title, const char* initial_path,
        FileDialogCallbacks callbacks) -> bool {
        std::string dialog_id{ id };

        if (active_dialogs.find(dialog_id) != active_dialogs.end()) {
            return false;
        }

        DialogState state;
        state.id = dialog_id;
        state.title = title;
        state.current_path = initial_path;
        state.type = FileDialogType::OpenFolder;
        state.callbacks = callbacks;
        state.needs_refresh = true;

        // Auto-select mount point if needed
        auto_select_mount_point(state);

        active_dialogs[dialog_id] = std::move(state);
        return true;
    }

    auto save_file_dialog(const char* id, const char* title, const char* initial_path,
        const char* default_filename, FileDialogCallbacks callbacks) -> bool {
        std::string dialog_id{ id };

        if (active_dialogs.find(dialog_id) != active_dialogs.end()) {
            return false;
        }

        DialogState state;
        state.id = dialog_id;
        state.title = title;
        state.current_path = initial_path;
        state.type = FileDialogType::SaveFile;
        state.callbacks = callbacks;
        state.needs_refresh = true;

        strncpy(state.filename_buffer, default_filename, sizeof(state.filename_buffer) - 1);
        state.filename_buffer[sizeof(state.filename_buffer) - 1] = '\0';

        // Auto-select mount point if needed
        auto_select_mount_point(state);

        active_dialogs[dialog_id] = std::move(state);
        return true;
    }

    auto render_file_dialogs() -> void {
        // Render all active dialogs
        auto it = active_dialogs.begin();
        while (it != active_dialogs.end()) {
            render_dialog(it->second);

            // Remove closed dialogs
            if (!it->second.is_open) {
                it = active_dialogs.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    auto close_dialog(const char* id) -> void {
        std::string dialog_id{ id };
        auto it = active_dialogs.find(dialog_id);
        if (it != active_dialogs.end()) {
            it->second.is_open = false;
        }
    }

    auto is_dialog_open(const char* id) -> bool {
        std::string dialog_id{ id };
        auto it = active_dialogs.find(dialog_id);
        return (it != active_dialogs.end() && it->second.is_open);
    }

    auto close_all_dialogs() -> void {
        for (auto& [id, state] : active_dialogs) {
            state.is_open = false;
        }
    }
}