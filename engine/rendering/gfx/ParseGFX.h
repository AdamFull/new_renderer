#pragma once

#include "Structures.h"

namespace engine
{
	namespace gfx
	{
		// ShaderStageFlags
		void to_json(nlohmann::json& json, const ShaderStageFlags& type);
		void from_json(const nlohmann::json& json, ShaderStageFlags& type);
		
		// QueueTypeFlags
		void to_json(nlohmann::json& json, const QueueTypeFlags& type);
		void from_json(const nlohmann::json& json, QueueTypeFlags& type);
		
		// CullModeFlags
		void to_json(nlohmann::json& json, const CullModeFlags& type);
		void from_json(const nlohmann::json& json, CullModeFlags& type);
		
		// ColorMaskFlags
		void to_json(nlohmann::json& json, const ColorMaskFlags& type);
		void from_json(const nlohmann::json& json, ColorMaskFlags& type);


		// Structures

		// RasterizationState
		void to_json(nlohmann::json& json, const RasterizationState& type);
		void from_json(const nlohmann::json& json, RasterizationState& type);
		
		// MultisampleState
		void to_json(nlohmann::json& json, const MultisampleState& type);
		void from_json(const nlohmann::json& json, MultisampleState& type);
		
		// DepthStencilState
		void to_json(nlohmann::json& json, const DepthStencilState& type);
		void from_json(const nlohmann::json& json, DepthStencilState& type);
		
		// ColorBlendAttachment
		void to_json(nlohmann::json& json, const ColorBlendAttachment& type);
		void from_json(const nlohmann::json& json, ColorBlendAttachment& type);
		
		// ColorBlendingState
		void to_json(nlohmann::json& json, const ColorBlendingState& type);
		void from_json(const nlohmann::json& json, ColorBlendingState& type);
	}
}