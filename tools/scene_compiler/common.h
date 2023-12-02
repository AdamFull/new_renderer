#pragma once

#include <vector>

namespace mesh_compiler
{
	struct FBuffer
	{
		uint8_t* get(size_t index = 0ull) { return &data.at(index * stride); }

		size_t stride{ 0ull };
		size_t count{ 0ull };
		std::vector<uint8_t> data{};
	};

	struct FCompilerSettings
	{
		
	};
}