#pragma once

#include <nlohmann/json.hpp>
#include <refl.hpp>
#include <entt/entt.hpp>
#include <array>

namespace engine
{
	using namespace entt::literals;

	namespace conc
	{
		template<typename _Ty>
		concept Reflectable = std::is_class_v<_Ty> && refl::is_reflectable<_Ty>();
	}

	namespace attr
	{
		struct Hide : refl::attr::usage::type, refl::attr::usage::field, refl::attr::usage::function {};
		struct Name : refl::attr::usage::type, refl::attr::usage::field {
			constexpr Name(std::string_view name) :
				name{ name } {}
			std::string_view name{};
		};
		struct Hint : refl::attr::usage::type, refl::attr::usage::field, refl::attr::usage::function {
			constexpr Hint(std::string_view text) :
				text{ text } {}
			std::string_view text{};
		};

		struct ReadOnly : refl::attr::usage::type, refl::attr::usage::field {
			constexpr ReadOnly(std::string_view format) :
				format{ format } {}
			std::string_view format{};
		};

		template<typename _Ty>
		struct Range : refl::attr::usage::field, refl::attr::usage::function
		{
			constexpr Range() {}
			constexpr Range(_Ty min, _Ty max) : min(min), max(max) {}
			_Ty min;
			_Ty max;
		};

		struct Slider : refl::attr::usage::type, refl::attr::usage::field, refl::attr::usage::function {};
		struct Input : refl::attr::usage::type, refl::attr::usage::field, refl::attr::usage::function {};

		struct ColorEdit : refl::attr::usage::field, refl::attr::usage::function {};

		template<size_t N>
		struct Dropdown : refl::attr::usage::field, refl::attr::usage::function { std::array<const char*, N> fields; };
	}

	template<typename _Ty>
	void Serialize(nlohmann::json& j, const _Ty& obj)
	{
		for_each(refl::reflect(obj).members, [&j, &obj](auto member) {
				if constexpr (is_readable(member)) {
					const char* name = get_display_name(member);
					if constexpr (refl::is_reflectable<_Ty>())
						Serialize(j[name], obj);
					else
						j[name] = member(obj);
				}
			});
	}

	template<typename _Ty>
	void Deserialize(const nlohmann::json& j, _Ty& obj)
	{
		for_each(refl::reflect(obj).members, [&j, &obj](auto member)  {
				if constexpr (is_writable(member)) {
					const char* name = get_display_name(member);
					if constexpr (refl::is_reflectable<_Ty>())
						Deserialize(j.at(name), obj);
					else {
						if (j.contains(name))
							member(obj) = j.at(name).get<std::decay_t<decltype(member(obj))>>();
					}
				}
			});
	}
}