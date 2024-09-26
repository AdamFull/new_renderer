#pragma once

#include <interface/EngineSystem.h>
#include <interface/render/RenderInterface.h>
#include <engine/runtime/InputEvents.h>

namespace engine
{
	class CDebugDrawEngineSystem : public IEngineSystem
	{
	public:
		CDebugDrawEngineSystem(render::IRenderInterface* render_interface);
		~CDebugDrawEngineSystem() = default;

		void Initialize() override;

		void PreUpdate(f32 dt) override;
		void Update(f32 dt) override;

		void Render() override;

		void Destroy() override;
	private:
		render::IRenderInterface* m_pRenderInterface{ nullptr };
	};
}