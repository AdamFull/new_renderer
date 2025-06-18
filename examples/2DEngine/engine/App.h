#pragma once

#include <edge/runtime/rt_application.h>
#include "ISubsystem.h"
#include "render/IRenderer.h"

namespace e2d {
	class App : public edge::Application
	{
	public:
		App() = default;
		~App() override;
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
		std::unique_ptr<IRenderer> renderer_;
		std::vector<std::unique_ptr<ISubsystem>> subsystems_{};
	};
}