#include "gfx_profiler.h"
#include "gfx_context.h"

#include "../util/utl_hash.hpp"
#include "../util/utl_imgui.hpp"
#include <imgui_internal.h>

static constexpr float _threshold_ms{ 1.f };
static constexpr float fg_bar_height = 20.0f;

namespace edge::gfx {
	void Profiler::ZoneNode::evaluate(std::span<uint64_t> timestamps, double timestamp_freq) {
		for (auto& child : children) {
			child->evaluate(timestamps, timestamp_freq);
		}

		if (begin_query == ~0ull || end_query == ~0ull) {
			time = get_included_time();
		}
		else {
			uint64_t elapsed_time = timestamps[end_query] - timestamps[begin_query];
			if (timestamps[end_query] <= timestamps[begin_query]) {
				elapsed_time = 0ull;
			}

			// Store time in seconds
			time = static_cast<double>(elapsed_time) * timestamp_freq * 1000000.0;
		}
		
		avg_time = avg_time * 0.95 + time * 0.05;
		min_time = std::min(min_time, time);
		max_time = std::max(max_time, time);

		// Update history
		for (int32_t i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
			history[i] = history[i + 1];
		}
		history[MAX_HISTORY_SIZE - 1] = static_cast<float>(time);
	}

	auto Profiler::ZoneNode::lookup_zone(std::string_view zone_name) -> ZoneNode* {
		for (auto& c : children) {
			if (!c->name.compare(zone_name)) {
				return c.get();
			}
		}

		auto new_node = std::make_unique<ZoneNode>();
		new_node->name = zone_name;
		new_node->name_hash = murmur_hash(zone_name);
		new_node->parent = this;
		children.push_back(std::move(new_node));
		return children.back().get();
	}

	auto Profiler::ZoneNode::get_included_time() -> double {
		float accum_time{ 0.f };

		if (!children.empty()) {
			for (auto& child : children) {
				accum_time += child->get_included_time();
			}
		}
		else {
			return time;
		}

		return accum_time;
	}

	auto Profiler::ZoneNode::get_max_depth() -> int {
		int max_depth = 0;
		for (const auto& child : children) {
			max_depth = std::max(max_depth, child->get_max_depth() + 1);
		}
		return max_depth;
	}

	void Profiler::ZoneNode::reset() {
		begin_query = ~0ull;
		end_query = ~0ull;

		for (auto& child : children) {
			child->reset();
		}
	}

	void Profiler::ZoneNode::draw_stats() {
		ImGui::NextColumn();

		// Color red if exceeds threshold
		ImVec4 text_color = time > _threshold_ms ?
			ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		ImGui::TextColored(text_color, "%.6f", time);

		ImGui::NextColumn();
		ImGui::TextColored(
			avg_time > _threshold_ms ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
			"%.6f", avg_time);

		ImGui::NextColumn();
		ImGui::Text("%.6f / %.6f", min_time, max_time);

		ImGui::NextColumn();
	}

	void Profiler::ZoneNode::draw() {
		if (auto push_id = edge::imui::make_scoped_push_id(this)) {
			ImVec4 text_color = time > _threshold_ms ?
				ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

			if (auto push_color = edge::imui::make_scoped_style_color(ImGuiCol_Text, text_color)) {
				bool has_children = !children.empty();
				if (has_children) {
					if (auto tree_node = edge::imui::make_scoped_tree_node(name.data())) {
						draw_stats();
						for (auto& child : children) {
							child->draw();
						}
					}
					else {
						draw_stats();
					}
				}
				else {
					ImGui::TextUnformatted(name.data());
					draw_stats();
				}
			}
		}
	}

