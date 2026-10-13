#ifndef BW1_DECOMP_LH3D_RENDER_INCLUDED_H
#define BW1_DECOMP_LH3D_RENDER_INCLUDED_H

#include <d3d.h>
#include <d3dtypes.h>
#include <stdint.h>    /* For uint16_t, uint32_t */
#include <re_common.h> /* For bool32_t */

#include "LHMatrix.h"
#include "LH3DMaterial.h" /* For LH3DMaterial */

// Forward Declares

struct LH3DColor;
struct LH3DMaterial;
struct LH3DRender;
struct LHRegion;
struct LH3DTexture;
struct LHMatrix;
struct Vertex3D;
class LH3DZSorter;

enum
{
	LH3D_RENDER_TEXTURE_STAGE_COUNT = 8,
	LH3D_RENDER_TEXTURE_STAGE_STATE_COUNT = 0x100,
	LH3D_RENDER_STATE_COUNT = 0x100,
	LH3D_RENDER_STATE_BLOCK_COUNT = 8,
	LH3D_RENDER_ZBUFFER_FORMAT_COUNT = 4,
	LH3D_RENDER_ZSORTER_SIZE = 0x800,
	LH3D_RENDER_MAX_FINISH_FRAME_CALLBACKS = 16,
	LH3D_RENDER_FINISH_FRAME_CALLBACK_AFTER = 0x80000000,
	LH3D_RENDER_MAX_SORTED_TRIANGLES = 3000,
	LH3D_RENDER_SORT_BUCKET_COUNT = 32,
	LH3D_RENDER_BLEND_TEXTURE = 2,
	LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA = 4,
};

enum LH3D_STATE_BLOCK_STYLE
{
	LH3D_STATE_BLOCK_STYLE_NVIDIA = 0,
	LH3D_STATE_BLOCK_STYLE_VOODOO = 1,
	LH3D_STATE_BLOCK_STYLE_MULTIPASS = 2,
};

struct LH3DRender
{
	struct StateBlock2Tex
	{
		unsigned long Handle;
		bool          Multipass;
	};

	typedef void(__stdcall* FinishFrameCallback)(void* data);

