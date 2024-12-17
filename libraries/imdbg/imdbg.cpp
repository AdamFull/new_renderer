#include "imdbg.h"

extern class Context* imdbg_context{ nullptr };

constexpr size_t k_startup_vertex_count{ 65535ull };
constexpr size_t k_startup_index_count{ k_startup_vertex_count * 3ull };
constexpr size_t k_startup_command_count{ 256ull };

namespace ImDbg
{
	template<typename _Ty>
	_Ty min(const _Ty& val)
	{

	}

	template<>
	ImDbgVec2 min<ImDbgVec2>(const ImDbgVec2& val)
	{

	}

	class DrawCommand
	{
	public:
		virtual ~Command() = default;

		virtual void Draw(std::vector<ImDbgVertex>& vertices, std::vector<ImDbgIndex>& indices) = 0;
	};

	class DrawLine2DCommand : public Command
	{
	public:
		void Draw(std::vector<ImDbgVertex>& vertices, std::vector<ImDbgIndex>& indices) override
		{

		}
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
		std::vector<ImDbgVertex> vertices{};
		std::vector<ImDbgIndex> indices{};

		std::vector<DrawCommand*> command_list{};
		std::vector<IndexedDrawData> indexed_draw_data{};
	};

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

	void DrawPoint2D(ImDbgVec2 position, ImDbgColor color, float size)
	{
		assert(imdbg_context && "ImDbg context is not initialized.");
	}

	void DrawLine2D(ImDbgVec2 from, ImDbgVec2 to, ImDbgColor color, float thickness)
	{
		assert(imdbg_context && "ImDbg context is not initialized.");
	}
}