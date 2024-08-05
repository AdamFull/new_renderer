#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <nlohmann/json.hpp>
#include <standart_library.h>

#include <entt/entt.hpp>

#include <engine/integrations/imgui/font/IconsMaterialDesignIcons.h>
#include <engine/integrations/imgui/ImGuiElements.h>

#include <tuple>
#include <type_traits>
#include <typeinfo>

#include <array>
#include <string_view>
#include <utility>

#include <engine/integrations/imgui/Attributes.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct IComponent
			{
				virtual void Serialize(nlohmann::json& j) const {}
				virtual void Deserialize(const nlohmann::json& j) {}

				virtual void OnEdit() {}
				virtual const char* GetName() { return nullptr; }
			};
		}
	}
}

namespace impl
{
	/// Base declaration of our constexpr string_view concatenation helper
	template <std::string_view const&, typename, std::string_view const&, typename>
	struct concat;

	/// Specialisation to yield indices for each char in both provided string_views,
	/// allows us flatten them into a single char array
	template <std::string_view const& S1,
		std::size_t... I1,
		std::string_view const& S2,
		std::size_t... I2>
	struct concat<S1, std::index_sequence<I1...>, S2, std::index_sequence<I2...>>
	{
		static constexpr const char value[]{ S1[I1]..., S2[I2]..., 0 };
	};
} // namespace impl

/// Base definition for compile time joining of strings
template <std::string_view const&...> struct join;

/// When no strings are given, provide an empty literal
template <>
struct join<>
{
	static constexpr std::string_view value = "";
};

/// Base case for recursion where we reach a pair of strings, we concatenate
/// them to produce a new constexpr string
template <std::string_view const& S1, std::string_view const& S2>
struct join<S1, S2>
{
	static constexpr std::string_view value =
		impl::concat<S1,
		std::make_index_sequence<S1.size()>,
		S2,
		std::make_index_sequence<S2.size()>>::value;
};

/// Main recursive definition for constexpr joining, pass the tail down to our
/// base case specialisation
template <std::string_view const& S, std::string_view const&... Rest>
struct join<S, Rest...>
{
	static constexpr std::string_view value =
		join<S, join<Rest...>::value>::value;
};

/// Join constexpr string_views to produce another constexpr string_view
template <std::string_view const&... Strs>
static constexpr auto join_v = join<Strs...>::value;

namespace icon
{
	constexpr std::string_view entity{ ICON_MDI_GRAPH };
	constexpr std::string_view transform{ ICON_MDI_AXIS }; /*ICON_MDI_VECTOR_LINE*/
	constexpr std::string_view spot_light{ ICON_MDI_HEAD_LIGHTBULB };
	constexpr std::string_view point_light{ ICON_MDI_LIGHTBULB };
	constexpr std::string_view directional_light{ ICON_MDI_BRIGHTNESS_7 };
	constexpr std::string_view camera{ ICON_MDI_CAMERA };
	constexpr std::string_view shape{ ICON_MDI_SHAPE };
	constexpr std::string_view audio{ ICON_MDI_VOLUME_HIGH };
	constexpr std::string_view script{ ICON_MDI_SCRIPT };
	constexpr std::string_view environment{ ICON_MDI_EARTH };
}

#define REGISTER_GLM_MATRIX_SERIALIZATION(MatrixType) \
inline void to_json(nlohmann::json& j, const MatrixType& mat) { \
	j = nlohmann::json::array(); \
	for (int i = 0; i < mat.length(); ++i) \
		for (int k = 0; k < mat[i].length(); ++k) \
			j.push_back(mat[i][k]); \
} \
inline void from_json(const nlohmann::json& j, MatrixType& mat) { \
	int index = 0; \
	for (int i = 0; i < mat.length(); ++i) \
		for (int k = 0; k < mat[i].length(); ++k) \
			mat[i][k] = j.at(index++); \
}

namespace glm
{
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ivec2, x, y);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec2, x, y);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dvec2, x, y);

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ivec3, x, y, z);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec3, x, y, z);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dvec3, x, y, z);

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ivec4, x, y, z, w);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec4, x, y, z, w);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dvec4, x, y, z, w);

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(quat, w, x, y, z);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dquat, w, x, y, z);

	REGISTER_GLM_MATRIX_SERIALIZATION(mat2);
	REGISTER_GLM_MATRIX_SERIALIZATION(mat3);
	REGISTER_GLM_MATRIX_SERIALIZATION(mat4);
}

#define DECLARE_COMPONENT(clazz, clazz_ico, clazz_name) \
	using _Self = clazz; \
	static constexpr std::string_view _NameRaw = std::string_view(clazz_name); \
	static constexpr std::string_view _Name = join_v<clazz_ico, _NameRaw>;

