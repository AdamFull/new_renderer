#pragma once

#include "Application.h"
#include "rendering/RendererInterface.h"

#include "InputMapper.h"
#include "EngineSystemManager.h"

#include "ui/windows/EditorWindow.h"

namespace engine
{
	namespace world
	{
		class Scene;
	}

	class EngineApplication : public Application
	{
	public:
		EngineApplication();

		virtual ~EngineApplication() override;
	protected:
		void OnInputEvent(const InputEvent& input_event) override;
		void Finish() override;
		bool OnResize(uint32_t width, uint32_t height) override;

		/**
		* @brief Additional sample initialization
		*/
		bool Prepare(const ApplicationOptions& options) override;

		/**
		 * @brief Main loop sample events
		 */
		void Update(float delta_time) override;
	private:
		std::unique_ptr<render::IRenderer> renderer{};
		//std::unique_ptr<CEngineSystemManager> m_pSystemManager{};
		//std::unique_ptr<CInputMapper> m_pInputMapper{};
		//std::unique_ptr<render::IRenderInterface> m_pRenderInterface{};
		std::unique_ptr<world::Scene> scene{};

		// Engine ui windows (imgui)
		utl::vector<std::unique_ptr<IEditorWindow>> app_windows{};
	};
}