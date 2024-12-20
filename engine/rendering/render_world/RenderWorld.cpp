#include "RenderWorld.h"
#include "../Rendering.h"

#include <glm/gtx/quaternion.hpp>

#include <edge/util/Profiler.h>

#include "../Globals.h"

namespace engine
{
	namespace render
	{
		RenderWorld::RenderWorld(IRenderer& renderer) :
			renderer{ renderer }
		{
			auto& resource_loader = renderer.GetResourceLoader();

			draw_descriptions = std::make_unique<DynamicBuffer<shlib::DrawDescription>>(resource_loader, 16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
			world_meshes = std::make_unique<DynamicBuffer<shlib::Mesh>>(resource_loader, 16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
			world_submeshes = std::make_unique<DynamicBuffer<shlib::SubMesh>>(resource_loader, 16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);

			scene_lights = std::make_unique<DynamicBuffer<shlib::LightCommit>>(resource_loader, 16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);

			scene_cascaded_shadows = std::make_unique<DynamicBuffer<shlib::CascadedShadowCommit>>(resource_loader, 16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
			omni_shadows = std::make_unique<DynamicBuffer<shlib::OmniShadowCommit>>(resource_loader, 16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
			spot_shadows = std::make_unique<DynamicBuffer<shlib::SpotShadowCommit>>(resource_loader, 16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);

			draw_materials = std::make_unique<DynamicBuffer<shlib::Material>>(resource_loader, 16ull, edge::gfx::BufferUsageFlag::eStorageBuffer);

			world_cameras = std::make_unique<DynamicBuffer<shlib::Camera>>(resource_loader, 8ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
			world_matrices = std::make_unique<DynamicBuffer<glm::mat4>>(resource_loader, 64ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
			world_frustum = std::make_unique<DynamicBuffer<glm::vec4>>(resource_loader, 64ull, edge::gfx::BufferUsageFlag::eStorageBuffer);
		}

		std::tuple<uint32_t, shlib::Camera&> RenderWorld::CreateCamera()
		{
			auto& world_resource = world_resources.emplace_back();
			world_resource.type = WorldResourceType::eCamera;
			world_resource.resource_id = camera_offset++;

			auto& camera = world_cameras->Get(world_resource.resource_id);
			camera.matrices_offset = matrices_offset;
			matrices_offset += shlib::matrix_offset_count;

			camera.frustum_offset = frustum_offset;
			frustum_offset += 6ull; // Because 6 frustum sides

			return { world_resources.size() - 1ull, camera };
		}

		uint32_t RenderWorld::GetResourceId(uint32_t resource_index) const
		{
			auto& world_resource = world_resources[resource_index];
			return world_resource.resource_id;
		}

		shlib::Camera& RenderWorld::GetRenderCamera(uint32_t camera_id)
		{
			auto& world_resource = world_resources[camera_id];
			assert(world_resource.type == WorldResourceType::eCamera);
			return world_cameras->Get(world_resource.resource_id);
		}

		glm::mat4 const& RenderWorld::GetCameraMatrix(uint32_t camera_id, uint32_t matrix_index)
		{
			auto& world_resource = world_resources[camera_id];
			auto& camera = world_cameras->Get(world_resource.resource_id);

			auto* matrix_begin = world_matrices->Data(camera.matrices_offset);
			return *(matrix_begin + matrix_index);
		}

		uint32_t RenderWorld::GetCameraCount() const
		{
			return camera_offset;
		}

		void RenderWorld::UpdateRenderCamera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto& world_resource = world_resources[camera_id];
			assert(world_resource.type == WorldResourceType::eCamera);

			auto& camera = world_cameras->Get(world_resource.resource_id);

			uint32_t camera_hash{ 0u };
			camera_hash = edge::murmur_hash(reinterpret_cast<const char*>(&translation.x), sizeof(glm::vec3), camera_hash);
			camera_hash = edge::murmur_hash(reinterpret_cast<const char*>(&rotation.x), sizeof(glm::quat), camera_hash);
			camera_hash = edge::murmur_hash(camera.fov, camera_hash);
			camera_hash = edge::murmur_hash(camera.z_near, camera_hash);
			camera_hash = edge::murmur_hash(camera.z_far, camera_hash);
			camera_hash = edge::murmur_hash(camera.aspect, camera_hash);

			bool dirty = camera_hash != camera.last_hash;
			camera.last_hash = camera_hash;

			float screen_size_x = 1920.f;
			float screen_size_y = 1080.f;

			if (dirty)
			{
				camera.aspect = screen_size_x / screen_size_y;

				camera.size = { screen_size_x, screen_size_y };
				camera.invSize = 1.f / camera.size;

				auto forward = glm::normalize(glm::rotate(rotation, glm::vec3(0.f, 0.f, -1.f)));
				auto right = glm::normalize(glm::cross(forward, glm::vec3{ 0.f, 1.f, 0.f }));
				auto up = glm::normalize(glm::cross(right, forward));

				auto* matrices = world_matrices->Data(camera.matrices_offset);
				world_matrices->MarkDirty(camera.matrices_offset, shlib::matrix_offset_count);

				matrices[shlib::view_matrix_offset] = glm::lookAt(translation, translation + forward, up);
				matrices[shlib::inv_view_matrix_offset] = glm::inverse(matrices[shlib::view_matrix_offset]);
				matrices[shlib::projection_matrix_offset] = glm::perspective(camera.fov, camera.aspect, camera.z_near, camera.z_far);
				matrices[shlib::inv_projection_matrix_offset] = glm::inverse(matrices[shlib::projection_matrix_offset]);
				matrices[shlib::prev_view_projection_matrix_offset] = matrices[shlib::view_projection_matrix_offset];
				matrices[shlib::view_projection_matrix_offset] = matrices[shlib::projection_matrix_offset] * matrices[shlib::view_matrix_offset];
				matrices[shlib::inv_view_projection_matrix_offset] = glm::inverse(matrices[shlib::view_projection_matrix_offset]);

				// Calculate frustum
				UpdateCameraFrustum(camera_id);

				// Update reprojection
				auto& camera_reprojection = matrices[shlib::reprojection_matrix_offset];
				camera_reprojection = matrices[shlib::prev_view_projection_matrix_offset] * matrices[shlib::inv_view_projection_matrix_offset];

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
				world_cameras->MarkDirty(world_resource.resource_id);
			}
		}

		void RenderWorld::UpdateRenderCamera(uint32_t camera_id, const glm::mat4& view, const glm::mat4& projection, bool perspective, uint32_t width, uint32_t height)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto& world_resource = world_resources[camera_id];
			auto& camera = world_cameras->Get(world_resource.resource_id);

			uint32_t camera_hash{ 0u };
			camera_hash = edge::murmur_hash(reinterpret_cast<const char*>(&view[0]), sizeof(glm::mat4), camera_hash);
			camera_hash = edge::murmur_hash(reinterpret_cast<const char*>(&projection[0]), sizeof(glm::mat4), camera_hash);
			camera_hash = edge::murmur_hash(camera.fov, camera_hash);
			camera_hash = edge::murmur_hash(camera.z_near, camera_hash);
			camera_hash = edge::murmur_hash(camera.z_far, camera_hash);
			camera_hash = edge::murmur_hash(camera.aspect, camera_hash);

			bool dirty = camera_hash != camera.last_hash;
			camera.last_hash = camera_hash;

			if (dirty)
			{
				camera.size = { static_cast<float>(width), static_cast<float>(height) };
				camera.invSize = 1.f / camera.size;

				camera.aspect = camera.size.x / camera.size.y;

				float m33 = projection[2][2];
				float m43 = projection[3][2];

				if (perspective)
				{
					camera.z_near = m43 / (m33 - 1.0f);
					camera.z_far = m43 / (m33 + 1.0f);
				}
				else
				{
					camera.z_near = (m43 + 1.0f) / m33;
					camera.z_far = (m43 - 1.0f) / m33;
				}

				auto* matrices = world_matrices->Data(camera.matrices_offset);
				world_matrices->MarkDirty(camera.matrices_offset, shlib::matrix_offset_count);

				matrices[shlib::view_matrix_offset] = view;
				matrices[shlib::projection_matrix_offset] = projection;
				matrices[shlib::view_projection_matrix_offset] = projection * view;

				UpdateCameraFrustum(camera_id);
			}
		}

		glm::mat4 const& RenderWorld::GetMatrix(uint32_t matrix_offset) const
		{
			return world_matrices->Get(matrix_offset);
		}

		void RenderWorld::AddGeometryDraw(uint32_t camera_id, uint32_t mesh_id)
		{
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
			uint32_t submesh_count{ 0u };
			{
				auto& world_resource = world_resources[mesh_id];
				assert(world_resource.type == WorldResourceType::eMesh);
				mesh_index = world_resource.resource_id;

				auto& mesh = world_meshes->Get(mesh_index);
				submesh_count = mesh.submesh_count;
			}

			for (int32_t idx = 0; idx < submesh_count; ++idx)
			{
				auto& batch_command = draw_descriptions->Get(draw_count + idx);
				batch_command.camera_index = camera_index;
				batch_command.mesh_index = mesh_index;
				batch_command.submesh_index = idx;
			}

			draw_descriptions->MarkDirty(draw_count, submesh_count);
			draw_count += submesh_count;
		}

		uint32_t RenderWorld::AddMaterial(const shlib::Material& material)
		{
			auto material_index = material_count++;
			auto& new_material = draw_materials->Get(material_index);
			draw_materials->MarkDirty(material_index);
			new_material = material;

			return material_index;
		}

		const shlib::Material& RenderWorld::GetMaterial(uint32_t index) const
		{
			return draw_materials->Get(index);
		}

		std::tuple<uint32_t, shlib::Mesh&> RenderWorld::CreateMesh(uint32_t joint_count)
		{
			auto& world_resource = world_resources.emplace_back();
			world_resource.type = WorldResourceType::eMesh;
			world_resource.resource_id = mesh_count++;

			shlib::Mesh& mesh = world_meshes->Get(world_resource.resource_id);
			world_meshes->MarkDirty(world_resource.resource_id);

			// Register slots for model matrix, old model matrix and normal matrix
			mesh.matrix_offset = matrices_offset;
			matrices_offset += 3ull;

			mesh.skin_offset = (joint_count == 0u ? ~0u : matrices_offset);
			matrices_offset += joint_count;

			mesh.submesh_offset = submesh_count;

			// TODO: Reserve skinning matrices if needed

			return { world_resources.size() - 1ull, mesh };
		}

		void RenderWorld::UpdateMeshMatrices(uint32_t mesh_id, const glm::mat4& matrix)
		{
			auto& world_resource = world_resources[mesh_id];
			assert(world_resource.type == WorldResourceType::eMesh);

			auto& mesh = world_meshes->Get(world_resource.resource_id);

			auto* matrices = world_matrices->Data(mesh.matrix_offset);
			world_matrices->MarkDirty(mesh.matrix_offset, 3ull);

			matrices[shlib::old_model_matrix_offset]	= matrices[shlib::model_matrix_offset];
			matrices[shlib::model_matrix_offset]		= matrix * shlib::MakeDecodePositionMatrix(mesh.bb_enc);
			// TODO: can be calculated in shader: (https://github.com/graphitemaster/normals_revisited)
			matrices[shlib::normal_matrix_offset]		= glm::inverse(glm::transpose(matrices[shlib::model_matrix_offset]));
		}

		void RenderWorld::UpdateJointMatrices(uint32_t mesh_id, std::span<const glm::mat4> joints)
		{
			auto& world_resource = world_resources[mesh_id];
			assert(world_resource.type == WorldResourceType::eMesh);

			auto& mesh = world_meshes->Get(world_resource.resource_id);

			auto* matrices = world_matrices->Data(mesh.skin_offset);
			world_matrices->MarkDirty(mesh.skin_offset, joints.size());

			std::memcpy(static_cast<void*>(matrices), joints.data(), joints.size() * sizeof(glm::mat4));
		}

		void RenderWorld::AttachSubmesh(uint32_t mesh_id, const shlib::SubMesh& submesh)
		{
			auto& world_resource = world_resources[mesh_id];
			assert(world_resource.type == WorldResourceType::eMesh);

			auto& mesh = world_meshes->Get(world_resource.resource_id);
			mesh.submesh_count++;

			uint32_t submesh_index = submesh_count++;
			world_submeshes->Get(submesh_index) = submesh;
			world_submeshes->MarkDirty(submesh_index);
		}

		const shlib::Mesh& RenderWorld::GetMesh(uint32_t index)
		{
			auto& world_resource = world_resources[index];
			assert(world_resource.type == WorldResourceType::eMesh);

			return world_meshes->Get(world_resource.resource_id);
		}

		void RenderWorld::AddLight(const shlib::LightCommit& light_data)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto light_index = light_count++;
			auto& new_light = scene_lights->Get(light_index);
			scene_lights->MarkDirty(light_index);
			new_light = light_data;
		}

		void RenderWorld::AddShadow(const shlib::CascadedShadowCommit& shadow_data)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto shadow_index = cascaded_shadow_count++;
			auto& new_shadow = scene_cascaded_shadows->Get(shadow_index);
			scene_cascaded_shadows->MarkDirty(shadow_index);
			new_shadow = shadow_data;
		}

		void RenderWorld::AddShadow(const shlib::OmniShadowCommit& shadow_data)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto shadow_index = omni_shadow_count++;
			auto& new_shadow = omni_shadows->Get(shadow_index);
			omni_shadows->MarkDirty(shadow_index);
			new_shadow = shadow_data;
		}

		void RenderWorld::AddShadow(const shlib::SpotShadowCommit& shadow_data)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto shadow_index = spot_shadow_count++;
			auto& new_shadow = spot_shadows->Get(shadow_index);
			spot_shadows->MarkDirty(shadow_index);
			new_shadow = shadow_data;
		}

