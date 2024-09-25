#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			/**
	* @brief An interface over platform-specific debug extensions.
	*/
			class DebugUtils
			{
			public:
				virtual ~DebugUtils() = default;

				/**
				 * @brief Sets the debug name for a Vulkan object.
				 */
				virtual void SetDebugName(vk::Device device, vk::ObjectType object_type, uint64_t object_handle, const char* name) const = 0;

				/**
				 * @brief Tags the given Vulkan object with some data.
				 */
				virtual void SetDebugTag(
					vk::Device device, vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const = 0;

				/**
				 * @brief Inserts a command to begin a new debug label/marker scope.
				 */
				virtual void BeginLabel(vk::CommandBuffer command_buffer, const char* name, glm::vec4 const color = {}) const = 0;

				/**
				 * @brief Inserts a command to end the current debug label/marker scope.
				 */
				virtual void EndLabel(vk::CommandBuffer command_buffer) const = 0;

				/**
				 * @brief Inserts a (non-scoped) debug label/marker in the command buffer.
				 */
				virtual void InsertLabel(vk::CommandBuffer command_buffer, const char* name, glm::vec4 const color = {}) const = 0;
			};

			/**
			 * @brief DebugUtils implemented on top of VK_EXT_debug_utils.
			 */
			class DebugUtilsExtDebugUtils final : public DebugUtils
			{
			public:
				~DebugUtilsExtDebugUtils() override = default;

				void SetDebugName(vk::Device device, vk::ObjectType object_type, uint64_t object_handle, const char* name) const override;
				void SetDebugTag(vk::Device device, vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const override;
				void BeginLabel(vk::CommandBuffer command_buffer, const char* name, glm::vec4 const color) const override;
				void EndLabel(vk::CommandBuffer command_buffer) const override;
				void InsertLabel(vk::CommandBuffer command_buffer, const char* name, glm::vec4 const color) const override;
			};

			/**
			 * @brief DebugUtils implemented on top of VK_EXT_debug_marker.
			 */
			class DebugMarkerExtDebugUtils final : public DebugUtils
			{
			public:
				~DebugMarkerExtDebugUtils() override = default;

				void SetDebugName(vk::Device device, vk::ObjectType object_type, uint64_t object_handle, const char* name) const override;
				void SetDebugTag(vk::Device device, vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const override;
				void BeginLabel(vk::CommandBuffer command_buffer, const char* name, glm::vec4 const color) const override;
				void EndLabel(vk::CommandBuffer command_buffer) const override;
				void InsertLabel(vk::CommandBuffer command_buffer, const char* name, glm::vec4 const color) const override;
			};

			/**
			 * @brief No-op DebugUtils.
			 */
			class DummyDebugUtils final : public DebugUtils
			{
			public:
				~DummyDebugUtils() override = default;

				inline void SetDebugName(vk::Device, vk::ObjectType, uint64_t, const char*) const override {}
				inline void SetDebugTag(vk::Device, vk::ObjectType, uint64_t, uint64_t, const void*, size_t) const override {}
				inline void BeginLabel(vk::CommandBuffer, const char*, glm::vec4 const) const override {}
				inline void EndLabel(vk::CommandBuffer) const override {}
				inline void InsertLabel(vk::CommandBuffer, const char*, glm::vec4 const) const override {}
			};
		}
	}
}