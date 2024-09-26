#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Device;

			/**
			 * @brief An interface over platform-specific debug extensions.
			 */
			class DebugUtils
			{
			public:
				DebugUtils(Device& device);
				virtual ~DebugUtils() = default;

				template<typename _Ty>
				void SetDebugName(const _Ty& object, const std::string& name)
				{
					SetDebugName(object, name.c_str());
				}

				template<typename _Ty>
				void SetDebugName(const _Ty& object, const char* name)
				{
					auto object_id = reinterpret_cast<uint64_t>(static_cast<_Ty::CType>(object));
					auto formatted_debug_object_name = std::format("{}:{} | {:x}", vk::to_string(object.objectType), name, object_id);

					SetDebugName(object.objectType, object_id, name);
				}

				/**
				 * @brief Sets the debug name for a Vulkan object.
				 */
				virtual void SetDebugName(vk::ObjectType object_type, uint64_t object_handle, const char* name) const = 0;

				/**
				 * @brief Tags the given Vulkan object with some data.
				 */
				virtual void SetDebugTag(vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const = 0;

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
			protected:
				Device& device;
			};

			/**
			 * @brief DebugUtils implemented on top of VK_EXT_debug_utils.
			 */
			class DebugUtilsExtDebugUtils final : public DebugUtils
			{
			public:
				DebugUtilsExtDebugUtils(Device& device);
				~DebugUtilsExtDebugUtils() override = default;

				void SetDebugName(vk::ObjectType object_type, uint64_t object_handle, const char* name) const override;
				void SetDebugTag(vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const override;
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
				DebugMarkerExtDebugUtils(Device& device);
				~DebugMarkerExtDebugUtils() override = default;

				void SetDebugName(vk::ObjectType object_type, uint64_t object_handle, const char* name) const override;
				void SetDebugTag(vk::ObjectType object_type, uint64_t object_handle, uint64_t tag_name, const void* tag_data, size_t tag_data_size) const override;
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
				DummyDebugUtils(Device& device);
				~DummyDebugUtils() override = default;

				inline void SetDebugName(vk::ObjectType, uint64_t, const char*) const override {}
				inline void SetDebugTag(vk::ObjectType, uint64_t, uint64_t, const void*, size_t) const override {}
				inline void BeginLabel(vk::CommandBuffer, const char*, glm::vec4 const) const override {}
				inline void EndLabel(vk::CommandBuffer) const override {}
				inline void InsertLabel(vk::CommandBuffer, const char*, glm::vec4 const) const override {}
			};
		}
	}
}