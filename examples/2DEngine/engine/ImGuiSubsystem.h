#pragma once
#include <unordered_map>

#include "ISubsystem.h"

#include <edge/gfx/gfx_base.h>

namespace e2d {
	class IRenderer;
	class IRGPass;

	class ImGuiSubsystem final : public ISubsystem {
	public:
		ImGuiSubsystem(IRenderer& renderer);
		~ImGuiSubsystem() override;

		static auto construct(IRenderer& renderer, OnSubsystemProcessCb cb) -> std::unique_ptr<ISubsystem>;

		auto initialize() -> void;

		auto set_on_process(OnSubsystemProcessCb cb) -> void override;
		auto process(float delta_time) -> void override;
		auto on_input_event(const edge::InputEvent& input_event) -> void override;

		auto load_font(std::string_view path) -> void;

		auto setup_resources(IRGPass& pass) -> void;
		auto execute_graph(IRGPass& pass, edge::gfx::ICommandList& cmd_list, float delta_time) -> void;
	private:
		void update_font_texture();
	private:
		IRenderer& renderer_;

		//std::unordered_map<uint32_t, ImFont*> fonts_;
		float last_mouse_x{};
		float last_mouse_y{};

		edge::gfx::ISampler* sampler_{ nullptr };
		edge::gfx::ITexture* font_image_{ nullptr };
		edge::gfx::IBuffer* vertex_buffer_{ nullptr };
		edge::gfx::IBuffer* index_buffer_{ nullptr };

		OnSubsystemProcessCb on_process_{};
	};
}