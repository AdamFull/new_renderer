#pragma once

#include "../util/Hash.hpp"
#include "gfx_structures.h"

namespace edge
{
	namespace fs
	{
		class VirtualFilesystem;
	}

	namespace gfx
	{
		class FrameGraph;

		class FrameGraphNode
		{
		public:
		private:
			FrameGraph& graph;
		};

		class FrameGraph
		{
		public:
			void RegisterResource(std::string_view name, IGFXResource* resource);
		private:
			std::unordered_map<std::string_view, IGFXResource*> resource_mappings{};
		};
	}
}