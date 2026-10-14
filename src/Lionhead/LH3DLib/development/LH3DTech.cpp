#include "LH3DTech.h"

#include <math.h>   /* For fabs, sqrt, tan */
#include <stdarg.h> /* For va_list */
#include <stdio.h>  /* For vsprintf */
#include <stdlib.h> /* For rand, RAND_MAX */
#include <string.h> /* For memcpy */
#include <windows.h>

#include <d3d.h>

#include <Lionhead/LHLib/ver5.0/LHTimer.inl>

#include "LH3DCameraChecker.h"
#include "LH3DMaterial.h"
#include "LH3DMath.h"
#include "LH3DObject.h"
#include "LH3DP3.h"
#include "LH3DPrimitive.h"
#include "LH3DRender.h"
#include "LH3DVertex.h"
#include "LHMatrix.h"
#include "SuperVillager.h"
#include "Black/Water.h"

// TODO: These render-system globals still have oversized extracted split owners.
// Dedicated SuperVillager/Water implementation TUs have not yet been separated.
SuperVillager* SuperVillager::g_first;
LH3DSprite*    GWater::g_sprite_circle;

uint32_t LH3DTech::g_game_time_inc;

// TODO: The original BSS still belongs to a different extracted unit in splits.txt.
LHPoint LH3DTech::g_ambient_wind_direction;

#define LH3D_TECH_NEAR_CLIPPING      3.5f
#define LH3D_TECH_DEFAULT_FOV        1.2217305f
#define LH3D_TECH_DEFAULT_CAMERA_POS 20.0f
#define LH3D_TECH_MIN_DIRECTION      0.0001f

static const float Dat_009A3700 = 1.0f / RAND_MAX;

// BW1W120 inlined BW1M119 null
static int Inline_0081AFF2(float value);
// BW1W120 inlined BW1M119 null
static D3DCOLOR Inline_0081AF97(D3DCOLOR inside, D3DCOLOR outside, float inside_weight, float outside_weight);
// BW1W120 0081b7a0 BW1M119 null
static void fn_0081B7A0(uint16_t a, uint16_t b, LH3DTech::ClipCode clip);
// BW1W120 inlined BW1M119 null
static void Inline_0081D0D9(LH3DMaterial* material);

void __fastcall LH3DTech::RenderInitialization(long width, long height)
{
	g_timer.Start();
	g_world_to_clipping.SetIdentity();
	g_copy_of_world_to_clipping.SetIdentity();
	g_clipping_to_world.SetInverse(g_world_to_clipping);
	g_world_to_camera.SetIdentity();
	g_camera_to_world.SetIdentity();
	Dat_00EA1C98.SetIdentity();
	g_local_to_clipping.SetIdentity();
	g_current_matrix = &g_world_to_clipping;
	SetNearClipping(LH3D_TECH_NEAR_CLIPPING);
	UpdateViewPort(width, height);
	g_camera.pos.x = LH3D_TECH_DEFAULT_CAMERA_POS;
	g_camera.pos.y = LH3D_TECH_DEFAULT_CAMERA_POS;
	g_camera.pos.z = LH3D_TECH_DEFAULT_CAMERA_POS;
	g_camera.foc.x = 0.0f;
	g_camera.foc.y = 0.0f;
	g_camera.foc.z = 0.0f;
	UpdateCamera(g_camera.pos, g_camera.foc);
	g_material_smooth = LH3DRender::CreateMaterial(LH3DMaterial::LH3D_MATERIAL_RENDER_MODE_0x0, NULL);
}

void __fastcall LH3DTech::UpdateViewPort(long width, long height)
{
	g_info_transform.resolution.x = width;
	g_info_transform.resolution.y = height;
	g_info_transform.HalfRes.x = (float)width * 0.5f;
	g_info_transform.HalfRes.y = (float)height * 0.5f;
	g_info_transform.InvHalfRes.x = 1.0f / g_info_transform.HalfRes.x;
	g_info_transform.InvHalfRes.y = 1.0f / g_info_transform.HalfRes.y;
	g_info_transform.AspectRatioXOverY = (float)width / (float)height;
	ChangeFov(LH3D_TECH_DEFAULT_FOV);
	g_MaxScreen[0] = (float)(width - 1);
	g_MaxScreen[1] = (float)(height - 1);
}

uint32_t __fastcall LH3DTech::ProjectPoint(LHPoint* point, int* x, int* y)
{
	LHMatrix& m = g_world_to_clipping;
	float     z = m._13 * point->x + m._33 * point->z + m._23 * point->y + m._43;
	if (z < g_info_transform.NearClip)
	{
		return LH3D_PROJECT_BEHIND_CAMERA;
	}
	float inv = 1.0f / z;
	float sy =
		(1.0f - (m._12 * point->x + m._32 * point->z + m._22 * point->y + m._42) * inv) * g_info_transform.HalfRes.y;
	*x = (int)(((m._11 * point->x + m._31 * point->z + m._21 * point->y + m._41) * inv + 1.0f) *
	           g_info_transform.HalfRes.x);
	*y = (int)sy;
	if (*x < 0 || *y < 0 || *x >= g_info_transform.resolution.x || *y >= g_info_transform.resolution.y)
	{
		return LH3D_PROJECT_OFF_SCREEN;
	}
	if (*x < 0)
	{
		*x = 0;
	}
	else if (*x > g_MaxScreen[0])
	{
		*x = (int)g_MaxScreen[0];
	}
	if (*y < 0)
	{
		*y = 0;
	}
	else if (*y > g_MaxScreen[1])
	{
		*y = (int)g_MaxScreen[1];
	}
	return LH3D_PROJECT_ON_SCREEN;
}

uint32_t __fastcall LH3DTech::ProjectPoint(LHPoint* point, float* x, float* y)
{
	LHMatrix& m = g_world_to_clipping;
	float     z = m._13 * point->x + m._33 * point->z + m._23 * point->y + m._43;
	if (z < g_info_transform.NearClip)
	{
		return LH3D_PROJECT_BEHIND_CAMERA;
	}
	float inv = 1.0f / z;
	float sy =
		(1.0f - (m._12 * point->x + m._32 * point->z + m._22 * point->y + m._42) * inv) * g_info_transform.HalfRes.y;
	*x = ((m._11 * point->x + m._31 * point->z + m._21 * point->y + m._41) * inv + 1.0f) * g_info_transform.HalfRes.x;
	*y = sy;
	if (*x < 0.0f || *y < 0.0f || *x >= g_info_transform.resolution.x || *y >= g_info_transform.resolution.y)
	{
		return LH3D_PROJECT_OFF_SCREEN;
	}
	if (*x < 0.0f)
	{
		*x = 0.0f;
	}
	else if (*x > g_MaxScreen[0])
	{
		*x = g_MaxScreen[0];
	}
	if (*y < 0.0f)
	{
		*y = 0.0f;
	}
	else if (*y > g_MaxScreen[1])
	{
		*y = g_MaxScreen[1];
	}
	return LH3D_PROJECT_ON_SCREEN;
}

uint32_t __fastcall LH3DTech::ProjectPoint(LHPoint* point, int* x, int* y, float* depth)
{
	LHMatrix& m = g_world_to_clipping;
	float     px = m._11 * point->x + m._31 * point->z + m._21 * point->y + m._41;
	float     py = m._12 * point->x + m._32 * point->z + m._22 * point->y + m._42;
	float     z = m._13 * point->x + m._33 * point->z + m._23 * point->y + m._43;
	*depth = z;
	if (z < g_info_transform.NearClip)
	{
		return LH3D_PROJECT_BEHIND_CAMERA;
	}
	float inv = 1.0f / z;
	float sy = (1.0f - py * inv) * g_info_transform.HalfRes.y;
	*x = (int)((px * inv + 1.0f) * g_info_transform.HalfRes.x);
	*y = (int)sy;
	if (*x < 0 || *y < 0 || *x >= g_info_transform.resolution.x || *y >= g_info_transform.resolution.y)
	{
		return LH3D_PROJECT_OFF_SCREEN;
	}
	return LH3D_PROJECT_ON_SCREEN;
}

