#pragma once

#include <standart_library.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "ImGuiTraits.h"
#include <imgui_internal.h>

enum ImGuiButtonRounding_
{
    ImGuiButtonRounding_None = 0,
    ImGuiButtonRounding_RoundTopLeft = 1 << 0,
    ImGuiButtonRounding_RoundTopRight = 1 << 1,
    ImGuiButtonRounding_RoundBottomLeft = 1 << 2,
    ImGuiButtonRounding_RoundBottomRight = 1 << 3,
    ImGuiButtonRounding_ConcaveTopLeft = 1 << 4,
    ImGuiButtonRounding_ConcaveTopRight = 1 << 5,
    ImGuiButtonRounding_ConcaveBottomLeft = 1 << 6,
    ImGuiButtonRounding_ConcaveBottomRight = 1 << 7
};

typedef int ImGuiButtonRoundingFlags;

namespace ImGui
{
    // TODO: Rename this and add ability to register callbacks for value changed (begin/end editing)
    constexpr const float widthScale = 0.2f;
    constexpr const float columnWidth{ 100.f };

    constexpr const char* axis_names[4]{ "X", "Y", "Z", "W" };
    constexpr const char* axis_sub_names[4]{ "##X", "##Y", "##Z", "##W" };
    constexpr const ImVec4 axis_button_colors[4]{ ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f } };
    constexpr const ImVec4 axis_button_hovered_colors[4]{ ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f }, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f }, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f }, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f } };
    constexpr const ImVec4 axis_button_active_colors[4]{ ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f } };

    bool SpartialRoundedButtonEx(const char* label, const ImVec2& size, ImGuiButtonRoundingFlags rounding);
    bool GButtonLR(const char* label, const ImVec2& size = ImVec2(0, 0));
    bool GButtonRR(const char* label, const ImVec2& size = ImVec2(0, 0));
    bool GButtonTR(const char* label, const ImVec2& size = ImVec2(0, 0));
    bool GButtonBR(const char* label, const ImVec2& size = ImVec2(0, 0));

    void GText(const char* label, const char* fmt, ...);
    bool GTextInput(const char* label, std::string* value, bool enabled = true);
    bool GCheckbox(const char* label, bool* value);
    bool GColorEdit3(const char* label, float* value);
    bool GColorEdit4(const char* label, float* value);

    bool GAssetHolder(const char* label, const std::string& source);
    bool GBeginTable(const char* label);
    void GEndTable();
    bool GCombo(const char* label, int* current_item, const char* const items[], int item_count, int height_in_items = -1);


    template<typename _Ty, size_t _Count, typename _Func>
    bool UniversalEdit(const char* label, _Ty* value, _Func&& function)
    {
        static _Ty old[_Count]{ (_Ty)0 };

        bool value_changed{ false };
        ImGuiIO& io = GetIO();
        auto bold_font = io.Fonts->Fonts[0];

        if (GBeginTable(label))
        {
            PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

            auto defautl_button_flags = ImGuiButtonRounding_RoundTopLeft | ImGuiButtonRounding_RoundBottomLeft | ImGuiButtonRounding_ConcaveTopRight | ImGuiButtonRounding_ConcaveBottomRight;

            float lineHeight = (GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f) * GImGui->IO.FontGlobalScale;
            ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

            float item_width = GetContentRegionAvail().x / (float)_Count - lineHeight;

            for (size_t idx = 0ull; idx < _Count; ++idx)
            {
                PushStyleColor(ImGuiCol_Button, axis_button_colors[idx]);
                PushStyleColor(ImGuiCol_ButtonHovered, axis_button_hovered_colors[idx]);
                PushStyleColor(ImGuiCol_ButtonActive, axis_button_active_colors[idx]);
                PushFont(bold_font);

                if (SpartialRoundedButtonEx(axis_names[idx], buttonSize, defautl_button_flags))
                //if(GButtonLR(axis_names[idx], buttonSize))
                {
                    value_changed = true;
                    value[idx] = 0;
                }

                PopFont();
                PopStyleColor(3);

                SameLine();

                PushItemWidth(item_width);
                value_changed |= function(axis_sub_names[idx], &value[idx]);
                //ApplyPropertyChange(oldValue[idx], &value[idx]);
                PopItemWidth();

                if (_Count > 1 && idx < _Count - 1)
                {
                    defautl_button_flags = ImGuiButtonRounding_ConcaveTopLeft | ImGuiButtonRounding_ConcaveBottomLeft | ImGuiButtonRounding_ConcaveTopRight | ImGuiButtonRounding_ConcaveBottomRight;
                    SameLine();
                }
            }

            PopStyleVar();
            GEndTable();
        }

        return value_changed;
    }

    template<typename _Ty, size_t _Count = 1>
    bool Drag(const char* label, _Ty* value, _Ty min, _Ty max, float speed)
    {
        return UniversalEdit<_Ty, _Count>(label, value,
            [min, max, speed](const char* label, _Ty* value)
            {
                return DragScalar(label, DataTypeByType_v<_Ty>, value, speed, &min, &max, FormatByType_v<_Ty>);
            });
    }

    template<typename _Ty, size_t _Count = 1>
    bool Slider(const char* label, _Ty* value, _Ty min, _Ty max)
    {
        return UniversalEdit<_Ty, _Count>(label, value,
            [min, max](const char* label, _Ty* value)
            {
                return SliderScalar(label, DataTypeByType_v<_Ty>, value, &min, &max, FormatByType_v<_Ty>);
            });
    }

    template<typename _Ty, size_t _Count = 1>
    bool Input(const char* label, _Ty* value, _Ty step, _Ty fast_step)
    {
        return UniversalEdit<_Ty, _Count>(label, value,
            [step, fast_step](const char* label, _Ty* value)
            {
                return InputScalar(label, DataTypeByType_v<_Ty>, value, &step, &fast_step, FormatByType_v<_Ty>);
            });
    }

    template<size_t _Count>
    bool GDragInt(const char* label, int* value, float speed = 1.f, int min = 0, int max = 0)
    {
        return Drag<int, _Count>(label, value, min, max, speed);
    }

    template<size_t _Count>
    bool GDragFloat(const char* label, float* value, float speed = 1.f, float min = 0.f, float max = 0.f)
    {
        return Drag<float, _Count>(label, value, min, max, speed);
    }

    template<size_t _Count>
    bool GDragDouble(const char* label, double* value, float speed = 1.f, double min = 0.0, double max = 0.0)
    {
        return Drag<double, _Count>(label, value, min, max, speed);
    }


    template<size_t _Count>
    bool GSliderInt(const char* label, int* value, int min, int max)
    {
        return Slider<int, _Count>(label, value, min, max);
    }

    template<size_t _Count>
    bool GSliderFloat(const char* label, float* value, float min, float max)
    {
        return Slider<float, _Count>(label, value, min, max);
    }

    template<size_t _Count>
    bool GSliderDouble(const char* label, double* value, double min, double max)
    {
        return Slider<double, _Count>(label, value, min, max);
    }


    template<size_t _Count>
    bool GInputInt(const char* label, int* value, int step, int fast_step)
    {
        return Input<int, _Count>(label, value, step, fast_step);
    }

    template<size_t _Count>
    bool GInputFloat(const char* label, float* value, float step, float fast_step)
    {
        return Input<float, _Count>(label, value, step, fast_step);
    }

    template<size_t _Count>
    bool GInputDouble(const char* label, double* value, double step, double fast_step)
    {
        return Input<double, _Count>(label, value, step, fast_step);
    }
}