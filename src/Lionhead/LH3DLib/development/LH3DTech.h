#ifndef BW1_DECOMP_LH3D_TECH_INCLUDED_H
#define BW1_DECOMP_LH3D_TECH_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <math.h>   /* For tan */
#include <stdint.h> /* For uint32_t */

#include "LH3DCamera.h" /* For struct LH3DCamera */
#include "LHCoord.h"    /* For struct LHCoord */
#include "LHPoint.h"    /* For struct Point2D */

// Forward Declares

struct LH3DColor;
struct LH3DMaterial;
class LH3DObject;
struct LHMatrix;
struct LHPoint;
struct LHTimer;
struct Vertex3D;
struct Vertex3DMT;

enum LH3D_PROJECT_RESULT
{
	LH3D_PROJECT_BEHIND_CAMERA = 0,
	LH3D_PROJECT_OFF_SCREEN = 1,
	LH3D_PROJECT_ON_SCREEN = 2,
};

enum
{
	LH3D_GROSSE_MERDE_FOCUS = 1,
	LH3D_GROSSE_MERDE_POSITION_AND_FOCUS = 2,
};

enum
{
	LH3D_VIEW_CONFIGURATION_0 = 0,
	LH3D_VIEW_CONFIGURATION_1 = 1,
	LH3D_VIEW_CONFIGURATION_2 = 2,
	LH3D_VIEW_CONFIGURATION_3 = 3,
};

struct InfoTransform
{
	float          NearClip;
	struct LHCoord resolution;
	float          AspectRatioXOverY;
	struct Point2D HalfRes;
	struct Point2D InvHalfRes;
	float          InvHalfTanFovY;
	float          InvHalfTanFovX;
	LHPoint        field_0x28;
	LHPoint        field_0x34;
};
static_assert(sizeof(InfoTransform) == 0x40, "Data type is of wrong size");

enum
{
	REPORT_3D_BUFFER_SIZE = 0x800,
};

class LH3DTech
{
public:
	enum ClipCode
	{
		CLIP_CODE_BOTTOM = 0x02,
		CLIP_CODE_TOP = 0x04,
		CLIP_CODE_LEFT = 0x08,
		CLIP_CODE_RIGHT = 0x10,
		CLIP_CODE_NEAR = 0x20,
		CLIP_CODE_ALL = 0x3f,
	};

