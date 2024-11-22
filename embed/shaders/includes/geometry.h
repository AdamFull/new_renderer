#ifndef SHLIB_GEOMETRY_H
#define SHLIB_GEOMETRY_H

#include "base.h"

SHLIB_NS_BEGIN

// Define vertex color type dependent on element count
#if (defined(COLOR_VEC3) || defined(COLOR_VEC4))
#define HAS_COLOR 1
#else
#define HAS_COLOR 0
#endif

#if defined(COLOR_VEC4)
#define Color vec4
#define VTX_COLOR_ELEM_CNT 4
#else
#define Color vec3
#define VTX_COLOR_ELEM_CNT 3
#endif

// Define vertex tangent type also dependent on element count
#if (defined(TANGENT_VEC3) || defined(TANGENT_VEC4))
#define HAS_TANGENT 1
#else
#define HAS_TANGENT 0
#endif

#if defined(TANGENT_VEC4)
#define Tangent vec4
#define VTX_TANGENT_ELEM_CNT 4
#else // TANGENT_VEC4
#define Tangent vec3
#define VTX_TANGENT_ELEM_CNT 3
#endif // TANGENT_VEC3

// Define joints type
#if defined(JOINTS_16BIT)
#define JointIndex uint16_t
#define Joints u16vec4
#define JOINT_ALIGN 2
#else
#define JointIndex uint8_t
#define Joints u8vec4
#define JOINT_ALIGN 1
#endif

#if defined(INDEX_16BIT)
#define Index uint16_t
#define INDEX_ALIGN 2
#else
#define Index uint32_t
#define INDEX_ALIGN 4
#endif

SHLIB_NS_END

#endif // SHLIB_GEOMETRY_H