#include "gfx_enum.h"

namespace edge::gfx
{
	template<typename _FlagT>
	std::vector<std::string> serialize_hr_flags(const flags<_FlagT>& flags) {
		std::vector<std::string> flags_vector{ enum_metadata<_FlagT>.size() };
		for (int32_t i = 0; i < enum_metadata<_FlagT>.size(); ++i) {
			if (eq_(flags, enum_metadata<_FlagT>[i].first)) {
				flags_vector[i] = enum_metadata<_FlagT>[i].second;
			}
		}

		return flags_vector;
	}

	template<typename _FlagT>
	flags<_FlagT> deserialize_hr_flags(const std::vector<std::string>& flags_vector) {
		flags<_FlagT> flags{};
		for (int32_t i = 0; i < flags_vector.size(); ++i) {
			flags |= enum_from_string<_FlagT>(flags_vector[i].c_str());
		}
		return flags;
	}


	void to_json(nlohmann::json& json, const ShaderStageFlags& type) { json = serialize_hr_flags(type); }
	void from_json(const nlohmann::json& json, ShaderStageFlags& type) { type = deserialize_hr_flags<ShaderStageFlag>(json.get<std::vector<std::string>>()); }


	void to_json(nlohmann::json& json, const QueueTypeFlags& type) { json = serialize_hr_flags(type); }
	void from_json(const nlohmann::json& json, QueueTypeFlags& type) { type = deserialize_hr_flags<QueueTypeFlag>(json.get<std::vector<std::string>>()); }


	void to_json(nlohmann::json& json, const AccessTypeFlags& type) { json = serialize_hr_flags(type); }
	void from_json(const nlohmann::json& json, AccessTypeFlags& type) { type = deserialize_hr_flags<AccessTypeFlag>(json.get<std::vector<std::string>>()); }


	void to_json(nlohmann::json& json, const BufferUsageFlags& type) { json = serialize_hr_flags(type); }
	void from_json(const nlohmann::json& json, BufferUsageFlags& type) { type = deserialize_hr_flags<BufferUsageFlag>(json.get<std::vector<std::string>>()); }


	void to_json(nlohmann::json& json, const TextureUsageFlags& type) { json = serialize_hr_flags(type); }
	void from_json(const nlohmann::json& json, TextureUsageFlags& type) { type = deserialize_hr_flags<TextureUsageFlag>(json.get<std::vector<std::string>>()); }


	void to_json(nlohmann::json& json, const CullModeFlags& type) { json = serialize_hr_flags(type); }
	void from_json(const nlohmann::json& json, CullModeFlags& type) { type = deserialize_hr_flags<CullModeFlag>(json.get<std::vector<std::string>>()); }


	void to_json(nlohmann::json& json, const ColorMaskFlags& type) { json = serialize_hr_flags(type); }
	void from_json(const nlohmann::json& json, ColorMaskFlags& type) { type = deserialize_hr_flags<ColorMaskFlag>(json.get<std::vector<std::string>>()); }
}