		void RenderWorld::BeginFrame(float delta_time)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto& render_context = renderer.GetRenderContext();

			// Post process data
			{
				shlib::GlobalPostProcessingData postprocessing{};
				// FXAA
				postprocessing.enable_fxaa = static_cast<uint32_t>(postprocess.fxaa.enable);
				postprocessing.fxaa_quality = fxaa_quality_variant[postprocess.fxaa.quality];
				postprocessing.fxaa_threshold = fxaa_threshold_variant[postprocess.fxaa.threshold];
				postprocessing.fxaa_threshold_min = fxaa_threshold_min_variant[postprocess.fxaa.threshold_min];
				// Tonemapping
				postprocessing.enable_tonemap = static_cast<uint32_t>(postprocess.tonemap.enable);
				postprocessing.tm_gamma = postprocess.tonemap.gamma;
				postprocessing.tm_exposure = postprocess.tonemap.exposure;
				postprocessing.tm_white_point = postprocess.tonemap.whitePoint;
				postprocessing.tm_adaptive = postprocess.tonemap.adaptive;
				// Bloom
				postprocessing.enable_bloom = static_cast<uint32_t>(postprocess.bloom.enable);
				postprocessing.b_bloom_strength = postprocess.bloom.strength;
				postprocessing.b_threshold = postprocess.bloom.threshold;
				postprocessing.b_filter_radius = postprocess.bloom.filter_radius;
				// Dof
				postprocessing.enable_dof = static_cast<uint32_t>(postprocess.dof.enable);
				postprocessing.df_sample_count = postprocess.dof.bokeh_samples;
				postprocessing.df_poly = postprocess.dof.bokeh_poly;
				postprocessing.df_focus_point = postprocess.dof.focus_point;
				postprocessing.df_near_field = postprocess.dof.near_field;
				postprocessing.df_near_transition = postprocess.dof.near_transition;
				postprocessing.df_far_field = postprocess.dof.far_field;
				postprocessing.df_far_transition = postprocess.dof.far_transition;
				// Chromatic aberration
				postprocessing.enable_chromatic_aberration = static_cast<uint32_t>(postprocess.aberration.enable);
				postprocessing.ca_distortion = postprocess.aberration.distortion;
				postprocessing.ca_iterations = postprocess.aberration.iterations;
				// Film grain
				postprocessing.enable_film_grain = static_cast<uint32_t>(postprocess.filmgrain.enable);
				postprocessing.fg_amount = postprocess.filmgrain.amount;
				// Vignette
				postprocessing.enable_vignette = static_cast<uint32_t>(postprocess.vignette.enable);
				postprocessing.v_inner_radius = postprocess.vignette.inner;
				postprocessing.v_outer_radius = postprocess.vignette.outer;
				postprocessing.v_opacity = postprocess.vignette.opacity;
				// Motion blur
				postprocessing.enable_motion_blur = static_cast<uint32_t>(postprocess.motionblur.enable);
				postprocessing.mb_scale = (delta_time / 0.01666666f) * postprocess.motionblur.scele;
				postprocessing.mb_max_samples = postprocess.motionblur.max_samples;
				postprocessing.mb_max_velocity = postprocess.motionblur.max_velocity;
				postprocessing.mb_threshold = postprocess.motionblur.threshold;

				const uint8_t* uniform_data = reinterpret_cast<const uint8_t*>(&postprocessing);
				uint64_t uniform_size = sizeof(shlib::GlobalPostProcessingData);

				render_context.UpdateGlobalBinding(2, std::span(uniform_data, uniform_size));
			}

