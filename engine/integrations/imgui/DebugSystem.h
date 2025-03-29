#pragma once

#include <interface/EngineSystem.h>
#include <engine/rendering/RendererInterface.h>
#include <edge/runtime/rt_input_events.h>

namespace engine
{
	class CDebugDrawEngineSystem : public IEngineSystem
	{
	public:
		CDebugDrawEngineSystem(render::IRenderer& renderer);
		~CDebugDrawEngineSystem() = default;

		void Initialize() override;

		void PreUpdate(float dt) override;
		void Update(float dt) override;

		void Render() override;

		void Destroy() override;
	private:
		render::IRenderWorld& render_world;
	};
}