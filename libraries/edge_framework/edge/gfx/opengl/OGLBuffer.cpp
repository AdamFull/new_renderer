#include "OGLBuffer.h"

#include "OGLUtil.h"

namespace edge
{
	namespace gfx
	{
		namespace opengl
		{
			OpenGLBuffer::OpenGLBuffer(const BufferCreateInfo& create_info) :
				OpenGLResource(~0u),
				size{ create_info.size },
				suballocation_count{ create_info.count },
				usage_flags{ create_info.usage }
			{
				target = BufferUsageToGLTarget(create_info.usage);

				glGenBuffers(1, &handle);
				glBindBuffer(target, handle);
				auto gl_usage = BufferUsageToGLUsage(create_info.usage);
				glBufferData(target, create_info.size, nullptr, gl_usage);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

			OpenGLBuffer::~OpenGLBuffer()
			{
				if (handle != ~0u)
				{
					glDeleteBuffers(1, &handle);
				}
			}

			void OpenGLBuffer::SetResourceName(std::string_view name)
			{
				glObjectLabel(GL_BUFFER, handle, static_cast<GLsizei>(name.size()), name.data());
			}

			uint8_t* OpenGLBuffer::Map()
			{
				assert(false && "NOT IMPLEMENTED");

				if (!mapped_memory)
				{
					glBindBuffer(target, handle);
					mapped_memory = static_cast<uint8_t*>(glMapBuffer(target, GL_WRITE_ONLY));
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}

				return mapped_memory;
			}

			void OpenGLBuffer::Unmap()
			{
				assert(false && "NOT IMPLEMENTED");

				assert(mapped_memory && "Buffer is not mapped yet!");
				if (mapped_memory)
				{
					glBindBuffer(target, handle);
					glUnmapBuffer(target);
					mapped_memory = nullptr;
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}
			}

			uint64_t OpenGLBuffer::Update(const void* data, uint64_t size, uint64_t offset)
			{
				assert(false && "NOT IMPLEMENTED");

				if (offset + size > this->size) 
				{
					//std::cerr << "Error: Buffer overflow during update." << std::endl;
					return 0;
				}

				glBindBuffer(target, handle);
				glBufferSubData(target, offset, size, data);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				return size;
			}

			uint64_t OpenGLBuffer::GetSize() const
			{
				return size * suballocation_count;
			}

			uint64_t OpenGLBuffer::GetAddress() const
			{
				assert(false && "FEATURE IS NOT SUPPORTED");
				return static_cast<uint64_t>(handle);
			}

			BufferUsageFlags OpenGLBuffer::GetUsageFlags() const
			{
				return usage_flags;
			}
		}
	}
}