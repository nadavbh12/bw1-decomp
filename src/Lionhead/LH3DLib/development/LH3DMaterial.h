#ifndef BW1_DECOMP_LH3D_MATERIAL_INCLUDED_H
#define BW1_DECOMP_LH3D_MATERIAL_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stddef.h> /* For NULL */
#include <stdint.h> /* For uint8_t */

#include "LH3DColor.h" /* For struct LH3DColor */

// Forward Declares

struct LH3DTexture;

struct MaterialProperties
{
	bool field_0x0;
	bool field_0x1;
	bool cull;
	bool field_0x3;
	bool field_0x4;
};
static_assert(sizeof(MaterialProperties) == 0x5, "Data type is of wrong size");

enum
{
	LH3D_MATERIAL_DOUBLE_SIDED = 0x1,
	LH3D_MATERIAL_NEED_TILLING = 0x4,
};

struct LH3DMaterial
{
	enum RenderMode
	{
		LH3D_MATERIAL_RENDER_MODE_0x1 = 0x1,
		LH3D_MATERIAL_RENDER_MODE_0x2 = 0x2,
		LH3D_MATERIAL_RENDER_MODE_0x3 = 0x3,
		LH3D_MATERIAL_RENDER_MODE_0x4 = 0x4,
		LH3D_MATERIAL_RENDER_MODE_0x5 = 0x5,
		LH3D_MATERIAL_RENDER_MODE_0x6 = 0x6,
		LH3D_MATERIAL_RENDER_MODE_0x8 = 0x8,
		LH3D_MATERIAL_RENDER_MODE_0x9 = 0x9,
		LH3D_MATERIAL_RENDER_MODE_0xc = 0xc,
		LH3D_MATERIAL_RENDER_MODE_0xd = 0xd,
		LH3D_MATERIAL_RENDER_MODE_0x10 = 0x10,
		_LH3D_MATERIAL_RENDER_MODE_COUNT = 0x11
	};

	struct RenderFunc
	{
		void(__fastcall* Func)(LH3DMaterial* material, int pass);
		int field_0x4;
	};

	RenderMode   render_mode; /* 0x0 */
	uint8_t      field_0x4;
	uint8_t      cull_mode;
	LH3DTexture* texture;
	LH3DColor    color;

	// BW1W120 inlined BW1M119 inlined
	LH3DMaterial(RenderMode mode, LH3DTexture* material_texture)
	{
		g_count++;
		render_mode = mode;
		field_0x4 = 0;
		texture = material_texture;
		cull_mode = 0;
	}
	// BW1W120 inlined BW1M119 01052420 (LHCombined Release)
	~LH3DMaterial()
	{
		g_count--;
		texture = NULL;
	}

	// BW1W120 inlined BW1M119 0102b300 (LHCombined Release)
	RenderMode GetMode() { return render_mode; }
	// BW1W120 inlined BW1M119 010303e0 (LHCombined Release)
	uint8_t GetAlphaReference() { return field_0x4; }
	// BW1W120 inlined BW1M119 010c6de0 (LHCombined Release)
	LH3DTexture* GetTexture() { return texture; }
	// BW1W120 inlined BW1M119 0102a5e0 (LHCombined Release)
	int IsDoubleSided() { return cull_mode & LH3D_MATERIAL_DOUBLE_SIDED; }
	// BW1W120 inlined BW1M119 0102b710 (LHCombined Release)
	int IsNeedTilling() { return cull_mode & LH3D_MATERIAL_NEED_TILLING; }

	// BW1W120 00eca654 BW1M119 012cf824 (LHCombined Release)
	static int g_count;

	// The render-mode function table LH3DRender dispatches through; points at one of the tables below.
	// BW1W120 00eca618 BW1M119 012cf91c (LHCombined Release)
	static void* g_list_render_func;
	// BW1W120 00c38728 BW1M119 011d0e38 (LHCombined Release)
	static void* g_list_render_func_normal[40];
	// BW1W120 00c387c8 BW1M119 011d0ed8 (LHCombined Release)
	static void* g_list_render_func_global_alpha[40];

	// BW1W120 0082ee50 BW1M119 010a0b80 (LHCombined Release)
	static void Close();
};
static_assert(sizeof(LH3DMaterial) == 0x10, "Data type is of wrong size");

#endif /* BW1_DECOMP_LH3D_MATERIAL_INCLUDED_H */
