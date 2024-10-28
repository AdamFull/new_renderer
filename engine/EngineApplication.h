#pragma once

#include <engine/Application.h>
#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/RendererInterface.h>

#include <engine/InputMapper.h>
#include <engine/EngineSystemManager.h>

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
		std::unique_ptr<filesystem::IVirtualFileSystemInterface> m_pVirtualFileSystem{};
		std::unique_ptr<world::Scene> scene{};
	};
}