	static InfoTransform g_info_transform; // 00e839e0
	static LH3DCamera    g_camera;         // 00ea1db8
	static int           g_delta_time;     // 00c38134
	// BW1W120 00c3812c BW1M119 011d1504 (LHCombined Release)
	static float g_meter_width_screen_on_2;
	// BW1W120 00c38130 BW1M119 011d1508 (LHCombined Release)
	static float g_meter_height_screen_on_2;
	// BW1W120 00ea1b78 BW1M119 012d3e80 (LHCombined Release)
	static LHTimer g_timer;
	// BW1W120 00ea9e40 BW1M119 012dc144 (LHCombined Release)
	static LHMatrix g_world_to_clipping;
	// BW1W120 00ea9ea0 BW1M119 012dbff0 (LHCombined Release)
	static LHMatrix* g_current_matrix;
	// BW1W120 00ea1d28 BW1M119 012dc0b4 (LHCombined Release)
	static LHMatrix g_world_to_camera;
	// BW1W120 00ea1cf8 BW1M119 012dc084 (LHCombined Release)
	static LHMatrix g_camera_to_world;
	// BW1W120 00ea9de0 BW1M119 012dc0e4 (LHCombined Release)
	static LHMatrix g_clipping_to_world;
	// BW1W120 00ea1d58 BW1M119 012dc114 (LHCombined Release)
	static LHMatrix g_copy_of_world_to_clipping;
	// BW1W120 00ea1c98
	static LHMatrix Dat_00EA1C98;
	// BW1W120 00ea1d88
	static LHMatrix Dat_00EA1D88;
	// BW1W120 00ea9e10
	static LHMatrix Dat_00EA9E10;
	// BW1W120 00ea9e80 BW1M119 012d3f9c (LHCombined Release)
	static LHPoint g_sun_light_clipping;
	// BW1W120 00ea9ebc BW1M119 012d3e6c (LHCombined Release)
	static int Dat_00EA9EBC;
	// BW1W120 00ea9e90 BW1M119 012d3fa8 (LHCombined Release)
	static LHPoint g_sun_light;
	// BW1W120 inlined BW1M119 010e7360
	static float GetValueForZSorter(const LHPoint& point)
	{
		return (point.x - g_camera.pos.x) * (point.x - g_camera.pos.x) +
		       (point.y - g_camera.pos.y) * (point.y - g_camera.pos.y) +
		       (point.z - g_camera.pos.z) * (point.z - g_camera.pos.z);
	}
	// Original Mac symbol: g_ambient_wind_direction__8LH3DTech.
	// BW1W120 00ea9e70 BW1M119 012d3f90 (LHCombined Release)
	static LHPoint g_ambient_wind_direction;
	// Original Mac symbol: g_game_time_inc__8LH3DTech.
	// BW1W120 00ea9ec0 BW1M119 012d3e68 (LHCombined Release)
	static uint32_t g_game_time_inc;
	// BW1W120 00ea9eb4 BW1M119 012d3e74 (LHCombined Release)
	static int g_b_P3;
	// BW1W120 00ea9eac BW1M119 012d3e7c (LHCombined Release)
	static int g_previous_time;
	// BW1W120 00ea9eb0 BW1M119 012d3e78 (LHCombined Release)
	static int g_current_time;
	// BW1W120 00819030 BW1M119 010bfe40 (LHCombined Release)
	static void __fastcall UpdateViewPort(long width, long height);
	// BW1W120 00819390 BW1M119 01037930 (LHCombined Release)
	static uint32_t __fastcall ProjectPoint(LHPoint* point, int* x, int* y, float* depth);
	// BW1W120 008190d0 BW1M119 0100c5c0 (LHCombined Release)
	static uint32_t __fastcall ProjectPoint(LHPoint* point, int* x, int* y);
	// BW1W120 00819230 BW1M119 010bfc40 (LHCombined Release)
	static uint32_t __fastcall ProjectPoint(LHPoint* point, float* x, float* y);
	// BW1W120 00819490 BW1M119 010bf9b0 (LHCombined Release)
	static uint32_t __fastcall ProjectPoint(LHPoint* point, float* x, float* y, float* depth);
	// BW1W120 008195b0 BW1M119 01011be0 (LHCombined Release)
	static void __fastcall ChangeFov(float fov);
	// BW1W120 00819690 BW1M119 01011e40 (LHCombined Release)
	static void __fastcall UpdateWorldToCamera(LHMatrix& matrix, LHPoint& position, LHPoint& focus, bool param_4);
	// BW1W120 inlined BW1M119 01095480
	static uint32_t GetDeltaTime();
	// BW1W120 inlined BW1M119 01038930
	static int GetGameTimeInc() { return g_game_time_inc; }
	// BW1W120 inlined BW1M119 01049620
	static float GetNearClipping() { return g_info_transform.NearClip; }
	// BW1W120 inlined BW1M119 inlined
	static void SetNearClipping(float near_clipping)
	{
		g_info_transform.NearClip = near_clipping;
		SetMeterScreen();
	}
	// BW1W120 inlined BW1M119 01009df0 (LHCombined Release)
	static void SetMeterScreen()
	{
		g_meter_width_screen_on_2 = (float)tan(g_camera.fov * 0.5f) * g_info_transform.NearClip;
		g_meter_height_screen_on_2 = g_meter_width_screen_on_2 / g_info_transform.AspectRatioXOverY;
	}
	// BW1W120 0045a7f0 BW1M119 010318a0
	static LHPoint* GetCameraPosition() { return &g_camera.pos; }
	// BW1W120 inlined BW1M119 01025530
	static LHPoint* GetCameraTarget() { return &g_camera.foc; }
	// BW1W120 inlined BW1M119 01020d00
	static LHPoint GetSun() { return g_sun_light; }
	// BW1W120 0081e1f0 BW1M119 0101c120 (LHCombined Release)
	static LHPoint SetSun(const LHPoint& sun);
	// BW1W120 00819920 BW1M119 01034f90 (LHCombined Release)
	static void __fastcall UpdateCamera(const LHPoint& position, const LHPoint& focus);
	// BW1W120 00819f50 BW1M119 010beec0 (LHCombined Release)
	static void __fastcall UpdateCamera(const LHPoint& position, const LHPoint& focus, LHMatrix& matrix);
	// BW1W120 00819670
	static void fn_00819670();
	// BW1W120 00818c60 BW1M119 010bffa0 (LHCombined Release)
	static void __fastcall RenderInitialization(long width, long height);
	// BW1W120 0081c5c0 BW1M119 010337e0 (LHCombined Release)
	static void __fastcall Draw3DScreenTriangle(long num_points, LHPoint* positions, LH3DColor* colors, float* uvs,
	                                            long num_indices, long* indices, LH3DMaterial* material, int param_8);
	// BW1W120 0081c780 BW1M119 010bccc0 (LHCombined Release)
	static void __fastcall Draw3DWorldTriangleSpecular(long num_points, LHPoint* positions, LH3DColor* colors,
	                                                   float* uvs, long num_triangles, long* indices,
	                                                   LH3DMaterial* material);
	// BW1W120 0081ccd0 BW1M119 010bc780 (LHCombined Release)
	static void __fastcall Draw3DWorldTriangleMT(long num_points, LHPoint* positions, LH3DColor* colors, float* uvs,
	                                             float* uvs2, long num_triangles, long* indices, LH3DMaterial* material,
	                                             LH3DMaterial* material2);
	// BW1W120 0081b6a0 BW1M119 010be410 (LHCombined Release)
	static void __fastcall ClipAndDrawLine(uint16_t a, uint16_t b, ClipCode clip);
	// BW1W120 0081b860 BW1M119 010bdfe0 (LHCombined Release)
	static void __fastcall ClipLine(uint16_t a, uint16_t b);
	// BW1W120 0081b9c0 BW1M119 0102f990 (LHCombined Release)
	static void __fastcall ClipAndDrawLines(int count, LHPoint* points, unsigned long* colors, float* uvs,
	                                        LH3DMaterial* material);
	// BW1W120 0081bc10 BW1M119 010bd280 (LHCombined Release)
	static void __fastcall Draw3DWorldTriangle_(long num_points, LHPoint* positions, LH3DColor* colors,
	                                            long num_triangles, long* indices, int use_world_to_clipping,
	                                            LH3DMaterial* material);
	// BW1W120 0081c090 BW1M119 0102ea40 (LHCombined Release)
	static void __fastcall Draw3DWorldTriangle(long num_points, LHPoint* positions, LH3DColor* colors, float* uvs,
	                                           long num_triangles, long* indices, LH3DMaterial* material,
	                                           int use_current_matrix);
	// BW1W120 inlined BW1M119 0102c1a0 (LHCombined Release)
	static Vertex3D* GetPointeurTPoint(long index);
	// BW1W120 inlined BW1M119 null
	static void Inline_0081AA7E(uint16_t a, uint16_t b, uint16_t c);
	// BW1W120 inlined BW1M119 null
	static void Inline_0081A99C(uint16_t a, uint16_t b, uint16_t c);
	// BW1W120 0081a760 BW1M119 0102c690 (LHCombined Release)
	static void __fastcall ClipTriangle(uint16_t a, uint16_t b, uint16_t c, ClipCode clip);
	// BW1W120 inlined BW1M119 010bed00 (LHCombined Release)
	static Vertex3DMT* GetPointeurTPointMT(long index);
	// BW1W120 inlined BW1M119 null
	static void Inline_0081ACC8(uint16_t a, uint16_t b, uint16_t c);
	// BW1W120 0081aab0 BW1M119 010bed50 (LHCombined Release)
	static void __fastcall ClipTriangleMT(uint16_t a, uint16_t b, uint16_t c, ClipCode clip);
	// BW1W120 0081adf0 BW1M119 010be860 (LHCombined Release)
	static void __fastcall IntersectMT(ClipCode clip, uint16_t inside, uint16_t outside, uint16_t result);
	// BW1W120 0081dd90 BW1M119 0102bd40 (LHCombined Release)
	static void __fastcall Intersect(ClipCode clip, uint16_t inside, uint16_t outside, uint16_t result);
	// BW1W120 0081b370 BW1M119 0101b840 (LHCombined Release)
	static void __fastcall Get3DPointFromScreen(const LHCoord& screen, LHPoint& point, float distance);
	// BW1W120 0081b450 BW1M119 01037ba0 (LHCombined Release)
	static void __fastcall TransformProjectFlagClipping(Vertex3D* out, unsigned long* clip_flags, LHPoint* points,
	                                                    long count);
	// BW1W120 0081b5f0 BW1M119 010be620 (LHCombined Release)
	static void __fastcall TransformProject(Vertex3D* out, LHPoint* points, long count);
};