uint32_t __fastcall LH3DTech::ProjectPoint(LHPoint* point, float* x, float* y, float* depth)
{
	LHMatrix& m = g_world_to_clipping;
	float     px = m._11 * point->x + m._31 * point->z + m._21 * point->y + m._41;
	float     py = m._12 * point->x + m._32 * point->z + m._22 * point->y + m._42;
	float     z = m._13 * point->x + m._33 * point->z + m._23 * point->y + m._43;
	*depth = z;
	if (z < g_info_transform.NearClip)
	{
		return LH3D_PROJECT_BEHIND_CAMERA;
	}
	float inv = 1.0f / z;
	py = (1.0f - py * inv) * g_info_transform.HalfRes.y;
	*x = (px * inv + 1.0f) * g_info_transform.HalfRes.x;
	*y = py;
	if (*x < 0.0f || *y < 0.0f || *x >= g_info_transform.resolution.x || *y >= g_info_transform.resolution.y)
	{
		return LH3D_PROJECT_OFF_SCREEN;
	}
	return LH3D_PROJECT_ON_SCREEN;
}

void __fastcall LH3DTech::ChangeFov(float fov)
{
	g_camera.fov = fov;
	float tanHalfFov = (float)tan(fov * 0.5f);
	float invTanHalfFov = 1.0f / tanHalfFov;
	g_info_transform.InvHalfTanFovY = invTanHalfFov;
	g_info_transform.InvHalfTanFovX = g_info_transform.AspectRatioXOverY * invTanHalfFov;
	g_info_transform.field_0x28.Set(tanHalfFov, 0.0f, 1.0f);
	g_info_transform.field_0x28.Normalise();
	g_info_transform.field_0x34.Set(0.0f, tanHalfFov / g_info_transform.AspectRatioXOverY, 1.0f);
	g_info_transform.field_0x34.Normalise();
	g_meter_width_screen_on_2 = g_info_transform.NearClip * tanHalfFov;
	g_meter_height_screen_on_2 = g_meter_width_screen_on_2 / g_info_transform.AspectRatioXOverY;
}

void LH3DTech::fn_00819670() {}

void __fastcall LH3DTech::UpdateWorldToCamera(LHMatrix& matrix, LHPoint& position, LHPoint& focus, bool update_look_at)
{
	LHPoint dir = focus - position;
	if (fabs(dir.x) < LH3D_TECH_MIN_DIRECTION && fabs(dir.z) < LH3D_TECH_MIN_DIRECTION)
	{
		dir.x = dir.x > 0.0f ? LH3D_TECH_MIN_DIRECTION : -LH3D_TECH_MIN_DIRECTION;
	}
	dir.Normalise();
	if (update_look_at)
	{
		g_camera.LookAtVector.Set(dir);
	}
	static LHPoint Dat_00EA1B48(0.0f, 1.0f, 0.0f);
	float          d = Dat_00EA1B48.DotProductInline(dir);
	LHPoint        up(Dat_00EA1B48.x - d * dir.x, Dat_00EA1B48.y - d * dir.y, Dat_00EA1B48.z - d * dir.z);
	up.Normalise();
	LHPoint right = up ^ dir;
	matrix._11 = right.x;
	matrix._21 = right.y;
	matrix._31 = right.z;
	matrix._12 = up.x;
	matrix._22 = up.y;
	matrix._32 = up.z;
	matrix._13 = dir.x;
	matrix._23 = dir.y;
	matrix._33 = dir.z;
	matrix._41 = -right.x * position.x - right.y * position.y - right.z * position.z;
	matrix._42 = -up.x * position.x - up.y * position.y - up.z * position.z;
	matrix._43 = -dir.x * position.x - dir.y * position.y - dir.z * position.z;
}

inline void Inline_00819BBB(LHMatrix& out, const LHMatrix& in)
{
	out.m[0] = in.m[0];
	out.m[1] = in.m[1];
	out.m[2] = in.m[2];
	out.m[3] = in.m[3];
	out.m[4] = in.m[4];
	out.m[5] = in.m[5];
	out.m[6] = in.m[6];
	out.m[7] = in.m[7];
	out.m[8] = in.m[8];
}

inline void Inline_00819AF8(LHMatrix& out, const LHMatrix& in, float scale_x, float scale_y)
{
	out._11 = in._11 * scale_x;
	out._21 = in._21 * scale_x;
	out._31 = in._31 * scale_x;
	out._41 = in._41 * scale_x;
	out._12 = in._12 * scale_y;
	out._22 = in._22 * scale_y;
	out._32 = in._32 * scale_y;
	out._42 = in._42 * scale_y;
	out._13 = in._13;
	out._23 = in._23;
	out._33 = in._33;
	out._43 = in._43;
}

void __fastcall LH3DTech::UpdateCamera(const LHPoint& position, const LHPoint& focus)
{
	static LHPoint Dat_00EA1B38(0.0f, 1.0f, 0.0f);
	LHPoint        pos = position;
	LHPoint        foc = focus;
	if (g_b_grosse_merde)
	{
		if (g_b_grosse_merde == LH3D_GROSSE_MERDE_POSITION_AND_FOCUS)
		{
			pos = grosse_merde_pos;
			foc = grosse_merde_foc;
		}
		else if (g_b_grosse_merde == LH3D_GROSSE_MERDE_FOCUS)
		{
			foc = grosse_merde_foc;
		}
	}
	if (merde_focus != NULL)
	{
		foc = merde_focus->matrix.GetPos();
	}
	LH3DCameraChecker::AdjustCameraPosTarget(&pos, &foc);
	g_camera.pos = pos;
	g_camera.foc = foc;
	UpdateWorldToCamera(g_world_to_camera, pos, foc, true);

	Dat_00EA1C98._11 = g_world_to_camera._11;
	Dat_00EA1C98._12 = -g_world_to_camera._13;
	Dat_00EA1C98._13 = g_world_to_camera._12;
	Dat_00EA1C98._21 = g_world_to_camera._21;
	Dat_00EA1C98._22 = -g_world_to_camera._23;
	Dat_00EA1C98._23 = g_world_to_camera._22;
	Dat_00EA1C98._31 = g_world_to_camera._31;
	Dat_00EA1C98._32 = -g_world_to_camera._33;
	Dat_00EA1C98._33 = g_world_to_camera._32;
	Dat_00EA1C98._41 = 0.0f;
	Dat_00EA1C98._42 = 0.0f;
	Dat_00EA1C98._43 = 0.0f;
	Dat_00EA1C98.SetInverse();

	Inline_00819AF8(g_world_to_clipping, g_world_to_camera, g_info_transform.InvHalfTanFovY,
	                g_info_transform.InvHalfTanFovX);
	g_clipping_to_world.SetInverse(g_world_to_clipping);
	Inline_00819BBB(g_copy_of_world_to_clipping, g_world_to_clipping);
	g_copy_of_world_to_clipping.SetTranslateOnly(g_world_to_clipping.GetPos());

	if (g_camera.pos.z < g_camera.foc.z)
	{
		Dat_00EA9EBC = g_camera.pos.x < g_camera.foc.x ? LH3D_VIEW_CONFIGURATION_0 : LH3D_VIEW_CONFIGURATION_1;
	}
	else
	{
		Dat_00EA9EBC = g_camera.pos.x > g_camera.foc.x ? LH3D_VIEW_CONFIGURATION_2 : LH3D_VIEW_CONFIGURATION_3;
	}

	LHPoint normal = LH3DPrimitive::g_clipping_plane_world.Normal;
	LHPoint point =
		LH3DPrimitive::g_clipping_plane_world.Normal *
		-(LH3DPrimitive::g_clipping_plane_world.Distance / LH3DPrimitive::g_clipping_plane_world.Normal.GetNorm());
	g_world_to_camera.TransformVector(normal);
	g_world_to_camera.TransformPoint(point);
	LH3DPrimitive::g_clipping_plane_camera.Normal = normal;
	LH3DPrimitive::g_clipping_plane_camera.Distance = -normal.DotProduct(&point);

	LHPoint cameraPos = g_camera.pos;
	LHPoint cameraFoc = g_camera.foc;
	float   angle = LH3DMath::GetYAngle(&cameraPos, &cameraFoc) + HALF_PI_F;
	Dat_00EA1D88.SetIdentity();
	Dat_00EA1D88._11 = Dat_00EA1D88._33 = cos(angle);
	Dat_00EA1D88._13 = sin(angle);
	Dat_00EA1D88._31 = -Dat_00EA1D88._13;
	g_sun_light_clipping = g_sun_light;
	g_world_to_clipping.TransformPoint(g_sun_light_clipping);
	LH3DRender::SetD3DMatrix(LH3DRender::g_d3d_world2view, g_world_to_camera);
	LH3DRender::Direct3DDevice7->SetTransform(D3DTRANSFORMSTATE_VIEW, &LH3DRender::g_d3d_world2view);
	g_camera_to_world.SetInverse(g_world_to_camera);
}

