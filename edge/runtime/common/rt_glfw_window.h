#pragma once

#include "../rt_window.h"

struct GLFWwindow;

namespace edge
{
	class Runtime;

	/**
	 * @brief An implementation of GLFW, inheriting the behaviour of the Window interface
	 */
	class GlfwWindow final : public Window {
	public:
		GlfwWindow(Runtime* platform, const Window::Properties& properties);

		virtual ~GlfwWindow();

		[[nodiscard]] auto should_close() const noexcept -> bool override;

		void process_events() override;

		void close() override;

		[[nodiscard]] auto get_dpi_factor() const noexcept -> float override;

		void set_title(const std::string_view new_title) const override;

		[[nodiscard]] auto get_content_scale_factor() const noexcept -> float override;

		[[nodiscard]] auto get_required_surface_extensions() const -> std::vector<const char*> override;
	protected:
		static void window_close_callback(GLFWwindow* window);
		static void window_size_callback(GLFWwindow* window, int width, int height);
		static void window_focus_callback(GLFWwindow* window, int focused);
		static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/);
		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
		static void mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/);
		static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		static void character_input_callback(GLFWwindow* window, uint32_t codepoint);

	private:
		GLFWwindow* handle{ nullptr };
	};
}