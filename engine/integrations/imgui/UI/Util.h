#pragma once

#include <imgui.h>

namespace engine
{
	template<typename T>
	struct ImGuiDataTypeByType;

    template<>
    struct ImGuiDataTypeByType<signed char> 
    { static constexpr ImGuiDataType_ value = ImGuiDataType_S8; };

    template<>
    struct ImGuiDataTypeByType<unsigned char> 
    { static constexpr ImGuiDataType_ value = ImGuiDataType_U8; };

    template<>
    struct ImGuiDataTypeByType<short> 
    { static constexpr ImGuiDataType_ value = ImGuiDataType_S16; };

    template<>
    struct ImGuiDataTypeByType<unsigned short> 
    { static constexpr ImGuiDataType_ value = ImGuiDataType_U16; };

    template<>
    struct ImGuiDataTypeByType<int> 
    { static constexpr ImGuiDataType_ value = ImGuiDataType_S32; };

    template<>
    struct ImGuiDataTypeByType<unsigned int> 
    { static constexpr ImGuiDataType_ value = ImGuiDataType_U32; };

    template<>
    struct ImGuiDataTypeByType<long long> 
    { static constexpr ImGuiDataType_ value = ImGuiDataType_S64; };

    template<>
    struct ImGuiDataTypeByType<unsigned long long> 
    { static constexpr ImGuiDataType_ value = ImGuiDataType_U64; };

    template<>
    struct ImGuiDataTypeByType<float>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_Float; };

    template<>
    struct ImGuiDataTypeByType<double>
    { static constexpr ImGuiDataType_ value = ImGuiDataType_Double; };

    template<typename _Ty>
    constexpr ImGuiDataType_ ImGuiDataTypeByType_v = ImGuiDataTypeByType<_Ty>::value;
}