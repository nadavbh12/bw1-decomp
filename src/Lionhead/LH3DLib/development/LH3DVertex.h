#ifndef BW1_DECOMP_LH3D_VERTEX_INCLUDED_H
#define BW1_DECOMP_LH3D_VERTEX_INCLUDED_H

#include <assert.h> /* For static_assert */

#include "LH3DColor.h" /* For struct LH3DColor */
#include "LHPoint.h"   /* For struct LHPoint, struct Point2D */

struct LH3DVertex
{
	struct LHPoint position; /* 0x0 */
	struct Point2D uv;
	struct LHPoint normal; /* 0x14 */
};
static_assert(sizeof(LH3DVertex) == 0x20, "Data type is of wrong size");

struct Vertex3D
{
	struct LHPoint   position; /* 0x0 */
	float            rhw;      /* 0xc */
	struct LH3DColor color;    /* 0x10 */
	struct LH3DColor specular; /* 0x14 */
	struct Point2D   uv;       /* 0x18 */
};
static_assert(sizeof(Vertex3D) == 0x20, "Data type is of wrong size");

struct Vertex3DMT
{
	struct LHPoint   position; /* 0x0 */
	float            rhw;      /* 0xc */
	struct LH3DColor color;    /* 0x10 */
	struct LH3DColor specular; /* 0x14 */
	struct Point2D   uv;       /* 0x18 */
	struct Point2D   uv2;      /* 0x20 */
};
static_assert(sizeof(Vertex3DMT) == 0x28, "Data type is of wrong size");

#endif /* BW1_DECOMP_LH3D_VERTEX_INCLUDED_H */