			// Bind global buffers
			{
				shlib::GlobalBufferReference buffers{};

				// Update world matrices buffer
				buffers.matrixBuffer = world_matrices->GetBuffer()->GetAddress();
				world_matrices->Flush();

				// Update frustum sides buffer
				buffers.frustumSidesBuffer = world_frustum->GetBuffer()->GetAddress();
				world_frustum->Flush();

				// Update buffer with cameras
				buffers.cameraBuffer = world_cameras->GetBuffer()->GetAddress();
				world_cameras->Flush();

				// Update mesh infos
				buffers.meshBuffer = world_meshes->GetBuffer()->GetAddress();
				world_meshes->Flush();

				buffers.submeshBuffer = world_submeshes->GetBuffer()->GetAddress();
				world_submeshes->Flush();

				// Update materials
				buffers.materialBuffer = draw_materials->GetBuffer()->GetAddress();
				draw_materials->Flush();

				// Update lights
				buffers.lightsBuffer = scene_lights->GetBuffer()->GetAddress();
				scene_lights->Flush();

				buffers.cascadedShadowBuffer = scene_cascaded_shadows->GetBuffer()->GetAddress();
				scene_cascaded_shadows->Flush();

				buffers.omniShadowBuffer = omni_shadows->GetBuffer()->GetAddress();
				omni_shadows->Flush();

				buffers.spotShadowBuffer = spot_shadows->GetBuffer()->GetAddress();
				spot_shadows->Flush();

				draw_descriptions->Flush();

				const uint8_t* uniform_data = reinterpret_cast<const uint8_t*>(&buffers);
				uint64_t uniform_size = sizeof(shlib::GlobalBufferReference);

				render_context.UpdateGlobalBinding(1, std::span(uniform_data, uniform_size));
			}
			
