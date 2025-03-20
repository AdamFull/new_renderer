#include "gfx_vk_debug.h"
#include "gfx_vk_device.h"

namespace edge::gfx::vulkan
{
	inline std::array<float, 4> make_color(uint32_t color) {
		std::array<float, 4> out_color{};
		out_color[0] = ((color >> 24) & 0xFF) / 255.0f;
		out_color[1] = ((color >> 16) & 0xFF) / 255.0f;
		out_color[2] = ((color >> 8) & 0xFF) / 255.0f;
		out_color[3] = (color & 0xFF) / 255.0f;

		return out_color;
	}

	DebugUtils::DebugUtils(Device& device) :
		device{ device } {
	}

	DebugUtilsExtDebugUtils::DebugUtilsExtDebugUtils(Device& device) :
		DebugUtils{ device }
	{
	}

	void DebugUtilsExtDebugUtils::set_debug_name(vk::ObjectType object_type, uint64_t object_handle, std::string_view name) const {
		vk::DebugUtilsObjectNameInfoEXT name_info(object_type, object_handle, name.data());
		device.get_handle().setDebugUtilsObjectNameEXT(name_info);
	}

	void DebugUtilsExtDebugUtils::set_debug_tag(vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const {
		vk::DebugUtilsObjectTagInfoEXT tag_info(object_type, object_handle, tag_name, tag_data_size, tag_data);
		device.get_handle().setDebugUtilsObjectTagEXT(tag_info);
	}

	void DebugUtilsExtDebugUtils::begin_label(vk::CommandBuffer command_buffer, std::string_view name, uint32_t color) const {
		vk::DebugUtilsLabelEXT label_info(name.data(), make_color(color));
		command_buffer.beginDebugUtilsLabelEXT(label_info);
	}

	void DebugUtilsExtDebugUtils::end_label(vk::CommandBuffer command_buffer) const {
		command_buffer.endDebugUtilsLabelEXT();
	}

	void DebugUtilsExtDebugUtils::insert_label(vk::CommandBuffer command_buffer, std::string_view name, uint32_t color) const {
		vk::DebugUtilsLabelEXT label_info(name.data(), make_color(color));
		command_buffer.insertDebugUtilsLabelEXT(label_info);
	}


	DebugMarkerExtDebugUtils::DebugMarkerExtDebugUtils(Device& device) :
		DebugUtils{ device }
	{
	}

	void DebugMarkerExtDebugUtils::set_debug_name(vk::ObjectType object_type, uint64_t object_handle, std::string_view name) const {
		vk::DebugMarkerObjectNameInfoEXT name_info(vk::debugReportObjectType(object_type), object_handle, name.data());
		device.get_handle().debugMarkerSetObjectNameEXT(name_info);
	}

	void DebugMarkerExtDebugUtils::set_debug_tag(vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const {
		vk::DebugMarkerObjectTagInfoEXT tag_info(vk::debugReportObjectType(object_type), object_handle, tag_name, tag_data_size, tag_data);
		device.get_handle().debugMarkerSetObjectTagEXT(tag_info);
	}

	void DebugMarkerExtDebugUtils::begin_label(vk::CommandBuffer command_buffer, std::string_view name, uint32_t color) const {
		vk::DebugMarkerMarkerInfoEXT marker_info(name.data(), make_color(color));
		command_buffer.debugMarkerBeginEXT(marker_info);
	}

	void DebugMarkerExtDebugUtils::end_label(vk::CommandBuffer command_buffer) const {
		command_buffer.debugMarkerEndEXT();
	}

	void DebugMarkerExtDebugUtils::insert_label(vk::CommandBuffer command_buffer, std::string_view name, uint32_t color) const {
		vk::DebugMarkerMarkerInfoEXT marker_info(name.data(), make_color(color));
		command_buffer.debugMarkerInsertEXT(marker_info);
	}


	DummyDebugUtils::DummyDebugUtils(Device& device) :
		DebugUtils{ device } {
	}
}