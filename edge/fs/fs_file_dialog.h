#pragma once

#include <functional>
#include <string_view>
#include <string>

namespace edge::fs::ui {
    // Callback types
    using FileSelectedCallback = std::function<void(std::string_view path)>;
    using FileCancelledCallback = std::function<void()>;

    enum class FileDialogType {
        OpenFile,
        OpenFolder,
        SaveFile
    };

    struct FileDialogCallbacks {
        FileSelectedCallback on_selected = nullptr;
        FileCancelledCallback on_cancelled = nullptr;
    };

    // File extension filter
    struct FileFilter {
        std::string name;                    // Display name like "Image Files"
        std::vector<std::string> extensions; // Extensions like {".png", ".jpg", ".jpeg"}
    };

    // Main dialog functions - call these where you want to open dialogs
    // Returns true if dialog was just opened, false otherwise
    auto open_file_dialog(const char* id,
        const char* title = "Open File",
        const char* initial_path = "/",
        const std::vector<FileFilter>& filters = {},
        FileDialogCallbacks callbacks = {}) -> bool;

    auto open_folder_dialog(const char* id,
        const char* title = "Select Folder",
        const char* initial_path = "/",
        FileDialogCallbacks callbacks = {}) -> bool;

    auto save_file_dialog(const char* id,
        const char* title = "Save File",
        const char* initial_path = "/",
        const char* default_filename = "",
        FileDialogCallbacks callbacks = {}) -> bool;

    // Call this in your main render loop to actually render active dialogs
    auto render_file_dialogs() -> void;

    // Utility functions
    auto close_dialog(const char* id) -> void;
    auto is_dialog_open(const char* id) -> bool;
    auto close_all_dialogs() -> void;
}