#define DECLARE_FIELDS(...) \
    static constexpr auto fields() { return std::make_tuple(__VA_ARGS__); } \
	void Serialize(nlohmann::json& j) const override { SerializeFields(j, *this); } \
	void Deserialize(const nlohmann::json& j) override { DeserializeFields(j, *this); } \
	void OnEdit() override { DrawFields(*this); } \
	const char* GetName() override { return _Name.data(); }

#define FDECL(member, name, meta) std::make_tuple(&_Self::member, #member, name, meta)
	

struct NoEdit {};

template<typename _Ty>
struct SliderField
{
	_Ty step;
	_Ty min;
	_Ty max;
};

template<typename _Ty>
struct DragField
{
	_Ty speed;
	_Ty min;
	_Ty max;
};

template<typename _Ty>
struct InputField
{
	_Ty step;
	_Ty fast_step;
};

template<size_t N>
struct DropdownField
{
	std::array<const char*, N> fields;
};

template<typename _Ty = float>
struct ColorEditField {};

struct ToggleField {};

template<typename _Ty>
struct ContainerField {};

template<typename T>
struct ElementCount { static constexpr std::size_t value = 1; };

template<>
struct ElementCount<glm::vec2> { static constexpr std::size_t value = 2; };
template<>
struct ElementCount<glm::vec3> { static constexpr std::size_t value = 3; };
template<>
struct ElementCount<glm::vec4> { static constexpr std::size_t value = 4; };
template<>
struct ElementCount<glm::quat> { static constexpr std::size_t value = 4; };

template<typename T, typename _ = void>
struct is_container : std::false_type {};

template<typename... Ts>
struct is_container_helper {};

template<typename T>
struct is_container<T, std::conditional_t<
	false,
	is_container_helper<typename T::value_type, decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>,
	void
	>> : public std::true_type {};

template<typename T>
inline constexpr bool is_container_v = is_container<T>::value;

template <typename T>
constexpr auto GetFieldPtr(const T& field)
{
	return std::get<0>(field);
}

template <typename T>
const char* GetFieldName(const T& field) 
{
	return std::get<1>(field);
}

template <typename T>
const char* GetEditorName(const T& field)
{
	return std::get<2>(field);
}

template <typename T>
constexpr auto GetFieldMeta(const T& field)
{
	return std::get<3>(field);
}

// SerializeField function
template<typename FieldType>
void SerializeField(nlohmann::json& j, const char* fieldName, const FieldType& field) 
{
	if constexpr (std::is_base_of_v<engine::world::ecs::IComponent, FieldType>)
	{
		nlohmann::json componentJson;
		field.Serialize(componentJson);
		j[fieldName] = componentJson;
	}
	else if constexpr (is_container_v<FieldType>)
	{
		nlohmann::json containerJson = nlohmann::json::array();
		for (const auto& item : field) 
		{
			nlohmann::json itemJson;
			SerializeField(itemJson, "", item);
			containerJson.push_back(itemJson);
		}

		j[fieldName] = containerJson;
	}
	else
		j[fieldName] = field;
}

template <typename T, std::size_t... I>
void SerializeFieldsImpl(nlohmann::json& j, const T& obj, std::index_sequence<I...>) 
{
	((SerializeField(j, GetFieldName(std::get<I>(T::fields())), obj.*GetFieldPtr(std::get<I>(T::fields())))), ...);
}

template <typename T>
void SerializeFields(nlohmann::json& j, const T& obj) 
{
	SerializeFieldsImpl(j, obj, std::make_index_sequence<std::tuple_size_v<decltype(T::fields())>>{});
}

template<typename FieldType>
void DeserializeField(const nlohmann::json& j, const char* fieldName, FieldType& field) 
{
	if constexpr (std::is_base_of_v<engine::world::ecs::IComponent, FieldType>) 
	{
		if (j.contains(fieldName))
			field.Deserialize(j.at(fieldName));
	}
	else if constexpr (is_container_v<FieldType>) 
	{
		// If the field is a container
		if (j.contains(fieldName)) 
		{
			const auto& containerJson = j.at(fieldName);
			field.clear();
			for (const auto& itemJson : containerJson) 
			{
				typename FieldType::value_type item;
				DeserializeField(itemJson, "", item);
				field.push_back(item);
			}
		}
	}
	else 
	{
		if (j.contains(fieldName))
			j.at(fieldName).get_to(field);
	}
}

template <typename T, std::size_t... I>
void DeserializeFieldsImpl(const nlohmann::json& j, T& obj, std::index_sequence<I...>) 
{
	((DeserializeField(j, GetFieldName(std::get<I>(T::fields())), obj.*GetFieldPtr(std::get<I>(T::fields())))), ...);
}

template <typename T>
void DeserializeFields(const nlohmann::json& j, T& obj) 
{
	DeserializeFieldsImpl(j, obj, std::make_index_sequence<std::tuple_size_v<decltype(T::fields())>>{});
}