void __fastcall LH3DTech::UpdateCamera(const LHPoint& position, const LHPoint& focus, LHMatrix& matrix)
{
	static LHPoint Dat_00EA1B28(0.0f, 1.0f, 0.0f);
	LHPoint        pos = position;
	LHPoint        foc = focus;
	if (g_b_grosse_merde)
	{
		if (g_b_grosse_merde == LH3D_GROSSE_MERDE_POSITION_AND_FOCUS)
		{
			pos = grosse_merde_pos;
			foc = grosse_merde_foc;
		}
	}
	if (merde_focus != NULL)
	{
		foc = merde_focus->matrix.GetPos();
	}
	LH3DCameraChecker::AdjustCameraPosTarget(&pos, &foc);
	g_camera.pos = pos;
	g_camera.foc = foc;

	float swap;
	swap = matrix._13;
	matrix._13 = matrix._12;
	matrix._12 = swap;
	swap = matrix._23;
	matrix._23 = matrix._22;
	matrix._22 = swap;
	swap = matrix._33;
	matrix._33 = matrix._32;
	matrix._32 = swap;
	swap = matrix._13;
	matrix._13 = matrix._31;
	matrix._31 = swap;
	swap = matrix._23;
	matrix._23 = matrix._32;
	matrix._32 = swap;
	swap = matrix._12;
	matrix._12 = matrix._21;
	matrix._21 = swap;
	swap = matrix._31;
	matrix._31 = matrix._21;
	matrix._21 = swap;
	swap = matrix._32;
	matrix._32 = matrix._22;
	matrix._22 = swap;
	swap = matrix._33;
	matrix._33 = matrix._23;
	matrix._23 = swap;
	matrix._13 = -matrix._13;
	matrix._23 = -matrix._23;
	matrix._33 = -matrix._33;
	matrix.NormaliseMatrixOnly();

	UpdateWorldToCamera(g_world_to_camera, pos, foc, true);
	Inline_00819BBB(g_world_to_camera, matrix);
	g_world_to_camera._41 =
		-g_world_to_camera._11 * pos.x - g_world_to_camera._21 * pos.y - g_world_to_camera._31 * pos.z;
	g_world_to_camera._42 =
		-g_world_to_camera._12 * pos.x - g_world_to_camera._22 * pos.y - g_world_to_camera._32 * pos.z;
	g_world_to_camera._43 =
		-g_world_to_camera._13 * pos.x - g_world_to_camera._23 * pos.y - g_world_to_camera._33 * pos.z;

	Dat_00EA1C98._11 = g_world_to_camera._11;
	Dat_00EA1C98._12 = -g_world_to_camera._13;
	Dat_00EA1C98._13 = g_world_to_camera._12;
	Dat_00EA1C98._21 = g_world_to_camera._21;
	Dat_00EA1C98._22 = -g_world_to_camera._23;
	Dat_00EA1C98._23 = g_world_to_camera._22;
	Dat_00EA1C98._31 = g_world_to_camera._31;
	Dat_00EA1C98._32 = -g_world_to_camera._33;
	Dat_00EA1C98._33 = g_world_to_camera._32;
	Dat_00EA1C98._41 = 0.0f;
	Dat_00EA1C98._42 = 0.0f;
	Dat_00EA1C98._43 = 0.0f;
	Dat_00EA1C98.SetInverse();

	Inline_00819AF8(g_world_to_clipping, g_world_to_camera, g_info_transform.InvHalfTanFovY,
	                g_info_transform.InvHalfTanFovX);
	g_clipping_to_world.SetInverse(g_world_to_clipping);
	Inline_00819BBB(g_copy_of_world_to_clipping, g_world_to_clipping);
	g_copy_of_world_to_clipping.SetTranslateOnly(g_world_to_clipping.GetPos());

	if (g_camera.pos.z < g_camera.foc.z)
	{
		Dat_00EA9EBC = g_camera.pos.x < g_camera.foc.x ? LH3D_VIEW_CONFIGURATION_0 : LH3D_VIEW_CONFIGURATION_1;
	}
	else
	{
		Dat_00EA9EBC = g_camera.pos.x > g_camera.foc.x ? LH3D_VIEW_CONFIGURATION_2 : LH3D_VIEW_CONFIGURATION_3;
	}

	LHPoint normal = LH3DPrimitive::g_clipping_plane_world.Normal;
	LHPoint point =
		LH3DPrimitive::g_clipping_plane_world.Normal *
		-(LH3DPrimitive::g_clipping_plane_world.Distance / LH3DPrimitive::g_clipping_plane_world.Normal.GetNorm());
	g_world_to_camera.TransformVector(normal);
	g_world_to_camera.TransformPoint(point);
	LH3DPrimitive::g_clipping_plane_camera.Normal = normal;
	LH3DPrimitive::g_clipping_plane_camera.Distance = -normal.DotProduct(&point);

	LHPoint cameraPos = g_camera.pos;
	LHPoint cameraFoc = g_camera.foc;
	float   angle = LH3DMath::GetYAngle(&cameraPos, &cameraFoc) + HALF_PI_F;
	Dat_00EA1D88.SetIdentity();
	Dat_00EA1D88._11 = Dat_00EA1D88._33 = cos(angle);
	Dat_00EA1D88._13 = sin(angle);
	Dat_00EA1D88._31 = -Dat_00EA1D88._13;
	g_sun_light_clipping = g_sun_light;
	g_world_to_clipping.TransformPoint(g_sun_light_clipping);
	LH3DRender::SetD3DMatrix(LH3DRender::g_d3d_world2view, g_world_to_camera);
	LH3DRender::Direct3DDevice7->SetTransform(D3DTRANSFORMSTATE_VIEW, &LH3DRender::g_d3d_world2view);
	g_camera_to_world.SetInverse(g_world_to_camera);
}

inline Vertex3D* LH3DTech::GetPointeurTPoint(long index)
{
	return (Vertex3D*)LH3DP3::Table1[index];
}

inline void LH3DTech::Inline_0081AA7E(uint16_t a, uint16_t b, uint16_t c)
{
	g_TriangleList[g_TriangleListSize++] = a;
	g_TriangleList[g_TriangleListSize++] = b;
	g_TriangleList[g_TriangleListSize++] = c;
}

inline void LH3DTech::Inline_0081A99C(uint16_t a, uint16_t b, uint16_t c)
{
	if (g_NoBackfaceCull == 0)
	{
		if ((GetPointeurTPoint(c)->position.y - GetPointeurTPoint(a)->position.y) *
		            (GetPointeurTPoint(b)->position.x - GetPointeurTPoint(a)->position.x) -
		        (GetPointeurTPoint(b)->position.y - GetPointeurTPoint(a)->position.y) *
		            (GetPointeurTPoint(c)->position.x - GetPointeurTPoint(a)->position.x) <=
		    0.0f)
		{
			return;
		}
	}
	Inline_0081AA7E(a, b, c);
}

void __fastcall LH3DTech::ClipTriangle(uint16_t a, uint16_t b, uint16_t c, ClipCode clip)
{
	if (g_b_P3)
	{
		LH3DP3::ClipTriangle(a, b, c, clip);
		return;
	}
	if (clip == 0)
	{
		Inline_0081A99C(a, b, c);
		return;
	}
	uint16_t first;
	uint16_t second;
	int      flagsA = LH3DP3::Table0[a];
	int      flagsB;
	int      flagsC;
	if (flagsA & clip)
	{
		flagsB = LH3DP3::Table0[b];
		if (flagsB & clip)
		{
			if (LH3DP3::Table0[c] & clip)
			{
				return;
			}
			first = g_NumClipVerts;
			g_NumClipVerts = first + 2;
			second = first + 1;
			Intersect(clip, c, a, first);
			Intersect(clip, c, b, second);
			ClipTriangle(c, first, second, (ClipCode)((unsigned long)clip >> 1));
		}
		else
		{
			first = g_NumClipVerts;
			g_NumClipVerts = first + 2;
			second = first + 1;
			if (LH3DP3::Table0[c] & clip)
			{
				Intersect(clip, b, a, first);
				Intersect(clip, b, c, second);
				ClipTriangle(first, b, second, (ClipCode)((unsigned long)clip >> 1));
			}
			else
			{
				Intersect(clip, b, a, first);
				Intersect(clip, c, a, second);
				ClipTriangle(first, b, c, (ClipCode)((unsigned long)clip >> 1));
				ClipTriangle(first, c, second, (ClipCode)((unsigned long)clip >> 1));
			}
		}
	}
	else
	{
		flagsB = LH3DP3::Table0[b];
		if (flagsB & clip)
		{
			first = g_NumClipVerts;
			g_NumClipVerts = first + 2;
			second = first + 1;
			if (LH3DP3::Table0[c] & clip)
			{
				Intersect(clip, a, b, first);
				Intersect(clip, a, c, second);
				ClipTriangle(a, first, second, (ClipCode)((unsigned long)clip >> 1));
			}
			else
			{
				Intersect(clip, a, b, first);
				Intersect(clip, c, b, second);
				ClipTriangle(a, first, c, (ClipCode)((unsigned long)clip >> 1));
				ClipTriangle(first, second, c, (ClipCode)((unsigned long)clip >> 1));
			}
		}
		else
		{
			flagsC = LH3DP3::Table0[c];
			if (flagsC & clip)
			{
				first = g_NumClipVerts;
				g_NumClipVerts = first + 2;
				second = first + 1;
				Intersect(clip, a, c, first);
				Intersect(clip, b, c, second);
				ClipTriangle(first, a, b, (ClipCode)((unsigned long)clip >> 1));
				ClipTriangle(first, b, second, (ClipCode)((unsigned long)clip >> 1));
			}
			else if (flagsA | flagsB | flagsC)
			{
				ClipTriangle(a, b, c, (ClipCode)((unsigned long)clip >> 1));
			}
			else
			{
				Inline_0081A99C(a, b, c);
			}
		}
	}
}