	// Original Mac imported names; storage remains extracted.
	static int          g_started_frame; // 00eca644
	static LH3DZSorter* g_zsorter;       // 00eca648
	static bool32_t     g_mode_cleaning; // 00c3871c
	// Original Mac import g_frame__10LH3DRender.
	// BW1W120 00eca640 BW1M119 012cf838 (LHCombined Release)
	static int g_frame;
	// Original Mac import g_b_do_update_material__10LH3DRender.
	// BW1W120 00c38714 BW1M119 011d0e2c (LHCombined Release)
	static int g_b_do_update_material;
	// BW1W120 00eca64c BW1M119 012cf82c (LHCombined Release)
	static LH3DMaterial* g_current_active_material;
	// Descriptive names for the depth-pass override state. Storage remains extracted.
	static int OverrideMaterial;   // 00eca658
	static int OverrideRenderMode; // 00eca65c
	// BW1W120 00eca624 BW1M119 012cf854 (LHCombined Release)
	static bool32_t b_need_init;
	// BW1W120 00ec7fd0 BW1M119 012cf920 (LHCombined Release)
	static D3DMATRIX g_d3d_view2proj;
	// BW1W120 00ec8010 BW1M119 012cf960 (LHCombined Release)
	static D3DMATRIX g_d3d_world2view;
	// BW1W120 00ec81f0 BW1M119 012cfa00 (LHCombined Release)
	static uint32_t g_texture_stage_state[LH3D_RENDER_TEXTURE_STAGE_COUNT][LH3D_RENDER_TEXTURE_STAGE_STATE_COUNT];
	// BW1W120 00eca1f0 BW1M119 012d1a00 (LHCombined Release)
	static uint32_t g_render_states[D3DRENDERSTATE_CLIPPLANEENABLE + 1];
	// BW1W120 00eca614 BW1M119 012d1e00 (LHCombined Release)
	static bool32_t g_b_need_tilling;
	// BW1W120 00eca620 BW1M119 012cf858 (LHCombined Release)
	static bool32_t b_open;
	// BW1W120 00eca638
	static IDirect3DDevice7* Direct3DDevice7;
	// BW1W120 00c386e8 BW1M119 011d0e00 (LHCombined Release)
	static unsigned long g_save_tile;
	// BW1W120 00c386f4 BW1M119 011d0e0c (LHCombined Release)
	static int g_lastblend[LH3D_RENDER_TEXTURE_STAGE_COUNT];
	// BW1W120 00c38718 BW1M119 011d0e30 (LHCombined Release)
	static int g_render_mode;
	// BW1W120 00c38868
	static uint32_t Dat_00C38868;
	// BW1W120 00ec80f0 BW1M119 012d1e84 (LHCombined Release)
	static void* g_finish_frame_cb_data[LH3D_RENDER_MAX_FINISH_FRAME_CALLBACKS];
	// BW1W120 00ec8130 BW1M119 012d1ec4 (LHCombined Release)
	static uint32_t g_finish_frame_cb_priority[LH3D_RENDER_MAX_FINISH_FRAME_CALLBACKS];
	// BW1W120 00ec8170 BW1M119 012d1f04 (LHCombined Release)
	static FinishFrameCallback g_finish_frame_cb[LH3D_RENDER_MAX_FINISH_FRAME_CALLBACKS];
	// BW1W120 00ec81d0 BW1M119 012cf9e0 (LHCombined Release)
	static IDirectDrawSurface7* g_texture_stage_ptr[LH3D_RENDER_TEXTURE_STAGE_COUNT];
	// BW1W120 00eca600 BW1M119 012d1e80 (LHCombined Release)
	static int g_num_finish_frame_cb;
	// BW1W120 00eca610 BW1M119 012d1e04 (LHCombined Release)
	static bool32_t g_b_sorted_object_drawn_this_frame;
	// BW1W120 00eca628 BW1M119 012cf850 (LHCombined Release)
	static bool32_t g_b_need_anim_uv;
	// BW1W120 00eca62c BW1M119 012cf84c (LHCombined Release)
	static float g_adu;
	// BW1W120 00eca630 BW1M119 012cf848 (LHCombined Release)
	static float g_adv;
	// BW1W120 00eca650 BW1M119 012cf828 (LHCombined Release)
	static LH3DMaterial* g_current_other_material;
	// BW1W120 00eca670 BW1M119 012cf808 (LHCombined Release)
	static bool32_t g_b_use_wireframe;
	// BW1W120 00eca664 BW1M119 012cf814 (LHCombined Release)
	static int g_motion_blur_value;
	// BW1W120 00ec7fc0 BW1M119 012cf910 (LHCombined Release)
	static float g_poly_rate_smooth;
	// BW1W120 00ec7fc4 BW1M119 012cf914 (LHCombined Release)
	static float g_poly_rate;
	// BW1W120 00ec7fc8 BW1M119 012cf918 (LHCombined Release)
	static int g_num_polys_drawn;
	// BW1W120 00ec8050 BW1M119 012cf9a0 (LHCombined Release)
	static D3DMATRIX g_d3d_local2world;
	// BW1W120 00ec7f10 BW1M119 012cf864 (LHCombined Release)
	static D3DMATERIAL7 g_d3d_mat;
	// BW1W120 00ec7f58 BW1M119 012cf8a8 (LHCombined Release)
	static D3DLIGHT7 g_d3d_light;
	// BW1W120 00ec8090 BW1M119 012d1e20 (LHCombined Release)
	static StateBlock2Tex g_stateblock2tex[LH3D_RENDER_STATE_BLOCK_COUNT];
	// BW1W120 00ec80d0 BW1M119 012d1e60 (LHCombined Release)
	static int g_stateblockstyle[LH3D_RENDER_STATE_BLOCK_COUNT];
	// BW1W120 00ec81c0
	static bool32_t Dat_00EC81C0;
	// BW1W120 00ec81c4 BW1M119 012d1e14 (LHCombined Release)
	static bool g_use_multipass;
	// BW1W120 00ec81c8 BW1M119 012d1e18 (LHCombined Release)
	static LH3DMaterial* g_curmat2tex;
	// BW1W120 00ec81cc BW1M119 012d1e1c (LHCombined Release)
	static int g_curmode2tex;
	// BW1W120 00eca634 BW1M119 012cf844 (LHCombined Release)
	static IDirect3D7* D3D;
	// BW1W120 00eca63c BW1M119 012cf83c (LHCombined Release)
	static IDirectDrawSurface7* ZBufferSurface;
	// BW1W120 00eca604 BW1M119 012d1e10 (LHCombined Release)
	static GUID* lpPrefferedDeviceGUID;
	// BW1W120 00eca608 BW1M119 012d1e0c (LHCombined Release)
	static bool32_t g_b_support_AGP;
	// BW1W120 00eca60c BW1M119 012d1e08 (LHCombined Release)
	static bool32_t g_b_support_TNL;
	// BW1W120 00c386e4 BW1M119 011d0dfc (LHCombined Release)
	static bool32_t g_b_disable_TNL;

