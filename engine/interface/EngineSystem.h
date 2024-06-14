#pragma once 

#include <standart_library.h>

namespace engine
{
	class IEngineSystem
	{
	public:
		virtual ~IEngineSystem() = default;

		virtual void PreInitialize() {}
		virtual void Initialize() = 0;
		virtual void PostInitialize() {}

		virtual void PreUpdate(f32 dt) {}
		virtual void Update(f32 dt) = 0;
		virtual void PostUpdate(f32 dt) {}

		virtual void PreRender() {}
		virtual void Render() = 0;
		virtual void PostRender() {}

		virtual void PreDestroy() {}
		virtual void Destroy() = 0;
		virtual void PostDestroy() {}

		// TODO: in future AddSubSystem...
	};
}