inline Vertex3DMT* LH3DTech::GetPointeurTPointMT(long index)
{
	return (Vertex3DMT*)LH3DP3::Table1 + index;
}

inline void LH3DTech::Inline_0081ACC8(uint16_t a, uint16_t b, uint16_t c)
{
	if (g_NoBackfaceCull == 0)
	{
		if ((GetPointeurTPointMT(c)->position.y - GetPointeurTPointMT(a)->position.y) *
		            (GetPointeurTPointMT(b)->position.x - GetPointeurTPointMT(a)->position.x) -
		        (GetPointeurTPointMT(b)->position.y - GetPointeurTPointMT(a)->position.y) *
		            (GetPointeurTPointMT(c)->position.x - GetPointeurTPointMT(a)->position.x) <=
		    0.0f)
		{
			return;
		}
	}
	Inline_0081AA7E(a, b, c);
}

void __fastcall LH3DTech::ClipTriangleMT(uint16_t a, uint16_t b, uint16_t c, ClipCode clip)
{
	if (clip == 0)
	{
		Inline_0081ACC8(a, b, c);
		return;
	}
	uint16_t first;
	uint16_t second;
	int      flagsA = LH3DP3::Table0[a];
	int      flagsB;
	int      flagsC;
	if (flagsA & clip)
	{
		flagsB = LH3DP3::Table0[b];
		if (flagsB & clip)
		{
			if (LH3DP3::Table0[c] & clip)
			{
				return;
			}
			first = g_NumClipVerts;
			g_NumClipVerts = first + 2;
			second = first + 1;
			IntersectMT(clip, c, a, first);
			IntersectMT(clip, c, b, second);
			ClipTriangleMT(c, first, second, (ClipCode)((unsigned long)clip >> 1));
		}
		else
		{
			first = g_NumClipVerts;
			g_NumClipVerts = first + 2;
			second = first + 1;
			if (LH3DP3::Table0[c] & clip)
			{
				IntersectMT(clip, b, a, first);
				IntersectMT(clip, b, c, second);
				ClipTriangleMT(first, b, second, (ClipCode)((unsigned long)clip >> 1));
			}
			else
			{
				IntersectMT(clip, b, a, first);
				IntersectMT(clip, c, a, second);
				ClipTriangleMT(first, b, c, (ClipCode)((unsigned long)clip >> 1));
				ClipTriangleMT(first, c, second, (ClipCode)((unsigned long)clip >> 1));
			}
		}
	}
	else
	{
		flagsB = LH3DP3::Table0[b];
		if (flagsB & clip)
		{
			first = g_NumClipVerts;
			g_NumClipVerts = first + 2;
			second = first + 1;
			if (LH3DP3::Table0[c] & clip)
			{
				IntersectMT(clip, a, b, first);
				IntersectMT(clip, a, c, second);
				ClipTriangleMT(a, first, second, (ClipCode)((unsigned long)clip >> 1));
			}
			else
			{
				IntersectMT(clip, a, b, first);
				IntersectMT(clip, c, b, second);
				ClipTriangleMT(a, first, c, (ClipCode)((unsigned long)clip >> 1));
				ClipTriangleMT(first, second, c, (ClipCode)((unsigned long)clip >> 1));
			}
		}
		else
		{
			flagsC = LH3DP3::Table0[c];
			if (flagsC & clip)
			{
				first = g_NumClipVerts;
				g_NumClipVerts = first + 2;
				second = first + 1;
				IntersectMT(clip, a, c, first);
				IntersectMT(clip, b, c, second);
				ClipTriangleMT(first, a, b, (ClipCode)((unsigned long)clip >> 1));
				ClipTriangleMT(first, b, second, (ClipCode)((unsigned long)clip >> 1));
			}
			else if (flagsA | flagsB | flagsC)
			{
				ClipTriangleMT(a, b, c, (ClipCode)((unsigned long)clip >> 1));
			}
			else
			{
				Inline_0081ACC8(a, b, c);
			}
		}
	}
}

static inline int Inline_0081AFF2(float value)
{
	int  result;
	int* resultPtr = &result;
	__asm
	{
		fld value
		mov edx, resultPtr
		fistp dword ptr [edx]
	}
	return result;
}

static inline D3DCOLOR Inline_0081AF97(D3DCOLOR inside, D3DCOLOR outside, float inside_weight, float outside_weight)
{
	return RGBA_MAKE(Inline_0081AFF2(RGBA_GETRED(inside) * inside_weight + RGBA_GETRED(outside) * outside_weight),
	                 Inline_0081AFF2(RGBA_GETGREEN(inside) * inside_weight + RGBA_GETGREEN(outside) * outside_weight),
	                 Inline_0081AFF2(RGBA_GETBLUE(inside) * inside_weight + RGBA_GETBLUE(outside) * outside_weight),
	                 Inline_0081AFF2(RGBA_GETALPHA(inside) * inside_weight + RGBA_GETALPHA(outside) * outside_weight));
}

void __fastcall LH3DTech::IntersectMT(ClipCode clip, uint16_t inside, uint16_t outside, uint16_t result)
{
	Vertex3DMT* pointOutside = GetPointeurTPointMT(outside);
	Vertex3DMT* pointResult = GetPointeurTPointMT(result);
	Vertex3DMT  unprojected;
	Vertex3DMT* pointInside;
	if ((LH3DP3::Table0[inside] & CLIP_CODE_ALL) == 0)
	{
		pointInside = GetPointeurTPointMT(inside);
		unprojected.rhw = g_info_transform.NearClip / pointInside->rhw;
		unprojected.position.x =
			(pointInside->position.x - g_info_transform.HalfRes.x) * unprojected.rhw * g_info_transform.InvHalfRes.x;
		unprojected.position.y =
			(g_info_transform.HalfRes.y - pointInside->position.y) * unprojected.rhw * g_info_transform.InvHalfRes.y;
		unprojected.color = pointInside->color;
		unprojected.specular = pointInside->specular;
		unprojected.uv = pointInside->uv;
		unprojected.uv2 = pointInside->uv2;
		pointInside = &unprojected;
	}
	else
	{
		pointInside = GetPointeurTPointMT(inside);
	}
	float distanceInside;
	float distanceOutside;
	switch (clip)
	{
	case CLIP_CODE_NEAR:
		distanceInside = pointInside->rhw - g_info_transform.NearClip;
		distanceOutside = pointOutside->rhw - g_info_transform.NearClip;
		break;
	case CLIP_CODE_LEFT:
		distanceInside = pointInside->rhw + pointInside->position.x;
		distanceOutside = pointOutside->rhw + pointOutside->position.x;
		break;
	case CLIP_CODE_RIGHT:
		distanceInside = pointInside->rhw - pointInside->position.x;
		distanceOutside = pointOutside->rhw - pointOutside->position.x;
		break;
	case CLIP_CODE_TOP:
		distanceInside = pointInside->rhw - pointInside->position.y;
		distanceOutside = pointOutside->rhw - pointOutside->position.y;
		break;
	case CLIP_CODE_BOTTOM:
		distanceInside = pointInside->rhw + pointInside->position.y;
		distanceOutside = pointOutside->rhw + pointOutside->position.y;
		break;
	}
	float t = distanceInside / (distanceInside - distanceOutside);
	float u = 1.0f - t;
	pointResult->position.x = pointOutside->position.x * t + pointInside->position.x * u;
	pointResult->position.y = pointInside->position.y * u + pointOutside->position.y * t;
	pointResult->rhw = pointInside->rhw * u + pointOutside->rhw * t;
	pointResult->uv.x = pointInside->uv.x * u + pointOutside->uv.x * t;
	pointResult->uv.y = pointInside->uv.y * u + pointOutside->uv.y * t;
	pointResult->uv2.x = pointInside->uv2.x * u + pointOutside->uv2.x * t;
	pointResult->uv2.y = pointInside->uv2.y * u + pointOutside->uv2.y * t;
	pointResult->color = Inline_0081AF97(pointInside->color.GetColor(), pointOutside->color.GetColor(), u, t);
	pointResult->specular = Inline_0081AF97(pointInside->specular.GetColor(), pointOutside->specular.GetColor(), u, t);
	unsigned long flags = 0;
	switch (clip)
	{
	case CLIP_CODE_NEAR:
		if (pointResult->position.x > pointResult->rhw)
		{
			flags |= CLIP_CODE_RIGHT;
		}
	case CLIP_CODE_RIGHT:
		if (pointResult->position.x < -pointResult->rhw)
		{
			flags |= CLIP_CODE_LEFT;
		}
	case CLIP_CODE_LEFT:
		if (pointResult->position.y > pointResult->rhw)
		{
			flags |= CLIP_CODE_TOP;
		}
	case CLIP_CODE_TOP:
		if (pointResult->position.y < -pointResult->rhw)
		{
			flags |= CLIP_CODE_BOTTOM;
		}
	}
	LH3DP3::Table0[result] = flags;
	if (flags == 0)
	{
		float rhw = 1.0f / pointResult->rhw;
		pointResult->rhw = rhw;
		pointResult->position.x = (rhw * pointResult->position.x + 1.0f) * g_info_transform.HalfRes.x;
		if (pointResult->position.x < 0.0f)
		{
			pointResult->position.x = 0.0f;
		}
		else if (pointResult->position.x > g_MaxScreen[0])
		{
			pointResult->position.x = g_MaxScreen[0];
		}
		pointResult->position.y =
			g_info_transform.HalfRes.y - rhw * pointResult->position.y * g_info_transform.HalfRes.y;
		if (pointResult->position.y < 0.0f)
		{
			pointResult->position.y = 0.0f;
		}
		else if (pointResult->position.y > g_MaxScreen[1])
		{
			pointResult->position.y = g_MaxScreen[1];
		}
		pointResult->rhw = rhw * g_info_transform.NearClip;
		pointResult->position.z = 1.0f - pointResult->rhw;
	}
}

