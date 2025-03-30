#pragma once

#include <nlohmann/json.hpp>
#include <refl.hpp>
#include <entt/entt.hpp>
#include <array>

namespace engine
{
	using namespace entt::literals;

	namespace traits
	{
		template<typename T>
		struct is_vector : std::false_type {};

		template<typename T, typename A>
		struct is_vector<std::vector<T, A>> : std::true_type {};

		template<typename T>
		struct is_map : std::false_type {};

		template<typename K, typename V, typename C, typename A>
		struct is_map<std::map<K, V, C, A>> : std::true_type {};

		template<typename K, typename V, typename H, typename E, typename A>
		struct is_map<std::unordered_map<K, V, H, E, A>> : std::true_type {};
	}

	namespace conc
	{
		template<typename _Ty>
		concept Reflectable = std::is_class_v<_Ty> && refl::is_reflectable<_Ty>();
	}

	namespace attr
	{
		struct HiddenField : refl::attr::usage::type, refl::attr::usage::field, refl::attr::usage::function {};

		template<typename _Ty>
		struct Range : refl::attr::usage::field, refl::attr::usage::function
		{
			constexpr Range() {}
			constexpr Range(_Ty min, _Ty max) : min(min), max(max) {}
			_Ty min;
			_Ty max;
		};

		struct SliderField : refl::attr::usage::type, refl::attr::usage::field, refl::attr::usage::function {};
		struct InputField : refl::attr::usage::type, refl::attr::usage::field, refl::attr::usage::function {};

		struct ColorField : refl::attr::usage::field, refl::attr::usage::function {};

		template<size_t N>
		struct DropdownField : refl::attr::usage::field, refl::attr::usage::function { std::array<const char*, N> fields; };
	}

	template<typename _Ty>
	void Serialize(nlohmann::json& j, const _Ty& obj)
	{
		for_each(refl::reflect(obj).members, [&j, &obj](auto member) 
			{
				if constexpr (is_readable(member))
				{
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
		for_each(refl::reflect(obj).members, [&j, &obj](auto member) 
			{
				if constexpr (is_writable(member))
				{
					const char* name = get_display_name(member);
					if constexpr (refl::is_reflectable<_Ty>())
						Deserialize(j.at(name), obj);
					else
					{
						if (j.contains(name))
							member(obj) = j.at(name).get<std::decay_t<decltype(member(obj))>>();
					}
				}
			});
	}
}