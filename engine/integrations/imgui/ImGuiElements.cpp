#include "ImGuiElements.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <string>
#include <vector>
#include <list>

#include <glm/gtc/type_ptr.hpp>

constexpr const float _columnWidth{ 100.f };

template<class _Ty>
void ApplyPropertyChange(_Ty& vold, _Ty* vnew)
{
    if (ImGui::IsItemActivated())
        vold = *vnew;

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        //auto& actionBuffer = EGEditor->getActionBuffer();
        //actionBuffer->addOperation(std::make_unique<CPropertyChangedOperation>(vold, vnew, utl::type_hash<_Ty>()));
    }
}

template<class _Ty, class _Func>
bool SimpleValueEdit(const char* label, _Ty* value, _Func&& function)
{
    static _Ty oldValue{};
    bool bValueChanged{ false };

    ImGui::PushID(label);

    if (ImGui::BeginTable("data_table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, ImGui::columnWidth);
        ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextColumn();
        ImGui::Text(label);

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-1.f);
        bValueChanged = function("##input", value);
        ApplyPropertyChange(oldValue, value);

        ImGui::EndTable();
    }

    ImGui::PopID();

    return bValueChanged;
}


bool combo_std_vector_pred(void* data, int idx, const char** out_text)
{
    std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(data);
    *out_text = vector->at(idx).c_str();
    return true;
}

bool combo_std_list_pred(void* data, int idx, const char** out_text)
{
    std::list<std::string>* vector = reinterpret_cast<std::list<std::string>*>(data);
    *out_text = std::next(vector->begin(), idx)->c_str();
    return true;
}