void __fastcall LH3DTech::Get3DPointFromScreen(const LHCoord& screen, LHPoint& point, float distance)
{
	float x = (screen.x - g_info_transform.HalfRes.x) * g_meter_width_screen_on_2 / g_info_transform.HalfRes.x;
	float y = (g_info_transform.HalfRes.y - screen.y) * g_meter_height_screen_on_2 / g_info_transform.HalfRes.y;
	float z = g_info_transform.NearClip;
	if (distance != 0.0f)
	{
		float ratio = distance / z;
		x *= ratio;
		y *= ratio;
		z = distance;
	}
	point.x = x * g_world_to_camera._11 + y * g_world_to_camera._12 + z * g_world_to_camera._13;
	point.y = x * g_world_to_camera._21 + y * g_world_to_camera._22 + z * g_world_to_camera._23;
	point.z = x * g_world_to_camera._31 + y * g_world_to_camera._32 + z * g_world_to_camera._33;
	point.x += g_camera.pos.x;
	point.y += g_camera.pos.y;
	point.z += g_camera.pos.z;
}

void __fastcall LH3DTech::TransformProjectFlagClipping(Vertex3D* out, unsigned long* clip_flags, LHPoint* points,
                                                       long count)
{
	for (; count > 0; count--)
	{
		out->position.x = points->x * g_current_matrix->_11 + points->y * g_current_matrix->_21 +
		                  points->z * g_current_matrix->_31 + g_current_matrix->_41;
		out->position.y = points->x * g_current_matrix->_12 + points->y * g_current_matrix->_22 +
		                  points->z * g_current_matrix->_32 + g_current_matrix->_42;
		out->rhw = points->x * g_current_matrix->_13 + points->y * g_current_matrix->_23 +
		           points->z * g_current_matrix->_33 + g_current_matrix->_43;
		if (out->rhw < g_info_transform.NearClip)
		{
			*clip_flags = CLIP_CODE_NEAR;
		}
		else
		{
			*clip_flags = 0;
		}
		if (out->position.x > out->rhw)
		{
			*clip_flags |= CLIP_CODE_RIGHT;
		}
		else if (out->position.x < -out->rhw)
		{
			*clip_flags |= CLIP_CODE_LEFT;
		}
		if (out->position.y > out->rhw)
		{
			*clip_flags |= CLIP_CODE_TOP;
		}
		else if (out->position.y < -out->rhw)
		{
			*clip_flags |= CLIP_CODE_BOTTOM;
		}
		if (*clip_flags == 0)
		{
			out->rhw = 1.0f / out->rhw;
			out->position.x = (out->rhw * out->position.x + 1.0f) * g_info_transform.HalfRes.x;
			if (out->position.x < 0.0f)
			{
				out->position.x = 0.0f;
			}
			else if (out->position.x > g_MaxScreen[0])
			{
				out->position.x = g_MaxScreen[0];
			}
			out->position.y = g_info_transform.HalfRes.y - out->rhw * out->position.y * g_info_transform.HalfRes.y;
			if (out->position.y < 0.0f)
			{
				out->position.y = 0.0f;
			}
			else if (out->position.y > g_MaxScreen[1])
			{
				out->position.y = g_MaxScreen[1];
			}
			out->rhw = g_info_transform.NearClip * out->rhw;
			out->position.z = 1.0f - out->rhw;
		}
		clip_flags++;
		out++;
		points++;
	}
}

void __fastcall LH3DTech::TransformProject(Vertex3D* out, LHPoint* points, long count)
{
	while (count--)
	{
		float x = points->x;
		float y = points->y;
		float z = points->z;
		out->rhw = 1.0f / (x * g_current_matrix->_13 + y * g_current_matrix->_23 + z * g_current_matrix->_33 +
		                   g_current_matrix->_43);
		out->position.x = ((x * g_current_matrix->_11 + y * g_current_matrix->_21 + z * g_current_matrix->_31 +
		                    g_current_matrix->_41) *
		                       out->rhw +
		                   1.0f) *
		                  g_info_transform.HalfRes.x;
		out->position.y = (1.0f - (x * g_current_matrix->_12 + y * g_current_matrix->_22 + z * g_current_matrix->_32 +
		                           g_current_matrix->_42) *
		                              out->rhw) *
		                  g_info_transform.HalfRes.y;
		out++;
		points++;
	}
}

void __fastcall LH3DTech::ClipAndDrawLine(uint16_t a, uint16_t b, ClipCode clip)
{
	while (clip)
	{
		int flagsA = LH3DP3::Table0[a];
		if (flagsA & clip)
		{
			if (LH3DP3::Table0[b] & clip)
			{
				return;
			}
			uint16_t point = g_NumClipVerts++;
			Intersect(clip, b, a, point);
			a = point;
		}
		else
		{
			int flagsB = LH3DP3::Table0[b];
			if (flagsB & clip)
			{
				uint16_t point = g_NumClipVerts++;
				Intersect(clip, a, b, point);
				b = a;
				a = point;
			}
			else if ((flagsA | flagsB) == 0)
			{
				break;
			}
		}
		clip = (ClipCode)((unsigned long)clip >> 1);
	}
	Vertex3D line[2];
	memcpy(&line[0], GetPointeurTPoint(a), sizeof(Vertex3D));
	memcpy(&line[1], GetPointeurTPoint(b), sizeof(Vertex3D));
	LH3DRender::Direct3DDevice7->DrawPrimitive(D3DPT_LINELIST, D3DFVF_TLVERTEX, line, 2, 0);
}

static void fn_0081B7A0(uint16_t a, uint16_t b, LH3DTech::ClipCode clip)
{
	while (clip)
	{
		int flagsA = LH3DP3::Table0[a];
		if (flagsA & clip)
		{
			if (LH3DP3::Table0[b] & clip)
			{
				return;
			}
			uint16_t point = g_NumClipVerts++;
			LH3DTech::Intersect(clip, b, a, point);
			a = point;
		}
		else
		{
			int flagsB = LH3DP3::Table0[b];
			if (flagsB & clip)
			{
				uint16_t point = g_NumClipVerts++;
				LH3DTech::Intersect(clip, a, b, point);
				b = a;
				a = point;
			}
			else if ((flagsA | flagsB) == 0)
			{
				break;
			}
		}
		clip = (LH3DTech::ClipCode)((unsigned long)clip >> 1);
	}
	g_TriangleList[g_TriangleListSize++] = a;
	g_TriangleList[g_TriangleListSize++] = b;
}