			{
				shlib::GlobalUniformData frame_data{};
				frame_data.lightCount = light_count;
				frame_data.camera_count = camera_offset;
				frame_data.defaultCameraID = 0;
				frame_data.draw_count = draw_count;
				frame_data.mesh_count = submesh_count;

				const uint8_t* uniform_data = reinterpret_cast<const uint8_t*>(&frame_data);
				uint64_t uniform_size = sizeof(shlib::GlobalUniformData);

				render_context.UpdateGlobalBinding(0, std::span(uniform_data, uniform_size));
			}
		}

		void RenderWorld::EndFrame()
		{
			draw_count = 0;
			light_count = 0;
			cascaded_shadow_count = 0;
			omni_shadow_count = 0;
			spot_shadow_count = 0;
		}

		edge::gfx::Buffer* RenderWorld::GetDrawBuffer()
		{
			return draw_descriptions->GetBuffer();
		}

		uint32_t RenderWorld::GetDrawCount() const
		{
			return draw_count;
		}

		uint32_t RenderWorld::GetMeshCount() const
		{
			return submesh_count;
		}

		void RenderWorld::UpdateCameraFrustum(uint32_t camera_id)
		{
			auto& world_resource = world_resources[camera_id];
			auto& camera = world_cameras->Get(world_resource.resource_id);

			auto* frustum_begin = world_frustum->Data(camera.frustum_offset);
			world_frustum->MarkDirty(camera.frustum_offset, 6u);

			auto* matrices = world_matrices->Data(camera.matrices_offset);

			auto& clip_matrix = matrices[shlib::view_projection_matrix_offset];

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
}