// BW1W120 00ea9e9c BW1M119 012dc174 (LHCombined Release)
extern int g_b_any_voodoo;
// BW1W120 00ea9ecc BW1M119 012d3e44 (LHCombined Release)
extern LH3DObject* merde_focus;
// BW1W120 00ea1b58 BW1M119 012d3e48 (LHCombined Release)
extern LHPoint grosse_merde_pos;
// BW1W120 00ea1b68 BW1M119 012d3e54 (LHCombined Release)
extern LHPoint grosse_merde_foc;
// BW1W120 00ea9ec8 BW1M119 012d3e60 (LHCombined Release)
extern int g_b_grosse_merde;
// BW1W120 00ea9ec4 BW1M119 012d3e64 (LHCombined Release)
extern LH3DMaterial* g_material_smooth;

// BW1W120 0081bbd0 BW1M119 010bd5b0 (LHCombined Release)
void __cdecl Report3D__FPCce(const char* fmt, ...);
// BW1W120 0081f1a0 BW1M119 01013fc0 (LHCombined Release)
int IsObjectOnScreen(LH3DObject* object);
// BW1W120 0081f1d0 BW1M119 01005310 (LHCombined Release)
int IsPointOnScreen(LHPoint* point);
// BW1W120 0081d180 BW1M119 0101f540 (LHCombined Release)
float __stdcall Random(float min, float max);

#endif /* BW1_DECOMP_LH3D_TECH_INCLUDED_H */
