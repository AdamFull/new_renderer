#pragma once

#include <nlohmann/json.hpp>
#include <refl.hpp>

namespace engine
{
	namespace conc
	{
		template<typename _Ty>
		concept Reflectable = std::is_class_v<_Ty> && refl::is_reflectable<_Ty>();


		template<typename _Ty>
		concept Serializable = requires(nlohmann::json & j, const _Ty & t) 
		{
			{ to_json(j, t) } -> std::same_as<void>;
			{ from_json(j, std::declval<_Ty&>()) } -> std::same_as<void>;
		};
	}

	namespace attr
	{
		struct HiddenField : refl::attr::usage::type, refl::attr::usage::field, refl::attr::usage::function {};

		template<typename _Ty>
		struct EditableField : refl::attr::usage::field, refl::attr::usage::function
		{
			constexpr EditableField(_Ty min, _Ty max) : min(min), max(max) {}
			_Ty min;
			_Ty max;
		};

		template<typename _Ty>
		struct SliderField : EditableField<_Ty> 
		{
			constexpr SliderField(_Ty min, _Ty max) : EditableField<_Ty>(min, max) {}
		};

		template<typename _Ty>
		struct DragField : EditableField<_Ty> 
		{
			constexpr DragField(_Ty min, _Ty max) : EditableField<_Ty>(min, max) {}
		};

		template<typename _Ty>
		struct InputField : EditableField<_Ty> 
		{
			constexpr InputField(_Ty min, _Ty max) : EditableField<_Ty>(min, max) {}
		};

		struct ColorField : refl::attr::usage::field, refl::attr::usage::function {};
		struct ToggleField : refl::attr::usage::field, refl::attr::usage::function {};

		template<size_t N>
		struct DropdownField : refl::attr::usage::field, refl::attr::usage::function { std::array<const char*, N> fields; };
	}
}