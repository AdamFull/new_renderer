#pragma once

#include <standart_library.h>

namespace engine
{
	class IApplication
	{
	public:
		virtual ~IApplication() = default;

		virtual void PreInitialize() {}
		virtual void Initialize() = 0;
		virtual void PostInitialize() {}

		virtual void PreUpdate(f32 dt) {}
		virtual void Update(f32 dt) = 0;
		virtual void PostUpdate(f32 dt) {}

		virtual void Render() = 0;

		virtual void SceneLoaded() {}
	};
}