#include "LH3DRender.h"

#include <d3d.h>
#include <float.h>  /* For FLT_MAX */
#include <math.h>   /* For tan */
#include <string.h> /* For memset */

#include <Lionhead/LHLib/ver5.0/LHTimer.inl>

#include "LH3DAnim.h" /* For LH3DAnim::AnimPack */
#include "LH3DCameraChecker.h"
#include "LH3DColor.h"
#include "LH3DMaterial.h"
#include "LH3DLandscape.h" /* For LH3DLandscape */
#include "LH3DLine.h"      /* For LH3DLine */
#include "LH3DMath.h"      /* For LH3DMath */
#include "LH3DMem.h"       /* For LH3DMem */
#include "LH3DMesh.h"      /* For LH3DMesh */
#include "LH3DMist.h"      /* For LH3DMist */
#include "LH3DObject.h"    /* For LH3DObject */
#include "LH3DP3.h"
#include "LH3DPrimitive.h" /* For LH3DPrimitive::MIN_LIGHT */
#include "LH3DTech.h"      /* For Report3D__FPCce, g_b_any_voodoo */
#include "LH3DTexture.h"   /* For g_b_voodoo2, g_b_banshee */
#include "LH3DVertex.h"
#include "LH3DVB.h"        /* For LH3DVB */
#include "LH3DWater.h"     /* For LH3DWater */
#include "LowResTexture.h" /* For LowResTexture */
#include "SmokyStuff.h"
#include "LHRegion.h"
#include "Black/LH3DZSorter.h"              /* For LH3DZSorter */
#include "Lionhead/LHLib/ver5.0/LHScreen.h" /* For LHResetFPU */
#include "Lionhead/LHLib/ver5.0/LHSystem.h" /* For LHSys */
#include "Lionhead/LHLog/ver4.0/LHLogger.h" /* For LHLogger */

#define FILEPATH "C:\\DEV\\Libs\\LIONHEAD\\LH3DLIB\\DEVELOPMENT\\LH3DRender.cpp"

#define LH3D_RENDER_DEFAULT_FOV         1.2217305f
#define LH3D_RENDER_ASPECT_RATIO        (4.0f / 3.0f)
#define LH3D_RENDER_FAR_PLANE           5000.0f
#define LH3D_RENDER_MATERIAL_AMBIENT    0.1953125f
#define LH3D_RENDER_POLY_RATE_SMOOTHING 0.8f

// TODO: Reconcile the original Render data split before source linkage.
int  g_frame_rate_stats = 5;
bool g_enable_callbacks = true;

// BW1W120 00c386d4 BW1M119 011d0dec (LHCombined Release)
bool32_t g_first_for_reset_all = true;
// BW1W120 00c386d8 BW1M119 011d0df0 (LHCombined Release)
bool32_t g_b_zbuffer_16b = true;

// BW1W120 00ec5a60 BW1M119 011d110c (LHCombined Release)
char* __LH3DLib__;
// BW1W120 00ec5a64 BW1M119 012cd364 (LHCombined Release)
uint32_t g_original_texture_stage_state[LH3D_RENDER_TEXTURE_STAGE_COUNT * LH3D_RENDER_TEXTURE_STAGE_STATE_COUNT];
// BW1W120 00ec7a64 BW1M119 012cf364 (LHCombined Release)
uint32_t g_original_render_states[LH3D_RENDER_STATE_COUNT];
// BW1W120 00ec7e64 BW1M119 012cf764 (LHCombined Release)
uint32_t g_mp_rs_zc;
// BW1W120 00ec7e68 BW1M119 012cf768 (LHCombined Release)
uint32_t g_mp_rs_abe;
// BW1W120 00ec7e6c BW1M119 012cf76c (LHCombined Release)
uint32_t g_mp_rs_db;
// BW1W120 00ec7e70 BW1M119 012cf770 (LHCombined Release)
uint32_t g_mp_rs_sb;
// BW1W120 00ec7e74 BW1M119 012cf774 (LHCombined Release)
uint32_t g_mp_rs_ao;
// BW1W120 00ec7e78 BW1M119 012cf778 (LHCombined Release)
uint32_t g_mp_rs_aa1;
// BW1W120 00ec7e7c BW1M119 012cf77c (LHCombined Release)
uint32_t g_mp_rs_co;
// BW1W120 00ec7e80 BW1M119 012cf780 (LHCombined Release)
uint32_t g_mp_rs_ca1;
// BW1W120 00ec7e88 BW1M119 012cf784 (LHCombined Release)
DDPIXELFORMAT zbuffer_ddpf[LH3D_RENDER_ZBUFFER_FORMAT_COUNT];
// BW1W120 00eca674 BW1M119 012cf804 (LHCombined Release)
int g_zddpf_count;
int g_b_any_kyro;

enum
{
	LH3D_RENDER_CLEAN_SCREEN_DELAY = 2000,
	LH3D_RENDER_MILLISECONDS_PER_SECOND = 1000,
	LH3D_RENDER_FRAME_RATE_PERIOD = 3000,
	LH3D_RENDER_FRAME_RATE_MIN_FRAMES = 4,
	LH3D_RENDER_FRAME_RATE_MIN = 5,
	LH3D_RENDER_FPU_CONTROL_MASK = 0xd00,
	LH3D_RENDER_FPU_PRECISION_53 = 0x200,
	LH3D_RENDER_RECTANGLE_VERTEX_COUNT = 4,
	LH3D_RENDER_RECTANGLE_INDEX_COUNT = 6,
};

enum LH3D_RENDER_MODE
{
	LH3D_RENDER_MODE_SMOOTH = 0,
	LH3D_RENDER_MODE_SMOOTH_ALPHA = 1,
	LH3D_RENDER_MODE_TEXTURED = 2,
	LH3D_RENDER_MODE_TEXTURED_ALPHA = 3,
	LH3D_RENDER_MODE_ALPHA_TEXTURED = 4,
	LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA = 5,
	LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_NZ = 6,
	LH3D_RENDER_MODE_SMOOTH_ALPHA_NZ = 7,
	LH3D_RENDER_MODE_TEXTURED_ALPHA_NZ = 8,
	LH3D_RENDER_MODE_TEXTURED_CHROMA = 9,
	LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF_CHROMA = 10,
	LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF_CHROMA_NZ = 11,
	LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF = 12,
	LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF_NZ = 13,
	LH3D_RENDER_MODE_TEXTURED_CHROMA_ALPHA = 15,
	LH3D_RENDER_MODE_TEXTURED_CHROMA_ALPHA_NZ = 16,
	LH3D_RENDER_MODE_CHROMA_JUST_Z = 18,
	LH3D_RENDER_MODE_NONE = 20,
};

static const float Dat_009A3980 = FLT_MAX;