	// BW1W120 0082f810 BW1M119 0102b030 (LHCombined Release)
	static void __fastcall DrawTriangle(Vertex3D* vertices, unsigned long vertex_count, uint16_t* indices,
	                                    unsigned long index_count);
	// BW1W120 00412940 BW1M119 01049520
	static int SetRenderState(D3DRENDERSTATETYPE type, unsigned long value);
	// BW1W120 inlined BW1M119 inlined
	static void GetRenderState(D3DRENDERSTATETYPE type, uint32_t& value);
	// BW1W120 0082b220 BW1M119 0103c670 (LHCombined Release)
	static void SetD3DMatrix(D3DMATRIX& d3d_matrix, LHMatrix& matrix);
	// BW1W120 0082b290 BW1M119 010a6280 (LHCombined Release)
	static void ResetD3DMatrix(D3DMATRIX& matrix);
	// BW1W120 0082b2d0 BW1M119 010a60b0 (LHCombined Release)
	static D3DMATRIX SetD3DProjectionMatrix(D3DMATRIX& matrix, float near_plane, float far_plane, float fov);
	// BW1W120 0082b3c0 BW1M119 010a5cb0 (LHCombined Release)
	static bool32_t Open();
	// BW1W120 0082b570 BW1M119 010a5b30 (LHCombined Release)
	static bool32_t Close();
	// BW1W120 0082b680 BW1M119 010a5930 (LHCombined Release)
	static void SetStatesNVidiaStyle(unsigned long color_op);
	// BW1W120 0082b790 BW1M119 010a5390 (LHCombined Release)
	static void SetStatesVoodooStyle(unsigned long color_op);
	// BW1W120 0082b9c0 BW1M119 0102dc80 (LHCombined Release)
	static int SetTextureStageState(unsigned long stage, D3DTEXTURESTAGESTATETYPE type, unsigned long value);
	// BW1W120 inlined BW1M119 inlined
	static void GetTextureStageState(unsigned long stage, D3DTEXTURESTAGESTATETYPE type, uint32_t& value);
	// BW1W120 0082ba10
	static void fn_0082BA10();
	// BW1W120 0082bbc0 BW1M119 01007f00 (LHCombined Release)
	static void SetStatesMultipassStyle(unsigned long color_op);
	// BW1W120 0082bf10 BW1M119 010a4bf0 (LHCombined Release)
	static void Init2TexStuff();
	// BW1W120 0082c3f0 BW1M119 010a4b60 (LHCombined Release)
	static int ApplyStateBlock(int color_op);
	// BW1W120 0082c430 BW1M119 0101cb10 (LHCombined Release)
	static void Set2ndTex(int color_op, LH3DMaterial* material, bool reset);
	// BW1W120 0082c640 BW1M119 010a4790 (LHCombined Release)
	static void Close2TexStuff();
	// BW1W120 0082c680 BW1M119 010a4460 (LHCombined Release)
	static void SetCurD3DMat(bool lit, unsigned long diffuse, unsigned long specular);
	// BW1W120 0082c8f0 BW1M119 010a3900 (LHCombined Release)
	static void ResetAllState();
	// BW1W120 007ace50
	static void fn_007ACE50(LH3DMaterial* material, int stage);
	// BW1W120 0082cd80 BW1M119 010a30f0 (LHCombined Release)
	static bool32_t OpenD3D();
	// BW1W120 0082d3f0 BW1M119 010a2fa0 (LHCombined Release)
	static bool32_t CloseD3D();
	// BW1W120 0082c610 BW1M119 0101ceb0 (LHCombined Release)
	static void SetTexture(IDirectDrawSurface7* surface, int stage);
	// BW1W120 inlined BW1M119 inlined
	static void Inline_0082D4E0(bool32_t enable, D3DBLEND source, D3DBLEND destination, bool32_t alpha_test);
	// BW1W120 inlined BW1M119 inlined
	static void Inline_0082D507(int stage, int blend);
	// BW1W120 inlined BW1M119 inlined
	static void Inline_0082EF67(LH3DMaterial* material);
	// BW1W120 0082d470 BW1M119 010a2db0 (LHCombined Release)
	static void __fastcall SetRenderModeSmooth(LH3DMaterial* material, int pass);
	// BW1W120 0082d5c0 BW1M119 01019f40 (LHCombined Release)
	static void __fastcall SetRenderModeSmoothAlpha(LH3DMaterial* material, int pass);
	// BW1W120 0082d6f0 BW1M119 010a2bb0 (LHCombined Release)
	static void __fastcall SetRenderModeSmoothAlphaNZ(LH3DMaterial* material, int pass);
	// BW1W120 0082d820 BW1M119 0102dd80 (LHCombined Release)
	static void __fastcall SetRenderModeTextured(LH3DMaterial* material, int pass);
	// BW1W120 0082d920 BW1M119 0102f790 (LHCombined Release)
	static void __fastcall SetRenderModeTexturedAlpha(LH3DMaterial* material, int pass);
	// BW1W120 0082daa0 BW1M119 010a2440 (LHCombined Release)
	static void __fastcall SetRenderModeTexturedAlphaNZ(LH3DMaterial* material, int pass);
	// BW1W120 0082dc20 BW1M119 0101c5d0 (LHCombined Release)
	static void __fastcall SetRenderModeAlphaTextured(LH3DMaterial* material, int pass);
	// BW1W120 0082dd90 BW1M119 0102ef80 (LHCombined Release)
	static void __fastcall SetRenderModeAlphaTexturedAlpha(LH3DMaterial* material, int pass);
	// BW1W120 0082df10 BW1M119 0102e880 (LHCombined Release)
	static void __fastcall SetRenderModeAlphaTexturedAlphaNZ(LH3DMaterial* material, int pass);
	// BW1W120 0082e080 BW1M119 0102d970 (LHCombined Release)
	static void __fastcall SetRenderModeTexturedChroma(LH3DMaterial* material, int pass);
	// BW1W120 0082e2a0 BW1M119 010a18e0 (LHCombined Release)
	static void __fastcall SetRenderModeChromaJustZ(LH3DMaterial* material, int pass);
	// BW1W120 0082e470 BW1M119 0102d650 (LHCombined Release)
	static void __fastcall SetRenderModeTexturedChromaAlpha(LH3DMaterial* material, int pass);
	// BW1W120 0082e6a0 BW1M119 0101ee00 (LHCombined Release)
	static void __fastcall SetRenderModeTexturedChromaAlphaNZ(LH3DMaterial* material, int pass);
	// BW1W120 0082e830 BW1M119 010a1520 (LHCombined Release)
	static void __fastcall SetRenderModeAlphaTexturedAlphaAdditifChroma(LH3DMaterial* material, int pass);
	// BW1W120 0082e9c0 BW1M119 010a10c0 (LHCombined Release)
	static void __fastcall SetRenderModeAlphaTexturedAlphaAdditifChromaNZ(LH3DMaterial* material, int pass);
	// BW1W120 0082eb50 BW1M119 010a0f00 (LHCombined Release)
	static void __fastcall SetRenderModeAlphaTexturedAlphaAdditif(LH3DMaterial* material, int pass);
	// BW1W120 0082ecd0 BW1M119 0102ed10 (LHCombined Release)
	static void __fastcall SetRenderModeAlphaTexturedAlphaAdditifNZ(LH3DMaterial* material, int pass);
	// BW1W120 0082ee70 BW1M119 010a09b0 (LHCombined Release)
	static void CleanScreen();
	// BW1W120 0082ef50 BW1M119 010a0690 (LHCombined Release)
	static void __fastcall DrawRectangle(LHRegion& region, unsigned long color);
	// BW1W120 0082f7f0
	static void fn_0082F7F0();
	// BW1W120 0082faa0 BW1M119 0109fe00 (LHCombined Release)
	static void __fastcall SetBackgroundColor(LH3DColor* color);
	// BW1W120 0082fab0 BW1M119 0109fb70 (LHCombined Release)
	static uint16_t* __fastcall SortTriangle(Vertex3D* vertices, unsigned long vertex_count, uint16_t* indices,
	                                         unsigned long index_count);
	// BW1W120 0082fcd0 BW1M119 0109fa70 (LHCombined Release)
	static void __fastcall SetUseWireFrame(bool32_t use_wireframe);
	// BW1W120 0082feb0 BW1M119 010108b0 (LHCombined Release)
	static void PushTillingMode(int index);
	// BW1W120 0082fed0 BW1M119 010107f0 (LHCombined Release)
	static void PopTillingMode(int index);
	// BW1W120 0082ff90
	static void fn_0082FF90(unsigned long value);
	// BW1W120 00830000
	static void fn_00830000();
	// BW1W120 0082f0e0 BW1M119 01013360 (LHCombined Release)
	static void StartFrame();
	// BW1W120 0082f280 BW1M119 010a0560 (LHCombined Release)
	static void DrawSortedObjects();
	// BW1W120 0082ff10 BW1M119 0102b830 (LHCombined Release)
	static void SetD3DTillingOn(int index);
	// BW1W120 0082ff50 BW1M119 0102eed0 (LHCombined Release)
	static void SetD3DTillingOff(int index);
	// BW1W120 0082fd30 BW1M119 0109f910 (LHCombined Release)
	static LH3DMaterial* CreateMaterial(LH3DMaterial::RenderMode render_mode, LH3DTexture* texture);
	// BW1W120 0082f2c0 BW1M119 010a0340 (LHCombined Release)
	static void RegisterFinishFrameCallback(unsigned long priority, bool after, FinishFrameCallback callback,
	                                        void* data);
	// BW1W120 0082f3b0 BW1M119 010a0210 (LHCombined Release)
	static void RemoveFinishFrameCallback(FinishFrameCallback callback, void* data);
	// BW1W120 0082f460 BW1M119 0101f090 (LHCombined Release)
	static void FinishFrame();
};

