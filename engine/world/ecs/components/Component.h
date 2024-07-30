#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <nlohmann/json.hpp>
#include <standart_library.h>

#include <entt/entt.hpp>

#include <engine/integrations/imgui/ImGuiElements.h>

#include <tuple>
#include <type_traits>
#include <typeinfo>

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

#define DECLARE_COMPONENT(clazz) \
	using _Self = clazz;

#define DECLARE_FIELDS(...) \
	bool IsSerializable() override { return true; } \
    static constexpr auto fields() { return std::make_tuple(__VA_ARGS__); } \
	void Serialize(nlohmann::json& j) override { SerializeFields(j, *this); } \
	void Deserialize(const nlohmann::json& j) override { DeserializeFields(j, *this); } \
	void OnEdit() override { DrawFields(*this); }

#define FDECL(name, meta) std::make_tuple(&_Self::name, #name, meta)
	

struct NoEdit {};

template<typename _Ty>
struct SliderField
{
	_Ty min;
	_Ty max;
	_Ty step;
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

template<typename _Ty = float>
struct ColorEditField {};

struct ToggleField {};

template<typename T>
struct ElementCount { static constexpr std::size_t value = 1; };

template<>
struct ElementCount<glm::vec2> { static constexpr std::size_t value = 2; };
template<>
struct ElementCount<glm::vec3> { static constexpr std::size_t value = 3; };
template<>
struct ElementCount<glm::vec4> { static constexpr std::size_t value = 4; };


template <typename T>
const char* GetFieldName(const T& field) 
{
	return std::get<1>(field);
}

template <typename T, typename U, typename Meta>
U T::* GetFieldPtr(const std::tuple<U T::*, const char*, Meta>& field)
{
	return std::get<0>(field);
}

template <typename T>
constexpr auto GetFieldMeta(const T& field)
{
	return std::get<2>(field);
}

template <typename T, std::size_t... I>
void SerializeFieldsImpl(nlohmann::json& j, const T& obj, std::index_sequence<I...>) 
{
	((j[GetFieldName(std::get<I>(T::fields()))] = obj.*GetFieldPtr(std::get<I>(T::fields()))), ...);
}

template <typename T>
void SerializeFields(nlohmann::json& j, const T& obj) 
{
	SerializeFieldsImpl(j, obj, std::make_index_sequence<std::tuple_size_v<decltype(T::fields())>>{});
}

template <typename T, std::size_t... I>
void DeserializeFieldsImpl(const nlohmann::json& j, T& obj, std::index_sequence<I...>) 
{
	((j.at(GetFieldName(std::get<I>(T::fields()))).get_to(obj.*GetFieldPtr(std::get<I>(T::fields())))), ...);
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
	auto field_name = GetFieldName(field);
	using ValueType = std::decay_t<decltype(std::declval<T>().*field_ptr)>;
	ImGui::Slider<U, ElementCount<ValueType>::value>(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)), editor.min, editor.max, ImGui::FormatByType_v<U>);
}

template <typename T, typename Field, typename U>
void DrawFieldImpl(T& obj, const Field& field, const DragField<U>& editor) 
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetFieldName(field);
	using ValueType = std::decay_t<decltype(std::declval<T>().*field_ptr)>;
	ImGui::Drag<U, ElementCount<ValueType>::value>(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)), editor.min, editor.max, editor.speed, ImGui::FormatByType_v<U>);
}

template <typename T, typename Field, typename U>
void DrawFieldImpl(T& obj, const Field& field, const InputField<U>& editor) 
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetFieldName(field);
	using ValueType = std::decay_t<decltype(std::declval<T>().*field_ptr)>;
	ImGui::Input<U, ElementCount<ValueType>::value>(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)), editor.step, editor.fast_step, ImGui::FormatByType_v<U>);
}

template <typename T, typename Field, typename U>
void DrawFieldImpl(T& obj, const Field& field, const ColorEditField<U>& editor)
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetFieldName(field);
	using ValueType = std::decay_t<decltype(std::declval<T>().*field_ptr)>;
	if constexpr (ElementCount<ValueType>::value == 3)
		ImGui::ColorEdit3(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)));
	else if (ElementCount<ValueType>::value == 4)
		ImGui::ColorEdit4(field_name, reinterpret_cast<U*>(&(obj.*field_ptr)));
	else
		static_assert(false, "Unknown control type for color edit.");
}

template <typename T, typename Field>
void DrawFieldImpl(T& obj, const Field& field, const ToggleField& editor)
{
	auto field_ptr = GetFieldPtr(field);
	auto field_name = GetFieldName(field);
	ImGui::GCheckbox(field_name, &(obj.*field_ptr));
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

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct IComponent
			{
				virtual void Serialize(nlohmann::json& j) {}
				virtual void Deserialize(const nlohmann::json& j) {}

				virtual void OnEdit() {}

				virtual bool IsSerializable() { return false; }
			};
		}
	}
}