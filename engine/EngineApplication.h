#pragma once

#include <edge/runtime/Application.h>
#include "rendering/RendererInterface.h"

#include "InputMapper.h"

#include "ui/EngineUI.h"
#include "ui/windows/EditorWindow.h"

namespace engine
{
	namespace world
	{
		class Scene;
	}

	class EngineApplication : public edge::Application
	{
	public:
		EngineApplication();

		virtual ~EngineApplication() override;
	protected:
		void OnInputEvent(const edge::InputEvent& input_event) override;
		void Finish() override;
		bool OnResize(uint32_t width, uint32_t height) override;

		/**
		* @brief Additional sample initialization
		*/
		bool Prepare(const edge::ApplicationOptions& options) override;

		/**
		 * @brief Main loop sample events
		 */
		void Update(float delta_time) override;
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