#pragma once

#include "Window.h"

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
		vk::SurfaceKHR CreateSurface(vk::Instance instance, vk::AllocationCallbacks* callbacks) override;

		bool ShouldClose() override;

		void Close() override;

		float GetDpiFactor() const override;

		void SetTitle(std::string_view new_title) override;

		std::vector<const char*> GetRequiredSurfaceExtensions() const override;

	private:
		bool closed{ false };
	};
}