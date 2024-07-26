#pragma once

#include <imgui.h>

namespace ImGui
{
    template<typename _Ty>
    struct DataTypeByType;

    template<>
    struct DataTypeByType<signed char>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_S8;
    };

    template<>
    struct DataTypeByType<unsigned char>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_U8;
    };

    template<>
    struct DataTypeByType<short>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_S16;
    };

    template<>
    struct DataTypeByType<unsigned short>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_U16;
    };

    template<>
    struct DataTypeByType<int>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_S32;
    };

    template<>
    struct DataTypeByType<unsigned int>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_U32;
    };

    template<>
    struct DataTypeByType<long long>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_S64;
    };

    template<>
    struct DataTypeByType<unsigned long long>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_U64;
    };

    template<>
    struct DataTypeByType<float>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_Float;
    };

    template<>
    struct DataTypeByType<double>
    {
        static constexpr ImGuiDataType_ value = ImGuiDataType_Double;
    };

    template<typename _Ty>
    constexpr ImGuiDataType_ DataTypeByType_v = DataTypeByType<_Ty>::value;
}