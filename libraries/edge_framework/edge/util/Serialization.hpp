#pragma once

#include "../../external/nlohmann/json.hpp"
//#include <charconv>

namespace edge
{
    template<class _Ty>
    inline void parse_from(const std::string& name, const nlohmann::json& json, _Ty& type, bool required = false)
    {
        if (auto obj = json.find(name); obj != json.end())
            type = obj->get<_Ty>();
        else
        {
            if (required)
                throw std::runtime_error("Required argument was not found");
        }
    }

    template<class _Ty>
    inline void serialize_to(const std::string& name, nlohmann::json& json, const _Ty& type, bool condition = true)
    {
        if (condition)
            json[name] = type;
    }

    //// Wstrings serialization
    //inline void to_json(nlohmann::json& j, const std::wstring& wstr)
    //{
    //    if (!wstr.empty())
    //        j = to_utf8(wstr);
    //}
    //
    //inline void from_json(const nlohmann::json& j, std::wstring& wstr)
    //{
    //    wstr = from_utf8<wchar_t>(j.get<std::string>());
    //}
    //
    //inline void to_json(nlohmann::json& j, const std::u16string& wstr)
    //{
    //    if (!wstr.empty())
    //        j = to_utf8(wstr);
    //}
    //
    //inline void from_json(const nlohmann::json& j, std::u16string& wstr)
    //{
    //    wstr = from_utf8<char16_t>(j.get<std::string>());
    //}
    //
    //inline void to_json(nlohmann::json& j, const std::u32string& wstr)
    //{
    //    if (!wstr.empty())
    //        j = to_utf8(wstr);
    //}
    //
    //inline void from_json(const nlohmann::json& j, std::u32string& wstr)
    //{
    //    wstr = from_utf8<char32_t>(j.get<std::string>());
    //}
    //
    //// id string serialization
    //template<typename _HashType>
    //inline void to_json(nlohmann::json& j, const id_string<char, _HashType>& idStr)
    //{
    //    if (idStr.valid())
    //        j = std::format("{}#{}", idStr.str(), idStr.hash());
    //}
    //
    //// Function to convert JSON to id_string
    //template<typename _HashType>
    //inline void from_json(const nlohmann::json& j, id_string<char, _HashType>& idStr)
    //{
    //    //using id_string_t = id_string<char, _HashType>;
    //    //using string_t = id_string_t::value_type;
    //    //
    //    //string_t::value_type serialized = j.get<string_t>();
    //    //auto pos = serialized.find('#');
    //    //if (pos != string_t::npos) 
    //    //{
    //    //    string_t name = serialized.substr(0, pos);
    //    //    _HashType hash;
    //    //    auto [ptr, ec] = std::from_chars(serialized.data() + pos + 1, serialized.data() + serialized.size(), hash);
    //    //    if (ec == std::errc())
    //    //        idStr = id_string_t(name, hash);
    //    //}
    //}
}