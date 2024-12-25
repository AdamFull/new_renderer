#include "OGLCommandList.h"

#include "glad/glad.h"

#include "OGLBuffer.h"

namespace edge
{
	namespace gfx
	{
		namespace opengl
		{
			void OpenGLCommandList::SetResourceName(std::string_view name)
			{

			}

			void OpenGLCommandList::Reset(CommandPoolResetMode reset_mode)
			{
				// Not needed to implement
			}

			void OpenGLCommandList::Begin()
			{
				// Not needed to implement
			}

			void OpenGLCommandList::End()
			{
				// Not needed to implement
			}

			void OpenGLCommandList::SetViewport(float x, float y, float width, float height, float min_depth, float max_depth)
			{
				glViewport(x, y, width, height);
				glDepthRange(min_depth, max_depth);
			}

			void OpenGLCommandList::SerScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
			{
				glScissor(x, y, width, height);
			}

			void OpenGLCommandList::BindPipeline(Pipeline* pipeline)
			{
				// ???
			}

			void OpenGLCommandList::BindDescriptors(const BindDescriptorsInfo& bind_info)
			{
				// ???
			}

			void OpenGLCommandList::PushConstants(Shader* shader, RootSignature* root_signature, std::span<const char> constant_data)
			{
				// ???
			}

			void OpenGLCommandList::BindIndexBuffer(Buffer* index_buffer, IndexType index_type, uint64_t offset)
			{
				assert(false && "NOT IMPLEMENTED");
				GLenum gl_index_type = (index_type == IndexType::eUint16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
				//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer->GetHandle());
			}

			void OpenGLCommandList::BindVertexBuffers(uint32_t buffer_count, Buffer** vertex_buffers, const uint32_t* strides, const uint64_t* offsets)
			{
				assert(false && "NOT IMPLEMENTED");
				//for (int32_t i = 0; i < buffer_count; ++i) 
				//{
				//	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[i]->GetHandle());
				//	glVertexAttribPointer(i, strides[i] / sizeof(float), GL_FLOAT, GL_FALSE, strides[i], reinterpret_cast<const void*>(offsets[i]));
				//	glEnableVertexAttribArray(i);
				//}
			}

			void OpenGLCommandList::Draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count)
			{
				glDrawArraysInstanced(GL_TRIANGLES, first_vertex, vertex_count, instance_count);
			}

			void OpenGLCommandList::DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
			{
				glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, reinterpret_cast<void*>(first_index * sizeof(uint32_t)), instance_count);
			}

			void OpenGLCommandList::DrawIndirect(const IndirectDrawCommand& draw_command)
			{
				assert(false && "NOT IMPLEMENTED");
				auto* argument_buffer = static_cast<OpenGLBuffer*>(draw_command.argument_buffer);
				auto* count_buffer = static_cast<OpenGLBuffer*>(draw_command.count_buffer);

				if (count_buffer)
				{
					assert(false && "Draw indirect count is not supported.");
					return;
				}

				glBindBuffer(GL_DRAW_INDIRECT_BUFFER, argument_buffer->GetHandle());

				if (draw_command.indexed_draw)
				{
					
				}
				else
				{
					glDrawArraysIndirect(GL_TRIANGLES, reinterpret_cast<void*>(draw_command.argument_offset));
				}

				glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			}

			void OpenGLCommandList::Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
			{
				glDispatchCompute(group_x, group_y, group_z);
			}

			void OpenGLCommandList::DispatchIndirect(Buffer* argument_buffer, uint64_t argument_offset)
			{
				auto* buffer = static_cast<OpenGLBuffer*>(argument_buffer);

				glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer->GetHandle());
				glDispatchComputeIndirect(argument_offset);
				glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
			}

			void OpenGLCommandList::CopyBuffer(const CopyBufferInfo& copy_info)
			{
				auto* src_buffer = static_cast<OpenGLBuffer*>(copy_info.src_buffer);
				auto* dst_buffer = static_cast<OpenGLBuffer*>(copy_info.dst_buffer);

				glBindBuffer(GL_COPY_READ_BUFFER, src_buffer->GetHandle());
				glBindBuffer(GL_COPY_WRITE_BUFFER, dst_buffer->GetHandle());

				for (int32_t i = 0; i < copy_info.region_count; ++i)
				{
					const auto& region = copy_info.regions[i];
					glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, region.src_offset, region.dst_offset, region.size);
				}

				glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
				glBindBuffer(GL_COPY_READ_BUFFER, 0);
			}

			void OpenGLCommandList::CopyTexture(const CopyTextureInfo& copy_info)
			{
				assert(false && "NOT IMPLEMENTED");
				//glCopyImageSubData(copy_info.srcTexture->GetHandle(), GL_TEXTURE_2D, copy_info.srcLevel, copy_info.srcX, copy_info.srcY, copy_info.srcZ,
				//	copy_info.dstTexture->GetHandle(), GL_TEXTURE_2D, copy_info.dstLevel, copy_info.dstX, copy_info.dstY, copy_info.dstZ,
				//	copy_info.width, copy_info.height, copy_info.depth);
			}

			void OpenGLCommandList::ResourceBarrier(const ResourceBarrierInfo& barrier)
			{
				assert(false && "NOT IMPLEMENTED");
				//glMemoryBarrier(barrier.barrierMask);
			}

			void OpenGLCommandList::ResetQuery(QueryPool* query_pool, uint32_t first_query, uint32_t query_count)
			{
				assert(false && "NOT IMPLEMENTED");
				//for (int32_t i = 0; i < query_count; ++i)
				//{
				//	glQueryCounter(query_pool->GetQueryHandle(first_query + i), GL_QUERY_RESULT_NO_WAIT);
				//}
			}

			void OpenGLCommandList::BeginQuery(QueryPool* query_pool, uint32_t query)
			{
				assert(false && "NOT IMPLEMENTED");
				//glBeginQuery(GL_TIME_ELAPSED, query_pool->GetQueryHandle(query));
			}

			void OpenGLCommandList::EndQuery(QueryPool* query_pool, uint32_t query)
			{
				assert(false && "NOT IMPLEMENTED");
				//glEndQuery(GL_TIME_ELAPSED);
			}

			void OpenGLCommandList::BeginRendering(const RenderingInfo& rendering_info)
			{
				assert(false && "NOT IMPLEMENTED");
				//glBindFramebuffer(GL_FRAMEBUFFER, rendering_info.framebuffer);
				//glClearColor(rendering_info.clearColor[0], rendering_info.clearColor[1], rendering_info.clearColor[2], rendering_info.clearColor[3]);
				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}

			void OpenGLCommandList::EndRendering()
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			void OpenGLCommandList::BeginMarker(std::string_view name, uint32_t color)
			{
				glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name.data());
			}

			void OpenGLCommandList::InsertMarker(std::string_view name, uint32_t color) 
			{
				glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, name.data());
			}

			void OpenGLCommandList::EndMarker()
			{
				glPopDebugGroup();
			}
		}
	}
}