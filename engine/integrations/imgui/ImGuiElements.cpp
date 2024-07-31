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
        ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_WidthFixed, ImGui::columnWidth);
        ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthStretch);

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

void ImGui::GText(const char* label, const char* fmt, ...)
{
    PushID(label);

    if (BeginTable("data_table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        TableSetupColumn("Parameter", 0, GetContentRegionAvail().x * widthScale);
        TableSetupColumn("Field", 0, GetContentRegionAvail().x * (1.f - widthScale));

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

bool ImGui::GColorEdit3(const char* label, glm::vec3& value)
{
    static glm::vec3 oldValue{ 0.f };

    ImGuiColorEditFlags misc_flags{};
    bool bValueChanged{ false };

    PushID(label);

    if (BeginTable("data_table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        TableSetupColumn("Parameter", 0, GetContentRegionAvail().x * widthScale);
        TableSetupColumn("Field", 0, GetContentRegionAvail().x * (1.f - widthScale));

        TableNextColumn();
        Text(label);

        TableNextColumn();
        bValueChanged = ColorEdit3("##light_color", glm::value_ptr(value));
        ApplyPropertyChange(oldValue, &value);

        EndTable();
    }

    PopID();

    return bValueChanged;
}

bool ImGui::GColorEdit4(const char* label, glm::vec4& value)
{
    static glm::vec4 oldValue{ 0.f };

    ImGuiColorEditFlags misc_flags{};
    bool bValueChanged{ false };

    PushID(label);

    if (BeginTable("data_table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        TableSetupColumn("Parameter", 0, GetContentRegionAvail().x * widthScale);
        TableSetupColumn("Field", 0, GetContentRegionAvail().x * (1.f - widthScale));

        TableNextColumn();
        Text(label);

        TableNextColumn();
        bValueChanged = ColorEdit4("##", glm::value_ptr(value), misc_flags);
        ApplyPropertyChange(oldValue, &value);

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

    if (BeginTable("table", 2))
    {
        TableNextColumn();
        Text(label);

        TableNextColumn();
        SetNextItemWidth(GetContentRegionAvail().x);
        result = Combo(label, current_item, items, item_count, height_in_items);
        ApplyPropertyChange(oldValue, current_item);

        EndTable();
    }

    PopID();

    return result;
}