#pragma once

#include <edge/runtime/rt_application.h>
#include "ISubsystem.h"
#include "render/IRenderer.h"

#include "audio/IAudioEngine.h"

#include "audio/EffectGraphEditor.h"

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
		auto on_ui_render(float delta_time) -> void;

		uint32_t source_handle_{ ~0u };
		std::unique_ptr<audio::IAudioEngine> audio_engine_;
		std::unique_ptr<audio::EffectGraphEditor> effect_editor_;

		std::unique_ptr<IRenderer> renderer_;
		std::vector<std::unique_ptr<ISubsystem>> subsystems_{};
	};
}