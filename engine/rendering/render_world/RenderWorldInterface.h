#pragma once

#include <engine/rendering/gfx/Structures.h>
#include <embed/shaders/includes/interop/base.h>

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
			virtual FCamera& GetRenderCamera(uint32_t camera_id) = 0;
			virtual glm::mat4 const& GetCameraMatrix(uint32_t camera_id, uint32_t matrix_index) = 0;

			virtual void UpdateRenderCamera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation) = 0;

			virtual void UpdateNode(uint32_t node_id, const glm::mat4& matrix) = 0;

			virtual void AddGeometryDraw(DrawCommand& draw_command) = 0;

			virtual void BeginFrame() = 0;
			virtual void EndFrame() = 0;

			virtual gfx::IndirectDrawCommand GetDrawCommand() = 0;
		};
	}
}