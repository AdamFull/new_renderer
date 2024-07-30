#pragma once

#include <imgui.h>

namespace ImGui
{
    template<typename _Ty>
    struct DataTypeByType;

    template<>
    struct DataTypeByType<signed char>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_S8; };

    template<>
    struct DataTypeByType<unsigned char>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_U8; };

    template<>
    struct DataTypeByType<short>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_S16; };

    template<>
    struct DataTypeByType<unsigned short>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_U16; };

    template<>
    struct DataTypeByType<int>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_S32; };

    template<>
    struct DataTypeByType<unsigned int>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_U32; };

    template<>
    struct DataTypeByType<long long>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_S64; };

    template<>
    struct DataTypeByType<unsigned long long>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_U64; };

    template<>
    struct DataTypeByType<float>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_Float; };

    template<>
    struct DataTypeByType<double>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_Double; };

    template<typename _Ty>
    constexpr ImGuiDataType_ DataTypeByType_v = DataTypeByType<_Ty>::value;


    template<typename _Ty>
    struct FormatByType;

    template<>
    struct FormatByType<signed char>
    {
        static constexpr const char* value = "%hhd";
    };

    template<>
    struct FormatByType<unsigned char>
    {
        static constexpr const char* value = "%hhu";
    };

    template<>
    struct FormatByType<short>
    {
        static constexpr const char* value = "%hd";
    };

    template<>
    struct FormatByType<unsigned short>
    {
        static constexpr const char* value = "%hu";
    };

    template<>
    struct FormatByType<int>
    {
        static constexpr const char* value = "%d";
    };

    template<>
    struct FormatByType<unsigned int>
    {
        static constexpr const char* value = "%u";
    };

    template<>
    struct FormatByType<long>
    {
        static constexpr const char* value = "%ld";
    };

    template<>
    struct FormatByType<unsigned long>
    {
        static constexpr const char* value = "%lu";
    };


    template<>
    struct FormatByType<long long>
    {
        static constexpr const char* value = "%lld";
    };

    template<>
    struct FormatByType<unsigned long long>
    {
        static constexpr const char* value = "%llu";
    };

    template<>
    struct FormatByType<float>
    {
        static constexpr const char* value = "%.3f";
    };

    template<>
    struct FormatByType<double>
    {
        static constexpr const char* value = "%.6f";
    };

    template<typename _Ty>
    constexpr const char* FormatByType_v = FormatByType<_Ty>::value;
}