#include <TextureObject.h>

#include <gfx/Context.h>

namespace engine
{
	namespace render
	{
		TextureObject::TextureObject(gfx::Context& context) :
			context{ context }
		{

		}

		TextureObject::~TextureObject()
		{
			context.Destroy(handle);
		}


		gfx::Texture& TextureObject::GetHandle()
		{
			return *handle;
		}

		gfx::Texture const& TextureObject::GetHandle() const
		{
			return *handle;
		}

		gfx::Sampler& TextureObject::GetSampler()
		{
			return *sampler;
		}

		gfx::Sampler const& TextureObject::GetSampler() const
		{
			return *sampler;
		}
	}
}