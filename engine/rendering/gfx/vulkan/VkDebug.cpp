#include "VkDebug.h"
#include "VkDevice.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			inline std::array<float, 4> MakeColor(uint32_t color)
			{
				std::array<float, 4> out_color{};
				out_color[0] = ((color >> 24) & 0xFF) / 255.0f;
				out_color[1] = ((color >> 16) & 0xFF) / 255.0f;
				out_color[2] = ((color >> 8) & 0xFF) / 255.0f;
				out_color[3] = (color & 0xFF) / 255.0f;

				return out_color;
			}

			DebugUtils::DebugUtils(Device& device) :
				device{ device }
			{
			}

			DebugUtilsExtDebugUtils::DebugUtilsExtDebugUtils(Device& device) :
				DebugUtils{ device }
			{
			}

			void DebugUtilsExtDebugUtils::SetDebugName(vk::ObjectType object_type, uint64_t object_handle, std::string_view name) const
			{
				vk::DebugUtilsObjectNameInfoEXT name_info(object_type, object_handle, name.data());
				Handle(device).setDebugUtilsObjectNameEXT(name_info);
			}

			void DebugUtilsExtDebugUtils::SetDebugTag(vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const
			{
				vk::DebugUtilsObjectTagInfoEXT tag_info(object_type, object_handle, tag_name, tag_data_size, tag_data);
				Handle(device).setDebugUtilsObjectTagEXT(tag_info);
			}

			void DebugUtilsExtDebugUtils::BeginLabel(vk::CommandBuffer command_buffer, std::string_view name, uint32_t color) const
			{
				vk::DebugUtilsLabelEXT label_info(name.data(), MakeColor(color));
				command_buffer.beginDebugUtilsLabelEXT(label_info);
			}

			void DebugUtilsExtDebugUtils::EndLabel(vk::CommandBuffer command_buffer) const
			{
				command_buffer.endDebugUtilsLabelEXT();
			}

			void DebugUtilsExtDebugUtils::InsertLabel(vk::CommandBuffer command_buffer, std::string_view name, uint32_t color) const
			{
				vk::DebugUtilsLabelEXT label_info(name.data(), MakeColor(color));
				command_buffer.insertDebugUtilsLabelEXT(label_info);
			}


			DebugMarkerExtDebugUtils::DebugMarkerExtDebugUtils(Device& device) :
				DebugUtils{ device }
			{
			}

			void DebugMarkerExtDebugUtils::SetDebugName(vk::ObjectType object_type, uint64_t object_handle, std::string_view name) const
			{
				vk::DebugMarkerObjectNameInfoEXT name_info(vk::debugReportObjectType(object_type), object_handle, name.data());
				Handle(device).debugMarkerSetObjectNameEXT(name_info);
			}

			void DebugMarkerExtDebugUtils::SetDebugTag(vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const
			{
				vk::DebugMarkerObjectTagInfoEXT tag_info(vk::debugReportObjectType(object_type), object_handle, tag_name, tag_data_size, tag_data);
				Handle(device).debugMarkerSetObjectTagEXT(tag_info);
			}

			void DebugMarkerExtDebugUtils::BeginLabel(vk::CommandBuffer command_buffer, std::string_view name, uint32_t color) const
			{
				vk::DebugMarkerMarkerInfoEXT marker_info(name.data(), MakeColor(color));
				command_buffer.debugMarkerBeginEXT(marker_info);
			}

			void DebugMarkerExtDebugUtils::EndLabel(vk::CommandBuffer command_buffer) const
			{
				command_buffer.debugMarkerEndEXT();
			}

			void DebugMarkerExtDebugUtils::InsertLabel(vk::CommandBuffer command_buffer, std::string_view name, uint32_t color) const
			{
				vk::DebugMarkerMarkerInfoEXT marker_info(name.data(), MakeColor(color));
				command_buffer.debugMarkerInsertEXT(marker_info);
			}


			DummyDebugUtils::DummyDebugUtils(Device& device) :
				DebugUtils{ device }
			{

			}
		}
	}
}