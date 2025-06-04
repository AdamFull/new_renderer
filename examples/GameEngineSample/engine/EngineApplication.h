#pragma once

#include <edge/runtime/rt_application.h>
#include "rendering/RendererInterface.h"

#include "ui/EngineUI.h"
#include "ui/windows/EditorWindow.h"

#include "world/Scene.h"

namespace engine {
	class EngineApplication : public edge::Application {
	public:
		EngineApplication() = default;
		virtual ~EngineApplication() override = default;
	protected:
		void on_input_event(const edge::InputEvent& input_event) override;
		void finish() override;
		bool on_resize(uint32_t width, uint32_t height) override;

		/**
		* @brief Additional sample initialization
		*/
		bool prepare(const edge::ApplicationOptions& options) override;

		/**
		 * @brief Main loop sample events
		 */
		void update(float delta_time) override;
	private:
		std::unique_ptr<EngineUI> ui{};
		std::unique_ptr<render::IRenderer> renderer{};
		//std::unique_ptr<CEngineSystemManager> m_pSystemManager{};
		//std::unique_ptr<CInputMapper> m_pInputMapper{};
		//std::unique_ptr<render::IRenderInterface> m_pRenderInterface{};
		std::unique_ptr<world::Scene> scene{};

		// Engine ui windows (imgui)
		std::vector<std::unique_ptr<IEditorWindow>> app_windows{};
	};
}