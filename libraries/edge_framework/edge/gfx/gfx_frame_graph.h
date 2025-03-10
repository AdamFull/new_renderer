#pragma once

#include "../util/Hash.hpp"
#include "gfx_structures.h"

namespace edge
{
	namespace filesystem
	{
		class IVirtualFileSystemInterface;
	}

	namespace gfx
	{
		class shader_includer;

#pragma pack(push, 1)
		struct FrameGraphShaderStage
		{
			uint32_t uuu;
		};

		struct FrameGraphTechnique
		{
			std::vector<std::string_view> stages;
		};

		struct FrameGraphPass
		{
			uint32_t technique;

			
		};
#pragma pack(pop)

		struct FrameGraphInitializeInfo
		{
			IContext* gfx_context;
			filesystem::IVirtualFileSystemInterface* filesystem;
		};

		class FrameGraph
		{
		public:
			static void Create(const FrameGraphInitializeInfo& initialize_info);
			static void Shutdown();

			static bool Load(std::string_view path);
			static void Store(std::string_view path);

			static void Compile();

			static void RegisterResource(std::string_view name, IGFXResource* resource, ResourceStateFlags initial_state = {});
			static void UnregisterResource(std::string_view name);

			static FrameGraphShaderStage& AddShaderStage(std::string_view pass_name, ShaderStageFlags stage_flags);
			static FrameGraphTechnique& AddTechnique(std::string_view pass_name, ShaderStageFlags stage_flags);
			static FrameGraphPass& AddPass(std::string_view pass_name, ShaderStageFlags stage_flags);
		private:
			static IContext* gfx_context;
			static filesystem::IVirtualFileSystemInterface* filesystem;
			static IPipelineCache* pipeline_cache;

			static std::vector<FrameGraphTechnique> techniques;
			static std::vector<FrameGraphPass> passes;
		};
	}
}