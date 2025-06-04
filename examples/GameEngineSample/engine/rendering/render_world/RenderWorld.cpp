#include "RenderWorld.h"
#include "../Rendering.h"

#include <glm/gtc/reciprocal.hpp>
#include <glm/gtx/quaternion.hpp>

#include <edge/util/utl_profiler.h>
#include <edge/util/utl_hash.hpp>

#include "../Globals.h"

namespace engine::render {
	template<typename _Ty>
	inline auto perspectiveRH_ReverseZ_InfiniteFar(_Ty fovy, _Ty aspect, _Ty znear) {
		auto cot_half_fovy = glm::cot(fovy / static_cast<_Ty>(2));
		auto result = glm::zero<glm::mat<4, 4, _Ty, glm::defaultp>>();
		result[0][0] = cot_half_fovy / aspect;
		result[1][1] = cot_half_fovy;
		result[2][3] = -static_cast<_Ty>(1);
		result[3][2] = znear;
		return result;
	}

	RenderWorld::RenderWorld(IRenderer& renderer) :
		renderer{ renderer } {
		draw_descriptions = std::make_unique<DynamicBuffer<interop::DrawDescription>>(16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
		world_meshes = std::make_unique<DynamicBuffer<interop::Mesh>>(16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
		world_submeshes = std::make_unique<DynamicBuffer<interop::SubMesh>>(16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);

		scene_lights = std::make_unique<DynamicBuffer<interop::LightCommit>>(16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);

		scene_cascaded_shadows = std::make_unique<DynamicBuffer<interop::CascadedShadowCommit>>(16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
		omni_shadows = std::make_unique<DynamicBuffer<interop::OmniShadowCommit>>(16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
		spot_shadows = std::make_unique<DynamicBuffer<interop::SpotShadowCommit>>(16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);

		draw_materials = std::make_unique<DynamicBuffer<interop::Material>>(16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);

		world_cameras = std::make_unique<DynamicBuffer<interop::Camera>>(8ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
		transform_matrices = std::make_unique<DynamicBuffer<glm::mat4x4>>(64ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
		world_frustum = std::make_unique<DynamicBuffer<glm::vec4>>(64ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
	}

	std::tuple<uint32_t, interop::Camera&> RenderWorld::create_camera() {
		auto& world_resource = world_resources.emplace_back();
		world_resource.type = WorldResourceType::eCamera;
		world_resource.resource_id = camera_offset++;

		auto& camera = world_cameras->get(world_resource.resource_id);
		camera.frustum_offset = frustum_offset;
		frustum_offset += 6ull; // Because 6 frustum sides

		return { static_cast<uint32_t>(world_resources.size() - 1ull), camera };
	}

	uint32_t RenderWorld::get_resource_id(uint32_t resource_index) const {
		auto& world_resource = world_resources[resource_index];
		return world_resource.resource_id;
	}

	interop::Camera& RenderWorld::get_render_camera(uint32_t camera_id) {
		auto& world_resource = world_resources[camera_id];
		assert(world_resource.type == WorldResourceType::eCamera);
		return world_cameras->get(world_resource.resource_id);
	}

	glm::mat4 const& RenderWorld::get_camera_matrix(uint32_t camera_id, uint32_t matrix_index) {
		auto& world_resource = world_resources[camera_id];
		auto& camera = world_cameras->get(world_resource.resource_id);
		return camera.matrices[matrix_index];
	}

	uint32_t RenderWorld::get_camera_count() const {
		return camera_offset;
	}

	void RenderWorld::update_render_camera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto& world_resource = world_resources[camera_id];
		assert(world_resource.type == WorldResourceType::eCamera);

		auto& camera = world_cameras->get(world_resource.resource_id);

		uint32_t camera_hash{ 0u };
		camera_hash = edge::murmur_hash(reinterpret_cast<const char*>(&translation.x), sizeof(glm::vec3), camera_hash);
		camera_hash = edge::murmur_hash(reinterpret_cast<const char*>(&rotation.x), sizeof(glm::quat), camera_hash);
		camera_hash = edge::murmur_hash(camera.fov, camera_hash);
		camera_hash = edge::murmur_hash(camera.z_near, camera_hash);
		camera_hash = edge::murmur_hash(camera.z_far, camera_hash);
		camera_hash = edge::murmur_hash(camera.aspect, camera_hash);

		bool dirty = camera_hash != camera.last_hash;
		camera.last_hash = camera_hash;

		float screen_size_x = 1280;
		float screen_size_y = 720;

		if (dirty) {
			camera.aspect = screen_size_x / screen_size_y;

			camera.size = { screen_size_x, screen_size_y };
			camera.inv_size = 1.f / camera.size;

			camera.position = translation;

			auto forward = glm::normalize(glm::rotate(rotation, glm::vec3(0.f, 0.f, -1.f)));
			auto right = glm::normalize(glm::cross(forward, glm::vec3{ 0.f, 1.f, 0.f }));
			auto up = glm::normalize(glm::cross(right, forward));

			camera.matrices[constants::view_matrix_offset] = glm::lookAt(translation, translation + forward, up);
			camera.matrices[constants::inv_view_matrix_offset] = glm::inverse(camera.matrices[constants::view_matrix_offset]);
			camera.matrices[constants::projection_matrix_offset] = perspectiveRH_ReverseZ_InfiniteFar(camera.fov, camera.aspect, camera.z_near);
			camera.matrices[constants::inv_projection_matrix_offset] = glm::inverse(camera.matrices[constants::projection_matrix_offset]);
			camera.matrices[constants::prev_view_projection_matrix_offset] = camera.matrices[constants::view_projection_matrix_offset];
			camera.matrices[constants::view_projection_matrix_offset] = camera.matrices[constants::projection_matrix_offset] * camera.matrices[constants::view_matrix_offset];
			camera.matrices[constants::inv_view_projection_matrix_offset] = glm::inverse(camera.matrices[constants::view_projection_matrix_offset]);

			// Calculate frustum
			update_camera_frustum(camera_id);

			// Update reprojection
			auto& camera_reprojection = camera.matrices[constants::reprojection_matrix_offset];
			camera_reprojection = camera.matrices[constants::prev_view_projection_matrix_offset] * camera.matrices[constants::inv_view_projection_matrix_offset];

			float rcp_half_dimx = 2.0f / screen_size_x;
			float rcp_half_dimy = 2.0f / screen_size_y;
			float rcp_z_magic = camera.z_near / (camera.z_far - camera.z_near);

			bool linear_z = false;

			// Transform from uv to clip space: texcoord * 2 - 1
			static glm::mat4 to_clip_space =
			{
				rcp_half_dimx, 0.0f, 0.0f, 0.0f,
				0.0f, -rcp_half_dimy, 0.0f, 0.0f,
				0.0f, 0.0f, linear_z ? rcp_z_magic : 1.0f, 0.0f,
				-1.0f, 1.0f, linear_z ? -rcp_z_magic : 0.0f, 1.0f
			};

			// Transform from clip to uv space: texcoord * 0.5 + 0.5
			static glm::mat4 to_uv_space =
			{
				1.0f / rcp_half_dimx, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f / rcp_half_dimy, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				1.0f / rcp_half_dimx, 1.0f / rcp_half_dimy, 0.0f, 1.0f
			};
			camera_reprojection = to_clip_space * camera_reprojection * to_uv_space;

			// Mark camera for update
			world_cameras->mark_dirty(world_resource.resource_id);
		}
	}

	void RenderWorld::update_render_camera(uint32_t camera_id, const glm::mat4& view, const glm::mat4& projection, bool perspective, uint32_t width, uint32_t height) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto& world_resource = world_resources[camera_id];
		auto& camera = world_cameras->get(world_resource.resource_id);

		uint32_t camera_hash{ 0u };
		camera_hash = edge::murmur_hash(reinterpret_cast<const char*>(&view[0]), sizeof(glm::mat4), camera_hash);
		camera_hash = edge::murmur_hash(reinterpret_cast<const char*>(&projection[0]), sizeof(glm::mat4), camera_hash);
		camera_hash = edge::murmur_hash(camera.fov, camera_hash);
		camera_hash = edge::murmur_hash(camera.z_near, camera_hash);
		camera_hash = edge::murmur_hash(camera.z_far, camera_hash);
		camera_hash = edge::murmur_hash(camera.aspect, camera_hash);

		bool dirty = camera_hash != camera.last_hash;
		camera.last_hash = camera_hash;

		if (dirty) {
			camera.size = { static_cast<float>(width), static_cast<float>(height) };
			camera.inv_size = 1.f / camera.size;

			camera.aspect = camera.size.x / camera.size.y;

			float m33 = projection[2][2];
			float m43 = projection[3][2];

			if (perspective) {
				camera.z_near = m43 / (m33 - 1.0f);
				camera.z_far = m43 / (m33 + 1.0f);
			}
			else {
				camera.z_near = (m43 + 1.0f) / m33;
				camera.z_far = (m43 - 1.0f) / m33;
			}

			camera.matrices[constants::view_matrix_offset] = view;
			camera.matrices[constants::projection_matrix_offset] = projection;
			camera.matrices[constants::view_projection_matrix_offset] = projection * view;

			update_camera_frustum(camera_id);
		}
	}

	void RenderWorld::add_geometry_draw(uint32_t camera_id, uint32_t mesh_id) {
		EDGE_PROFILE_ZONE_SCOPED;

		

		// Lookup for camera
		uint32_t camera_index{ ~0u };
		{
			auto& world_resource = world_resources[camera_id];
			assert(world_resource.type == WorldResourceType::eCamera);
			camera_index = world_resource.resource_id;
		}

		// Lookup for mesh
		uint32_t mesh_index{ ~0u };
		uint32_t mesh_submesh_count{ 0u };
		{
			auto& world_resource = world_resources[mesh_id];
			assert(world_resource.type == WorldResourceType::eMesh);
			mesh_index = world_resource.resource_id;

			auto& mesh = world_meshes->get(mesh_index);
			mesh_submesh_count = mesh.submesh_count;
		}

		for (int32_t idx = 0; idx < static_cast<int32_t>(mesh_submesh_count); ++idx) {
			auto& batch_command = draw_descriptions->get(draw_count + idx);
			batch_command.camera_index = camera_index;
			batch_command.mesh_index = mesh_index;
			batch_command.submesh_index = idx;
		}

		draw_descriptions->mark_dirty(draw_count, mesh_submesh_count);
		draw_count += mesh_submesh_count;
	}

	uint32_t RenderWorld::add_material(const interop::Material& material) {
		auto material_index = material_count++;
		auto& new_material = draw_materials->get(material_index);
		draw_materials->mark_dirty(material_index);
		new_material = material;

		return material_index;
	}

	const interop::Material& RenderWorld::get_material(uint32_t index) const {
		return draw_materials->get(index);
	}

	std::tuple<uint32_t, interop::Mesh&> RenderWorld::create_mesh(uint32_t joint_count) {
		auto& world_resource = world_resources.emplace_back();
		world_resource.type = WorldResourceType::eMesh;
		world_resource.resource_id = mesh_count++;

		interop::Mesh& mesh = world_meshes->get(world_resource.resource_id);
		world_meshes->mark_dirty(world_resource.resource_id);

		// Register slots for model matrix, old model matrix and normal matrix
		mesh.transform_offset = transform_offset;
		transform_offset += 2ull;

		mesh.skin_offset = (joint_count == 0u ? ~0u : transform_offset);
		transform_offset += joint_count;

		mesh.submesh_offset = submesh_count;

		// TODO: Reserve skinning matrices if needed

		return { world_resources.size() - 1ull, mesh };
	}

	void RenderWorld::update_mesh_matrices(uint32_t mesh_id, const glm::mat4& matrix) {
		auto& world_resource = world_resources[mesh_id];
		assert(world_resource.type == WorldResourceType::eMesh);

		auto& mesh = world_meshes->get(world_resource.resource_id);

		auto* matrices = transform_matrices->data(mesh.transform_offset);
		transform_matrices->mark_dirty(mesh.transform_offset, 2ull);

		matrices[constants::old_model_matrix_offset] = matrices[constants::model_matrix_offset];
		matrices[constants::model_matrix_offset] = matrix * interop::make_decode_position_matrix(mesh.bb_enc);
	}

	void RenderWorld::update_joint_matrices(uint32_t mesh_id, std::span<const glm::mat4> joints) {
		auto& world_resource = world_resources[mesh_id];
		assert(world_resource.type == WorldResourceType::eMesh);

		auto& mesh = world_meshes->get(world_resource.resource_id);

		//auto* matrices = world_matrices->data(mesh.skin_offset);
		//world_matrices->mark_dirty(mesh.skin_offset, static_cast<uint32_t>(joints.size()));

		//std::memcpy(static_cast<void*>(matrices), joints.data(), joints.size() * sizeof(glm::mat4));
	}

	void RenderWorld::add_submeshes(std::span<const interop::SubMesh> submeshes) {
		for (int32_t i = 0; i < submeshes.size(); ++i) {
			world_submeshes->get(submesh_count + i) = submeshes[i];
		}

		world_submeshes->mark_dirty(submesh_count, submeshes.size());
		submesh_count += submeshes.size();
	}

	const interop::Mesh& RenderWorld::get_mesh(uint32_t index) {
		auto& world_resource = world_resources[index];
		assert(world_resource.type == WorldResourceType::eMesh);

		return world_meshes->get(world_resource.resource_id);
	}

	void RenderWorld::add_light(const interop::LightCommit& light_data) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto light_index = light_count++;
		auto& new_light = scene_lights->get(light_index);
		scene_lights->mark_dirty(light_index);
		new_light = light_data;
	}

	void RenderWorld::add_shadow(const interop::CascadedShadowCommit& shadow_data) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto shadow_index = cascaded_shadow_count++;
		auto& new_shadow = scene_cascaded_shadows->get(shadow_index);
		scene_cascaded_shadows->mark_dirty(shadow_index);
		new_shadow = shadow_data;
	}

	void RenderWorld::add_shadow(const interop::OmniShadowCommit& shadow_data) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto shadow_index = omni_shadow_count++;
		auto& new_shadow = omni_shadows->get(shadow_index);
		omni_shadows->mark_dirty(shadow_index);
		new_shadow = shadow_data;
	}

	void RenderWorld::add_shadow(const interop::SpotShadowCommit& shadow_data) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto shadow_index = spot_shadow_count++;
		auto& new_shadow = spot_shadows->get(shadow_index);
		spot_shadows->mark_dirty(shadow_index);
		new_shadow = shadow_data;
	}

	void RenderWorld::begin_frame(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto& render_context = renderer.get_render_context();

		{
			interop::FrameData fd{};

			fd.transform_ptr = transform_matrices->get_buffer()->get_address();
			transform_matrices->flush();

			fd.frustum_sides_ptr = world_frustum->get_buffer()->get_address();
			world_frustum->flush();

			fd.camera_ptr = world_cameras->get_buffer()->get_address();
			world_cameras->flush();

			fd.mesh_ptr = world_meshes->get_buffer()->get_address();
			world_meshes->flush();

			fd.submesh_ptr = world_submeshes->get_buffer()->get_address();
			world_submeshes->flush();

			fd.material_ptr = draw_materials->get_buffer()->get_address();
			draw_materials->flush();

			fd.lights_ptr = scene_lights->get_buffer()->get_address();
			scene_lights->flush();

			fd.cascaded_shadow_ptr = scene_cascaded_shadows->get_buffer()->get_address();
			scene_cascaded_shadows->flush();

			fd.omni_shadow_ptr = omni_shadows->get_buffer()->get_address();
			omni_shadows->flush();

			fd.spot_shadow_ptr = spot_shadows->get_buffer()->get_address();
			spot_shadows->flush();

			draw_descriptions->flush();

			fd.dt = delta_time;
			fd.light_count = light_count;
			fd.camera_count = camera_offset;
			fd.default_camera_id = 0;
			fd.draw_count = draw_count;
			fd.mesh_count = submesh_count;
			fd.frame_number = frame_counter;

			const uint8_t* uniform_data = reinterpret_cast<const uint8_t*>(&fd);
			uint64_t uniform_size = sizeof(interop::FrameData);

			render_context.update_global_binding(0, std::span(uniform_data, uniform_size));
		}
	}

	void RenderWorld::end_frame() {
		++frame_counter;

		draw_count = 0;
		light_count = 0;
		cascaded_shadow_count = 0;
		omni_shadow_count = 0;
		spot_shadow_count = 0;
	}

	edge::gfx::IBuffer* RenderWorld::get_draw_buffer() {
		return draw_descriptions->get_buffer();
	}

	uint32_t RenderWorld::get_draw_count() const {
		return draw_count;
	}

	uint32_t RenderWorld::get_mesh_count() const {
		return submesh_count;
	}

	void RenderWorld::update_camera_frustum(uint32_t camera_id) {
		auto& world_resource = world_resources[camera_id];
		auto& camera = world_cameras->get(world_resource.resource_id);

		auto* frustum_begin = world_frustum->data(camera.frustum_offset);
		world_frustum->mark_dirty(camera.frustum_offset, 6u);

		auto& clip_matrix = camera.matrices[constants::view_projection_matrix_offset];

		// Lesft
		auto frustum_left = glm::vec4(
			clip_matrix[0][3] - clip_matrix[0][0],
			clip_matrix[1][3] - clip_matrix[1][0],
			clip_matrix[2][3] - clip_matrix[2][0],
			clip_matrix[3][3] - clip_matrix[3][0]
		);
		frustum_begin[1] = glm::normalize(frustum_left);

		// Rigth
		auto frustum_right = glm::vec4(
			clip_matrix[0][3] + clip_matrix[0][0],
			clip_matrix[1][3] + clip_matrix[1][0],
			clip_matrix[2][3] + clip_matrix[2][0],
			clip_matrix[3][3] + clip_matrix[3][0]
		);
		frustum_begin[0] = glm::normalize(frustum_right);

		// Top
		auto frustum_top = glm::vec4(
			clip_matrix[0][3] - clip_matrix[0][1],
			clip_matrix[1][3] - clip_matrix[1][1],
			clip_matrix[2][3] - clip_matrix[2][1],
			clip_matrix[3][3] - clip_matrix[3][1]
		);
		frustum_begin[3] = glm::normalize(frustum_top);

		// Bottom
		auto frustum_bottom = glm::vec4(
			clip_matrix[0][3] + clip_matrix[0][1],
			clip_matrix[1][3] + clip_matrix[1][1],
			clip_matrix[2][3] + clip_matrix[2][1],
			clip_matrix[3][3] + clip_matrix[3][1]
		);
		frustum_begin[2] = glm::normalize(frustum_bottom);

		// Back
		auto frustum_back = glm::vec4(
			clip_matrix[0][3] + clip_matrix[0][2],
			clip_matrix[1][3] + clip_matrix[1][2],
			clip_matrix[2][3] + clip_matrix[2][2],
			clip_matrix[3][3] + clip_matrix[3][2]
		);
		frustum_begin[4] = glm::normalize(frustum_back);

		// Front
		auto frustum_front = glm::vec4(
			clip_matrix[0][3] - clip_matrix[0][2],
			clip_matrix[1][3] - clip_matrix[1][2],
			clip_matrix[2][3] - clip_matrix[2][2],
			clip_matrix[3][3] - clip_matrix[3][2]
		);
		frustum_begin[5] = glm::normalize(frustum_front);
	}
}