void fn_0082B0F0()
{
	static int s_last_time = LH3DTech::g_timer.MSeconds();
	int        time = LH3DTech::g_timer.MSeconds();
	int        elapsed = time - s_last_time;
	if (elapsed >= LH3D_RENDER_FRAME_RATE_PERIOD)
	{
		int        frame = LH3DRender::g_frame;
		static int s_last_frame = frame;
		int        frames = frame - s_last_frame;
		if (frames >= LH3D_RENDER_FRAME_RATE_MIN_FRAMES)
		{
			int rate = frames * LH3D_RENDER_MILLISECONDS_PER_SECOND / elapsed;
			if (rate >= LH3D_RENDER_FRAME_RATE_MIN)
			{
				g_frame_rate_stats = rate;
				s_last_time = time;
				s_last_frame = frame;
			}
		}
	}
}

LH3DColor g_color_background(0, 0, 0);

void LH3DRender::SetD3DMatrix(D3DMATRIX& d3d_matrix, LHMatrix& matrix)
{
	d3d_matrix._11 = matrix._11;
	d3d_matrix._12 = matrix._12;
	d3d_matrix._13 = matrix._13;
	d3d_matrix._14 = 0.0f;
	d3d_matrix._21 = matrix._21;
	d3d_matrix._22 = matrix._22;
	d3d_matrix._23 = matrix._23;
	d3d_matrix._24 = 0.0f;
	d3d_matrix._31 = matrix._31;
	d3d_matrix._32 = matrix._32;
	d3d_matrix._33 = matrix._33;
	d3d_matrix._34 = 0.0f;
	d3d_matrix._41 = matrix._41;
	d3d_matrix._42 = matrix._42;
	d3d_matrix._43 = matrix._43;
	d3d_matrix._44 = 1.0f;
}

void LH3DRender::ResetD3DMatrix(D3DMATRIX& matrix)
{
	matrix._11 = 1.0f;
	matrix._12 = 0.0f;
	matrix._13 = 0.0f;
	matrix._14 = 0.0f;
	matrix._21 = 0.0f;
	matrix._22 = 1.0f;
	matrix._23 = 0.0f;
	matrix._24 = 0.0f;
	matrix._31 = 0.0f;
	matrix._32 = 0.0f;
	matrix._33 = 1.0f;
	matrix._34 = 0.0f;
	matrix._41 = 0.0f;
	matrix._42 = 0.0f;
	matrix._43 = 0.0f;
	matrix._44 = 1.0f;
}

D3DMATRIX LH3DRender::SetD3DProjectionMatrix(D3DMATRIX& matrix, float near_plane, float far_plane, float fov)
{
	float angle = fov;
	if (angle == 0.0f)
	{
		angle = LH3D_RENDER_DEFAULT_FOV;
	}
	float h = 1.0f / (float)tan(angle * 0.5);
	memset(&matrix, 0, sizeof(matrix));
	matrix._34 = 1.0f;
	float q = far_plane / (far_plane - near_plane);
	matrix._11 = h;
	matrix._22 = h * LH3D_RENDER_ASPECT_RATIO;
	matrix._33 = q;
	matrix._43 = -(q * near_plane);
	Direct3DDevice7->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matrix);
	return matrix;
}

// BW1W120 0082b370
static HRESULT CALLBACK EnumZBufferFormatsCallback(DDPIXELFORMAT* pixel_format, void* context);

static HRESULT CALLBACK EnumZBufferFormatsCallback(DDPIXELFORMAT* pixel_format, void* context)
{
	if (g_zddpf_count == LH3D_RENDER_ZBUFFER_FORMAT_COUNT)
	{
		return D3DENUMRET_OK;
	}
	if (pixel_format->dwFlags == DDPF_ZBUFFER)
	{
		zbuffer_ddpf[g_zddpf_count++] = *pixel_format;
		Report3D__FPCce("ZBUFFER: %d\n", pixel_format->dwZBufferBitDepth);
	}
	return D3DENUMRET_OK;
}

inline void LH3DAnim::Inline_0082B5D0()
{
	if (AnimPack)
	{
		LH3DAnimPack* pack = AnimPack;
		for (int i = 0; i < pack->AnimCount; i++)
		{
			pack->Anims[i]->Release();
		}
		LH3DMem::Free(AnimPack);
		AnimPack = NULL;
	}
}

bool32_t LH3DRender::Open()
{
	Report3D__FPCce("#OPEN#\n");
	if (!b_need_init)
	{
		return false;
	}
	if (b_open)
	{
		Report3D__FPCce("<<<<< LH3DRender Start: multi calls. >>>>>\n");
		return false;
	}
	Report3D__FPCce("%d TEXTURE AVAILABLE \n", LHSys::TheSystem.screen.GetAvailableVidMem());
	LH3DMem::Open();
	LH3DMath::Open();
	b_open = true;
	g_frame = 0;
	g_started_frame = 0;
	OpenD3D();
	LH3DTech::RenderInitialization(LHSys::TheSystem.screen.width, LHSys::TheSystem.screen.height);
	LH3DTexture::Open();
	LH3DTech::g_ambient_wind_direction.x = 1.0f;
	LH3DTech::g_ambient_wind_direction.y = 0.0f;
	LH3DTech::g_ambient_wind_direction.z = 2.0f;
	LH3DTech::g_ambient_wind_direction.FastNormalize();
	LowResTexture::Open();
	LH3DMaterial::g_count = 0;
	LH3DMaterial::g_list_render_func = LH3DMaterial::g_list_render_func_normal;
	LH3DLine::Open();
	LH3DMesh::Open();
	LH3DObject::Open();
	LH3DWater::Open(LHSys::TheSystem.screen.height);
	LH3DLandscape::Open();
	fn_0081E820();
	fn_00880F20(0, 0xc);
	fn_007AC9E0();
	g_motion_blur_value = 0;
	g_b_need_anim_uv = false;
	fn_0081FAA0();
	Report3D__FPCce(g_b_zbuffer_16b ? "using z buffer for sun before check\n"
	                                : "no z buffer compatibility before check\n");
	return true;
}

bool32_t LH3DRender::Close()
{
	if (!b_need_init)
	{
		return false;
	}
	if (!b_open)
	{
		Report3D__FPCce("<<<<< LH3DRender: Finish without start. >>>>>\n");
		return false;
	}
	Report3D__FPCce("#CLOSE#\n");
	b_open = false;
	fn_0081FE10();
	fn_0081E870();
	LH3DMist::Close();
	LH3DLandscape::Close();
	LH3DWater::Close();
	LH3DObject::Close();
	LH3DMesh::Close();
	LH3DAnim::Inline_0082B5D0();
	LH3DLine::Close();
	LH3DMaterial::Close();
	LH3DTexture::Close();
	LH3DMath::Close();
	g_zsorter->fn_0083F2F0();
	CloseD3D();
	LH3DMem::Close();
	return true;
}

void LH3DRender::SetStatesNVidiaStyle(unsigned long color_op)
{
	SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	SetTextureStageState(1, D3DTSS_COLOROP, color_op);
	SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
}