inline int LH3DRender::SetRenderState(D3DRENDERSTATETYPE type, unsigned long value)
{
	HRESULT result = S_OK;

	if (g_render_states[type] != value)
	{
		result = Direct3DDevice7->SetRenderState(type, value);
		g_render_states[type] = result ? 0xffffffff : value;
	}
	return result;
}

inline void LH3DRender::SetTexture(IDirectDrawSurface7* surface, int stage)
{
	if (g_texture_stage_ptr[stage] != surface)
	{
		Direct3DDevice7->SetTexture(stage, surface);
		g_texture_stage_ptr[stage] = surface;
	}
}

inline void LH3DRender::Inline_0082D4E0(bool32_t enable, D3DBLEND source, D3DBLEND destination, bool32_t alpha_test)
{
	if (enable)
	{
		SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		SetRenderState(D3DRENDERSTATE_SRCBLEND, source);
		SetRenderState(D3DRENDERSTATE_DESTBLEND, destination);
	}
	else
	{
		SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	}
	SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, alpha_test);
}

inline void LH3DRender::Inline_0082D507(int stage, int blend)
{
	if (g_lastblend[stage] != blend)
	{
		g_lastblend[stage] = blend;
		SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		if (blend == LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA)
		{
			SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		}
		else
		{
			SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		}
	}
}

