#ifndef BW1_DECOMP_LH3D_PRIMITIVE_INCLUDED_H
#define BW1_DECOMP_LH3D_PRIMITIVE_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stdint.h> /* For uint8_t */

#include "LH3DMaterial.h" /* For struct LH3DMaterial */
#include "LHPlane.h"      /* For struct LHPlane */

// Forward Declares

struct LH3DVertex;
struct LH3DVertexGroup;
struct LHPoint;

struct LH3DPrimitive
{
	LH3DMaterial     Material;    /* 0x0 */
	int              NumVertices; /* 0x10 */
	LH3DVertex*      Vertices;
	int              NumTriangles;
	uint8_t*         Triangles;
	int              NumVertexGroups; /* 0x20 */
	LH3DVertexGroup* VertexGroups;
	int              NumBlends;
	uint8_t*         Blends;

	// BW1W120 00c39264 BW1M119 011d0d54 (LHCombined Release)
	static int MIN_LIGHT;
	// BW1W120 00f03118 BW1M119 012bf340 (LHCombined Release)
	static LHPlane g_clipping_plane_camera;
	// BW1W120 00f03128 BW1M119 012bf350 (LHCombined Release)
	static LHPlane g_clipping_plane_world;

	// Static methods

	// BW1W120 0084ab50 BW1M119 0109e410 (LHCombined Release)
	static void Create(const uint8_t* buffer, LH3DPrimitive* primitive);

	// Non-virtual methods

	// BW1W120 00807f30 BW1M119 01068ba0 (LHCombined Release)
	void ComputeBoundingBox(LHPoint& maxima, LHPoint& minima, int& initialized);
};

// BW1W120 0085bf00
void __cdecl fn_0085BF00();

#endif /* BW1_DECOMP_LH3D_PRIMITIVE_INCLUDED_H */
