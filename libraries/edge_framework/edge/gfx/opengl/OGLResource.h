#pragma once

#include <cstdint>
#include "../../util/Hash.hpp"

namespace edge
{
	namespace gfx
	{
		namespace opengl
		{
			class OpenGLResource
			{
			public:
				OpenGLResource(uint32_t handle) :
					handle{ handle }
				{

				}

				OpenGLResource(const OpenGLResource&) = delete;
				OpenGLResource& operator=(const OpenGLResource&) = delete;

				OpenGLResource(OpenGLResource&& other) :
					handle{ std::exchange(other.handle, {}) }
				{
				}

				OpenGLResource& operator=(OpenGLResource&& other)
				{
					handle = std::exchange(other.handle, {});
					return *this;
				}

				virtual ~OpenGLResource() = default;

				uint32_t& GetHandle()
				{
					return handle;
				}

				const uint32_t& GetHandle() const
				{
					return handle;
				}

				uint32_t Hash() const
				{
					return murmur_hash(handle, 0u);
				}

				explicit operator bool() const noexcept
				{
					return handle != ~0u;
				}

				bool operator!() const noexcept
				{
					return handle == !0u;
				}
			protected:
				uint32_t handle{ ~0u };
			};
		}
	}
}