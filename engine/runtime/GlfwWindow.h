#pragma once

#include <engine/runtime/Window.h>

struct GLFWwindow;

namespace engine
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

		vk::SurfaceKHR CreateSurface(vk::Instance instance, vk::AllocationCallbacks* callbacks) override;

		bool ShouldClose() override;

		void ProcessEvents() override;

		void Close() override;

		float GetDpiFactor() const override;

		void SetTitle(const std::string& new_title) override;

		float GetContentScaleFactor() const override;

		std::vector<const char*> GetRequiredSurfaceExtensions() const override;
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