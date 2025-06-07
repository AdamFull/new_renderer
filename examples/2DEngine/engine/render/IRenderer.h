#pragma once

#include <edge/gfx/gfx_base.h>

namespace e2d {
	enum RendererBufferization {
		eAuto,
		eDouble,
		eTriple
	};

	struct RendererCreateInfo {
		const edge::Window* window{ nullptr };
		RendererBufferization bufferization{};
	};

	class IRendererFrame {
	public:
		virtual ~IRendererFrame() = default;

		virtual void reset() = 0;
		virtual void update(const edge::gfx::ISwapchain& swapchain) = 0;

		virtual auto get_backbuffer() const -> edge::gfx::ITexture& = 0;
		virtual auto get_or_allocate_binding_layout(edge::gfx::IDescriptorLayout& descriptor_layout) -> edge::gfx::IBindingLayout& = 0;
	};

	class IRenderer {
	public:
		virtual ~IRenderer() = default;
		
		virtual void begin_frame(float delta_time) = 0;
		virtual void end_frame(float delta_time) = 0;

		virtual void on_surface_update() = 0;
	};
}