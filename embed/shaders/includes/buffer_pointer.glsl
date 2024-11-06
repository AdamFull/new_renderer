#ifndef BUFFER_POINTER_GLSL
#define BUFFER_POINTER_GLSL

#define PTRCAST(pointer_type, address) pointer_type(address)
#define GET_BUFFER_POINTER(pointer_type, buffer_desc) pointer_type(buffer_desc.address)
#define PTRVAL(buffer, index) buffer.data[nonuniformEXT(index)]

#endif