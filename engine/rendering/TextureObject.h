#pragma once

namespace engine
{
	namespace gfx
	{
		class Context;
		class Texture;
		class Sampler;
	}

	namespace render
	{
		class TextureObject final
		{
		public:
			TextureObject(gfx::Context& context);
			~TextureObject();

			gfx::Texture& GetHandle();
			gfx::Texture const& GetHandle() const;

			gfx::Sampler& GetSampler();
			gfx::Sampler const& GetSampler() const;
		private:
			gfx::Context& context;

			gfx::Texture* handle{ nullptr };
			gfx::Sampler* sampler{ nullptr };
		};
	}
}