void __fastcall LH3DTech::ClipLine(uint16_t a, uint16_t b)
{
	int flagsA = LH3DP3::Table0[a];
	int flagsB = LH3DP3::Table0[b];
	if (flagsA & flagsB)
	{
		return;
	}
	if ((flagsA | flagsB) == 0)
	{
		g_TriangleList[g_TriangleListSize++] = a;
		g_TriangleList[g_TriangleListSize++] = b;
		return;
	}
	uint16_t originalA = a;
	uint16_t originalB = b;
	uint16_t next = g_NumClipVerts;
	for (unsigned long clip = CLIP_CODE_NEAR; clip; clip >>= 1)
	{
		if ((flagsA ^ flagsB) & clip)
		{
			uint16_t point = next++;
			if (flagsA & clip)
			{
				Intersect((ClipCode)clip, b, a, point);
				a = point;
				flagsA = LH3DP3::Table0[point];
			}
			else
			{
				Intersect((ClipCode)clip, a, b, point);
				b = point;
				flagsB = LH3DP3::Table0[point];
			}
		}
	}
	if (a != originalA)
	{
		memcpy(GetPointeurTPoint(originalA), GetPointeurTPoint(a), sizeof(Vertex3D));
	}
	if (b != originalB)
	{
		memcpy(GetPointeurTPoint(originalB), GetPointeurTPoint(b), sizeof(Vertex3D));
	}
	g_TriangleList[g_TriangleListSize++] = originalA;
	g_TriangleList[g_TriangleListSize++] = originalB;
}

void __fastcall LH3DTech::ClipAndDrawLines(int count, LHPoint* points, unsigned long* colors, float* uvs,
                                           LH3DMaterial* material)
{
	float maxX = g_MaxScreen[0];
	float maxY = g_MaxScreen[1];
	g_MaxScreen[0] -= 1.0f;
	g_MaxScreen[1] -= 1.0f;
	long           numPoints = count * 2;
	Vertex3D*      vertices = (Vertex3D*)LH3DP3::Table1;
	unsigned long* flags = (unsigned long*)LH3DP3::Table0;
	TransformProjectFlagClipping(vertices, flags, points, numPoints);
	g_TriangleListSize = 0;
	g_NumClipVerts = count * 2;
	for (int i = 0; i < numPoints; i += 2)
	{
		*(unsigned long*)&vertices[0].color = colors[0];
		*(unsigned long*)&vertices[1].color = colors[1];
		vertices[0].uv.x = uvs[0];
		vertices[0].uv.y = uvs[1];
		vertices[1].uv.x = uvs[2];
		vertices[1].uv.y = uvs[3];
		if (flags[0] | flags[1])
		{
			if ((flags[0] & flags[1]) == 0)
			{
				fn_0081B7A0(i, i + 1, CLIP_CODE_NEAR);
			}
		}
		else
		{
			g_TriangleList[g_TriangleListSize++] = i;
			g_TriangleList[g_TriangleListSize++] = i + 1;
		}
		colors += 2;
		flags += 2;
		vertices += 2;
		uvs += 4;
	}
	if (g_TriangleListSize)
	{
		if (material)
		{
			LH3DRender::Inline_0082EF67(material);
		}
		else
		{
			LH3DRender::Inline_0082EF67(g_material_smooth);
		}
		LH3DRender::Direct3DDevice7->DrawIndexedPrimitive(D3DPT_LINELIST, D3DFVF_TLVERTEX, LH3DP3::Table1,
		                                                  g_NumClipVerts, g_TriangleList, g_TriangleListSize, 0);
	}
	g_MaxScreen[0] = maxX;
	g_MaxScreen[1] = maxY;
}

void __cdecl Report3D__FPCce(const char* fmt, ...)
{
	char    buffer[REPORT_3D_BUFFER_SIZE];
	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	OutputDebugStringA(buffer);
}

void __fastcall LH3DTech::Draw3DWorldTriangle_(long num_points, LHPoint* positions, LH3DColor* colors,
                                               long num_triangles, long* indices, int use_world_to_clipping,
                                               LH3DMaterial* material)
{
	LHMatrix* matrix = use_world_to_clipping ? &g_world_to_clipping : &g_local_to_clipping;
	g_current_matrix = matrix;
	uint16_t       count = num_points;
	unsigned long* flags = (unsigned long*)LH3DP3::Table0;
	Vertex3D*      vertex = (Vertex3D*)LH3DP3::Table1;
	for (long i = 0; i < count; i++)
	{
		vertex->position.x =
			positions->x * matrix->_11 + positions->y * matrix->_21 + positions->z * matrix->_31 + matrix->_41;
		vertex->position.y =
			positions->x * matrix->_12 + positions->y * matrix->_22 + positions->z * matrix->_32 + matrix->_42;
		vertex->rhw =
			positions->x * matrix->_13 + positions->y * matrix->_23 + positions->z * matrix->_33 + matrix->_43;
		if (vertex->rhw < g_info_transform.NearClip)
		{
			*flags = CLIP_CODE_NEAR;
		}
		else
		{
			*flags = 0;
		}
		if (vertex->position.x > vertex->rhw)
		{
			*flags |= CLIP_CODE_RIGHT;
		}
		else if (vertex->position.x < -vertex->rhw)
		{
			*flags |= CLIP_CODE_LEFT;
		}
		if (vertex->position.y > vertex->rhw)
		{
			*flags |= CLIP_CODE_TOP;
		}
		else if (vertex->position.y < -vertex->rhw)
		{
			*flags |= CLIP_CODE_BOTTOM;
		}
		if (*flags == 0)
		{
			vertex->rhw = 1.0f / vertex->rhw;
			vertex->position.x = (vertex->rhw * vertex->position.x + 1.0f) * g_info_transform.HalfRes.x;
			if (vertex->position.x < 0.0f)
			{
				vertex->position.x = 0.0f;
			}
			else if (vertex->position.x > g_MaxScreen[0])
			{
				vertex->position.x = g_MaxScreen[0];
			}
			vertex->position.y =
				g_info_transform.HalfRes.y - vertex->rhw * vertex->position.y * g_info_transform.HalfRes.y;
			if (vertex->position.y < 0.0f)
			{
				vertex->position.y = 0.0f;
			}
			else if (vertex->position.y > g_MaxScreen[1])
			{
				vertex->position.y = g_MaxScreen[1];
			}
			vertex->rhw = g_info_transform.NearClip * vertex->rhw;
			vertex->position.z = 1.0f - vertex->rhw;
		}
		vertex->color = *colors;
		*(unsigned long*)&vertex->specular = 0;
		flags++;
		vertex++;
		colors++;
		positions++;
	}
	g_TriangleListSize = 0;
	g_NumClipVerts = num_points;
	g_NoBackfaceCull = material ? material->IsDoubleSided() != 0 : false;
	for (; num_triangles > 0; num_triangles--)
	{
		uint16_t a = *indices++;
		uint16_t b = *indices++;
		uint16_t c = *indices++;
		int      flagsA = LH3DP3::Table0[a];
		int      flagsB = LH3DP3::Table0[b];
		int      flagsC = LH3DP3::Table0[c];
		if (flagsA | flagsB | flagsC)
		{
			if ((flagsA & flagsB & flagsC) == 0)
			{
				ClipTriangle(a, b, c, CLIP_CODE_NEAR);
			}
		}
		else
		{
			if ((GetPointeurTPoint(c)->position.y - GetPointeurTPoint(a)->position.y) *
			            (GetPointeurTPoint(b)->position.x - GetPointeurTPoint(a)->position.x) -
			        (GetPointeurTPoint(b)->position.y - GetPointeurTPoint(a)->position.y) *
			            (GetPointeurTPoint(c)->position.x - GetPointeurTPoint(a)->position.x) >
			    0.0f)
			{
				Inline_0081AA7E(a, b, c);
			}
		}
	}
	if (g_TriangleListSize)
	{
		if (material)
		{
			LH3DRender::Inline_0082EF67(material);
		}
		else
		{
			LH3DRender::Inline_0082EF67(g_material_smooth);
		}
		LH3DRender::DrawTriangle((Vertex3D*)LH3DP3::Table1, g_NumClipVerts, g_TriangleList, g_TriangleListSize);
	}
}

