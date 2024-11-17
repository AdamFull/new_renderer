#include "RenderWorld.h"
#include <engine/rendering/Rendering.h>

#include <glm/gtx/quaternion.hpp>

#include "Globals.h"

namespace engine
{
	namespace render
	{
		RenderWorld::RenderWorld(IRenderer& renderer) :
			renderer{ renderer }
		{
			auto& resource_context = renderer.GetResourceContext();

			world_nodes = std::make_unique<DynamicBuffer<FSceneNode>>(resource_context, 8ull, gfx::BufferUsageFlag::eStorageBuffer);
			draw_descriptions = std::make_unique<DynamicBuffer<DrawDescription>>(resource_context, 16ull, gfx::BufferUsageFlag::eStorageBuffer);
			world_meshes = std::make_unique<DynamicBuffer<FMesh>>(resource_context, 16ull, gfx::BufferUsageFlag::eStorageBuffer);

			scene_lights = std::make_unique<DynamicBuffer<FLightCommit>>(resource_context, 16ull, gfx::BufferUsageFlag::eStorageBuffer);

			scene_cascaded_shadows = std::make_unique<DynamicBuffer<FCascadedShadowCommit>>(resource_context, 16ull, gfx::BufferUsageFlag::eStorageBuffer);
			omni_shadows = std::make_unique<DynamicBuffer<FOmniShadowCommit>>(resource_context, 16ull, gfx::BufferUsageFlag::eStorageBuffer);
			spot_shadows = std::make_unique<DynamicBuffer<FSpotShadowCommit>>(resource_context, 16ull, gfx::BufferUsageFlag::eStorageBuffer);

			draw_materials = std::make_unique<DynamicBuffer<FMaterial>>(resource_context, 16ull, gfx::BufferUsageFlag::eStorageBuffer);

			world_cameras = std::make_unique<DynamicBuffer<FCamera>>(resource_context, 8ull, gfx::BufferUsageFlag::eStorageBuffer);
			world_matrices = std::make_unique<DynamicBuffer<glm::mat4>>(resource_context, 64ull, gfx::BufferUsageFlag::eStorageBuffer);
			world_frustum = std::make_unique<DynamicBuffer<glm::vec4>>(resource_context, 64ull, gfx::BufferUsageFlag::eStorageBuffer);

			indirect_draw_commands = std::make_unique<DynamicBuffer<FDrawIndexedIndirectData>>(resource_context, 64ull, gfx::BufferUsageFlag::eIndirectBuffer);
		}

		uint32_t RenderWorld::CreateRenderCamera()
		{
			auto& world_resource = world_resources.emplace_back();
			world_resource.type = WorldResourceType::eCamera;
			world_resource.resource_id = camera_offset++;

			auto& camera = world_cameras->Get(world_resource.resource_id);
			camera.matrices_offset = matrices_offset;
			matrices_offset += matrix_offset_count;

			camera.frustum_offset = frustum_offset;
			frustum_offset += 6ull; // Because 6 frustum sides

			return world_resources.size() - 1ull;
		}

		FCamera& RenderWorld::GetRenderCamera(uint32_t camera_id)
		{
			auto& world_resource = world_resources[camera_id];
			return world_cameras->Get(world_resource.resource_id);
		}

		glm::mat4 const& RenderWorld::GetCameraMatrix(uint32_t camera_id, uint32_t matrix_index)
		{
			auto& world_resource = world_resources[camera_id];
			auto& camera = world_cameras->Get(world_resource.resource_id);

			auto* matrix_begin = world_matrices->Data(camera.matrices_offset);
			return *(matrix_begin + matrix_index);
		}

