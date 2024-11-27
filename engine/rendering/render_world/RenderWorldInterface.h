#pragma once

#include <engine/rendering/gfx/Structures.h>
#include <embed/shaders/includes/shlib.h>

#include <hash.hpp>

namespace engine
{
	namespace render
	{
		enum class WorldResourceType
		{
			eCamera,
			eLight
		};

		struct WorldResource
		{
			WorldResourceType type{};
			uint32_t resource_id{};
		};

		struct DrawCommand
		{
			uint32_t node_index{};

			uint32_t vertex_buffers{};
			uint32_t material_index{};

			uint32_t first_index{};
			uint32_t index_count{};
			uint32_t first_vertex{};
			uint32_t vertex_count{};
		};

		class IRenderWorld
		{
		public:
			virtual ~IRenderWorld() = default;

			virtual uint32_t CreateRenderCamera() = 0;
			virtual shlib::Camera& GetRenderCamera(uint32_t camera_id) = 0;
			virtual glm::mat4 const& GetCameraMatrix(uint32_t camera_id, uint32_t matrix_index) = 0;

			virtual void UpdateRenderCamera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation) = 0;
			virtual void UpdateRenderCamera(uint32_t camera_id, const glm::mat4& view, const glm::mat4& projection, uint32_t width, uint32_t height) = 0;

			virtual void UpdateNode(uint32_t node_id, const glm::mat4& matrix) = 0;

			virtual void AddGeometryDraw(DrawCommand& draw_command) = 0;

			virtual void AddMaterial(const shlib::Material& material) = 0;
			virtual const shlib::Material& GetMaterial(uint32_t index) const = 0;

			virtual void AddLight(const shlib::LightCommit& light_data) = 0;

			virtual void AddShadow(const shlib::CascadedShadowCommit& shadow_data) = 0;
			virtual void AddShadow(const shlib::OmniShadowCommit& shadow_data) = 0;
			virtual void AddShadow(const shlib::SpotShadowCommit& shadow_data) = 0;

			virtual void BeginFrame(float delta_time) = 0;
			virtual void EndFrame() = 0;

			virtual uint32_t GetDrawCount() const = 0;
		};
	}
}