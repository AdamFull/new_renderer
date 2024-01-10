#pragma once 

#include <string>

#include <embed/shaders/includes/interop/base.h>

namespace engine
{
	namespace render
	{
		class IRenderPass
		{
		public:
			virtual ~IRenderPass() = default;

			virtual void create() = 0;

			virtual void prepare() = 0;
			virtual void execute(float dt) = 0;
		};

		class IRenderGraph
		{
		public:
			virtual ~IRenderGraph() = default;

			virtual size_t addRenderResource(const std::string& name) = 0; // One for texture and other for buffer

			virtual void bake() = 0;

			virtual void execute(float dt) = 0;
		};
	}
}