void LH3DRender::SetStatesVoodooStyle(unsigned long color_op)
{
	SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	SetTextureStageState(1, D3DTSS_COLOROP, color_op);
	SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 1);
	SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
	SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
	SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	if (g_curmat2tex)
	{
		fn_007ACE50(g_curmat2tex, 1);
	}
}

void LH3DRender::fn_0082BA10()
{
	SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, g_mp_rs_abe);
	SetTextureStageState(0, D3DTSS_COLORARG1, g_mp_rs_ca1);
	SetTextureStageState(0, D3DTSS_COLOROP, g_mp_rs_co);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, g_mp_rs_aa1);
	SetTextureStageState(0, D3DTSS_ALPHAOP, g_mp_rs_ao);
	SetRenderState(D3DRENDERSTATE_SRCBLEND, g_mp_rs_sb);
	SetRenderState(D3DRENDERSTATE_DESTBLEND, g_mp_rs_db);
	SetRenderState(D3DRENDERSTATE_ZFUNC, g_mp_rs_zc);
}

void LH3DRender::SetStatesMultipassStyle(unsigned long color_op)
{
	GetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, g_mp_rs_abe);
	GetTextureStageState(0, D3DTSS_COLORARG1, g_mp_rs_ca1);
	GetTextureStageState(0, D3DTSS_COLOROP, g_mp_rs_co);
	GetTextureStageState(0, D3DTSS_ALPHAARG1, g_mp_rs_aa1);
	GetTextureStageState(0, D3DTSS_ALPHAOP, g_mp_rs_ao);
	GetRenderState(D3DRENDERSTATE_SRCBLEND, g_mp_rs_sb);
	GetRenderState(D3DRENDERSTATE_DESTBLEND, g_mp_rs_db);
	GetRenderState(D3DRENDERSTATE_ZFUNC, g_mp_rs_zc);
	SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1);
	SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_EQUAL);
	switch (color_op)
	{
	case D3DTOP_SELECTARG2:
		SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
		break;
	case D3DTOP_SELECTARG1:
		SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
		SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
		break;
	case D3DTOP_MODULATE:
	case D3DTOP_MODULATE2X:
		SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
		SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
		break;
	case D3DTOP_ADD:
		SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
		break;
	}
}

void LH3DRender::Init2TexStuff()
{
	D3DDEVICEDESC7 desc;
	unsigned long  passes;

	ResetAllState();
	Direct3DDevice7->BeginStateBlock();
	SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	Direct3DDevice7->EndStateBlock(&g_stateblock2tex[0].Handle);
	memset(&desc, 0, sizeof(desc));
	Direct3DDevice7->GetCaps(&desc);
	for (unsigned long color_op = D3DTOP_DISABLE; color_op < LH3D_RENDER_STATE_BLOCK_COUNT; color_op++)
	{
		if (color_op == D3DTOP_ADD)
		{
			Report3D__FPCce("check additive: ");
		}
		else if (color_op == D3DTOP_MODULATE)
		{
			Report3D__FPCce("check modulate: ");
		}
		else if (color_op == D3DTOP_MODULATE2X)
		{
			Report3D__FPCce("check modulate 2x: ");
		}
		else
		{
			Report3D__FPCce("check mode %d: ", color_op);
		}
		bool32_t      found = false;
		unsigned long supported = desc.dwTextureOpCaps & (1 << (color_op - 1));
		Report3D__FPCce("(caps say it's %ssupported) ", supported ? "" : "NOT ");
		for (int attempt = 0; attempt < 2; attempt++)
		{
			bool32_t voodoo = attempt;
			if (g_b_any_voodoo)
			{
				voodoo = attempt == 0;
				if (color_op == D3DTOP_MODULATE2X)
				{
					supported = 0;
				}
			}
			if (g_b_voodoo2 || g_b_banshee)
			{
				supported = 0;
			}
			if (!voodoo)
			{
				if (!found && supported)
				{
					Direct3DDevice7->ApplyStateBlock(g_stateblock2tex[0].Handle);
					SetStatesNVidiaStyle(color_op);
					if (Direct3DDevice7->ValidateDevice(&passes) == D3D_OK && passes == 1)
					{
						Direct3DDevice7->ApplyStateBlock(g_stateblock2tex[0].Handle);
						Direct3DDevice7->BeginStateBlock();
						SetStatesNVidiaStyle(color_op);
						Direct3DDevice7->EndStateBlock(&g_stateblock2tex[color_op].Handle);
						g_stateblockstyle[color_op] = LH3D_STATE_BLOCK_STYLE_NVIDIA;
						Report3D__FPCce("state block for mode %d is nvidia method!\n", color_op);
						found = true;
					}
					Direct3DDevice7->ApplyStateBlock(g_stateblock2tex[0].Handle);
				}
			}
			else if (!found && supported)
			{
				Direct3DDevice7->ApplyStateBlock(g_stateblock2tex[0].Handle);
				SetStatesVoodooStyle(color_op);
				if (Direct3DDevice7->ValidateDevice(&passes) == D3D_OK && passes == 1)
				{
					Direct3DDevice7->ApplyStateBlock(g_stateblock2tex[0].Handle);
					Direct3DDevice7->BeginStateBlock();
					SetStatesVoodooStyle(color_op);
					Direct3DDevice7->EndStateBlock(&g_stateblock2tex[color_op].Handle);
					Report3D__FPCce("state block for mode %d is voodoo method!\n", color_op);
					found = true;
					g_stateblockstyle[color_op] = LH3D_STATE_BLOCK_STYLE_VOODOO;
				}
				Direct3DDevice7->ApplyStateBlock(g_stateblock2tex[0].Handle);
			}
		}
		if (!found)
		{
			Direct3DDevice7->ApplyStateBlock(g_stateblock2tex[0].Handle);
			g_stateblock2tex[color_op].Multipass = true;
			Direct3DDevice7->ApplyStateBlock(g_stateblock2tex[0].Handle);
			Direct3DDevice7->BeginStateBlock();
			SetStatesMultipassStyle(color_op);
			Direct3DDevice7->EndStateBlock(&g_stateblock2tex[color_op].Handle);
			Report3D__FPCce("state block for mode %d is multipass method!\n", color_op);
			g_stateblockstyle[color_op] = LH3D_STATE_BLOCK_STYLE_MULTIPASS;
		}
		Direct3DDevice7->ApplyStateBlock(g_stateblock2tex[0].Handle);
	}
	ResetAllState();
}

int LH3DRender::ApplyStateBlock(int color_op)
{
	switch (g_stateblockstyle[color_op])
	{
	case LH3D_STATE_BLOCK_STYLE_VOODOO:
		SetStatesVoodooStyle(color_op);
		break;
	case LH3D_STATE_BLOCK_STYLE_MULTIPASS:
		SetStatesMultipassStyle(color_op);
		break;
	default:
		SetStatesNVidiaStyle(color_op);
		break;
	}
	return 0;
}

