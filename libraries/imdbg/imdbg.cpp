#include "imdbg.h"

#include <cassert>

constexpr size_t k_startup_vertex_count{ 65535ull };
constexpr size_t k_startup_index_count{ k_startup_vertex_count * 3ull };
constexpr size_t k_startup_command_count{ 256ull };

namespace ImDbg
{
	class DrawCommand
	{
	public:
		virtual ~DrawCommand() = default;

		virtual void Draw(std::vector<Vertex>& vertices, std::vector<Index>& indices) = 0;
	};

	class DrawLine2DCommand : public DrawCommand
	{
	public:
		DrawLine2DCommand(Vec2 from, Vec2 to, Color color, float thickness = 1.0f) :
			pos_from{ from.x, from.y, 0, 0 }, 
			pos_to{ to.x, to.y, 0, 0 }, 
			color{ color }, thickness{ thickness }
		{
		}

		~DrawLine2DCommand() = default;

		void Draw(std::vector<Vertex>& vertices, std::vector<Index>& indices) override
		{
			auto min = Min(pos_from, pos_to);
			auto range = Max(pos_from, pos_to) - min;

			vertices.emplace_back(Vertex{ pos_from, {}, color });
			vertices.emplace_back(Vertex{ pos_to, {}, color });
			indices.emplace_back(0);
			indices.emplace_back(1);
		}
	private:
		Vec4 pos_from;
		Vec4 pos_to;
		Color color; 
		float thickness;
	};

	class DrawPolygon2DCommand : public DrawCommand
	{
	public:
		DrawPolygon2DCommand();
		~DrawPolygon2DCommand() = default;

		void Draw(std::vector<Vertex>& vertices, std::vector<Index>& indices) override
		{
			assert(points.size() > 2 && "Incorrect points count for draw polygon.");
		}
	private:
		std::span<const Vec2> points{};
	};



	struct IndexedDrawData
	{
		uint32_t first_index;
		uint32_t index_count;
		uint32_t first_vertex;
		uint32_t vertex_count;
	};

	class Context
	{
	public:
		Context()
		{
			vertices.reserve(k_startup_vertex_count);
			indices.reserve(k_startup_index_count);

			command_list.reserve(k_startup_command_count);
		}

		void Execute()
		{
			// Execute command lists
			for (DrawCommand* command : command_list)
			{
				auto& draw_data = indexed_draw_data.emplace_back();
				draw_data.first_vertex = vertices.size();
				draw_data.first_index = indices.size();

				command->Draw(vertices, indices);

				draw_data.vertex_count = vertices.size() - draw_data.first_vertex;
				draw_data.index_count = indices.size() - draw_data.first_index;

				assert(draw_data.vertex_count && draw_data.index_count && "Incorrect or empty draw command. No draw data generated.");
			}
		}

		void Reset()
		{
			// Clear vertices
			vertices.clear();
			indices.clear();

			for (DrawCommand* command : command_list)
			{
				delete command;
				command = nullptr;
			}

			indexed_draw_data.clear();
		}
	private:
		std::vector<Vertex> vertices{};
		std::vector<Index> indices{};

		std::vector<DrawCommand*> command_list{};
		std::vector<IndexedDrawData> indexed_draw_data{};
	};

	extern Context* imdbg_context{ nullptr };

	void CreateContext()
	{
		assert(!imdbg_context && "ImDbg context is already created.");
		imdbg_context = new Context;
	}

	void DestroyContext()
	{
		assert(imdbg_context && "ImDbg context is already destroyed.");
		delete imdbg_context;
		imdbg_context = nullptr;
	}

	void NewFrame()
	{
		assert(imdbg_context && "ImDbg context is not initialized.");
		imdbg_context->Reset();
	}

	void EndFrame()
	{
		assert(imdbg_context && "ImDbg context is not initialized.");
		imdbg_context->Execute();
	}

	void DrawPoint2D(Vec2 position, Color color, float size)
	{
		assert(imdbg_context && "ImDbg context is not initialized.");


	}

	void DrawLine2D(Vec2 from, Vec2 to, Color color, float thickness)
	{
		assert(imdbg_context && "ImDbg context is not initialized.");
	}
}