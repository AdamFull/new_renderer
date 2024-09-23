#pragma once

#include <engine/Application.h>
#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/interface/render/RenderInterface.h>

#include <engine/InputMapper.h>

namespace engine
{
	class EngineApplication : public Application
	{
	public:
		EngineApplication() = default;

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

	};
}