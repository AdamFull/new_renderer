#pragma once

#include <engine/rendering/gfx/ResourceLoader.h>
#include <vector>

namespace engine
{
	namespace render
	{
		//template<typename _Ty>
		//class GPUVector
		//{
		//public:
		//	GPUVector(gfx::Context& context, gfx::BufferUsageFlags usage, uint64_t element_count = 64ull) :
		//		context{ context }
		//	{
		//		gfx::BufferCreateInfo create_info{};
		//		create_info.size = sizeof(_Ty);
		//		create_info.count = element_count;
		//		create_info.usage = usage;
		//
		//		buffer = resource_context.CreateBuffer(create_info);
		//	}
		//
		//	~GPUVector()
		//	{
		//		context.Destroy(buffer);
		//	}
		//
		//private:
		//	gfx::ResourceContext& resource_context;
		//
		//	gfx::Buffer* buffer{ nullptr };
		//	std::vector<_Ty> local_data{};
		//};
	}
}