inline void LH3DRender::Inline_0082EF67(LH3DMaterial* material)
{
	if (g_b_do_update_material)
	{
		g_current_active_material = material;
		if (material)
		{
			((LH3DMaterial::RenderFunc*)LH3DMaterial::g_list_render_func)[material->GetMode()].Func(material, 0);
			if (g_b_need_tilling || material->IsNeedTilling())
			{
				SetD3DTillingOn(0);
			}
			else
			{
				SetD3DTillingOff(0);
			}
			SetRenderState(D3DRENDERSTATE_CULLMODE, material->IsDoubleSided() ? D3DCULL_NONE : D3DCULL_CCW);
		}
	}
}

inline void LH3DRender::GetRenderState(D3DRENDERSTATETYPE type, uint32_t& value)
{
	if (g_render_states[type] == 0xffffffff)
	{
		g_render_states[type] = Direct3DDevice7->GetRenderState(type, (LPDWORD)&value) ? 0xffffffff : value;
	}
	else
	{
		value = g_render_states[type];
	}
}

inline void LH3DRender::GetTextureStageState(unsigned long stage, D3DTEXTURESTAGESTATETYPE type, uint32_t& value)
{
	if (g_texture_stage_state[stage][type] == 0xffffffff)
	{
		g_texture_stage_state[stage][type] =
			Direct3DDevice7->GetTextureStageState(stage, type, (LPDWORD)&value) ? 0xffffffff : value;
	}
	else
	{
		value = g_texture_stage_state[stage][type];
	}
}

