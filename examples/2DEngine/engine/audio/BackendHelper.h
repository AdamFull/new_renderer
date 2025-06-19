#pragma once

#include <type_traits>
#include <utility>

struct ALCdevice;

#define alCall(function, ...) e2d::audio::al_call_impl(function, __VA_ARGS__)
#define alcCall(function, device, ...) e2d::audio::alc_call_impl(function, device, __VA_ARGS__)

namespace e2d::audio {
	auto check_al_errors() -> bool;

    template<typename alFunction, typename... Params>
    auto al_call_impl(alFunction function, Params... params)
        -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))> {
        auto ret = function(std::forward<Params>(params)...);
        check_al_errors();
        return ret;
    }

    template<typename alFunction, typename... Params>
    auto al_call_impl(alFunction function, Params... params)
        -> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool> {
        function(std::forward<Params>(params)...);
        return check_al_errors();
    }

    auto check_alc_errors(ALCdevice* device) -> bool;

    template<typename alcFunction, typename... Params>
    auto alc_call_impl(alcFunction function,
        ALCdevice* device, Params... params)
        -> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool> {
        function(std::forward<Params>(params)...);
        return check_alc_errors(device);
    }

    template<typename alcFunction, typename ReturnType, typename... Params>
    auto alc_call_impl(alcFunction function, ReturnType& return_value,
        ALCdevice* device, Params... params)
        -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, bool> {
        return_value = function(std::forward<Params>(params)...);
        return check_alc_errors(device);
    }
}