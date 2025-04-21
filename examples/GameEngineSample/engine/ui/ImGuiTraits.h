#pragma once

#include <imgui.h>

namespace ImGui
{
    template<typename> constexpr ImGuiDataType DataTypeEnum = ImGuiDataType_COUNT;
    template<> constexpr inline ImGuiDataType DataTypeEnum<int8_t> = ImGuiDataType_S8;
    template<> constexpr inline ImGuiDataType DataTypeEnum<uint8_t> = ImGuiDataType_U8;
    template<> constexpr inline ImGuiDataType DataTypeEnum<int16_t> = ImGuiDataType_S16;
    template<> constexpr inline ImGuiDataType DataTypeEnum<uint16_t> = ImGuiDataType_U16;
    template<> constexpr inline ImGuiDataType DataTypeEnum<int32_t> = ImGuiDataType_S32;
    template<> constexpr inline ImGuiDataType DataTypeEnum<uint32_t> = ImGuiDataType_U32;
    template<> constexpr inline ImGuiDataType DataTypeEnum<int64_t> = ImGuiDataType_S64;
    template<> constexpr inline ImGuiDataType DataTypeEnum<uint64_t> = ImGuiDataType_U64;
    template<> constexpr inline ImGuiDataType DataTypeEnum<float> = ImGuiDataType_Float;
    template<> constexpr inline ImGuiDataType DataTypeEnum<double> = ImGuiDataType_Double;

    template<typename> constexpr const char* DataTypeFormat = "";
    template<> constexpr inline const char* DataTypeFormat<int8_t> = "%hhd";
    template<> constexpr inline const char* DataTypeFormat<uint8_t> = "%hhu";
    template<> constexpr inline const char* DataTypeFormat<int16_t> = "%hd";
    template<> constexpr inline const char* DataTypeFormat<uint16_t> = "%hu";
    template<> constexpr inline const char* DataTypeFormat<int32_t> = "%d";
    template<> constexpr inline const char* DataTypeFormat<uint32_t> = "%u";
    template<> constexpr inline const char* DataTypeFormat<int64_t> = "%lld";
    template<> constexpr inline const char* DataTypeFormat<uint64_t> = "%llu";
    template<> constexpr inline const char* DataTypeFormat<float> = "%.3f";
    template<> constexpr inline const char* DataTypeFormat<double> = "%.6f";
}