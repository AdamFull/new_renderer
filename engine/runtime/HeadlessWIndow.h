#pragma once

#include <engine/runtime/Window.h>

namespace engine
{
	class HeadlessWindow final : public Window
	{
	public:
		HeadlessWindow(const Window::Properties& properties);
		virtual ~HeadlessWindow() = default;

		/**
		 * @brief A direct window doesn't have a surface
		 * @returns nullptr
		 */
		//vk::SurfaceKHR CreateSurface(vk::Instance instance) override;

		bool ShouldClose() override;

		void Close() override;

		float GetDpiFactor() const override;

		std::vector<const char*> GetRequiredSurfaceExtensions() const override;

	private:
		bool closed{ false };
	};
}