void LH3DRender::Set2ndTex(int color_op, LH3DMaterial* material, bool reset)
{
	g_lastblend[1] = -1;
	g_lastblend[2] = -1;
	g_curmode2tex = color_op;
	if (material)
	{
		g_curmat2tex = material;
	}
	if (reset)
	{
		g_curmat2tex = NULL;
		g_current_other_material = NULL;
		material = NULL;
	}
	if (color_op == 0 || color_op == D3DTOP_DISABLE)
	{
		g_lastblend[0] = -1;
		g_render_mode = LH3D_RENDER_MODE_NONE;
		g_use_multipass = false;
		SetTexture(NULL, 1);
		SetTexture(NULL, 2);
		SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		g_current_active_material = NULL;
		return;
	}

	g_use_multipass = g_stateblock2tex[color_op].Multipass;
	if (material == NULL)
	{
		if (g_use_multipass)
		{
			return;
		}
		material = g_current_other_material;
	}
	if (material)
	{
		fn_007ACE50(material, 1);
		if (g_use_multipass)
		{
			return;
		}
	}
	ApplyStateBlock(color_op);
	if (material)
	{
		((LH3DMaterial::RenderFunc*)LH3DMaterial::g_list_render_func)[material->render_mode].Func(material, 1);
		if (g_b_need_tilling || material->cull_mode & LH3D_MATERIAL_NEED_TILLING)
		{
			SetD3DTillingOn(1);
		}
		else
		{
			SetD3DTillingOff(1);
		}
	}
}

void LH3DRender::Close2TexStuff()
{
	Set2ndTex(0, NULL, true);
	for (int i = 0; i < LH3D_RENDER_STATE_BLOCK_COUNT; i++)
	{
		if (g_stateblock2tex[i].Handle)
		{
			Direct3DDevice7->DeleteStateBlock(g_stateblock2tex[i].Handle);
		}
		g_stateblock2tex[i].Handle = 0;
	}
}

void LH3DRender::SetCurD3DMat(bool lit, unsigned long diffuse, unsigned long specular)
{
	static unsigned long s_diffuse = 0xffffffff;
	static unsigned long s_specular = 0xffffffff;
	static bool          s_lit;

	if (!Dat_00EC81C0 && lit == s_lit && diffuse == s_diffuse && specular == s_specular)
	{
		return;
	}
	g_d3d_mat.specular.b = (specular & 0xff) / 256.0f;
	g_d3d_mat.specular.g = ((specular >> 8) & 0xff) / 256.0f;
	s_specular = specular;
	g_d3d_mat.specular.r = ((specular >> 16) & 0xff) / 256.0f;
	Dat_00EC81C0 = false;
	s_lit = lit;
	s_diffuse = diffuse;
	g_d3d_mat.specular.a = (specular >> 24) / 256.0f;
	if (lit)
	{
		float diffuse_scale = (0x100 - LH3DPrimitive::MIN_LIGHT) / 65536.0f;
		float b = (float)(diffuse & 0xff);
		g_d3d_mat.diffuse.b = b * diffuse_scale;
		float g = (float)((diffuse >> 8) & 0xff);
		g_d3d_mat.diffuse.g = g * diffuse_scale;
		float r = (float)((diffuse >> 16) & 0xff);
		g_d3d_mat.diffuse.r = r * diffuse_scale;
		g_d3d_mat.diffuse.a = (diffuse >> 24) / 256.0f;
		float ambient_scale = LH3DPrimitive::MIN_LIGHT / 65536.0f;
		g_d3d_mat.ambient.b = ambient_scale * b;
		g_d3d_mat.ambient.g = ambient_scale * g;
		g_d3d_mat.ambient.r = ambient_scale * r;
		g_d3d_mat.ambient.a = g_d3d_mat.diffuse.a;
	}
	else
	{
		g_d3d_mat.ambient.b = (diffuse & 0xff) / 256.0f;
		g_d3d_mat.ambient.g = ((diffuse >> 8) & 0xff) / 256.0f;
		g_d3d_mat.diffuse.r = 0.0f;
		g_d3d_mat.diffuse.g = 0.0f;
		g_d3d_mat.ambient.r = ((diffuse >> 16) & 0xff) / 256.0f;
		g_d3d_mat.diffuse.b = 0.0f;
		g_d3d_mat.diffuse.a = g_d3d_mat.ambient.a = (diffuse >> 24) / 256.0f;
	}
	Direct3DDevice7->SetMaterial(&g_d3d_mat);
}

void LH3DRender::ResetAllState()
{
	int      i;
	uint32_t value;

	Direct3DDevice7->BeginScene();
	if (g_first_for_reset_all)
	{
		g_first_for_reset_all = false;
		for (int stage = 0; stage < LH3D_RENDER_TEXTURE_STAGE_COUNT; stage++)
		{
			for (i = 0; i < LH3D_RENDER_TEXTURE_STAGE_STATE_COUNT; i++)
			{
				value = 0xffffffff;
				if (Direct3DDevice7->GetTextureStageState(stage, (D3DTEXTURESTAGESTATETYPE)i, (LPDWORD)&value) ==
				    D3D_OK)
				{
					g_original_texture_stage_state[(stage << 8) + i] = value;
				}
				else
				{
					g_original_texture_stage_state[(stage << 8) + i] = 0xffffffff;
				}
			}
		}
		for (i = 0; i < LH3D_RENDER_STATE_COUNT; i++)
		{
			value = 0xffffffff;
			if (Direct3DDevice7->GetRenderState((D3DRENDERSTATETYPE)i, (LPDWORD)&value) == D3D_OK)
			{
				g_original_render_states[i] = value;
			}
			else
			{
				g_original_render_states[i] = 0xffffffff;
			}
		}
	}
	else
	{
		for (i = 0; i < LH3D_RENDER_STATE_COUNT; i++)
		{
			if (g_original_render_states[i] != 0xffffffff)
			{
				SetRenderState((D3DRENDERSTATETYPE)i, g_original_render_states[i]);
			}
		}
		for (i = 0; i < LH3D_RENDER_TEXTURE_STAGE_STATE_COUNT; i++)
		{
			for (int stage = 0; stage < LH3D_RENDER_TEXTURE_STAGE_COUNT; stage++)
			{
				if (g_original_texture_stage_state[(stage << 8) + i] != 0xffffffff)
				{
					SetTextureStageState(stage, (D3DTEXTURESTAGESTATETYPE)i,
					                     g_original_texture_stage_state[(stage << 8) + i]);
				}
			}
		}
	}
	memset(g_texture_stage_ptr, 0, sizeof(g_texture_stage_ptr));
	SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
	SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
	SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
	SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
	SetRenderState(D3DRENDERSTATE_ZBIAS, 0);
	SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	if (g_b_any_voodoo)
	{
		SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	}
	else
	{
		SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);
	}
	SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
	SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
	SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
	SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
	SetRenderState(D3DRENDERSTATE_COLORVERTEX, FALSE);
	SetRenderState(D3DRENDERSTATE_LOCALVIEWER, FALSE);
	SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, TRUE);
	SetRenderState(D3DRENDERSTATE_CLIPPING, FALSE);
	Direct3DDevice7->EndScene();
}

