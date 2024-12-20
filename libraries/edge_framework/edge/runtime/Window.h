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
		virtual NativeSurface* GetNativeSurface();

		/**
		 * @brief Checks if the window should be closed
		 */
		virtual bool ShouldClose() = 0;

		/**
		 * @brief Handles the processing of all underlying window events
		 */
		virtual void ProcessEvents();

		/**
		 * @brief Requests to close the window
		 */
		virtual void Close() = 0;

		/**
		 * @return The dot-per-inch scale factor
		 */
		virtual float GetDpiFactor() const = 0;

		/**
		 * @return The scale factor for systems with heterogeneous window and pixel coordinates
		 */
		virtual float GetContentScaleFactor() const;

		/**
		 * @brief Windows title setter
		 * @param new_title New window title
		 */
		virtual void SetTitle(std::string_view new_title) = 0;

		/**
		 * @brief Attempt to Resize the window - not guaranteed to change
		 *
		 * @param extent The preferred window extent
		 * @return Extent The new window extent
		 */
		Extent Resize(const Extent& extent);

		virtual std::vector<const char*> GetRequiredSurfaceExtensions() const = 0;

		const Extent& GetExtent() const;

		Mode GetWindowMode() const;

		inline const Properties& GetProperties() const
		{
			return properties;
		}

	protected:
		Properties properties;
		NativeSurface* native_surface{ nullptr };
	};
}