	void Profiler::ZoneNode::draw_flame(float x, float y, float width, float total_time, int depth) {
		if (width < 1.0f) {
			return;
		}

		const ImVec2 node_min(x, y + depth * fg_bar_height);
		const ImVec2 node_max(x + width, node_min.y + fg_bar_height);

		// Generate color from name hash
		ImU32 color = IM_COL32(
			(name_hash & 0xFF),
			((name_hash >> 8) & 0xFF),
			((name_hash >> 16) & 0xFF),
			200
		);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(node_min, node_max, color);
		draw_list->AddRect(node_min, node_max, IM_COL32(0, 0, 0, 255));

		// Draw zone name
		ImVec2 text_size = ImGui::CalcTextSize(name.data());
		if (text_size.x < (width - 4.0f)) {
			draw_list->AddText(ImVec2(node_min.x + 2.0f, node_min.y + (fg_bar_height - text_size.y) * 0.5f),
				IM_COL32_WHITE, name.data());
		}

		// Draw time
		char time_str[32];
		snprintf(time_str, sizeof(time_str), "%.4f ms", time);
		ImVec2 time_text_size = ImGui::CalcTextSize(time_str);
		if (time_text_size.x < (width - 4.0f)) {
			draw_list->AddText(ImVec2(node_max.x - time_text_size.x - 2.0f, node_min.y + (fg_bar_height - time_text_size.y) * 0.5f),
				IM_COL32_WHITE, time_str);
		}

		// Tooltip on hover
		if (ImGui::IsMouseHoveringRect(node_min, node_max)) {
			float percentage = total_time > 0.0f ? (time / total_time) * 100.0f : 0.0f;

			ImGui::BeginTooltip();
			ImGui::Text("%s", name.data());
			ImGui::Text("Time: %.2f ms (%.2f%%)", time, percentage);
			ImGui::Text("Avg: %.6f ms", avg_time);
			ImGui::Text("Min: %.6f ms / Max: %.2f ms", min_time, max_time);
			ImGui::EndTooltip();
		}

		// Draw children
		if (!children.empty()) {
			float child_x = x;
			for (const auto& child : children) {
				float child_width = static_cast<float>((child->time / time) * width);
				child->draw_flame(child_x, y, child_width, total_time, depth + 1);
				child_x += child_width;
			}
		}
	}

	auto Profiler::_evaluate() -> bool {
		if (!query_index) {
			return false;
		}

		uint64_t timestamps[MAX_PROFILE_ZONES * 2u] = {};
		query_pool->get_data(0u, query_index, &timestamps[0]);
		query_pool->reset(0u, query_index);
		query_index = 0u;

		current_zone = &root;
		root.evaluate(timestamps, timestamp_freq);
		root.reset();

		return true;
	}

