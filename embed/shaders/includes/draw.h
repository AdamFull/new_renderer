#ifndef SHLIB_DRAW_H
#define SHLIB_DRAW_H

#include "ptr.h"

SHLIB_NS_BEGIN

struct DrawIndexedIndirectData
{
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t  vertexOffset;
	uint32_t firstInstance;
};

REGISTER_TYPE_PTR(DrawIndexedIndirectData, 4);

struct DrawDescription
{
	uint32_t camera_index;
	uint32_t mesh_index;
	uint32_t submesh_index;
	uint32_t sorting_order;
};

REGISTER_TYPE_PTR(DrawDescription, 4);

CB_BEGIN(DrawDescriptionBuffers, 1, 16)
	GET_PTR_T(DrawDescription) draw_descriptions_ptr;
	GET_PTR_T(DrawIndexedIndirectData) draw_commands_ptr;
CB_END_E()

SHLIB_INLINE DrawDescription GetDrawInfo(uint32_t index)
{ 
#ifdef __cplusplus
	return {};
#else
	return draw_descriptions_ptr[index].data;
#endif
}

SHLIB_INLINE DrawIndexedIndirectData GetDrawIndexedCommand(uint32_t index)
{ 
#ifdef __cplusplus
	return {};
#else
	return draw_commands_ptr[index].data;
#endif
}

SHLIB_NS_END

#endif // SHLIB_DRAW_H