		void RenderWorld::UpdateRenderCamera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation)
		{
			auto& world_resource = world_resources[camera_id];
			auto& camera = world_cameras->Get(world_resource.resource_id);

			uint32_t camera_hash{ 0u };
			camera_hash = utl::murmur_hash(reinterpret_cast<const char*>(&translation.x), sizeof(glm::vec3), camera_hash);
			camera_hash = utl::murmur_hash(reinterpret_cast<const char*>(&rotation.x), sizeof(glm::quat), camera_hash);
			camera_hash = utl::murmur_hash(camera.fov, camera_hash);
			camera_hash = utl::murmur_hash(camera.z_near, camera_hash);
			camera_hash = utl::murmur_hash(camera.z_far, camera_hash);
			camera_hash = utl::murmur_hash(camera.aspect, camera_hash);

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
				world_matrices->MarkDirty(camera.matrices_offset, matrix_offset_count);

				matrices[view_matrix_offset] = glm::lookAt(translation, translation + forward, up);
				matrices[inv_view_matrix_offset] = glm::inverse(matrices[view_matrix_offset]);
				matrices[projection_matrix_offset] = glm::perspective(camera.fov, camera.aspect, camera.z_near, camera.z_far);
				matrices[inv_projection_matrix_offset] = glm::inverse(matrices[projection_matrix_offset]);
				matrices[prev_view_projection_matrix_offset] = matrices[view_projection_matrix_offset];
				matrices[view_projection_matrix_offset] = matrices[projection_matrix_offset] * matrices[view_matrix_offset];
				matrices[inv_view_projection_matrix_offset] = glm::inverse(matrices[view_projection_matrix_offset]);

				// Calculate frustum
				UpdateCameraFrustum(camera_id);

				// Update reprojection
				auto& camera_reprojection = matrices[reprojection_matrix_offset];
				camera_reprojection = matrices[prev_view_projection_matrix_offset] * matrices[inv_view_projection_matrix_offset];

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

		void RenderWorld::UpdateRenderCamera(uint32_t camera_id, const glm::mat4& view, const glm::mat4& projection, uint32_t width, uint32_t height)
		{
			auto& world_resource = world_resources[camera_id];
			auto& camera = world_cameras->Get(world_resource.resource_id);

			uint32_t camera_hash{ 0u };
			camera_hash = utl::murmur_hash(reinterpret_cast<const char*>(&view[0]), sizeof(glm::mat4), camera_hash);
			camera_hash = utl::murmur_hash(reinterpret_cast<const char*>(&projection[0]), sizeof(glm::mat4), camera_hash);
			camera_hash = utl::murmur_hash(camera.fov, camera_hash);
			camera_hash = utl::murmur_hash(camera.z_near, camera_hash);
			camera_hash = utl::murmur_hash(camera.z_far, camera_hash);
			camera_hash = utl::murmur_hash(camera.aspect, camera_hash);

			bool dirty = camera_hash != camera.last_hash;
			camera.last_hash = camera_hash;

			if (dirty)
			{
				camera.size = { static_cast<float>(width), static_cast<float>(height) };
				camera.invSize = 1.f / camera.size;

				camera.aspect = camera.size.x / camera.size.y;

				auto* matrices = world_matrices->Data(camera.matrices_offset);
				world_matrices->MarkDirty(camera.matrices_offset, matrix_offset_count);

				matrices[view_matrix_offset] = view;
				matrices[projection_matrix_offset] = projection;
				matrices[view_projection_matrix_offset] = projection * view;

				UpdateCameraFrustum(camera_id);
			}
		}

		void RenderWorld::UpdateNode(uint32_t node_id, const glm::mat4& matrix)
		{
			auto& node = world_nodes->Get(node_id);
			world_nodes->MarkDirty(node_id);

			if (node.matrixOffset == 0u)
			{
				node.matrixOffset = matrices_offset;
				matrices_offset += 2u;
			}

			auto* matrices = world_matrices->Data(node.matrixOffset);
			matrices[1ull] = matrices[0ull];
			matrices[0ull] = matrix;

			world_matrices->MarkDirty(node.matrixOffset, 2ull);
		}

		void RenderWorld::AddGeometryDraw(DrawCommand& draw_command)
		{
			uint32_t draw_index = draw_count++;

			// Describes current draw command resources
			auto& batch_command = draw_descriptions->Get(draw_index);
			draw_descriptions->MarkDirty(draw_index);

			batch_command.nodeID = draw_command.node_index;
			batch_command.meshID = draw_index;
			batch_command.vboID = draw_command.vertex_buffers;

			auto& mesh = world_meshes->Get(draw_index);
			world_meshes->MarkDirty(draw_index);
			mesh.materialID = draw_command.material_index;


			auto& indirect_command = indirect_draw_commands->Get(draw_index);
			indirect_draw_commands->MarkDirty(draw_index);

			indirect_command.indexCount = draw_command.index_count;
			indirect_command.instanceCount = 1;
			indirect_command.firstIndex = draw_command.first_index;
			indirect_command.vertexOffset = int(draw_command.first_vertex);
			indirect_command.firstInstance = 0;
		}

		void RenderWorld::AddMaterial(const FMaterial& material)
		{
			auto material_index = material_count++;
			auto& new_material = draw_materials->Get(material_index);
			draw_materials->MarkDirty(material_index);
			new_material = material;
		}

		const FMaterial& RenderWorld::GetMaterial(uint32_t index) const
		{
			return draw_materials->Get(index);
		}

		void RenderWorld::AddLight(const FLightCommit& light_data)
		{
			auto light_index = light_count++;
			auto& new_light = scene_lights->Get(light_index);
			scene_lights->MarkDirty(light_index);
			new_light = light_data;
		}

		void RenderWorld::AddShadow(const FCascadedShadowCommit& shadow_data)
		{
			auto shadow_index = cascaded_shadow_count++;
			auto& new_shadow = scene_cascaded_shadows->Get(shadow_index);
			scene_cascaded_shadows->MarkDirty(shadow_index);
			new_shadow = shadow_data;
		}

		void RenderWorld::AddShadow(const FOmniShadowCommit& shadow_data)
		{
			auto shadow_index = omni_shadow_count++;
			auto& new_shadow = omni_shadows->Get(shadow_index);
			omni_shadows->MarkDirty(shadow_index);
			new_shadow = shadow_data;
		}

		void RenderWorld::AddShadow(const FSpotShadowCommit& shadow_data)
		{
			auto shadow_index = spot_shadow_count++;
			auto& new_shadow = spot_shadows->Get(shadow_index);
			spot_shadows->MarkDirty(shadow_index);
			new_shadow = shadow_data;
		}

		void RenderWorld::BeginFrame(float delta_time)
		{
			auto& render_graph = renderer.GetRenderGraph();

			// Post process data
			{
				FGlobalPostProcessingData postprocessing{};
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
				uint64_t uniform_size = sizeof(FGlobalPostProcessingData);

				render_graph.UpdateGlobalBinding(2, std::span(uniform_data, uniform_size));
			}

			// Bind global buffers
			{
				FGlobalBufferReference buffers{};

				// Update nodes buffer
				buffers.nodeBuffer.address = world_nodes->GetBuffer()->GetAddress();
				world_nodes->Flush();

				// Update world matrices buffer
				buffers.matrixBuffer.address = world_matrices->GetBuffer()->GetAddress();
				world_matrices->Flush();

				// Update frustum sides buffer
				buffers.frustumSidesBuffer.address = world_frustum->GetBuffer()->GetAddress();
				world_frustum->Flush();

				// Update buffer with cameras
				buffers.cameraBuffer.address = world_cameras->GetBuffer()->GetAddress();
				world_cameras->Flush();

				// Update buffer with indirect draw commands
				buffers.indirectCommandBuffer.address = indirect_draw_commands->GetBuffer()->GetAddress();
				indirect_draw_commands->Flush();

				// Update buffer with per draw descriptions
				buffers.drawInfoBuffer.address = draw_descriptions->GetBuffer()->GetAddress();
				draw_descriptions->Flush();

				// Update mesh infos
				buffers.meshBuffer.address = world_meshes->GetBuffer()->GetAddress();
				world_meshes->Flush();

				// Update materials
				buffers.materialBuffer.address = draw_materials->GetBuffer()->GetAddress();
				draw_materials->Flush();

				// Update lights
				buffers.lightsBuffer.address = scene_lights->GetBuffer()->GetAddress();
				scene_lights->Flush();

				buffers.cascadedShadowBuffer.address = scene_cascaded_shadows->GetBuffer()->GetAddress();
				scene_cascaded_shadows->Flush();

				buffers.omniShadowBuffer.address = omni_shadows->GetBuffer()->GetAddress();
				omni_shadows->Flush();

				buffers.spotShadowBuffer.address = spot_shadows->GetBuffer()->GetAddress();
				spot_shadows->Flush();

				const uint8_t* uniform_data = reinterpret_cast<const uint8_t*>(&buffers);
				uint64_t uniform_size = sizeof(FGlobalBufferReference);

				render_graph.UpdateGlobalBinding(1, std::span(uniform_data, uniform_size));
			}
			
			{
				FGlobalUniformData frame_data{};
				frame_data.lightCount = light_count;
				frame_data.cameraCount = camera_offset;

				const uint8_t* uniform_data = reinterpret_cast<const uint8_t*>(&frame_data);
				uint64_t uniform_size = sizeof(FGlobalUniformData);

				render_graph.UpdateGlobalBinding(0, std::span(uniform_data, uniform_size));
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

		gfx::IndirectDrawCommand RenderWorld::GetDrawCommand()
		{
			gfx::IndirectDrawCommand draw_command{};
			draw_command.indexed_draw = true;
			draw_command.argument_buffer = indirect_draw_commands->GetBuffer();
			draw_command.draw_count = draw_count;
			draw_command.command_stride = sizeof(FDrawIndexedIndirectData);

			return draw_command;
		}

		void RenderWorld::UpdateCameraFrustum(uint32_t camera_id)
		{
			auto& world_resource = world_resources[camera_id];
			auto& camera = world_cameras->Get(world_resource.resource_id);

			auto* frustum_begin = world_frustum->Data(camera.frustum_offset);
			world_frustum->MarkDirty(camera.frustum_offset, 6u);

			auto* matrices = world_matrices->Data(camera.matrices_offset);

			auto& clip_matrix = matrices[view_projection_matrix_offset];

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