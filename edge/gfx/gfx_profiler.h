#pragma once

#include <memory>

#include "gfx_structures.h"
#include "../util/utl_design.h"

#define ENABLE_EDGE_GFX_PROFILER
#ifdef ENABLE_EDGE_GFX_PROFILER
#define EDGE_GFX_PROFILER_BEGIN_ZONE(cmd_list, label) edge::gfx::Profiler::get_instance()->begin_zone(cmd_list, "NOT IMPLEMENTED");
#define EDGE_GFX_PROFILER_BEGIN_ZONE_N(cmd_list, label) edge::gfx::Profiler::get_instance()->begin_zone(cmd_list, label);
#define EDGE_GFX_PROFILER_BEGIN_ZONE_NC(cmd_list, label, color) edge::gfx::Profiler::get_instance()->begin_zone(cmd_list, label, color);
#define EDGE_GFX_PROFILER_END_ZONE(cmd_list) edge::gfx::Profiler::get_instance()->end_zone(cmd_list);
#define EDGE_GFX_PROFILER_FRAME_MARK() edge::gfx::Profiler::get_instance()->frame_mark();
#define EDGE_GFX_PROFILER_DRAW_UI() edge::gfx::Profiler::get_instance()->draw_ui();
#else
#define EDGE_GFX_PROFILER_BEGIN_ZONE(cmd_list, label)
#define EDGE_GFX_PROFILER_BEGIN_ZONE_N(cmd_list, label) edge::gfx::Profiler::get_instance()->begin_zone(cmd_list, label);
#define EDGE_GFX_PROFILER_BEGIN_ZONE_NC(cmd_list, label, color)
#define EDGE_GFX_PROFILER_END_ZONE()
#define EDGE_GFX_PROFILER_FRAME_MARK()
#define EDGE_GFX_PROFILER_DRAW_UI() 
#endif

namespace edge::gfx {
	class Profiler : public utl::Singleton<Profiler> {
	public:
		Profiler();
		~Profiler();

		void begin_zone(ICommandList& cmd_list, std::string_view label, uint32_t color = 0xFFFFFFFF);
		void end_zone(ICommandList& cmd_list);

		void frame_mark();
		void draw_ui();
	private:
		static constexpr size_t MAX_PROFILE_ZONES{ 128 };
		static constexpr size_t MAX_HISTORY_SIZE{ 120 };

		struct ZoneNode {
			std::string_view name{};
			uint32_t name_hash{ 0u };

			double time{ 0.0 };
			double avg_time{ 0.0 };
			double min_time{ std::numeric_limits<double>::max() };
			double max_time{ 0.0 };

			std::array<float, MAX_HISTORY_SIZE> history;

			uint32_t begin_query{ ~0u };
			uint32_t end_query{ ~0u };

			ZoneNode* parent{ nullptr };
			std::vector<std::unique_ptr<ZoneNode>> children{};

			void evaluate(std::span<uint64_t> timestamps, double timestamp_freq);
			auto lookup_zone(std::string_view zone_name) -> ZoneNode*;
			auto get_included_time() -> double;
			auto get_max_depth() -> int;
			void reset();

			void draw_flame(float x, float y, float width, double total_time, int depth);
			bool draw_table_row(float depth = 0.f);
		};

		ZoneNode root{};
		ZoneNode* current_zone{ nullptr };

		IQueryPool* query_pool{ nullptr };
		uint32_t query_index{ 0u };

		double timestamp_freq{ 0.0 };
		bool is_paused{ false };
	};
}