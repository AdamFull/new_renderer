#pragma once

#include "../gfx_structures.h"
#include "glad/glad.h"

namespace edge
{
	namespace gfx
	{
		namespace opengl
		{
			inline uint32_t BufferUsageToGLTarget(BufferUsageFlags flags)
			{
				if (flags & BufferUsageFlag::eVertex)
					return GL_ARRAY_BUFFER;
				else if (flags & BufferUsageFlag::eIndex)
					return GL_ELEMENT_ARRAY_BUFFER;
				else if (and_(flags, BufferUsageFlag::eCopy | BufferUsageFlag::eCpuToGpu))
					return GL_COPY_READ_BUFFER;
				else if (and_(flags, BufferUsageFlag::eCopy | BufferUsageFlag::eGpuOnly))
					return GL_COPY_WRITE_BUFFER;
				else if (flags & BufferUsageFlag::eIndirect)
					return GL_DRAW_INDIRECT_BUFFER;
				else if (flags & BufferUsageFlag::eUniform)
					return GL_UNIFORM_BUFFER;
				else if (flags & BufferUsageFlag::eStorage)
					return GL_SHADER_STORAGE_BUFFER;

				assert(false && "NOT SUPPORTED");
				return 0;
			}

			inline uint32_t BufferUsageToGLUsage(BufferUsageFlags flags)
			{
				if ((flags & BufferUsageFlag::eUniform) || (flags & BufferUsageFlag::eStorage))
					return GL_DYNAMIC_DRAW;

				return GL_STATIC_DRAW;
			}
		}
	}
}