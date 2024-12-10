#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include "runtime/InputEvents.h"

namespace engine
{
	class Window;
	class InputManager;
	class Application;

	struct ApplicationCreateInfo
	{
		std::string name;
		std::function<std::unique_ptr<Application>()> app_allocator;
	};

	struct ApplicationOptions
	{
		bool benchmark_enabled;
		Window* window;
		filesystem::IVirtualFileSystemInterface* virtual_filesystem;
		InputManager* input_manager;
	};

	class Application
	{
	public:
		Application() = default;

		virtual ~Application() = default;

		/**
		* @brief Prepares the application for execution
		*/
		virtual bool Prepare(const ApplicationOptions& options);

		/**
		 * @brief Updates the application
		 * @param delta_time The time since the last update
		 */
		virtual void Update(float delta_time);

		/**
		 * @brief Handles cleaning up the application
		 */
		virtual void Finish();

		virtual bool OnResize(uint32_t width, uint32_t height);

		virtual void OnInputEvent(const InputEvent& input_event);

		inline bool ShouldClose() const
		{
			return requested_close;
		}

		// request the app to close
		// does not guarantee that the app will close immediately
		inline void Close()
		{
			requested_close = true;
		}

		void SetName(std::string_view name);
		const std::string& GetName() const;

	protected:
		Window* window;

		std::string name;
		bool lock_simulation_speed{ false };
	private:
		bool requested_close{ false };
	};
}