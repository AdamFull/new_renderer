#pragma once

#include <optional>
#include <string>
#include <vector>

#include "NativeSurface.h"

namespace edge
{
	/**
	 * @brief An interface class, declaring the behaviour of a Window
	 */
	class Window
	{
	public:
		struct Extent
		{
			uint32_t width;
			uint32_t height;
		};

		struct OptionalExtent
		{
			std::optional<uint32_t> width;
			std::optional<uint32_t> height;
		};

		enum class Mode
		{
			Headless,
			Fullscreen,
			FullscreenBorderless,
			FullscreenStretch,
			Default
		};

		enum class Vsync
		{
			OFF,
			ON,
			Default
		};

		struct OptionalProperties
		{
			std::optional<std::string> title;
			std::optional<Mode> mode;
			std::optional<bool> resizable;
			std::optional<Vsync> vsync;
			OptionalExtent extent;
		};

		struct Properties
		{
			std::string title = "";
			Mode mode = Mode::Default;
			bool resizable = true;
			Vsync vsync = Vsync::Default;
			Extent extent = { 1280, 720 };
		};

		/**
		 * @brief Constructs a Window
		 * @param properties The preferred configuration of the window
		 */
		Window(const Properties& properties);

		virtual ~Window() = default;

		/**
		 * @brief Gets current platform native surface
		 * @returns A pointer to native surface handle
		 */
		[[nodiscard]] auto GetNativeSurface() const noexcept -> NativeSurface*
		{
			return native_surface;
		}

		/**
		 * @brief Checks if the window should be closed
		 */
		[[nodiscard]] virtual auto ShouldClose() const noexcept -> bool = 0;

		/**
		 * @brief Handles the processing of all underlying window events
		 */
		virtual void ProcessEvents() {}

		/**
		 * @brief Requests to close the window
		 */
		virtual void Close() = 0;

		/**
		 * @return The dot-per-inch scale factor
		 */
		[[nodiscard]] virtual auto GetDpiFactor() const noexcept -> float = 0;

		/**
		 * @return The scale factor for systems with heterogeneous window and pixel coordinates
		 */
		[[nodiscard]] virtual auto GetContentScaleFactor() const noexcept -> float
		{
			return 1.0f;
		}

		/**
		 * @brief Windows title setter
		 * @param new_title New window title
		 */
		virtual void SetTitle(std::string_view new_title) const = 0;

		/**
		 * @brief Attempt to Resize the window - not guaranteed to change
		 *
		 * @param extent The preferred window extent
		 * @return Extent The new window extent
		 */
		[[nodiscard]] auto Resize(const Extent& extent) noexcept -> Extent;

		[[nodiscard]] virtual auto GetRequiredSurfaceExtensions() const ->std::vector<const char*> = 0;

		[[nodiscard]] auto GetExtent() const noexcept -> Extent const&
		{
			return properties.extent;
		}

		[[nodiscard]] auto GetWindowMode() const noexcept -> Mode
		{
			return properties.mode;
		}

		inline const Properties& GetProperties() const
		{
			return properties;
		}

	protected:
		Properties properties;
		NativeSurface* native_surface{ nullptr };
	};
}