bool ImGui::SpartialRoundedButtonEx(const char* label, const ImVec2& size, ImGuiButtonRoundingFlags rounding_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GetCurrentContext();
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size_arg = CalcItemSize(size, label_size.x + g.Style.FramePadding.x * 2.0f, label_size.y + g.Style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size_arg.x, pos.y + size_arg.y));
    ItemSize(size_arg, g.Style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Custom rounding
    float rounding = g.Style.FrameRounding;
    ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

    ImDrawList* draw_list = GetWindowDrawList();
    
    // Helper function to add corner arcs
    auto AddArc = [&](ImVec2 center, float radius, float a_min, float a_max) {
        draw_list->PathArcTo(center, radius, a_min, a_max, 16);
        };

    // Draw filled rectangle with custom rounded and concave corners
    draw_list->PathClear();
    
    // Top left corner
    if (rounding_flags & ImGuiButtonRounding_RoundTopLeft)
        AddArc(ImVec2(bb.Min.x + rounding, bb.Min.y + rounding), rounding, IM_PI, IM_PI + IM_PI * 0.5f);
    else if (rounding_flags & ImGuiButtonRounding_ConcaveTopLeft)
        AddArc(ImVec2(bb.Min.x - rounding, bb.Min.y + rounding), rounding, 0.f, -IM_PI * 0.5f);
    else
        draw_list->PathLineTo(ImVec2(bb.Min.x, bb.Min.y));

    // Top right corner
    if (rounding_flags & ImGuiButtonRounding_RoundTopRight)
        AddArc(ImVec2(bb.Max.x - rounding, bb.Min.y + rounding), rounding, IM_PI * 1.5f, IM_PI * 2.0f);
    else if (rounding_flags & ImGuiButtonRounding_ConcaveTopRight)
        AddArc(ImVec2(bb.Max.x + rounding, bb.Min.y + rounding), rounding, IM_PI * 1.5f, IM_PI);
    else
        draw_list->PathLineTo(ImVec2(bb.Max.x, bb.Min.y));

    // Bottom right corner
    if (rounding_flags & ImGuiButtonRounding_RoundBottomRight)
        AddArc(ImVec2(bb.Max.x - rounding, bb.Max.y - rounding), rounding, 0.0f, IM_PI * 0.5f);
    else if (rounding_flags & ImGuiButtonRounding_ConcaveBottomRight)
        AddArc(ImVec2(bb.Max.x + rounding, bb.Max.y - rounding), rounding, IM_PI, IM_PI * 0.5f);
    else
        draw_list->PathLineTo(ImVec2(bb.Max.x, bb.Max.y));

    // Bottom left corner
    if (rounding_flags & ImGuiButtonRounding_RoundBottomLeft)
        AddArc(ImVec2(bb.Min.x + rounding, bb.Max.y - rounding), rounding, IM_PI * 0.5f, IM_PI);
    else if (rounding_flags & ImGuiButtonRounding_ConcaveBottomLeft)
        AddArc(ImVec2(bb.Min.x - rounding, bb.Max.y - rounding), rounding, IM_PI * 0.5f, 0.f);
    else
        draw_list->PathLineTo(ImVec2(bb.Min.x, bb.Max.y));

    draw_list->PathFillConcave(col);
    
    if (g.Style.FrameBorderSize > 0.0f)
        draw_list->PathStroke(GetColorU32(ImGuiCol_Border), true, g.Style.FrameBorderSize);

    if (g.LogEnabled)
        LogSetNextTextDecoration("[", "]");
    RenderTextClipped(bb.Min + g.Style.FramePadding, bb.Max - g.Style.FramePadding, label, NULL, &label_size, g.Style.ButtonTextAlign, &bb);
    
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool ImGui::GButtonLR(const char* label, const ImVec2& size)
{
    return SpartialRoundedButtonEx(label, size, ImGuiButtonRounding_ConcaveTopLeft | ImGuiButtonRounding_ConcaveBottomLeft | ImGuiButtonRounding_ConcaveTopRight | ImGuiButtonRounding_ConcaveBottomRight);
}

bool ImGui::GButtonRR(const char* label, const ImVec2& size)
{
    return SpartialRoundedButtonEx(label, size, ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomRight);
}

bool ImGui::GButtonTR(const char* label, const ImVec2& size)
{
    return SpartialRoundedButtonEx(label, size, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight);
}

bool ImGui::GButtonBR(const char* label, const ImVec2& size)
{
    return SpartialRoundedButtonEx(label, size, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight);
}

void ImGui::GText(const char* label, const char* fmt, ...)
{
    PushID(label);

    if (BeginTable("data_table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
        TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthStretch);

        TableNextColumn();
        Text(label);

        TableNextColumn();

        va_list args;
        va_start(args, fmt);
        TextV(fmt, args);
        va_end(args);

        EndTable();
    }

    PopID();
}

bool ImGui::GTextInput(const char* label, std::string* value, bool enabled)
{
    return SimpleValueEdit(label, value,
        [](const char* label, std::string* value)
        {
            return InputText(label, value);
        });
}

bool ImGui::GCheckbox(const char* label, bool* value)
{
    return SimpleValueEdit(label, value,
        [](const char* label, bool* value)
        {
            return Checkbox(label, value);
        });
}

bool ImGui::GColorEdit3(const char* label, float* value)
{
    static glm::vec3 oldValue{ 0.f };

    ImGuiColorEditFlags misc_flags{};
    bool bValueChanged{ false };

    PushID(label);

    if (BeginTable("data_table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
        TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthStretch);

        TableNextColumn();
        Text(label);

        TableNextColumn();
        PushItemWidth(GetContentRegionAvail().x);
        bValueChanged = ColorEdit3("##light_color", value);
        PopItemWidth();
        //ApplyPropertyChange(oldValue, &value);

        EndTable();
    }

    PopID();

    return bValueChanged;
}

bool ImGui::GColorEdit4(const char* label, float* value)
{
    static glm::vec4 oldValue{ 0.f };

    ImGuiColorEditFlags misc_flags{};
    bool bValueChanged{ false };

    PushID(label);

    if (BeginTable("data_table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
        TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthStretch);

        TableNextColumn();
        Text(label);

        TableNextColumn();
        bValueChanged = ColorEdit4("##", value, misc_flags);
        //ApplyPropertyChange(oldValue, &value);

        EndTable();
    }

    PopID();

    return bValueChanged;
}

bool ImGui::GDragTransform(glm::vec3& pos, glm::quat& rot, glm::vec3& scale, float step, float min, float max)
{
    bool bValueChanged{ false };
    bValueChanged |= GDragFloat<3>("position", glm::value_ptr(pos), step, min, max);
    bValueChanged |= GDragFloat<4>("rotation", glm::value_ptr(rot), step, min, max);
    bValueChanged |= GDragFloat<3>("scale", glm::value_ptr(scale), step, min, max);
    return bValueChanged;
}

bool ImGui::GAssetHolder(const char* label, const std::string& source)
{
    bool result{ false };

    PushID(label);

    if (BeginTable("table", 2))
    {
        TableNextColumn();
        Text(label);

        TableNextColumn();
        BeginDisabled(!source.empty());
        result = Button(source.empty() ? "+" : source.c_str(), ImVec2(-1.f, 0.f));
        EndDisabled();

        EndTable();
    }

    PopID();

    return result;
}

bool ImGui::GCombo(const char* label, int* current_item, const char* const items[], int item_count, int height_in_items)
{
    static int oldValue{ 0 };
    bool result{ false };

    PushID(label);

    if (BeginTable("data_table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
        TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthStretch);

        TableNextColumn();
        Text(label);

        TableNextColumn();

        const char* preview = items[*current_item];

        SetNextItemWidth(GetContentRegionAvail().x);
        if (BeginCombo("##", preview))
        {
            for (int n = 0; n < item_count; n++)
            {
                bool is_selected = (*current_item == n);
                if (Selectable(items[n], is_selected))
                    *current_item = n;
                if (is_selected)
                    SetItemDefaultFocus();
            }

            EndCombo();
        }

        ApplyPropertyChange(oldValue, current_item);
        EndTable();
    }

    PopID();

    return result;
}