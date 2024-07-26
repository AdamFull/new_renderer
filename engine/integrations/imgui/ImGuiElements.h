#pragma once

#include <standart_library.h>
#include <glm/glm.hpp>
#include "ImGuiTraits.h"
#include <imgui_internal.h>

namespace ImGui
{
    constexpr const float widthScale = 0.2f;

    constexpr const char* axis_names[4]{ "X", "Y", "Z", "W" };
    constexpr const char* axis_sub_names[4]{ "##X", "##Y", "##Z", "##W" };
    constexpr const ImVec4 axis_button_colors[4]{ ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f } };
    constexpr const ImVec4 axis_button_hovered_colors[4]{ ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f }, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f }, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f }, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f } };
    constexpr const ImVec4 axis_button_active_colors[4]{ ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f } };

    template<typename _Ty, size_t _Count, typename _Func>
    bool UniversalEdit(const char* label, _Ty* value, _Func&& function)
    {
        static _Ty old[_Count]{ (_Ty)0 };

        bool value_changed{ false };
        ImGuiIO& io = GetIO();
        auto bold_font = io.Fonts->Fonts[0];

        PushID(label);

        if (BeginTable("data_table", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableSetupColumn("Parameter", 0, GetContentRegionAvail().x * widthScale);
            TableSetupColumn("Field", 0, GetContentRegionAvail().x * (1.f - widthScale));

            TableNextColumn();
            Text(label);

            TableNextColumn();

            PushMultiItemsWidths(_Count, GetContentRegionAvail().x * (1.f - widthScale));
            PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

            for (size_t idx = 0ull; idx < _Count; ++idx)
            {
                PushStyleColor(ImGuiCol_Button, axis_button_colors[idx]);
                PushStyleColor(ImGuiCol_ButtonHovered, axis_button_hovered_colors[idx]);
                PushStyleColor(ImGuiCol_ButtonActive, axis_button_active_colors[idx]);
                PushFont(bold_font);

                if (Button(axis_names[idx]))
                {
                    value_changed = true;
                    value[idx] = 0;
                }

                PopFont();
                PopStyleColor(3);

                SameLine();
                value_changed |= function(axis_sub_names[idx], &value[idx]);
                //ApplyPropertyChange(oldValue[idx], &value[idx]);
                PopItemWidth();

                if (_Count > 1)
                    SameLine();
            }

            PopStyleVar();
            EndTable();
        }

        PopID();
        return value_changed;
    }

    template<typename _Ty, size_t _Count = 1>
    bool Drag(const char* label, _Ty* value, _Ty min, _Ty max, float speed, const char* format)
    {
        return UniversalEdit<_Ty, _Count>(label, value,
            [min, max, speed, format](const char* label, _Ty* value)
            {
                return DragScalar(label, DataTypeByType_v<_Ty>, value, speed, &min, &max, format);
            });
    }

    template<typename _Ty, size_t _Count = 1>
    bool Slider(const char* label, _Ty* value, _Ty min, _Ty max, const char* format)
    {
        return UniversalEdit<_Ty, _Count>(label, value,
            [min, max, format](const char* label, _Ty* value)
            {
                return SliderScalar(label, DataTypeByType_v<_Ty>, value, &min, &max, format);
            });
    }

    template<typename _Ty, size_t _Count = 1>
    bool Input(const char* label, _Ty* value, _Ty step, _Ty fast_step, const char* format)
    {
        return UniversalEdit<_Ty, _Count>(label, value,
            [step, fast_step, format](const char* label, _Ty* value)
            {
                return InputScalar(label, DataTypeByType_v<_Ty>, value, &step, &fast_step, format);
            });
    }

    template<size_t _Count>
    bool GDragInt(const char* label, int* value, float speed = 1.f, int min = 0, int max = 0)
    {
        return Drag<int, _Count>(label, value, min, max, speed, "%d");
    }

    template<size_t _Count>
    bool GDragFloat(const char* label, float* value, float speed = 1.f, float min = 0.f, float max = 0.f)
    {
        return Drag<float, _Count>(label, value, min, max, speed, "%.3f");
    }

    template<size_t _Count>
    bool GDragDouble(const char* label, double* value, float speed = 1.f, double min = 0.0, double max = 0.0)
    {
        return Drag<double, _Count>(label, value, min, max, speed, "%.3f");
    }


    template<size_t _Count>
    bool GSliderInt(const char* label, int* value, int min, int max)
    {
        return Slider<int, _Count>(label, value, min, max, "%d");
    }

    template<size_t _Count>
    bool GSliderFloat(const char* label, float* value, float min, float max)
    {
        return Slider<float, _Count>(label, value, min, max, "%.3f");
    }

    template<size_t _Count>
    bool GSliderDouble(const char* label, double* value, double min, double max)
    {
        return Slider<double, _Count>(label, value, min, max, "%.6f");
    }


    template<size_t _Count>
    bool GInputInt(const char* label, int* value, int step, int fast_step)
    {
        return Input<int, _Count>(label, value, step, fast_step, "%d");
    }

    template<size_t _Count>
    bool GInputFloat(const char* label, float* value, float step, float fast_step)
    {
        return Input<float, _Count>(label, value, step, fast_step, "%.3f");
    }

    template<size_t _Count>
    bool GInputDouble(const char* label, double* value, double step, double fast_step)
    {
        return Input<double, _Count>(label, value, step, fast_step, "%.6f");
    }

    void GText(const char* label, const char* fmt, ...);
    bool GTextInput(const char* label, std::string* value, bool enabled = true);
    bool GCheckbox(const char* label, bool* value);
    bool GColorEdit3(const char* label, glm::vec3& value);
    bool GColorEdit4(const char* label, glm::vec4& value);

    bool GDragTransform(glm::vec3& pos, glm::quat& rot, glm::vec3& scale, float step = 0.01f, float min = 0.0f, float max = 0.0f);
    bool GAssetHolder(const char* label, const std::string& source);
    bool GCombo(const char* label, int* current_item, const char* const items[], int item_count, int height_in_items = -1);
}