// Tag dispatching for drawing fields
template <typename T, typename Field, typename Editor>
void DrawFieldImpl(T& obj, const Field& field, const Editor& editor) 
{
	// Do nothing for non-editable fields
}

template <typename T, typename Field, typename U>
void DrawFieldImpl(T& obj, const Field& field, const SliderField<U>& editor) 
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetEditorName(field);
	using ValueType = std::decay_t<decltype(std::declval<T>().*field_ptr)>;
	ImGui::Slider<U, ElementCount<ValueType>::value>(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)), editor.min, editor.max, ImGui::FormatByType_v<U>);
}

template <typename T, typename Field, typename U>
void DrawFieldImpl(T& obj, const Field& field, const DragField<U>& editor) 
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetEditorName(field);
	using ValueType = std::decay_t<decltype(std::declval<T>().*field_ptr)>;
	ImGui::Drag<U, ElementCount<ValueType>::value>(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)), editor.min, editor.max, editor.speed, ImGui::FormatByType_v<U>);
}

template <typename T, typename Field, typename U>
void DrawFieldImpl(T& obj, const Field& field, const InputField<U>& editor) 
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetEditorName(field);
	using ValueType = std::decay_t<decltype(std::declval<T>().*field_ptr)>;
	ImGui::Input<U, ElementCount<ValueType>::value>(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)), editor.step, editor.fast_step, ImGui::FormatByType_v<U>);
}

template <typename T, typename Field, typename U>
void DrawFieldImpl(T& obj, const Field& field, const ColorEditField<U>& editor)
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetEditorName(field);
	using ValueType = std::decay_t<decltype(std::declval<T>().*field_ptr)>;
	if constexpr (ElementCount<ValueType>::value == 3)
		ImGui::ColorEdit3(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)));
	else if (ElementCount<ValueType>::value == 4)
		ImGui::ColorEdit4(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)));
	else
		static_assert(false, "Unknown control type for color edit.");
}

template <typename T, typename Field, size_t N>
void DrawFieldImpl(T& obj, const Field& field, const DropdownField<N>& editor)
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetEditorName(field);

	int* current = reinterpret_cast<int*>(&(obj.*field_ptr));
	const char* preview = editor.fields[*current];
	
	if(ImGui::GCombo(field_name, current, editor.fields.data(), editor.fields.size()));
}

template <typename T, typename Field>
void DrawFieldImpl(T& obj, const Field& field, const ToggleField& editor)
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetEditorName(field);
	ImGui::GCheckbox(field_name, &(obj.*field_ptr));
}

template <typename T, typename Field, typename U>
void DrawFieldImpl(T& obj, const Field& field, const ContainerField<U>& editor)
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetEditorName(field);
	using ValueType = std::decay_t<decltype(std::declval<T>().*field_ptr)>;

	ImGui::DrawContainerEdit(field_name, obj.*field_ptr);
}

template <typename T, typename Field>
void DrawField(T& obj, const Field& field) 
{
	auto editor_config = GetFieldMeta(field);
	DrawFieldImpl(obj, field, editor_config);
}

template <typename T, std::size_t... I>
void DrawFieldsImpl(T& obj, std::index_sequence<I...>) 
{
	((DrawField(obj, std::get<I>(T::fields()))), ...);
}

template <typename T>
void DrawFields(T& obj) 
{
	DrawFieldsImpl(obj, std::make_index_sequence<std::tuple_size_v<decltype(T::fields())>>{});
}


template<typename _Ty>
void SerializeField(const char* field, nlohmann::json& j, const _Ty& value, const _Ty default_value = _Ty{})
{
	if (value != default_value)
		j[field] = value;
}

template<typename _Ty>
void SerializeFields(nlohmann::json& j, const _Ty& field, const char* fieldName)
{
	SerializeField(fieldName, j, field);
}

template<typename _Ty, typename... _Args>
void SerializeFields(nlohmann::json& j, const _Ty& field, const char* fieldName, _Args... args)
{
	SerializeField(fieldName, j, field);
	SerializeFields(j, args...);
}

template<typename _Ty>
void DeserializeField(const char* field, const nlohmann::json& j, _Ty& value, bool expected = false)
{
	if (auto obj = j.find(field); obj != j.end())
		obj->get_to(value);
	else if (expected)
		throw std::runtime_error("Expected field was not found!");
}

template<typename _Ty>
void DeserializeFields(const nlohmann::json& j, _Ty& field, const char* fieldName)
{
	DeserializeField(fieldName, j, field);
}

template<typename _Ty, typename... _Args>
void DeserializeFields(const nlohmann::json& j, _Ty& field, const char* fieldName, _Args... args)
{
	DeserializeField(fieldName, j, field);
	DeserializeFields(j, args...);
}