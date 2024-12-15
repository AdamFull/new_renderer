#pragma once

#include <edge/gfx/gfx_structures.h>
#include <embed/shaders/includes/shlib.h>

#include <hash.hpp>

namespace engine
{
	namespace render
	{
		enum class WorldResourceType
		{
			eCamera,
			eNode,
			eMesh,
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

			virtual uint32_t GetResourceId(uint32_t resource_index) const = 0;

			virtual std::tuple<uint32_t, shlib::Camera&> CreateCamera() = 0;
			virtual shlib::Camera& GetRenderCamera(uint32_t camera_id) = 0;
			virtual glm::mat4 const& GetCameraMatrix(uint32_t camera_id, uint32_t matrix_index) = 0;
			virtual uint32_t GetCameraCount() const = 0;

			virtual void UpdateRenderCamera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation) = 0;
			virtual void UpdateRenderCamera(uint32_t camera_id, const glm::mat4& view, const glm::mat4& projection, bool perspective, uint32_t width, uint32_t height) = 0;

			virtual glm::mat4 const& GetMatrix(uint32_t matrix_offset) const = 0;

			virtual void AddGeometryDraw(uint32_t camera_id, uint32_t mesh_id) = 0;

			virtual uint32_t AddMaterial(const shlib::Material& material) = 0;
			virtual const shlib::Material& GetMaterial(uint32_t index) const = 0;

			virtual std::tuple<uint32_t, shlib::Mesh&> CreateMesh() = 0;
			virtual void UpdateMeshMatrices(uint32_t mesh_id, const glm::mat4& matrix) = 0;
			virtual void AttachSubmesh(uint32_t mesh_id, const shlib::SubMesh& submesh) = 0;
			virtual const shlib::Mesh& GetMesh(uint32_t index)  = 0;

			virtual void AddLight(const shlib::LightCommit& light_data) = 0;

			virtual void AddShadow(const shlib::CascadedShadowCommit& shadow_data) = 0;
			virtual void AddShadow(const shlib::OmniShadowCommit& shadow_data) = 0;
			virtual void AddShadow(const shlib::SpotShadowCommit& shadow_data) = 0;

			virtual void BeginFrame(float delta_time) = 0;
			virtual void EndFrame() = 0;

			virtual edge::gfx::Buffer* GetDrawBuffer() = 0;
			virtual uint32_t GetDrawCount() const = 0;

			virtual uint32_t GetMeshCount() const = 0;
		};
	}
}