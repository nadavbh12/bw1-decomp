#ifndef BW1_DECOMP_CAMERA_HELP_INCLUDED_H
#define BW1_DECOMP_CAMERA_HELP_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stdint.h> /* For uint32_t */

#include <string.h> /* For strcpy */

#include <Lionhead/LH3DLib/development/LH3DColor.h> /* For struct LH3DColor */
#include <Lionhead/LH3DLib/development/LHPoint.h>   /* For struct LHPoint */
#include <Lionhead/LHLib/ver5.0/LHWin.h>            /* For operator new(size_t, const char*, uint32_t) */

#include "CameraHelpTypes.h" /* For enum CH_ANIMTYPE, enum KEYALIGN */

#define CAMERA_HELP_H_FILEPATH "C:\\dev\\MP\\Black\\camerahelp.h"

enum CameraHelpReason
{
	CameraHelpReason_DoubleClickInExcludedArea = 0x100,
	CameraHelpReason_DoubleClickInIncludedArea = 0x101,
	CameraHelpReason_HandTooFarAway = 0x102,
	CameraHelpReason_CollideWithGround = 0x200,
	CameraHelpReason_CollideWithExcludedArea = 0x201,
	CameraHelpReason_CollideWithEdgeOfAllowedArea = 0x202,
	CameraHelpReason_CollideWithCeiling = 0x203,
	CameraHelpReason_Rotate = 0x300,
	CameraHelpReason_RotatePositive = 0x301,
	CameraHelpReason_RotateNegative = 0x302,
	CameraHelpReason_Pitching = 0x303,
	CameraHelpReason_Zooming = 0x304,
	CameraHelpReason_DoubleClickOnPos = 0x305,
	CameraHelpReason_DoubleClickOnObject = 0x306,
	CameraHelpReason_ZoomToCitadel = 0x307,
	CameraHelpReason_Dragging = 0x308,
	_CameraHelpReason_COUNT = 0x309
};
static_assert(sizeof(enum CameraHelpReason) == 0x4, "Data type is of wrong size");

enum
{
	CAMERA_HELP_REASON_GROUP_MASK = 0xf00,
	CAMERA_HELP_REASON_INDEX_MASK = 0xff,
	CAMERA_HELP_REASON_GROUP_ERROR = 0x100,
	CAMERA_HELP_REASON_GROUP_COLLIDE = 0x200,
	CAMERA_HELP_REASON_GROUP_ACTION = 0x300,
};

enum CAMERA_FEATURE
{
	CAMERA_FEATURE_PITCH = 0x1,
	CAMERA_FEATURE_ROTATE = 0x2,
	CAMERA_FEATURE_ZOOM = 0x4,
	CAMERA_FEATURE_MOVE = 0x8,
	CAMERA_FEATURE_DOUBLE_CLICK = 0x10,
	CAMERA_FEATURE_ZOOM_LANDSCAPE = 0x20,
	CAMERA_FEATURE_AUTO_PITCH = 0x40,
	CAMERA_FEATURE_HELP = 0x80,
	CAMERA_FEATURE_EDGE_SCROLL = 0x100,
};

enum
{
	CAMERA_HELP_ERROR_COUNT = 3,
	CAMERA_HELP_COLLIDE_COUNT = 4,
	CAMERA_HELP_CONTROL_COUNT = 5,
	KM_ICON_TEXT_LENGTH = 0x80,
	CE_MARKER_NAME_LENGTH = 0x80,
};

struct KMIcon
{
	float       StartAlpha;                /* 0x0 */
	float       EndAlpha;                  /* 0x4 */
	float       FadeTime;                  /* 0x8 */
	float       Alpha;                     /* 0xc */
	float       Time;                      /* 0x10 */
	CH_ANIMTYPE AnimType;                  /* 0x14 */
	int         Key;                       /* 0x18 */
	int         MouseType;                 /* 0x1c */
	wchar_t     Text[KM_ICON_TEXT_LENGTH]; /* 0x20 */
	int         X;                         /* 0x120 */
	int         Y;                         /* 0x124 */
	int         Size;                      /* 0x128 */
	KEYALIGN    Align;                     /* 0x12c */
	LH3DColor   Colour1;                   /* 0x130 */
	LH3DColor   Colour2;                   /* 0x134 */
	int         TextAlpha;                 /* 0x138 */
	KMIcon*     Next;                      /* 0x13c */

	// BW1W120 00448be0 BW1M119 011a5f30
	KMIcon();
	// BW1W120 00448c00 BW1M119 011a5e70
	~KMIcon();

	// BW1W120 00448ac0 BW1M119 0107e4b0
	void Draw(float delta_time);
};
static_assert(sizeof(KMIcon) == 0x140, "Data type is of wrong size");

struct CEMarker
{
	// BW1W120 00c5afe8
	static CEMarker* First;

	CEMarker* Next;                        /* 0x0 */
	char      Name[CE_MARKER_NAME_LENGTH]; /* 0x4 */
	uint32_t  Colour;                      /* 0x84 */
	LHPoint   Pos;                         /* 0x88 */
	float     Scale;                       /* 0x94 */

	// BW1W120 00449e10 BW1M119 011a5750
	CEMarker()
	{
		Next = First;
		First = this;
		Colour = 0xffffffff;
		Pos = LHPoint(0.0f, 0.0f, 0.0f);
		Scale = 1.0f;
	}
	// BW1W120 inlined BW1M119 inlined
	~CEMarker()
	{
		if (First == this)
		{
			First = Next;
		}
		else
		{
			for (CEMarker* marker = First; marker != NULL; marker = marker->Next)
			{
				if (marker->Next == this)
				{
					marker->Next = Next;
					break;
				}
			}
		}
	}

