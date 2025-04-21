#pragma once

#include <imgui.h>

namespace edge::imui {
    template <auto _Fn, auto... _Args>
    class ScopeExit {
    public:
        constexpr ScopeExit() = default;
        constexpr ScopeExit(bool turn_on) : turn_on(turn_on) { }
        constexpr ScopeExit(const ScopeExit&) = delete;
        constexpr ScopeExit& operator=(const ScopeExit&) = delete;
        constexpr ScopeExit(ScopeExit&& rhs) noexcept : turn_on(rhs.turn_on) { rhs.turn_on = false; }
        constexpr ScopeExit& operator=(ScopeExit&& rhs) noexcept {
            bool temp = rhs.turn_on;
            rhs.turn_on = false;
            turn_on = temp;
            return *this;
        }
        constexpr ~ScopeExit() {
            if (turn_on) {
                _Fn(_Args...);
            }
        }
        constexpr operator bool() const noexcept {
            return turn_on;
        }

        bool turn_on = true;
    };

    template <auto _Fn>
    struct WindowGuard {
        ScopeExit<_Fn> guard;
        bool not_skip;

        operator bool() const noexcept {
            return not_skip;
        }
    };

    [[nodiscard]] inline auto make_scoped_window(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0) 
        -> WindowGuard<ImGui::End> {
        return { .not_skip = ImGui::Begin(name, p_open, flags) };
    }

    [[nodiscard]] inline auto make_scoped_child_window(const char* str_id, const ImVec2& size_arg, ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0)
        -> ScopeExit<ImGui::EndChild> {
        return { ImGui::BeginChild(str_id, size_arg, child_flags, window_flags) };
    }

    [[nodiscard]] inline auto make_scoped_style_color(ImGuiCol idx, ImU32 col) 
        -> ScopeExit<ImGui::PopStyleColor, 1> {
        ImGui::PushStyleColor(idx, col);
        return {};
    }

    [[nodiscard]] inline auto make_scoped_style_color(ImGuiCol idx, ImVec4 col)
        -> ScopeExit<ImGui::PopStyleColor, 1> {
        ImGui::PushStyleColor(idx, col);
        return {};
    }

    [[nodiscard]] inline auto make_scoped_menu_bar() 
        -> ScopeExit<ImGui::EndMainMenuBar> {
        return { ImGui::BeginMainMenuBar() };
    }

    [[nodiscard]] inline auto make_scoped_tree_node(const char* name)
        -> ScopeExit<ImGui::TreePop> {
        return { ImGui::TreeNode(name) };
    }

    [[nodiscard]] inline auto make_scoped_tree_node_ex(const char* label, ImGuiTreeNodeFlags flags = 0) 
        -> ScopeExit<ImGui::TreePop> {
        return { ImGui::TreeNodeEx(label, flags) };
    }

    [[nodiscard]] inline auto make_scoped_tab_bar(const char* label, ImGuiTabBarFlags flags = 0)
        -> ScopeExit<ImGui::EndTabBar> {
        return { ImGui::BeginTabBar(label, flags) };
    }

    [[nodiscard]] inline auto make_scoped_tab_item(const char* label, bool* p_open = nullptr, ImGuiTabItemFlags flags = 0)
        -> ScopeExit<ImGui::EndTabItem> {
        return { ImGui::BeginTabItem(label, p_open, flags) };
    }

    [[nodiscard]] inline auto make_scoped_push_id(const void* id)
        -> ScopeExit<ImGui::PopID> {
        ImGui::PushID(id);
        return {};
    }
}