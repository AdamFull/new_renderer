#include "gfx_frame_graph.h"

#include "gfx_context.h"

#include "../util/fs_helper.h"
#include "../util/Logger.h"
#include "../filesystem/VirtualFileSystemInterface.h"

namespace edge
{
	namespace gfx
	{
		IContext* FrameGraph::gfx_context;
		filesystem::IVirtualFileSystemInterface* FrameGraph::filesystem;
		IPipelineCache* FrameGraph::pipeline_cache;

		std::vector<FrameGraphTechnique> FrameGraph::techniques;
		std::vector<FrameGraphPass> FrameGraph::passes;

		void FrameGraph::Create(const FrameGraphInitializeInfo& initialize_info)
		{
			FrameGraph::gfx_context = initialize_info.gfx_context;
			FrameGraph::filesystem = initialize_info.filesystem;
		}

		void FrameGraph::Shutdown()
		{
			gfx_context->Destroy(pipeline_cache);
			pipeline_cache = nullptr;
		}

		bool FrameGraph::Load(std::string_view path)
		{
			return false;
		}

		void FrameGraph::Store(std::string_view path)
		{

		}

		void FrameGraph::Compile()
		{

		}

		void FrameGraph::RegisterResource(std::string_view name, IGFXResource* resource, ResourceStateFlags initial_state)
		{

		}

		void FrameGraph::UnregisterResource(std::string_view name)
		{

		}

		FrameGraphTechnique& FrameGraph::AddTechnique(std::string_view pass_name, ShaderStageFlags stage_flags)
		{
			return techniques.emplace_back();
		}

		FrameGraphPass& FrameGraph::AddPass(std::string_view pass_name, ShaderStageFlags stage_flags)
		{
			return passes.emplace_back();
		}
	}
}