	void Profiler::_draw() {
		if (auto window = edge::imui::make_scoped_window("Frame Viewer")) {
			if (is_paused) {
				if (ImGui::Button("Continue Profiling")) {
					is_paused = false;
				}
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "PAUSED");
			}
			else {
				if (ImGui::Button("Pause Profiling")) {
					is_paused = true;
				}
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.2f, 1.0f), "RUNNING");
			}

			ImGui::TextColored(
				root.time > _threshold_ms ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
				"Total Frame: %.6f ms (Avg: %.6f ms)",
				root.time,
				root.avg_time
			);

			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("Min: %.6f ms", root.min_time);
				ImGui::Text("Max: %.6f ms", root.max_time);
				ImGui::EndTooltip();
			}

			{
				ImVec4 plot_color = ImVec4(0.0f, 0.7f, 0.0f, 1.0f);
				if (root.avg_time > _threshold_ms) {
					plot_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
				}

				ImGui::PlotLines("##FrameTime", root.history.data(), MAX_HISTORY_SIZE, 0,
					nullptr, 0.0f, root.max_time * 1.1f, ImVec2(-1, 80));

				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				ImVec2 p_min = ImGui::GetItemRectMin();
				ImVec2 p_max = ImGui::GetItemRectMax();
				float threshold_y = p_max.y - (p_max.y - p_min.y) * (_threshold_ms / (root.max_time * 1.1f));
				draw_list->AddLine(ImVec2(p_min.x, threshold_y), ImVec2(p_max.x, threshold_y),
					ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 0.5f)));
			}

			ImGui::Separator();

			if (auto tab_bar = edge::imui::make_scoped_tab_bar("ProfilerViews")) {
				if (auto detailed_stats = edge::imui::make_scoped_tab_item("Detailed Stats")) {
					ImGui::Columns(4, nullptr, true);
					ImGui::Text("Scope"); ImGui::NextColumn();
					ImGui::Text("Time (ms)"); ImGui::NextColumn();
					ImGui::Text("Avg (ms)"); ImGui::NextColumn();
					ImGui::Text("Min/Max (ms)"); ImGui::NextColumn();
					ImGui::Separator();
					for (auto& child : root.children) {
						child->draw();
					}
					ImGui::Columns(1);
				}

				if (auto flame_graph = edge::imui::make_scoped_tab_item("Flame Graph")) {
					ImGuiWindowFlags child_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar;
					if (auto child_window = edge::imui::make_scoped_child_window("FlameGraph", ImVec2(0, 0), 0, child_flags)) {
						static float zoom = 1.0f;
						static float scroll_x = 0.0f;

						const float zoom_step{ 0.1f };
						const float zoom_min{ 0.1f };
						const float zoom_max{ 50.0f };

						ImGui::SliderFloat("##zoom_slider", &zoom, zoom_min, zoom_max);
						ImGui::SameLine();
						if (ImGui::Button("Reset Zoom")) {
							zoom = 1.0f;
							scroll_x = 0.0f;
						}
						ImGui::Separator();

						ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
						ImVec2 canvas_size = ImGui::GetContentRegionAvail();

						auto& io = ImGui::GetIO();
						ImVec2 mouse_pos = io.MousePos;

						if (ImGui::IsWindowHovered()) {
							float wheel = io.MouseWheel;
							if (wheel != 0.0f) {
								float old_zoom = zoom;
								zoom = std::clamp(zoom * (1.0f + zoom_step * wheel), zoom_min, zoom_max);

								float mouse_pos_in_canvas = mouse_pos.x - canvas_pos.x;
								scroll_x = (scroll_x + mouse_pos_in_canvas) * (zoom / old_zoom) - mouse_pos_in_canvas;
							}
						}

						if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
							scroll_x -= io.MouseDelta.x;
						}

						float total_width = canvas_size.x * zoom;
						float max_scroll = std::max(0.0f, total_width - canvas_size.x);
						scroll_x = std::clamp(scroll_x, 0.0f, max_scroll);

						if (root.time > 0.0) {
							ImDrawList* draw_list = ImGui::GetWindowDrawList();
							ImGui::PushClipRect(canvas_pos, canvas_pos + canvas_size, true);

							float x = canvas_pos.x - scroll_x;
							for (const auto& child : root.children) {
								float child_width = static_cast<float>((child->time / root.time) * total_width);
								child->draw_flame(x, canvas_pos.y, child_width, root.time, 0);
								x += child_width;
							}

							ImGui::PopClipRect();
						}

						// Set scrollable area
						int max_depth = root.get_max_depth();
						ImGui::Dummy(ImVec2(canvas_size.x, (max_depth + 1) * fg_bar_height));
					}
				}

				if (auto memory_stats_tab = edge::imui::make_scoped_tab_item("Memory Stats")) {
				}

				if (auto resource_stats_tab = edge::imui::make_scoped_tab_item("Resource Stats")) {
				}

				if (auto hw_info_tab = edge::imui::make_scoped_tab_item("Hardware Info")) {
				}
			}
		}
	}

	Profiler::Profiler() {
		current_zone = &root;

		QueryPoolCreateInfo create_info{};
		create_info.type = QueryType::eTimestamp;
		create_info.query_count = MAX_PROFILE_ZONES * 2u;
		query_pool = create_query_pool(create_info);
		query_pool->reset(0u, create_info.query_count);

		request_timestamp_frequency(&timestamp_freq);
	}

	Profiler::~Profiler() {
		if (query_pool) {
			destroy(query_pool);
		}
	}


	void Profiler::begin_zone(ICommandList& cmd_list, std::string_view label, uint32_t color) {
		if (is_paused) {
			return;
		}

		cmd_list.begin_marker(label, color);
		
		current_zone = current_zone->lookup_zone(label);
		current_zone->begin_query = query_index++;
		cmd_list.begin_query(query_pool, current_zone->begin_query);
	}

	void Profiler::end_zone(ICommandList& cmd_list) {
		if (is_paused) {
			return;
		}

		cmd_list.end_marker();

		current_zone->end_query = query_index++;
		cmd_list.end_query(query_pool, current_zone->end_query);

		if (current_zone->parent) {
			current_zone = current_zone->parent;
		}
	}

	void Profiler::frame_mark() {
		if (is_paused || !_evaluate()) {
			return;
		}

		_draw();
	}
}