	// BW1W120 inlined BW1M119 011a5870
	static CEMarker* FindIt(LHPoint& pos)
	{
		for (CEMarker* marker = First; marker != NULL; marker = marker->Next)
		{
			if (marker->Pos.GetDistance(pos) < 0.01f)
			{
				return marker;
			}
		}
		return NULL;
	}
	// BW1W120 inlined BW1M119 inlined
	static CEMarker* Create(LHPoint& pos, uint32_t colour, char* name)
	{
		CEMarker* marker = FindIt(pos);
		if (marker == NULL)
		{
			marker = new (CAMERA_HELP_H_FILEPATH, 352) CEMarker;
		}
		marker->Pos = pos;
		marker->Scale = 1.0f;
		marker->Colour = colour;
		marker->Name[0] = '\0';
		if (name != NULL)
		{
			strcpy(marker->Name, name);
		}
		return marker;
	}

	// BW1W120 00449eb0
	void fn_00449EB0();
};
static_assert(sizeof(CEMarker) == 0x98, "Data type is of wrong size");

enum CAMERA_EDITOR_MESSAGE
{
	CAMERA_EDITOR_MESSAGE_INIT = 0,
	CAMERA_EDITOR_MESSAGE_POSITION_X = 1,
	CAMERA_EDITOR_MESSAGE_POSITION_Y = 2,
	CAMERA_EDITOR_MESSAGE_POSITION_Z = 3,
	CAMERA_EDITOR_MESSAGE_FOCUS_X = 4,
	CAMERA_EDITOR_MESSAGE_FOCUS_Y = 5,
	CAMERA_EDITOR_MESSAGE_FOCUS_Z = 6,
	CAMERA_EDITOR_MESSAGE_FOV = 7,
	CAMERA_EDITOR_MESSAGE_MARKER_START = 8,
	CAMERA_EDITOR_MESSAGE_MARKER_X = 9,
	CAMERA_EDITOR_MESSAGE_MARKER_Y = 10,
	CAMERA_EDITOR_MESSAGE_MARKER_Z = 11,
	CAMERA_EDITOR_MESSAGE_MARKER_NAME = 12,
	CAMERA_EDITOR_MESSAGE_MARKER_END = 13,
};

enum CAMERA_EDITOR_DIALOG_MESSAGE
{
	CAMERA_EDITOR_DIALOG_MESSAGE_POSITION_X = 0x501,
	CAMERA_EDITOR_DIALOG_MESSAGE_POSITION_Y = 0x502,
	CAMERA_EDITOR_DIALOG_MESSAGE_POSITION_Z = 0x503,
	CAMERA_EDITOR_DIALOG_MESSAGE_FOCUS_X = 0x504,
	CAMERA_EDITOR_DIALOG_MESSAGE_FOCUS_Y = 0x505,
	CAMERA_EDITOR_DIALOG_MESSAGE_FOCUS_Z = 0x506,
	CAMERA_EDITOR_DIALOG_MESSAGE_FOV = 0x507,
};

class CameraHelp
{
public:
	// BW1W120 009cdd64
	static float AutoPitchParam1;
	// BW1W120 009cdd68
	static float AutoPitchParam2;
	// BW1W120 009cdd6c
	static int EnabledFeatures;
	// BW1W120 004473f0 BW1M119 011a6a20
	static void SetAutoPitch(float param_1, float param_2, bool enable);
	// BW1W120 00447430 BW1M119 011a69c0
	static void EnableCameraFeatures(int enable, int disable);
	// BW1W120 00447450 BW1M119 01067fd0
	static void DrawMouse(CH_CLICKTYPE click_type, CH_MOUSETYPE mouse_type, CH_ANIMTYPE anim_type, int x, int y,
	                      int size, unsigned long colour);
	// BW1W120 00447850 BW1M119 0107e660
	static void DrawMouseCallback();
	// BW1W120 00447910 BW1M119 011a66b0
	static float GetKeyWidth(wchar_t* text, float scale, int top, int bottom);
	// BW1W120 00447990 BW1M119 011a6390
	static void DrawKey(wchar_t* text, float scale, int left, int top, int right, int bottom, int alpha);
	// BW1W120 00447ba0 BW1M119 01053100
	static void DrawGlow(int left, int top, int right, int bottom, unsigned long colour);
	// BW1W120 00447ea0 BW1M119 01063100
	static void DrawKeyOrMouse(CH_ANIMTYPE anim_type, int key, int mouse_type, wchar_t* text, int x, int y, int size,
	                           KEYALIGN align, LH3DColor* colour1, LH3DColor* colour2, int alpha);
	// BW1W120 004489d0 BW1M119 011a6080
	static KMIcon* CreateKeyOrMouseIcon(float start_alpha, float end_alpha, float fade_time, CH_ANIMTYPE anim_type,
	                                    int key, int mouse_type, wchar_t* text, int x, int y, int size, KEYALIGN align,
	                                    LH3DColor* colour1, LH3DColor* colour2, int alpha);
	// BW1W120 00449140 BW1M119 010018f0
	static void CameraHelpCallback(CameraHelpReason reason, LHPoint& point, unsigned long param_3);
	// BW1W120 004491e0 BW1M119 011a59b0
	static void ResetStats(bool param_1);
	// BW1W120 00449240 BW1M119 01095160
	static void ResetTriggers();
	// BW1W120 004492f0
	static void fn_004492F0(int x, int y);
};

// BW1W120 004496b0 BW1M119 011a5210
void __stdcall camera_editor_callback(unsigned long message, unsigned long param, unsigned long value);

#endif /* BW1_DECOMP_CAMERA_HELP_INCLUDED_H */