void __fastcall LH3DTech::Draw3DWorldTriangle(long num_points, LHPoint* positions, LH3DColor* colors, float* uvs,
                                              long num_triangles, long* indices, LH3DMaterial* material,
                                              int use_current_matrix)
{
	if (!use_current_matrix)
	{
		g_local_to_clipping = g_world_to_clipping;
		g_current_matrix = &g_local_to_clipping;
	}
	LHMatrix*      matrix = g_current_matrix;
	uint16_t       count = num_points;
	unsigned long* flags = (unsigned long*)LH3DP3::Table0;
	Vertex3D*      vertex = (Vertex3D*)LH3DP3::Table1;
	for (long i = 0; i < count; i++)
	{
		vertex->position.x =
			positions->x * matrix->_11 + positions->y * matrix->_21 + positions->z * matrix->_31 + matrix->_41;
		vertex->position.y =
			positions->x * matrix->_12 + positions->y * matrix->_22 + positions->z * matrix->_32 + matrix->_42;
		vertex->rhw =
			positions->x * matrix->_13 + positions->y * matrix->_23 + positions->z * matrix->_33 + matrix->_43;
		if (vertex->rhw < g_info_transform.NearClip)
		{
			*flags = CLIP_CODE_NEAR;
		}
		else
		{
			*flags = 0;
		}
		if (vertex->position.x > vertex->rhw)
		{
			*flags |= CLIP_CODE_RIGHT;
		}
		else if (vertex->position.x < -vertex->rhw)
		{
			*flags |= CLIP_CODE_LEFT;
		}
		if (vertex->position.y > vertex->rhw)
		{
			*flags |= CLIP_CODE_TOP;
		}
		else if (vertex->position.y < -vertex->rhw)
		{
			*flags |= CLIP_CODE_BOTTOM;
		}
		if (*flags == 0)
		{
			vertex->rhw = 1.0f / vertex->rhw;
			vertex->position.x = (vertex->rhw * vertex->position.x + 1.0f) * g_info_transform.HalfRes.x;
			if (vertex->position.x < 0.0f)
			{
				vertex->position.x = 0.0f;
			}
			else if (vertex->position.x > g_MaxScreen[0])
			{
				vertex->position.x = g_MaxScreen[0];
			}
			vertex->position.y =
				g_info_transform.HalfRes.y - vertex->rhw * vertex->position.y * g_info_transform.HalfRes.y;
			if (vertex->position.y < 0.0f)
			{
				vertex->position.y = 0.0f;
			}
			else if (vertex->position.y > g_MaxScreen[1])
			{
				vertex->position.y = g_MaxScreen[1];
			}
			vertex->rhw = g_info_transform.NearClip * vertex->rhw;
			vertex->position.z = 1.0f - vertex->rhw;
		}
		vertex->color = *colors;
		*(unsigned long*)&vertex->specular = 0;
		vertex->uv.x = uvs[0];
		vertex->uv.y = uvs[1];
		flags++;
		vertex++;
		colors++;
		positions++;
		uvs += 2;
	}
	g_TriangleListSize = 0;
	g_NumClipVerts = num_points;
	g_NoBackfaceCull = material ? material->IsDoubleSided() != 0 : true;
	if (g_NoBackfaceCull)
	{
		for (; num_triangles > 0; num_triangles--)
		{
			uint16_t a = *indices++;
			uint16_t b = *indices++;
			uint16_t c = *indices++;
			int      flagsA = LH3DP3::Table0[a];
			int      flagsB = LH3DP3::Table0[b];
			int      flagsC = LH3DP3::Table0[c];
			if (flagsA | flagsB | flagsC)
			{
				if ((flagsA & flagsB & flagsC) == 0)
				{
					ClipTriangle(a, b, c, CLIP_CODE_NEAR);
				}
			}
			else
			{
				Inline_0081AA7E(a, b, c);
			}
		}
	}
	else
	{
		for (; num_triangles > 0; num_triangles--)
		{
			uint16_t a = *indices++;
			uint16_t b = *indices++;
			uint16_t c = *indices++;
			int      flagsA = LH3DP3::Table0[a];
			int      flagsB = LH3DP3::Table0[b];
			int      flagsC = LH3DP3::Table0[c];
			if (flagsA | flagsB | flagsC)
			{
				if ((flagsA & flagsB & flagsC) == 0)
				{
					ClipTriangle(a, b, c, CLIP_CODE_NEAR);
				}
			}
			else
			{
				if ((GetPointeurTPoint(c)->position.y - GetPointeurTPoint(a)->position.y) *
				            (GetPointeurTPoint(b)->position.x - GetPointeurTPoint(a)->position.x) -
				        (GetPointeurTPoint(b)->position.y - GetPointeurTPoint(a)->position.y) *
				            (GetPointeurTPoint(c)->position.x - GetPointeurTPoint(a)->position.x) >
				    0.0f)
				{
					Inline_0081AA7E(a, b, c);
				}
			}
		}
	}
	if (g_TriangleListSize)
	{
		if (material)
		{
			LH3DRender::Inline_0082EF67(material);
		}
		LH3DRender::DrawTriangle((Vertex3D*)LH3DP3::Table1, g_NumClipVerts, g_TriangleList, g_TriangleListSize);
	}
}

void __fastcall LH3DTech::Draw3DScreenTriangle(long num_points, LHPoint* positions, LH3DColor* colors, float* uvs,
                                               long num_indices, long* indices, LH3DMaterial* material, int param_8)
{
	uint16_t  count = num_points;
	Vertex3D* vertex = (Vertex3D*)LH3DP3::Table1;
	for (long i = 0; i < count; i++)
	{
		vertex->position.x = positions->x;
		vertex->position.y = positions->y;
		vertex->rhw = 1.0f / positions->z * g_info_transform.NearClip;
		vertex->position.z = 1.0f - vertex->rhw;
		vertex->color = *colors;
		*(unsigned long*)&vertex->specular = 0;
		vertex->uv.x = uvs[0];
		vertex->uv.y = uvs[1];
		vertex++;
		colors++;
		positions++;
		uvs += 2;
	}
	g_TriangleListSize = 0;
	g_NumClipVerts = num_points;
	g_NoBackfaceCull = true;
	for (; num_indices > 0; num_indices--)
	{
		uint16_t a = *indices++;
		uint16_t b = *indices++;
		uint16_t c = *indices++;
		Inline_0081AA7E(a, b, c);
	}
	if (g_TriangleListSize)
	{
		if (material)
		{
			LH3DRender::Inline_0082EF67(material);
		}
		LH3DRender::SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE);
		DrawAndClip2D(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, (Vertex3D*)LH3DP3::Table1, g_NumClipVerts, g_TriangleList,
		              g_TriangleListSize);
		LH3DRender::SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
	}
}