bool32_t LH3DRender::OpenD3D()
{
	D3DDEVICEDESC7 device_desc;
	D3DVIEWPORT7   viewport;
	HRESULT        result;

	Report3D__FPCce("#OPEN D3D#\n");
	LHSys::TheSystem.screen.PDirectDraw->QueryInterface(IID_IDirect3D7, (void**)&D3D);
	ZBufferSurface = NULL;
	g_zsorter = LH3DZSorter::Create(LH3D_RENDER_ZSORTER_SIZE);
	g_mode_cleaning = true;
	result = D3D->EnumZBufferFormats(IID_IDirect3DHALDevice, EnumZBufferFormatsCallback, NULL);
	if (result != D3D_OK)
	{
		LHLogger::LogErrorCodeS(__LH3DLib__, FILEPATH, 972, LH_DX_Errors, result);
	}
	bool32_t found = false;
	if (g_zddpf_count)
	{
		int i = 0;
		while (i < g_zddpf_count && !found)
		{
			DDSURFACEDESC2 desc;
			memset(&desc, 0, sizeof(desc));
			desc.dwSize = sizeof(desc);
			desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
			desc.dwWidth = LHSys::TheSystem.screen.width;
			desc.dwHeight = LHSys::TheSystem.screen.height;
			desc.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
			desc.ddpfPixelFormat = zbuffer_ddpf[i];
			result = LHSys::TheSystem.screen.PDirectDraw->CreateSurface(&desc, &ZBufferSurface, NULL);
			g_b_zbuffer_16b = zbuffer_ddpf[i].dwZBufferBitDepth == 16;
			if (g_b_any_kyro)
			{
				g_b_zbuffer_16b = false;
			}
			else if (g_b_zbuffer_16b)
			{
				Report3D__FPCce("ZBUFFER 16 bits\n");
			}
			if (result != DD_OK)
			{
				i++;
				continue;
			}
			if (LHSys::TheSystem.screen.PBackSurface->AddAttachedSurface(ZBufferSurface) != DD_OK)
			{
				if (ZBufferSurface)
				{
					ZBufferSurface->Release();
				}
				ZBufferSurface = NULL;
				i++;
				continue;
			}
			found = true;
			Report3D__FPCce("We choose  : (%2d bits) Zbuffer %s%s%s %08X %08X %08X %08X \n",
			                zbuffer_ddpf[i].dwZBufferBitDepth,
			                zbuffer_ddpf[i].dwFlags & (DDPF_ALPHAPIXELS | DDPF_ALPHA) ? "ALPHA " : "      ",
			                zbuffer_ddpf[i].dwFlags & DDPF_RGB ? "RGB " : "    ",
			                zbuffer_ddpf[i].dwFlags & (DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8) ? "PAL " : "    ",
			                zbuffer_ddpf[i].dwRGBAlphaBitMask, zbuffer_ddpf[i].dwRBitMask, zbuffer_ddpf[i].dwGBitMask,
			                zbuffer_ddpf[i].dwBBitMask);
		}
	}
	if (lpPrefferedDeviceGUID == NULL ||
	    (result = D3D->CreateDevice(*lpPrefferedDeviceGUID, LHSys::TheSystem.screen.PBackSurface, &Direct3DDevice7)) !=
	        D3D_OK)
	{
		result = D3D->CreateDevice(IID_IDirect3DHALDevice, LHSys::TheSystem.screen.PBackSurface, &Direct3DDevice7);
	}
	if (result != D3D_OK)
	{
		return false;
	}
	if (Direct3DDevice7->GetCaps(&device_desc) != D3D_OK)
	{
		g_b_support_AGP = true;
		g_b_support_TNL = false;
	}
	else
	{
		if (device_desc.dwDevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM)
		{
			Report3D__FPCce("Support AGP.\n");
			g_b_support_AGP = true;
		}
		else
		{
			Report3D__FPCce("Doesn't support AGP.\n");
			g_b_support_AGP = false;
		}
		if (device_desc.dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT && !g_b_disable_TNL &&
		    device_desc.deviceGUID == IID_IDirect3DTnLHalDevice)
		{
			Report3D__FPCce("Support Hardware T'n'L.\n");
			g_b_support_TNL = true;
		}
		else
		{
			Report3D__FPCce("Doesn't support hardware T'n'L / tnl disabled.\n");
			g_b_support_TNL = false;
		}
	}
	memset(&viewport, 0, sizeof(viewport));
	viewport.dwX = 0;
	viewport.dwY = 0;
	viewport.dwWidth = LHSys::TheSystem.screen.width;
	viewport.dwHeight = LHSys::TheSystem.screen.height;
	viewport.dvMinZ = 0.0f;
	viewport.dvMaxZ = 1.0f;
	if (Direct3DDevice7->SetViewport(&viewport) != D3D_OK)
	{
		return false;
	}
	memset(g_lastblend, 0xff, sizeof(g_lastblend));
	ResetAllState();
	Direct3DDevice7->BeginScene();
	SetD3DProjectionMatrix(g_d3d_view2proj, LH3DTech::g_info_transform.NearClip, LH3D_RENDER_FAR_PLANE,
	                       LH3DTech::g_camera.fov);
	ResetD3DMatrix(g_d3d_local2world);
	ResetD3DMatrix(g_d3d_world2view);
	Direct3DDevice7->SetTransform(D3DTRANSFORMSTATE_WORLD, &g_d3d_local2world);
	Direct3DDevice7->SetTransform(D3DTRANSFORMSTATE_VIEW, &g_d3d_world2view);
	memset(&g_d3d_light, 0, sizeof(g_d3d_light));
	g_d3d_light.dcvSpecular.r = 0.0f;
	g_d3d_light.dcvSpecular.g = 0.0f;
	g_d3d_light.dcvSpecular.b = 0.0f;
	g_d3d_light.dcvSpecular.a = 0.0f;
	memset(&g_d3d_mat, 0, sizeof(g_d3d_mat));
	g_d3d_light.dltType = D3DLIGHT_POINT;
	g_d3d_light.dcvDiffuse.r = 1.0f;
	g_d3d_light.dcvDiffuse.g = 1.0f;
	g_d3d_light.dcvDiffuse.b = 1.0f;
	g_d3d_light.dcvDiffuse.a = 1.0f;
	g_d3d_light.dcvAmbient.r = 1.0f;
	g_d3d_light.dcvAmbient.g = 1.0f;
	g_d3d_light.dcvAmbient.b = 1.0f;
	g_d3d_light.dcvAmbient.a = 1.0f;
	g_d3d_light.dvPosition.x = LH3DTech::g_sun_light.x;
	g_d3d_light.dvPosition.y = LH3DTech::g_sun_light.y;
	g_d3d_light.dvPosition.z = LH3DTech::g_sun_light.z;
	g_d3d_light.dvRange = D3DLIGHT_RANGE_MAX;
	g_d3d_light.dvAttenuation0 = 1.0f;
	g_d3d_mat.diffuse.r = 1.0f;
	g_d3d_mat.diffuse.g = 1.0f;
	g_d3d_mat.diffuse.b = 1.0f;
	g_d3d_mat.diffuse.a = 1.0f;
	g_d3d_mat.ambient.r = LH3D_RENDER_MATERIAL_AMBIENT;
	g_d3d_mat.ambient.g = LH3D_RENDER_MATERIAL_AMBIENT;
	g_d3d_mat.ambient.b = LH3D_RENDER_MATERIAL_AMBIENT;
	g_d3d_mat.ambient.a = 1.0f;
	Direct3DDevice7->SetMaterial(&g_d3d_mat);
	Direct3DDevice7->SetLight(0, &g_d3d_light);
	Direct3DDevice7->LightEnable(0, TRUE);
	if (g_b_use_wireframe)
	{
		SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
	}
	else
	{
		SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	}
	Direct3DDevice7->EndScene();
	result = Direct3DDevice7->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	if (g_b_support_TNL)
	{
		LH3DVB::Init();
	}
	Init2TexStuff();
	return result;
}

