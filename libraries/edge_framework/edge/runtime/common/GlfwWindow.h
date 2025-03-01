#pragma once

#include "../Window.h"

struct GLFWwindow;

namespace edge
{
	class Runtime;

	/**
	 * @brief An implementation of GLFW, inheriting the behaviour of the Window interface
	 */
	class GlfwWindow final : public Window
	{
	public:
		GlfwWindow(Runtime* platform, const Window::Properties& properties);

		virtual ~GlfwWindow();

		[[nodiscard]] auto ShouldClose() const noexcept -> bool override;

		void ProcessEvents() override;

		void Close() override;

		[[nodiscard]] auto GetDpiFactor() const noexcept -> float override;

		void SetTitle(const std::string_view new_title) const override;

		[[nodiscard]] auto GetContentScaleFactor() const noexcept -> float override;

		[[nodiscard]] auto GetRequiredSurfaceExtensions() const -> std::vector<const char*> override;
	protected:
		static void WindowCloseCallback(GLFWwindow* window);
		static void WindowSizeCallback(GLFWwindow* window, int width, int height);
		static void WindowFocusCallback(GLFWwindow* window, int focused);
		static void KeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/);
		static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/);
		static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void CharacterInputCallback(GLFWwindow* window, uint32_t codepoint);

	private:
		GLFWwindow* handle{ nullptr };
	};
}