void __fastcall LH3DTech::Draw3DWorldTriangleSpecular(long num_points, LHPoint* positions, LH3DColor* colors,
                                                      float* uvs, long num_triangles, long* indices,
                                                      LH3DMaterial* material)
{
	g_local_to_clipping = g_world_to_clipping;
	g_current_matrix = &g_local_to_clipping;
	uint16_t       count = num_points;
	unsigned long* flags = (unsigned long*)LH3DP3::Table0;
	Vertex3D*      vertex = (Vertex3D*)LH3DP3::Table1;
	for (long i = 0; i < count; i++)
	{
		vertex->position.x = positions->x * g_world_to_clipping._11 + positions->y * g_world_to_clipping._21 +
		                     positions->z * g_world_to_clipping._31 + g_world_to_clipping._41;
		vertex->position.y = positions->x * g_world_to_clipping._12 + positions->y * g_world_to_clipping._22 +
		                     positions->z * g_world_to_clipping._32 + g_world_to_clipping._42;
		vertex->rhw = positions->x * g_world_to_clipping._13 + positions->y * g_world_to_clipping._23 +
		              positions->z * g_world_to_clipping._33 + g_world_to_clipping._43;
		if (vertex->rhw < g_info_transform.NearClip)
		{
			*flags = CLIP_CODE_NEAR;
		}
		else
		{
			*flags = 0;
		}
		if (vertex->position.x > vertex->rhw)
		{
			*flags |= CLIP_CODE_RIGHT;
		}
		else if (vertex->position.x < -vertex->rhw)
		{
			*flags |= CLIP_CODE_LEFT;
		}
		if (vertex->position.y > vertex->rhw)
		{
			*flags |= CLIP_CODE_TOP;
		}
		else if (vertex->position.y < -vertex->rhw)
		{
			*flags |= CLIP_CODE_BOTTOM;
		}
		if (*flags == 0)
		{
			vertex->rhw = 1.0f / vertex->rhw;
			vertex->position.x = (vertex->rhw * vertex->position.x + 1.0f) * g_info_transform.HalfRes.x;
			if (vertex->position.x < 0.0f)
			{
				vertex->position.x = 0.0f;
			}
			else if (vertex->position.x > g_MaxScreen[0])
			{
				vertex->position.x = g_MaxScreen[0];
			}
			vertex->position.y =
				g_info_transform.HalfRes.y - vertex->rhw * vertex->position.y * g_info_transform.HalfRes.y;
			if (vertex->position.y < 0.0f)
			{
				vertex->position.y = 0.0f;
			}
			else if (vertex->position.y > g_MaxScreen[1])
			{
				vertex->position.y = g_MaxScreen[1];
			}
			vertex->rhw = g_info_transform.NearClip * vertex->rhw;
			vertex->position.z = 1.0f - vertex->rhw;
		}
		vertex->color = colors[0];
		vertex->specular = colors[1];
		vertex->uv.x = uvs[0];
		vertex->uv.y = uvs[1];
		flags++;
		vertex++;
		colors += 2;
		positions++;
		uvs += 2;
	}
	g_TriangleListSize = 0;
	g_NumClipVerts = num_points;
	if (material)
	{
		g_NoBackfaceCull = material->IsDoubleSided() != 0;
	}
	if (material && material->IsDoubleSided())
	{
		for (; num_triangles > 0; num_triangles--)
		{
			uint16_t a = *indices++;
			uint16_t b = *indices++;
			uint16_t c = *indices++;
			int      flagsA = LH3DP3::Table0[a];
			int      flagsB = LH3DP3::Table0[b];
			int      flagsC = LH3DP3::Table0[c];
			if (flagsA | flagsB | flagsC)
			{
				if ((flagsA & flagsB & flagsC) == 0)
				{
					ClipTriangle(a, b, c, CLIP_CODE_NEAR);
				}
			}
			else
			{
				Inline_0081AA7E(a, b, c);
			}
		}
	}
	else
	{
		for (; num_triangles > 0; num_triangles--)
		{
			uint16_t a = *indices++;
			uint16_t b = *indices++;
			uint16_t c = *indices++;
			int      flagsA = LH3DP3::Table0[a];
			int      flagsB = LH3DP3::Table0[b];
			int      flagsC = LH3DP3::Table0[c];
			if (flagsA | flagsB | flagsC)
			{
				if ((flagsA & flagsB & flagsC) == 0)
				{
					ClipTriangle(a, b, c, CLIP_CODE_NEAR);
				}
			}
			else
			{
				if ((GetPointeurTPoint(c)->position.y - GetPointeurTPoint(a)->position.y) *
				            (GetPointeurTPoint(b)->position.x - GetPointeurTPoint(a)->position.x) -
				        (GetPointeurTPoint(b)->position.y - GetPointeurTPoint(a)->position.y) *
				            (GetPointeurTPoint(c)->position.x - GetPointeurTPoint(a)->position.x) >
				    0.0f)
				{
					Inline_0081AA7E(a, b, c);
				}
			}
		}
	}
	if (g_TriangleListSize)
	{
		if (material)
		{
			LH3DRender::Inline_0082EF67(material);
		}
		LH3DRender::DrawTriangle((Vertex3D*)LH3DP3::Table1, g_NumClipVerts, g_TriangleList, g_TriangleListSize);
	}
}

static inline void Inline_0081D0D9(LH3DMaterial* material)
{
	if (LH3DRender::g_b_do_update_material)
	{
		LH3DRender::g_current_other_material = material;
		if (LH3DRender::g_curmode2tex)
		{
			LH3DRender::g_curmat2tex = material;
		}
		if (!LH3DRender::g_use_multipass && LH3DRender::g_curmode2tex)
		{
			((LH3DMaterial::RenderFunc*)LH3DMaterial::g_list_render_func)[material->GetMode()].Func(material, 1);
			if (LH3DRender::g_b_need_tilling || material->IsNeedTilling())
			{
				LH3DRender::SetD3DTillingOn(1);
			}
			else
			{
				LH3DRender::SetD3DTillingOff(1);
			}
		}
	}
}

void __fastcall LH3DTech::Draw3DWorldTriangleMT(long num_points, LHPoint* positions, LH3DColor* colors, float* uvs,
                                                float* uvs2, long num_triangles, long* indices, LH3DMaterial* material,
                                                LH3DMaterial* material2)
{
	g_local_to_clipping = g_world_to_clipping;
	g_current_matrix = &g_local_to_clipping;
	uint16_t       count = num_points;
	unsigned long* flags = (unsigned long*)LH3DP3::Table0;
	Vertex3DMT*    vertex = (Vertex3DMT*)LH3DP3::Table1;
	for (long i = 0; i < count; i++)
	{
		vertex->position.x = positions->x * g_world_to_clipping._11 + positions->y * g_world_to_clipping._21 +
		                     positions->z * g_world_to_clipping._31 + g_world_to_clipping._41;
		vertex->position.y = positions->x * g_world_to_clipping._12 + positions->y * g_world_to_clipping._22 +
		                     positions->z * g_world_to_clipping._32 + g_world_to_clipping._42;
		vertex->rhw = positions->x * g_world_to_clipping._13 + positions->y * g_world_to_clipping._23 +
		              positions->z * g_world_to_clipping._33 + g_world_to_clipping._43;
		if (vertex->rhw < g_info_transform.NearClip)
		{
			*flags = CLIP_CODE_NEAR;
		}
		else
		{
			*flags = 0;
		}
		if (vertex->position.x > vertex->rhw)
		{
			*flags |= CLIP_CODE_RIGHT;
		}
		else if (vertex->position.x < -vertex->rhw)
		{
			*flags |= CLIP_CODE_LEFT;
		}
		if (vertex->position.y > vertex->rhw)
		{
			*flags |= CLIP_CODE_TOP;
		}
		else if (vertex->position.y < -vertex->rhw)
		{
			*flags |= CLIP_CODE_BOTTOM;
		}
		if (*flags == 0)
		{
			vertex->rhw = 1.0f / vertex->rhw;
			vertex->position.x = (vertex->rhw * vertex->position.x + 1.0f) * g_info_transform.HalfRes.x;
			if (vertex->position.x < 0.0f)
			{
				vertex->position.x = 0.0f;
			}
			else if (vertex->position.x > g_MaxScreen[0])
			{
				vertex->position.x = g_MaxScreen[0];
			}
			vertex->position.y =
				g_info_transform.HalfRes.y - vertex->rhw * vertex->position.y * g_info_transform.HalfRes.y;
			if (vertex->position.y < 0.0f)
			{
				vertex->position.y = 0.0f;
			}
			else if (vertex->position.y > g_MaxScreen[1])
			{
				vertex->position.y = g_MaxScreen[1];
			}
			vertex->rhw = g_info_transform.NearClip * vertex->rhw;
			vertex->position.z = 1.0f - vertex->rhw;
		}
		vertex->color = *colors;
		*(unsigned long*)&vertex->specular = 0;
		vertex->uv.x = uvs[0];
		vertex->uv.y = uvs[1];
		vertex->uv2.x = uvs2[0];
		vertex->uv2.y = uvs2[1];
		flags++;
		vertex++;
		colors++;
		positions++;
		uvs += 2;
		uvs2 += 2;
	}
	g_NumClipVerts = num_points;
	g_TriangleListSize = 0;
	g_NoBackfaceCull = material->IsDoubleSided() != 0;
	for (; num_triangles > 0; num_triangles--)
	{
		uint16_t a = *indices++;
		uint16_t b = *indices++;
		uint16_t c = *indices++;
		int      flagsA = LH3DP3::Table0[a];
		int      flagsB = LH3DP3::Table0[b];
		int      flagsC = LH3DP3::Table0[c];
		if (flagsA | flagsB | flagsC)
		{
			if ((flagsA & flagsB & flagsC) == 0)
			{
				ClipTriangleMT(a, b, c, CLIP_CODE_NEAR);
			}
		}
		else
		{
			Inline_0081AA7E(a, b, c);
		}
	}
	if (g_TriangleListSize)
	{
		LH3DRender::Inline_0082EF67(material);
		if (material2 == NULL)
		{
			LH3DRender::SetTexture(NULL, 1);
		}
		else
		{
			Inline_0081D0D9(material2);
		}
		LH3DRender::DrawTriangleMT((Vertex3DMT*)LH3DP3::Table1, g_NumClipVerts, g_TriangleList, g_TriangleListSize);
		LH3DRender::SetTexture(NULL, 1);
	}
}

float __stdcall Random(float min, float max)
{
	return min + (max - min) * (rand() * Dat_009A3700);
}