bool32_t LH3DRender::CloseD3D()
{
	Report3D__FPCce("#CLOSE D3D#\n");
	Close2TexStuff();
	if (g_b_support_TNL)
	{
		LH3DVB::Close();
	}
	if (Direct3DDevice7)
	{
		Direct3DDevice7->Release();
		Direct3DDevice7 = NULL;
	}
	if (D3D)
	{
		D3D->Release();
		D3D = NULL;
	}
	if (ZBufferSurface)
	{
		ZBufferSurface->Release();
		ZBufferSurface = NULL;
	}
	Report3D__FPCce("#END CLOSE D3D#\n");
	return true;
}

void __fastcall LH3DRender::SetRenderModeSmooth(LH3DMaterial* material, int pass)
{
	if (pass != 0)
	{
		SetTexture(NULL, pass);
		return;
	}
	if (g_render_mode != LH3D_RENDER_MODE_SMOOTH)
	{
		g_render_mode = LH3D_RENDER_MODE_SMOOTH;
		SetTexture(NULL, 0);
		Inline_0082D4E0(FALSE, D3DBLEND_ONE, D3DBLEND_ONE, FALSE);
		Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE);
	}
	SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
}

void __fastcall LH3DRender::SetRenderModeSmoothAlpha(LH3DMaterial* material, int pass)
{
	if (pass != 0)
	{
		SetTexture(NULL, pass);
		return;
	}
	if (g_render_mode != LH3D_RENDER_MODE_SMOOTH_ALPHA)
	{
		g_render_mode = LH3D_RENDER_MODE_SMOOTH_ALPHA;
		SetTexture(NULL, 0);
		SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	}
	SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
}

void __fastcall LH3DRender::SetRenderModeSmoothAlphaNZ(LH3DMaterial* material, int pass)
{
	if (pass != 0)
	{
		SetTexture(NULL, pass);
		return;
	}
	if (g_render_mode != LH3D_RENDER_MODE_SMOOTH_ALPHA_NZ)
	{
		g_render_mode = LH3D_RENDER_MODE_SMOOTH_ALPHA_NZ;
		SetTexture(NULL, 0);
		SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	}
	SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
}