inline int LH3DRender::SetTextureStageState(unsigned long stage, D3DTEXTURESTAGESTATETYPE type, unsigned long value)
{
	if (g_texture_stage_state[stage][type] != value)
	{
		HRESULT result = Direct3DDevice7->SetTextureStageState(stage, type, value);
		g_texture_stage_state[stage][type] = result ? 0xffffffff : value;
		return result;
	}
	return S_OK;
}

// Original free symbol imported by the Mac executable.
// BW1W120 00c386d0 BW1M119 011d0de8 (LHCombined Release)
extern bool g_enable_callbacks;

// BW1W120 00eca5f0 BW1M119 012d1f58 (LHCombined Release)
extern int g_b_any_kyro;

// Original free symbol imported by the Mac executable.
// BW1W120 00ec7f08 BW1M119 012cf860 (LHCombined Release)
extern LH3DColor g_color_background;

// LH Combined Release Shared 001ccfe0.
// Original free global, updated by FrameRate(), not a member of LH3DRender.
// BW1W120 00c386e0 BW1M119 011d0df8 (LHCombined Release)
extern int g_frame_rate_stats;

// BW1W120 0082b0f0
void __cdecl fn_0082B0F0();
// BW1W120 0081e820
bool32_t __cdecl fn_0081E820();
// BW1W120 0081e870
bool32_t __cdecl fn_0081E870();
// BW1W120 0081faa0
void __cdecl fn_0081FAA0();
// BW1W120 0081fe10
void __cdecl fn_0081FE10();
// BW1W120 00880f20
void __cdecl fn_00880F20(int param_1, uint8_t param_2);
// BW1W120 007ac9e0
void __cdecl fn_007AC9E0();
// BW1W120 005f4e40 BW1M119 01109f80
void __cdecl RenderLoadingFrame(bool param_1);
// BW1W120 0082a500 BW1M119 0100b580 (LHCombined Release)
void __cdecl DrawAndClip(D3DPRIMITIVETYPE primitive_type, unsigned long fvf, Vertex3D* vertices,
                         unsigned long vertex_count, uint16_t* indices, unsigned long index_count);
// BW1W120 0082a5b0 BW1M119 01030470 (LHCombined Release)
void __cdecl DrawAndClip2D(D3DPRIMITIVETYPE primitive_type, unsigned long fvf, Vertex3D* vertices,
                           unsigned long vertex_count, uint16_t* indices, unsigned long index_count);
// BW1W120 006419f0 BW1M119 01370680
void __cdecl stop_draw_sprite_to_screen();
// BW1W120 0054c160 BW1M119 inlined
void __cdecl RenderLoadingFrame(char* param_1);
// BW1W120 00841c20 BW1M119 010622b0 (LHCombined Release)
void __cdecl BMan_Display();
// BW1W120 008415a0 BW1M119 0103ca40 (LHCombined Release)
void __cdecl BMan_Zero();
// BW1W120 0054d6f0 BW1M119 013e8dd0
void __cdecl DrawMouseCross();

#endif /* BW1_DECOMP_LH3D_RENDER_INCLUDED_H */
