#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <standart_library.h>

#include <entt/entt.hpp>

#include <engine/integrations/imgui/font/IconsMaterialDesignIcons.h>
#include <engine/integrations/imgui/ImGuiElements.h>

#include <utility>

#include <engine/integrations/imgui/Attributes.h>
#include <string.hpp>

#include <enum.hpp>

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
	static constexpr std::string_view _Name = utl::join_v<clazz_ico, _NameRaw>;