void __fastcall LH3DRender::SetRenderModeTextured(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_TEXTURED)
		{
			g_render_mode = LH3D_RENDER_MODE_TEXTURED;
			Inline_0082D4E0(FALSE, D3DBLEND_ONE, D3DBLEND_ONE, FALSE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeTexturedAlpha(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_TEXTURED_ALPHA)
		{
			g_render_mode = LH3D_RENDER_MODE_TEXTURED_ALPHA;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, FALSE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeTexturedAlphaNZ(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_TEXTURED_ALPHA_NZ)
		{
			g_render_mode = LH3D_RENDER_MODE_TEXTURED_ALPHA_NZ;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, FALSE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeAlphaTextured(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_ALPHA_TEXTURED)
		{
			g_render_mode = LH3D_RENDER_MODE_ALPHA_TEXTURED;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, FALSE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeAlphaTexturedAlpha(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA)
		{
			g_render_mode = LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, FALSE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeAlphaTexturedAlphaNZ(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_NZ)
		{
			g_render_mode = LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_NZ;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, FALSE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeChromaJustZ(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_CHROMA_JUST_Z)
		{
			g_render_mode = LH3D_RENDER_MODE_CHROMA_JUST_Z;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_ONE, TRUE);
			SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
	if (OverrideMaterial)
	{
		SetRenderState(D3DRENDERSTATE_ALPHAREF, OverrideRenderMode);
	}
	else
	{
		SetRenderState(D3DRENDERSTATE_ALPHAREF, material->GetAlphaReference());
	}
	if (material->texture)
	{
		material->texture->Activate(pass);
	}
	else
	{
		SetTexture(NULL, pass);
	}
}

void __fastcall LH3DRender::SetRenderModeTexturedChromaAlphaNZ(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_TEXTURED_CHROMA_ALPHA_NZ)
		{
			g_render_mode = LH3D_RENDER_MODE_TEXTURED_CHROMA_ALPHA_NZ;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, TRUE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	}
	if (OverrideMaterial)
	{
		SetRenderState(D3DRENDERSTATE_ALPHAREF, OverrideRenderMode);
	}
	else
	{
		SetRenderState(D3DRENDERSTATE_ALPHAREF, material->GetAlphaReference());
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeAlphaTexturedAlphaAdditifChroma(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF_CHROMA)
		{
			g_render_mode = LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF_CHROMA;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_ONE, TRUE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
	if (OverrideMaterial)
	{
		SetRenderState(D3DRENDERSTATE_ALPHAREF, OverrideRenderMode);
	}
	else
	{
		SetRenderState(D3DRENDERSTATE_ALPHAREF, material->GetAlphaReference());
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeAlphaTexturedAlphaAdditifChromaNZ(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF_CHROMA_NZ)
		{
			g_render_mode = LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF_CHROMA_NZ;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_ONE, TRUE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	}
	if (OverrideMaterial)
	{
		SetRenderState(D3DRENDERSTATE_ALPHAREF, OverrideRenderMode);
	}
	else
	{
		SetRenderState(D3DRENDERSTATE_ALPHAREF, material->GetAlphaReference());
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeAlphaTexturedAlphaAdditif(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF)
		{
			g_render_mode = LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_ONE, FALSE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
	material->texture->Activate(pass);
}

void __fastcall LH3DRender::SetRenderModeAlphaTexturedAlphaAdditifNZ(LH3DMaterial* material, int pass)
{
	if (pass == 0)
	{
		if (g_render_mode != LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF_NZ)
		{
			g_render_mode = LH3D_RENDER_MODE_ALPHA_TEXTURED_ALPHA_ADDITIF_NZ;
			Inline_0082D4E0(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_ONE, FALSE);
			Inline_0082D507(0, LH3D_RENDER_BLEND_TEXTURE_DIFFUSE_ALPHA);
		}
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	}
	material->texture->Activate(pass);
}

void LH3DMaterial::Close()
{
	if (LH3DMaterial::g_count)
	{
		Report3D__FPCce("LH3DMaterial: %d materials not released.\n", LH3DMaterial::g_count);
	}
}

void LH3DRender::CleanScreen()
{
	static int      s_last_clean_time;
	static bool32_t s_b_clean_pending;
	int             mode = g_mode_cleaning;

	if (mode == 0 && g_enable_callbacks)
	{
		int now = LH3DTech::g_timer.MSeconds();
		if (now < s_last_clean_time || s_last_clean_time + LH3D_RENDER_CLEAN_SCREEN_DELAY < now)
		{
			mode = 1;
			s_last_clean_time = now;
			s_b_clean_pending = true;
		}
		else if (s_b_clean_pending)
		{
			s_b_clean_pending = false;
			mode = 1;
		}
	}
	switch (mode)
	{
	case 0:
		return;
	case 1:
		Direct3DDevice7->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, g_color_background.GetColor(), 1.0f, 0);
		return;
	case 2:
		Direct3DDevice7->Clear(0, NULL, D3DCLEAR_ZBUFFER, g_color_background.GetColor(), 1.0f, 0);
		return;
	}
}

void __fastcall LH3DRender::DrawRectangle(LHRegion& region, unsigned long color)
{
	uint16_t      indices[LH3D_RENDER_RECTANGLE_INDEX_COUNT];
	LH3DMaterial* material = CreateMaterial(LH3DMaterial::LH3D_MATERIAL_RENDER_MODE_0x1, NULL);

	Inline_0082EF67(material);

	Vertex3D* vertices = (Vertex3D*)LH3DP3::Table1;
	vertices[0].position.x = region.X1();
	vertices[0].position.y = region.Y1();
	vertices[0].position.z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[0].color = color;
	vertices[1].position.x = region.X2();
	vertices[1].position.y = region.Y1();
	vertices[1].position.z = 0.0f;
	vertices[1].rhw = 1.0f;
	vertices[1].color = color;
	vertices[2].position.x = region.X2();
	vertices[2].position.y = region.Y2();
	vertices[2].position.z = 0.0f;
	vertices[2].rhw = 1.0f;
	vertices[2].color = color;
	vertices[3].position.x = region.X1();
	vertices[3].position.y = region.Y2();
	vertices[3].position.z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[3].color = color;
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;
	DrawAndClip2D(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, vertices, LH3D_RENDER_RECTANGLE_VERTEX_COUNT, indices,
	              LH3D_RENDER_RECTANGLE_INDEX_COUNT);
	if (material)
	{
		delete material;
	}
}

void LH3DRender::StartFrame()
{
	fn_0082B0F0();
	fn_00824400();
	g_curmode2tex = 0;
	g_curmat2tex = NULL;
	Dat_00EC81C0 = true;
	LH3DVB::newframe = true;
	LH3DVB::newframemt = true;
	LH3DVB::curv = 0;
	LH3DVB::curvmt = 0;
	g_b_sorted_object_drawn_this_frame = false;

	uint16_t controlWord;
	uint16_t newControlWord;
	__asm
		{
		finit
		fwait
		fstcw controlWord
		fwait
		mov ax, controlWord
		and eax, not LH3D_RENDER_FPU_CONTROL_MASK
		or eax, LH3D_RENDER_FPU_PRECISION_53
		mov newControlWord, ax
		fldcw newControlWord
		fwait
	    }

	caca_static_draw = 0;
	LH3DTech::g_current_time = LH3DTech::g_timer.MSeconds();
	LH3DTech::g_delta_time = LH3DTech::g_current_time - LH3DTech::g_previous_time;
	if (LH3DTech::g_delta_time <= 0)
	{
		LH3DTech::g_delta_time = 1;
	}
	LH3DTech::g_previous_time = LH3DTech::g_current_time;
	g_poly_rate = g_num_polys_drawn * (float)LH3D_RENDER_MILLISECONDS_PER_SECOND / LH3DTech::g_delta_time;
	g_num_polys_drawn = 0;
	g_poly_rate_smooth = (g_poly_rate - g_poly_rate_smooth) * LH3D_RENDER_POLY_RATE_SMOOTHING + g_poly_rate_smooth;
	Direct3DDevice7->BeginScene();
	g_frame++;
	g_started_frame = true;
	g_zsorter->OpenSort();
	fn_00813770();
	fn_0085BF00();
	g_d3d_light.dvPosition.x = LH3DTech::g_sun_light.x;
	g_d3d_light.dvPosition.y = LH3DTech::g_sun_light.y;
	g_d3d_light.dvPosition.z = LH3DTech::g_sun_light.z;
	if (!g_b_disable_TNL && g_b_support_TNL)
	{
		Direct3DDevice7->SetLight(0, &g_d3d_light);
		SetD3DProjectionMatrix(g_d3d_view2proj, LH3DTech::g_info_transform.NearClip, Dat_009A3980,
		                       LH3DTech::g_camera.fov);
	}
	LH3DCameraChecker::Process();
	LHResetFPU();
}

void LH3DRender::DrawSortedObjects()
{
	g_b_sorted_object_drawn_this_frame = true;
	LH3DLine::DrawAllPreStored();
	for (LH3DZSorter::ZObject* object = LH3DZSorter::Dat_00EE9D38; object != NULL; object = object->Next)
	{
		LH3DZSorter::UserData = object->Flags;
		(((DrawingObject*)object->Object)->*object->Draw)();
	}
}

void LH3DRender::RegisterFinishFrameCallback(unsigned long priority, bool after, FinishFrameCallback callback,
                                             void* data)
{
	if (g_num_finish_frame_cb < LH3D_RENDER_MAX_FINISH_FRAME_CALLBACKS && callback)
	{
		RemoveFinishFrameCallback(callback, data);
		if (after)
		{
			priority |= LH3D_RENDER_FINISH_FRAME_CALLBACK_AFTER;
		}
		int i;
		for (i = 0; i < g_num_finish_frame_cb; i++)
		{
			if (g_finish_frame_cb_priority[i] > priority)
			{
				break;
			}
		}
		memmove(&g_finish_frame_cb[i + 1], &g_finish_frame_cb[i],
		        (g_num_finish_frame_cb - i) * sizeof(FinishFrameCallback));
		memmove(&g_finish_frame_cb_priority[i + 1], &g_finish_frame_cb_priority[i],
		        (g_num_finish_frame_cb - i) * sizeof(uint32_t));
		memmove(&g_finish_frame_cb_data[i + 1], &g_finish_frame_cb_data[i],
		        (g_num_finish_frame_cb - i) * sizeof(void*));
		g_finish_frame_cb[i] = callback;
		g_finish_frame_cb_priority[i] = priority;
		g_finish_frame_cb_data[i] = data;
		g_num_finish_frame_cb++;
	}
}

void LH3DRender::RemoveFinishFrameCallback(FinishFrameCallback callback, void* data)
{
	for (int i = 0; i < g_num_finish_frame_cb; i++)
	{
		if (g_finish_frame_cb[i] == callback && g_finish_frame_cb_data[i] == data)
		{
			g_num_finish_frame_cb--;
			memmove(&g_finish_frame_cb[i], &g_finish_frame_cb[i + 1],
			        (g_num_finish_frame_cb - i) * sizeof(FinishFrameCallback));
			memmove(&g_finish_frame_cb_priority[i], &g_finish_frame_cb_priority[i + 1],
			        (g_num_finish_frame_cb - i) * sizeof(uint32_t));
			memmove(&g_finish_frame_cb_data[i], &g_finish_frame_cb_data[i + 1],
			        (g_num_finish_frame_cb - i) * sizeof(void*));
			return;
		}
	}
}

void LH3DRender::fn_0082F7F0() {}

void __fastcall LH3DRender::SetBackgroundColor(LH3DColor* color)
{
	g_color_background = *color;
}

uint16_t* __fastcall LH3DRender::SortTriangle(Vertex3D* vertices, unsigned long vertex_count, uint16_t* indices,
                                              unsigned long index_count)
{
	static uint16_t s_sorted_list[LH3D_RENDER_MAX_SORTED_TRIANGLES * 3];
	static uint16_t s_next[LH3D_RENDER_MAX_SORTED_TRIANGLES];
	static uint16_t s_first[LH3D_RENDER_SORT_BUCKET_COUNT];
	int             triangle_count = index_count / 3;

	if (triangle_count > LH3D_RENDER_MAX_SORTED_TRIANGLES)
	{
		Report3D__FPCce("######################MERDAGE/n");
		triangle_count = LH3D_RENDER_MAX_SORTED_TRIANGLES;
	}
	memset(s_first, 0xff, sizeof(s_first));
	memset(s_next, 0xff, triangle_count * sizeof(uint16_t));

	float z = LH3DP3::Table1[g_TriangleList[0]][3] + LH3DP3::Table1[g_TriangleList[1]][3] +
	          LH3DP3::Table1[g_TriangleList[2]][3];
	float min_z = z;
	float max_z = z;

	for (uint16_t i = 3; i < triangle_count * 3; i += 3)
	{
		float z = LH3DP3::Table1[g_TriangleList[i]][3] + LH3DP3::Table1[g_TriangleList[i + 1]][3] +
		          LH3DP3::Table1[g_TriangleList[i + 2]][3];
		if (min_z > z)
		{
			min_z = z;
		}
		else if (max_z < z)
		{
			max_z = z;
		}
	}

	float scale = (LH3D_RENDER_SORT_BUCKET_COUNT - 1) / (max_z - min_z);
	for (uint16_t triangle = 0; triangle < triangle_count; triangle++)
	{
		int index = triangle * 3;
		int bucket = (int)((LH3DP3::Table1[g_TriangleList[index]][3] + LH3DP3::Table1[g_TriangleList[index + 1]][3] +
		                    LH3DP3::Table1[g_TriangleList[index + 2]][3] - min_z) *
		                   scale) &
		             (LH3D_RENDER_SORT_BUCKET_COUNT - 1);
		if (s_first[bucket] != 0xffff)
		{
			s_next[triangle] = s_first[bucket];
		}
		s_first[bucket] = triangle;
	}

	int count = 0;
	for (uint16_t bucket = 0; bucket < LH3D_RENDER_SORT_BUCKET_COUNT; bucket++)
	{
		int triangle = s_first[bucket];
		int index = triangle * 3;
		while (triangle != 0xffff)
		{
			s_sorted_list[count] = g_TriangleList[index];
			s_sorted_list[count + 1] = g_TriangleList[index + 1];
			s_sorted_list[count + 2] = g_TriangleList[index + 2];
			count += 3;
			triangle = s_next[triangle];
			index = triangle * 3;
		}
	}
	return s_sorted_list;
}

void __fastcall LH3DRender::SetUseWireFrame(bool32_t use_wireframe)
{
	g_b_use_wireframe = use_wireframe;
	if (Direct3DDevice7)
	{
		if (use_wireframe)
		{
			SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
		}
		else
		{
			SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
		}
	}
}

LH3DMaterial* LH3DRender::CreateMaterial(LH3DMaterial::RenderMode render_mode, LH3DTexture* texture)
{
	LH3DMaterial* material = new LH3DMaterial(render_mode, texture);
	material->color = LH3DColor(0xff0000ff);
	return material;
}

void LH3DRender::PushTillingMode(int index)
{
	Direct3DDevice7->GetTextureStageState(index, D3DTSS_ADDRESS, &g_save_tile);
}

void LH3DRender::PopTillingMode(int index)
{
	SetTextureStageState(index, D3DTSS_ADDRESS, g_save_tile);
}

void LH3DRender::SetD3DTillingOn(int index)
{
	SetTextureStageState(index, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
}

void LH3DRender::SetD3DTillingOff(int index)
{
	SetTextureStageState(index, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
}

void LH3DRender::fn_0082FF90(unsigned long value)
{
	GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, Dat_00C38868);
	SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, value);
}

void LH3DRender::fn_00830000()
{
	if (Dat_00C38868 != 0xffffffff)
	{
		SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, Dat_00C38868);
		Dat_00C38868 = 0xffffffff;
	}
}

unsigned long LH3DRender::g_save_tile = D3DTADDRESS_CLAMP;
int           LH3DRender::g_lastblend[LH3D_RENDER_TEXTURE_STAGE_COUNT] = {-1, -1, -1, -1, -1, -1, -1, -1};
int           LH3DRender::g_render_mode = LH3D_RENDER_MODE_NONE;
uint32_t      LH3DRender::Dat_00C38868 = 0xffffffff;
void*         LH3DRender::g_finish_frame_cb_data[LH3D_RENDER_MAX_FINISH_FRAME_CALLBACKS];
uint32_t      LH3DRender::g_finish_frame_cb_priority[LH3D_RENDER_MAX_FINISH_FRAME_CALLBACKS];
LH3DRender::FinishFrameCallback LH3DRender::g_finish_frame_cb[LH3D_RENDER_MAX_FINISH_FRAME_CALLBACKS];
IDirectDrawSurface7*            LH3DRender::g_texture_stage_ptr[LH3D_RENDER_TEXTURE_STAGE_COUNT];
int                             LH3DRender::g_num_finish_frame_cb;
bool32_t                        LH3DRender::g_b_sorted_object_drawn_this_frame;
bool32_t                        LH3DRender::g_b_need_anim_uv;
float                           LH3DRender::g_adu;
float                           LH3DRender::g_adv;
LH3DMaterial*                   LH3DRender::g_current_other_material;
bool32_t                        LH3DRender::g_b_use_wireframe;
int                             LH3DMaterial::g_count;
