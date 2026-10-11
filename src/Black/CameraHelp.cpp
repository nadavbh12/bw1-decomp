#include "ColourConstants.h" /* For White */
#include "GameTimeConstants.h"
#include "HelpProfile.h"
#include "CameraHelp.h"

#include <stdio.h>  /* For sprintf */
#include <string.h> /* For strcat */
#include <wchar.h>  /* For wcscpy */

#include <Lionhead/LHLib/ver5.0/LHWin.h>     /* For operator new(size_t, const char*, uint32_t) */
#include <Lionhead/LHLog/ver4.0/LHSPrintf.h> /* For UNICODE_sprintf */

#include <Lionhead/LH3DLib/development/LH3DAtmos.h>         /* For LH3DAtmos */
#include <Lionhead/LH3DLib/development/LH3DAnim.h>          /* For LH3DAnim */
#include <Lionhead/LH3DLib/development/LH3DComplexObject.h> /* For LH3DComplexObject */
#include <Lionhead/LH3DLib/development/LH3DIsland.h>        /* For LH3DIsland */
#include <Lionhead/LH3DLib/development/LH3DMesh.h>          /* For LH3DMesh */
#include <Lionhead/LH3DLib/development/LH3DRender.h>        /* For LH3DRender */
#include <Lionhead/LH3DLib/development/LH3DTexture.h>       /* For LH3DTexture */
#include <Lionhead/LH3DLib/development/LH3DTech.h>          /* For LH3DTech */
#include <Lionhead/LH3DLib/development/LH3DText.h>          /* For GatheringText */

#include "alexmfc.h"        /* For SetupBox, SetupThing */
#include "Camera.h"         /* For GCamera */
#include "ControlHand.h"    /* For CHand */
#include "ControlMap.h"     /* For GetKeyNameText_ */
#include "Interface.h"      /* For GInterface */
#include "CameraModeNew3.h" /* For CameraModeNew3 */
#include "Game.h"           /* For GGame */

#include <Lionhead/LHLib/ver5.0/LHSystem.h> /* For LHSys */

#define FILEPATH "C:\\dev\\MP\\Black\\camerahelp.cpp"

enum
{
	CAMERA_HELP_REASON_GROUP_COUNT = 4,
	CAMERA_HELP_REASON_NAME_COUNT = 8,
	KEY_CLIP_Y_START = -0xa000,
	KEY_CLIP_Y_END = 0xa000,
	KEY_TEXT_LENGTH = 0x80,
	KEY_NAME_LENGTH = 0x20,
	ACCUMULATOR_TEXT_LENGTH = 0x80,
	CONTROLS_TEXT_LENGTH = 0x400,
	MARKER_NAME_LENGTH = 0x100,
};

static CameraHelpAccumulator ErrorStats[CAMERA_HELP_ERROR_COUNT];
static CameraHelpAccumulator CollideStats[CAMERA_HELP_COLLIDE_COUNT];
static CameraHelpAccumulator ControlStats[CAMERA_HELP_CONTROL_COUNT];

float CameraHelp::AutoPitchParam1 = 0.52359879f;
float CameraHelp::AutoPitchParam2 = 75.0f;
int   CameraHelp::EnabledFeatures = CAMERA_FEATURE_PITCH | CAMERA_FEATURE_ROTATE | CAMERA_FEATURE_ZOOM |
                                    CAMERA_FEATURE_MOVE | CAMERA_FEATURE_DOUBLE_CLICK | CAMERA_FEATURE_ZOOM_LANDSCAPE |
                                    CAMERA_FEATURE_HELP | CAMERA_FEATURE_EDGE_SCROLL;

static char* ReasonNames[CAMERA_HELP_REASON_GROUP_COUNT][CAMERA_HELP_REASON_NAME_COUNT] = {
	{"", "", "", "", "", "", "", ""},
	{"Error: Double clicked in excluded area", "Error: Double clicked in included area", "Error: Hand is too far away"},
	{"Collide with ground", "Collide with excluded area", "Collide with edge of allowed area", "Collide with ceiling"},
	{"Rotating", "Pitching", "Zooming", "Double click on pos", "Double click on object", "Zoom to Citadel", "Dragging"},
};

static char* ControlNames[CAMERA_HELP_CONTROL_COUNT] = {
	"keyboard", "mouse button", "both mouse buttons", "wheel spin", "wheel down",
};

const float KeyTextSize = 0.4f;

static unsigned long MouseHelpColour;
static float         MouseHelpSize;
static float         MouseHelpY;
static float         MouseHelpX;
static CH_CLICKTYPE  MouseHelpClickType;
static CH_MOUSETYPE  MouseHelpMouseType;
static bool32_t      MouseHelpAnimate;
static bool32_t      KeyOnRightOfScreen;
static KMIcon*       FirstIcon;
static LH3DTexture*  MouseTexture;
static LH3DMaterial* MouseMaterial;

unsigned int HelpProfile::AccumulatedTime;

CEMarker* CEMarker::First;

static LH3DObject* MarkerObject;

// BW1W120 00449080
static char* fn_00449080(int reason);
// BW1W120 004490a0
static char* fn_004490A0(unsigned long controls);

void CameraHelp::SetAutoPitch(float param_1, float param_2, bool enable)
{
	EnableCameraFeatures(enable ? CAMERA_FEATURE_AUTO_PITCH : 0, enable ? 0 : CAMERA_FEATURE_AUTO_PITCH);
	AutoPitchParam1 = param_1;
	AutoPitchParam2 = param_2;
}

void CameraHelp::EnableCameraFeatures(int enable, int disable)
{
	EnabledFeatures = (EnabledFeatures & ~disable) | enable;
}

void CameraHelp::DrawMouse(CH_CLICKTYPE click_type, CH_MOUSETYPE mouse_type, CH_ANIMTYPE anim_type, int x, int y,
                           int size, unsigned long colour)
{
	if (mouse_type == CH_MOUSETYPE_NONE)
	{
		return;
	}
	bool32_t mirror = false;
	int      frame = (GetTickCount() / 100) % 5;
	bool32_t visible = true;
	switch (anim_type)
	{
	case CH_ANIMTYPE_0x1:
		visible = frame < 2;
		break;
	case CH_ANIMTYPE_0x2:
		frame = (GetTickCount() / 80) % 12;
		visible = frame < 2 || (frame >= 4 && frame < 6);
		break;
	case CH_ANIMTYPE_0x3:
		frame = (GetTickCount() / 100) % 5;
		break;
	}
	int icon;
	switch (click_type)
	{
	case CH_CLICKTYPE_0x8:
		icon = frame > 3 ? 11 : 15 - frame;
		break;
	case CH_CLICKTYPE_0x10:
		icon = frame > 3 ? 11 : frame + 12;
		break;
	default:
		icon = 1;
		mirror = true;
		if (!visible)
		{
			icon = 0;
		}
		break;
	case CH_CLICKTYPE_0x2:
		mirror = false;
		icon = 1;
		if (!visible)
		{
			icon = 0;
		}
		break;
	case CH_CLICKTYPE_0x3:
		icon = 2;
		if (!visible)
		{
			icon = 0;
		}
		break;
	case CH_CLICKTYPE_0x4:
		icon = 3;
		if (!visible)
		{
			icon = 0;
		}
		break;
	case CH_CLICKTYPE_0x0:
		icon = 0;
		break;
	}
	if (click_type != CH_CLICKTYPE_0x8 && click_type != CH_CLICKTYPE_0x10 && icon >= 0)
	{
		icon += mouse_type * 4;
	}
	if (MouseMaterial == NULL)
	{
		MouseTexture = LH3DTexture::Create("data\\textures\\mousehelp.raw", 0x41, 0, NULL);
		MouseMaterial = LH3DRender::CreateMaterial(LH3DMaterial::LH3D_MATERIAL_RENDER_MODE_0x5, MouseTexture);
		if (MouseMaterial == NULL)
		{
			return;
		}
	}
	if (icon < 0 || colour == 0)
	{
		return;
	}
	float u = (icon & 3) * 0.25f;
	float v = (icon / 4) * 0.25f;
	int   right = x + size;
	int   bottom = y + size;
	float width = 0.25f;
	if (mirror)
	{
		u += 0.25f;
		width = -0.25f;
	}
	D3DTLVERTEX* vertices = LH3DRender::g_quad_vertices;
	vertices[0].sx = x;
	vertices[0].sy = y;
	vertices[0].tu = u;
	vertices[0].tv = v;
	vertices[0].color = colour;
	vertices[1].sx = right;
	vertices[1].sy = y;
	vertices[1].tu = u + width;
	vertices[1].tv = v;
	vertices[1].color = colour;
	vertices[2].sx = right;
	vertices[2].sy = bottom;
	vertices[2].tu = u + width;
	vertices[2].tv = v + 0.25f;
	vertices[2].color = colour;
	vertices[3].sx = x;
	vertices[3].sy = bottom;
	vertices[3].tu = u;
	vertices[3].tv = v + 0.25f;
	vertices[3].color = colour;
	LH3DRender::Inline_0082EF67(MouseMaterial);
	LH3DRender::SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
	LH3DRender::SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	DrawAndClip2D(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, LH3DRender::g_quad_vertices,
	              ARRAY_SIZE(LH3DRender::g_quad_vertices), LH3DRender::g_quad_indices,
	              ARRAY_SIZE(LH3DRender::g_quad_indices));
	LH3DRender::SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
	LH3DRender::SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
}

void CameraHelp::DrawMouseCallback()
{
	int width = LHSys::GetScreen().Width();
	if (MouseHelpClickType != -1)
	{
		DrawMouse(MouseHelpClickType, MouseHelpMouseType, (CH_ANIMTYPE)(MouseHelpAnimate != false),
		          (int)((MouseHelpX + MouseHelpSize) * width), (int)(LHSys::GetScreen().Height() * MouseHelpY),
		          (int)(width * MouseHelpSize), MouseHelpColour);
	}
	float deltaTime = LH3DTech::g_delta_time * 0.001f;
	for (KMIcon* icon = FirstIcon; icon != NULL; icon = icon->Next)
	{
		icon->Draw(deltaTime);
	}
}

float CameraHelp::GetKeyWidth(wchar_t* text, float scale, int top, int bottom)
{
	if (text != NULL)
	{
		GatheringText* font = GatheringText::gamefont;
		int            length = wcslen(text);
		int            height = bottom - top;
		float          minWidth = height;
		float          width = font->GetStringWidth(text, length, (height * scale) * KeyTextSize) + height * 2 / 3;
		if (width < minWidth)
		{
			return minWidth;
		}
		return width;
	}
	return 0.0f;
}

void CameraHelp::DrawKey(wchar_t* text, float scale, int left, int top, int right, int bottom, int alpha)
{
	SetupThing::DrawAlpha = alpha;
	int height = bottom - top;
	int edge = height / 4;
	int middleLeft = left + edge;
	int middleRight = right - edge;
	SetupThing::DrawBox(left, top, middleLeft, bottom, 0.501953125f, 0.001953125f, 0.564453125f, 0.248046875f,
	                    LH3DAtmos::AtmosMaterial, &LH3DColor(0xff, 0xff, 0xff, alpha), 0, KEY_CLIP_Y_START,
	                    KEY_CLIP_Y_END, false, 100.0f);
	SetupThing::DrawBox(middleLeft, top, middleRight, bottom, 0.564453125f, 0.001953125f, 0.685546875f, 0.248046875f,
	                    LH3DAtmos::AtmosMaterial, &LH3DColor(0xff, 0xff, 0xff, alpha), 0, KEY_CLIP_Y_START,
	                    KEY_CLIP_Y_END, false, 100.0f);
	SetupThing::DrawBox(middleRight, top, right, bottom, 0.685546875f, 0.001953125f, 0.748046875f, 0.248046875f,
	                    LH3DAtmos::AtmosMaterial, &LH3DColor(0xff, 0xff, 0xff, alpha), 0, KEY_CLIP_Y_START,
	                    KEY_CLIP_Y_END, false, 100.0f);
	if (text != NULL)
	{
		GatheringText* font = GatheringText::gamefont;
		font->DrawTextRaw(text, wcslen(text), left + height / 5, top + height / 7,
		                  LH3DTech::g_info_transform.NearClip * 1.1f, (bottom - top) * scale * KeyTextSize,
		                  &LH3DColor(0, 0, 0, alpha), 0, NULL, 0.0f, 4096.0f);
	}
}

void CameraHelp::DrawGlow(int left, int top, int right, int bottom, unsigned long colour)
{
	int edge = (bottom - top) / 4;
	int outerLeft = left - edge;
	int innerLeft = left + edge;
	int innerRight = right - edge;
	int outerRight = right + edge;
	int outerTop = top - edge;
	int innerTop = top + edge;
	int innerBottom = bottom - edge;
	int outerBottom = bottom + edge;
	SetupThing::DrawAlpha = colour >> 24;
	SetupThing::DrawBox(outerLeft, outerTop, innerLeft, innerTop, 0.0f, 0.25f, 0.078125f, 0.328125f,
	                    LH3DAtmos::AdditiveMaterial, &LH3DColor(colour), 0, KEY_CLIP_Y_START, KEY_CLIP_Y_END, false,
	                    100.0f);
	SetupThing::DrawBox(innerLeft, outerTop, innerRight, innerTop, 0.078125f, 0.25f, 0.16796875f, 0.328125f,
	                    LH3DAtmos::AdditiveMaterial, &LH3DColor(colour), 0, KEY_CLIP_Y_START, KEY_CLIP_Y_END, false,
	                    100.0f);
	SetupThing::DrawBox(innerRight, outerTop, outerRight, innerTop, 0.16796875f, 0.25f, 0.24609375f, 0.328125f,
	                    LH3DAtmos::AdditiveMaterial, &LH3DColor(colour), 0, KEY_CLIP_Y_START, KEY_CLIP_Y_END, false,
	                    100.0f);
	SetupThing::DrawBox(outerLeft, innerTop, innerLeft, innerBottom, 0.0f, 0.328125f, 0.078125f, 0.41796875f,
	                    LH3DAtmos::AdditiveMaterial, &LH3DColor(colour), 0, KEY_CLIP_Y_START, KEY_CLIP_Y_END, false,
	                    100.0f);
	SetupThing::DrawBox(innerLeft, innerTop, innerRight, innerBottom, 0.078125f, 0.328125f, 0.16796875f, 0.41796875f,
	                    LH3DAtmos::AdditiveMaterial, &LH3DColor(colour), 0, KEY_CLIP_Y_START, KEY_CLIP_Y_END, false,
	                    100.0f);
	SetupThing::DrawBox(innerRight, innerTop, outerRight, innerBottom, 0.16796875f, 0.328125f, 0.24609375f, 0.41796875f,
	                    LH3DAtmos::AdditiveMaterial, &LH3DColor(colour), 0, KEY_CLIP_Y_START, KEY_CLIP_Y_END, false,
	                    100.0f);
	SetupThing::DrawBox(outerLeft, innerBottom, innerLeft, outerBottom, 0.0f, 0.41796875f, 0.078125f, 0.49609375f,
	                    LH3DAtmos::AdditiveMaterial, &LH3DColor(colour), 0, KEY_CLIP_Y_START, KEY_CLIP_Y_END, false,
	                    100.0f);
	SetupThing::DrawBox(innerLeft, innerBottom, innerRight, outerBottom, 0.078125f, 0.41796875f, 0.16796875f,
	                    0.49609375f, LH3DAtmos::AdditiveMaterial, &LH3DColor(colour), 0, KEY_CLIP_Y_START,
	                    KEY_CLIP_Y_END, false, 100.0f);
	SetupThing::DrawBox(innerRight, innerBottom, outerRight, outerBottom, 0.16796875f, 0.41796875f, 0.24609375f,
	                    0.49609375f, LH3DAtmos::AdditiveMaterial, &LH3DColor(colour), 0, KEY_CLIP_Y_START,
	                    KEY_CLIP_Y_END, false, 100.0f);
}

void CameraHelp::DrawKeyOrMouse(CH_ANIMTYPE anim_type, int key, int mouse_type, wchar_t* text, int x, int y, int size,
                                KEYALIGN align, LH3DColor* colour1, LH3DColor* colour2, int alpha)
{
	if ((align & KEYALIGN_HAND) == KEYALIGN_HAND)
	{
		LHPoint pos = GGame::g_game->MyInterface()->GetRenderHand()->DynamicShadow->matrix.GetPos();
		int     screenX;
		int     screenY;
		float   depth;
		if (!LH3DTech::ProjectPoint(&pos, &screenX, &screenY, &depth))
		{
			return;
		}
		x += screenX;
		y += screenY;
		if (y < 0)
		{
			y = 0;
		}
		if (y > LHSys::GetScreen().height - size)
		{
			y = LHSys::GetScreen().height - size;
		}
		if (x < 0)
		{
			x = 0;
		}
		if (x > LHSys::GetScreen().width - size)
		{
			x = LHSys::GetScreen().width - size;
		}
	}
	else
	{
		y += -(size / 2);
	}
	GatheringText* font = GatheringText::gamefont;
	LH3DColor*     top;
	if (colour1 != NULL)
	{
		top = colour1;
	}
	else
	{
		LH3DColor white(0xff, 0xff, 0xff, 0xff);
		top = &white;
	}
	LH3DColor* bottom;
	if (colour2 != NULL)
	{
		bottom = colour2;
	}
	else
	{
		bottom = top;
	}
	LH3DColor shadow(top->GetColor() & 0xff000000);
	if (top->a < 4 && bottom->a < 4)
	{
		return;
	}
	wchar_t keyText[KEY_TEXT_LENGTH];
	float   keyWidth;
	if (anim_type == CH_ANIMTYPE_KEY)
	{
		if (key == 0)
		{
			wcscpy(keyText, (wchar_t*)mouse_type);
		}
		else
		{
			wchar_t keyName[KEY_NAME_LENGTH];
			wchar_t modifierName[KEY_NAME_LENGTH];
			memset(keyName, 0, sizeof(keyName));
			memset(modifierName, 0, sizeof(modifierName));
			GetKeyNameText_((((key & 0x80) | 0x100) << 1 | (key & 0x7f)) << 16, keyName, 0x10);
			GetKeyNameText_((((mouse_type & 0x80) | 0x100) << 1 | (mouse_type & 0x7f)) << 16, modifierName, 0x10);
			if (mouse_type != 0)
			{
				UNICODE_sprintf(keyText, L"%s + %s", modifierName, keyName);
			}
			else
			{
				wcscpy(keyText, keyName);
			}
		}
		keyWidth = GetKeyWidth(keyText, 1.0f, 0, size);
	}
	else
	{
		keyWidth = size;
		if (mouse_type == CH_MOUSETYPE_NONE)
		{
			keyWidth = 0.0f;
		}
	}
	float baseKeyWidth = keyWidth;
	int   arrowLeft = align & KEYALIGN_ARROW_LEFT;
	if (arrowLeft)
	{
		keyWidth += size / 2;
	}
	int arrowRight = align & KEYALIGN_ARROW_RIGHT;
	if (arrowRight)
	{
		keyWidth += size / 2;
	}
	float textSize = size * 2 / 3;
	if (NeedsBiggerText())
	{
		textSize = size * 4 / 5;
	}
	float textWidth;
	float totalWidth;
	if (text != NULL)
	{
		int length = wcslen(text);
		textWidth = font->GetStringWidth(text, length, textSize);
		totalWidth = textWidth + keyWidth + 2.0f;
	}
	else
	{
		totalWidth = keyWidth;
		textWidth = 0.0f;
	}
	bool32_t onRight = false;
	if (align & KEYALIGN_RIGHT)
	{
		onRight = true;
	}
	if (align & KEYALIGN_SCREEN_SIDE)
	{
		if (x > LHSys::GetScreen().width * 2 / 3)
		{
			KeyOnRightOfScreen = true;
		}
		if (x < LHSys::GetScreen().width / 3)
		{
			KeyOnRightOfScreen = false;
		}
		if (!KeyOnRightOfScreen)
		{
			onRight = false;
			x += -(size / 2);
		}
		else
		{
			onRight = true;
			x += size / 2;
		}
	}
	if (onRight)
	{
		x = x - totalWidth;
	}
	bool32_t textFirst = false;
	if (align & KEYALIGN_TEXT_LEFT)
	{
		textFirst = true;
	}
	if (align & KEYALIGN_TEXT_SIDE)
	{
		textFirst = onRight;
	}
	int keyX = textFirst ? x + textWidth + 2.0f : x;
	int textX = textFirst ? x : x + keyWidth + 2.0f;
	if (arrowLeft)
	{
		keyX += size / 2;
	}
	if (alpha != 0)
	{
		if (baseKeyWidth != 0.0f)
		{
			DrawGlow(keyX, y, keyX + baseKeyWidth, y + size, (bottom->GetColor() & 0xffffff) + (alpha << 24));
		}
		unsigned long glowColour = ((alpha / 3) << 24) + (top->GetColor() & 0xffffff);
		if (text[0] != L'\0')
		{
			float offset = (size - textSize) * 0.5f;
			DrawGlow(textX, y + offset, textX + textWidth, (y + size) - offset, glowColour);
		}
		SetupThing::DrawAlpha = glowColour >> 24;
		if (arrowLeft)
		{
			DrawGlow(keyX - size / 2, y + size / 4, keyX, y + size * 3 / 4, glowColour);
		}
		if (arrowRight)
		{
			DrawGlow(keyX + baseKeyWidth, y + size / 4, (float)(size / 2) + keyX + baseKeyWidth, y + size * 3 / 4,
			         glowColour);
		}
		if (align & KEYALIGN_ARROW_UP)
		{
			DrawGlow(keyX + size / 4, y - size / 2, keyX + size * 3 / 4, y, glowColour);
		}
		if (align & KEYALIGN_ARROW_DOWN)
		{
			DrawGlow(keyX + size / 4, y + size, keyX + size * 3 / 4, size / 2 + y + size, glowColour);
		}
	}
	if (anim_type == CH_ANIMTYPE_KEY)
	{
		DrawKey(keyText, 1.0f, keyX, y, keyX + baseKeyWidth, y + size, bottom->a);
	}
	else if (mouse_type != CH_MOUSETYPE_NONE)
	{
		DrawMouse((CH_CLICKTYPE)key, (CH_MOUSETYPE)mouse_type, anim_type, keyX, y, size, bottom->GetColor());
	}
	float inset = -size * 0.11111111f;
	SetupThing::DrawAlpha = bottom->a;
	if (arrowLeft)
	{
		SetupThing::DrawBox((keyX - size / 2) + inset, (y + size / 4) + inset, keyX - inset, (y + size * 3 / 4) - inset,
		                    0.75390625f, 0.12890625f, 0.87109375f, 0.24609375f, LH3DAtmos::AtmosMaterial, bottom, 0,
		                    KEY_CLIP_Y_START, KEY_CLIP_Y_END, false, 100.0f);
	}
	if (arrowRight)
	{
		SetupThing::DrawBox(keyX + inset + baseKeyWidth, (y + size / 4) + inset, keyX - inset + size / 2 + baseKeyWidth,
		                    (y + size * 3 / 4) - inset, 0.87890625f, 0.00390625f, 0.99609375f, 0.12109375f,
		                    LH3DAtmos::AtmosMaterial, bottom, 0, KEY_CLIP_Y_START, KEY_CLIP_Y_END, false, 100.0f);
	}
	if (align & KEYALIGN_ARROW_UP)
	{
		SetupThing::DrawBox((keyX + size / 4) + inset, (y - size / 2) + inset, keyX - inset + size * 3 / 4, y - inset,
		                    0.75390625f, 0.00390625f, 0.87109375f, 0.12109375f, LH3DAtmos::AtmosMaterial, bottom, 0,
		                    KEY_CLIP_Y_START, KEY_CLIP_Y_END, false, 100.0f);
	}
	if (align & KEYALIGN_ARROW_DOWN)
	{
		SetupThing::DrawBox((keyX + size / 4) + inset, (y + size) + inset, keyX - inset + size * 3 / 4,
		                    (size / 2 + y + size) - inset, 0.87890625f, 0.12890625f, 0.99609375f, 0.24609375f,
		                    LH3DAtmos::AtmosMaterial, bottom, 0, KEY_CLIP_Y_START, KEY_CLIP_Y_END, false, 100.0f);
	}
	if (text != NULL)
	{
		float offset = (size - textSize) * 0.5f;
		font->DrawTextRaw(text, wcslen(text), textX - 1, (y - 1) + offset, LH3DTech::g_info_transform.NearClip * 1.1f,
		                  textSize, &shadow, 0, NULL, 0.0f, 4096.0f);
		font->DrawTextRaw(text, wcslen(text), textX + 1, (y + 1) + offset, LH3DTech::g_info_transform.NearClip * 1.1f,
		                  textSize, &shadow, 0, NULL, 0.0f, 4096.0f);
		font->DrawTextRaw(text, wcslen(text), textX, y + offset, LH3DTech::g_info_transform.NearClip * 1.1f, textSize,
		                  &LH3DColor(*top), 0, NULL, 0.0f, 4096.0f);
	}
}

KMIcon* CameraHelp::CreateKeyOrMouseIcon(float start_alpha, float end_alpha, float fade_time, CH_ANIMTYPE anim_type,
                                         int key, int mouse_type, wchar_t* text, int x, int y, int size, KEYALIGN align,
                                         LH3DColor* colour1, LH3DColor* colour2, int alpha)
{
	KMIcon* icon = new (FILEPATH, 387) KMIcon;
	icon->StartAlpha = start_alpha;
	icon->EndAlpha = end_alpha;
	icon->FadeTime = fade_time;
	icon->Alpha = start_alpha;
	icon->AnimType = anim_type;
	icon->Key = key;
	icon->MouseType = mouse_type;
	icon->X = x;
	icon->Y = y;
	icon->Size = size;
	icon->Time = 0.0f;
	icon->Align = align;
	icon->TextAlpha = alpha;
	if (text != NULL)
	{
		wcscpy(icon->Text, text);
	}
	else
	{
		icon->Text[0] = L'\0';
	}
	icon->Colour1 = colour1 != NULL ? *colour1 : LH3DColor(0xff, 0xff, 0xff, 0xff);
	icon->Colour2 = colour2 != NULL ? *colour2 : icon->Colour1;
	return icon;
}

void KMIcon::Draw(float delta_time)
{
	Time += delta_time;
	if (Time < 0.0f)
	{
		Time = 0.0f;
	}
	if (Time > FadeTime)
	{
		Time = FadeTime;
	}
	Alpha = EndAlpha;
	if (FadeTime > 0.0f)
	{
		Alpha = (EndAlpha - StartAlpha) * (Time / FadeTime) + StartAlpha;
	}
	float fade;
	if (Alpha > 0.0f)
	{
		if (Alpha < 1.0f)
		{
			fade = Alpha;
		}
		else
		{
			fade = 1.0f;
		}
	}
	else
	{
		fade = 0.0f;
	}
	Colour2.a = Colour1.a = (uint8_t)(fade * 255.0f);
	if (SetupBox::GetCurrentActiveBox() == NULL ||
	    SetupBox::GetCurrentActiveBox()->BackgroundStyle == SETUP_BACKGROUND_NONE)
	{
		CameraHelp::DrawKeyOrMouse(AnimType, Key, MouseType, Text, X, Y, Size, Align, &Colour1, &Colour2,
		                           (int)(TextAlpha * fade));
	}
}

KMIcon::KMIcon()
{
	Next = FirstIcon;
	FirstIcon = this;
}

KMIcon::~KMIcon()
{
	if (this == FirstIcon)
	{
		FirstIcon = Next;
	}
	else
	{
		for (KMIcon* icon = FirstIcon; icon != NULL && icon->Next != NULL; icon = icon->Next)
		{
			if (icon->Next == this)
			{
				icon->Next = Next;
				break;
			}
		}
	}
}

void CameraHelpAccumulator::Draw(char* name, int x, int y)
{
	static LHColor red(0xff, 0, 0);
	static LHColor green(0, 0xff, 0);
	static LHColor white(0xff, 0xff, 0xff);
	static char    text[ACCUMULATOR_TEXT_LENGTH];
	sprintf(text, "%s %6d %0.2f, %0.2f ; %0.2f", name, GetTotal(), GetTimeSinceLastUsed(), GetRate(),
	        AverageTriggers * 10.0f);
	LHSys::GetText().Draw(x - LHSys::GetText().StringWidth(text, NULL) - 2, y, text, 1);
	LHSys::GetDraw().Box(x, y, x - (int)(GetRate() * -10.0f), y + 5, red, 1);
	LHSys::GetDraw().Box(x, y + 6, x - (int)(AverageTriggers * -100.0f), y + 11, green, 1);
}

int CameraHelpAccumulator::GetDrawHeight()
{
	return 12;
}

void CameraHelpAccumulator::Reset()
{
	TotalTriggerCount = 0;
	TriggerTimeCount = 0;
	TriggerTimeHead = 0;
	AverageTriggers = 0;
	TriggeredThisFrame = 0;
}

float CameraHelpAccumulator::GetTimeSinceLastUsed()
{
	if (TriggerTimeCount == 0)
	{
		return 0.0f;
	}
	int time =
		HelpProfile::AccumulatedTime - TriggerTimes[(TriggerTimeHead - 1) & (CAMERA_HELP_TRIGGER_TIME_COUNT - 1)];
	if (time < 0)
	{
		fn_00448F90();
		time = 0;
	}
	return time * 0.001f;
}

void CameraHelpAccumulator::fn_00448F90()
{
	for (int i = 0; i < CAMERA_HELP_TRIGGER_TIME_COUNT; i++)
	{
		if (TriggerTimes[i] > HelpProfile::AccumulatedTime)
		{
			TriggerTimes[i] = HelpProfile::AccumulatedTime;
		}
	}
}

float CameraHelpAccumulator::GetRate()
{
	if (TriggerTimeCount < 2)
	{
		return 0.0f;
	}
	int time = HelpProfile::AccumulatedTime -
	           TriggerTimes[(TriggerTimeHead - TriggerTimeCount) & (CAMERA_HELP_TRIGGER_TIME_COUNT - 1)];
	if (time < 0)
	{
		fn_00448F90();
		return 0.0f;
	}
	if (time == 0)
	{
		return 0.0f;
	}
	return (float)((TriggerTimeCount - 1) * 1000) / time;
}

void CameraHelpAccumulator::Trigger()
{
	if (TriggeredThisFrame)
	{
		return;
	}
	TotalTriggerCount++;
	TriggeredThisFrame = 1;
	TriggerTimes[TriggerTimeHead] = HelpProfile::AccumulatedTime;
	TriggerTimeHead = (TriggerTimeHead + 1) & (CAMERA_HELP_TRIGGER_TIME_COUNT - 1);
	if (TriggerTimeCount < CAMERA_HELP_TRIGGER_TIME_COUNT)
	{
		TriggerTimeCount++;
	}
}

char* fn_00449080(int reason)
{
	return ReasonNames[(reason >> 8) & (CAMERA_HELP_REASON_GROUP_COUNT - 1)]
					  [reason & (CAMERA_HELP_REASON_NAME_COUNT - 1)];
}

char* fn_004490A0(unsigned long controls)
{
	static char text[CONTROLS_TEXT_LENGTH];
	text[0] = '\0';
	bool32_t any = false;
	for (int i = 0; i < CAMERA_HELP_CONTROL_COUNT; i++)
	{
		if (controls & (1 << i))
		{
			if (any)
			{
				strcat(text, ", ");
			}
			strcat(text, ControlNames[i]);
			any = true;
		}
	}
	return text;
}

void CameraHelp::CameraHelpCallback(CameraHelpReason reason, LHPoint& point, unsigned long param_3)
{
	int index = reason & CAMERA_HELP_REASON_INDEX_MASK;
	int group = reason & CAMERA_HELP_REASON_GROUP_MASK;
	if (group == CAMERA_HELP_REASON_GROUP_ACTION)
	{
		GGame::g_game->help_profile->Trigger((HELP_EVENT_TYPE)(HELP_EVENT_TYPE_25 + index));
		for (int i = 0; i < CAMERA_HELP_CONTROL_COUNT; i++)
		{
			if (param_3 & (1 << i))
			{
				ControlStats[i].Trigger();
			}
		}
	}
	else if (group == CAMERA_HELP_REASON_GROUP_COLLIDE)
	{
		CollideStats[index].Trigger();
	}
	else if (group == CAMERA_HELP_REASON_GROUP_ERROR)
	{
		ErrorStats[index].Trigger();
	}
}

void CameraHelp::ResetStats(bool param_1)
{
	int i;
	for (i = 0; i < CAMERA_HELP_ERROR_COUNT; i++)
	{
		ErrorStats[i].Reset();
	}
	for (i = 0; i < CAMERA_HELP_COLLIDE_COUNT; i++)
	{
		CollideStats[i].Reset();
	}
	for (i = 0; i < CAMERA_HELP_CONTROL_COUNT; i++)
	{
		ControlStats[i].Reset();
	}
}

void CameraHelp::ResetTriggers()
{
	int i;
	int triggered;
	for (i = 0; i < CAMERA_HELP_ERROR_COUNT; i++)
	{
		triggered = ErrorStats[i].TriggeredThisFrame;
		ErrorStats[i].TriggeredThisFrame = 0;
		ErrorStats[i].AverageTriggers += (triggered - ErrorStats[i].AverageTriggers) * 0.005f;
	}
	for (i = 0; i < CAMERA_HELP_COLLIDE_COUNT; i++)
	{
		triggered = CollideStats[i].TriggeredThisFrame;
		CollideStats[i].TriggeredThisFrame = 0;
		CollideStats[i].AverageTriggers += (triggered - CollideStats[i].AverageTriggers) * 0.005f;
	}
	for (i = 0; i < CAMERA_HELP_CONTROL_COUNT; i++)
	{
		triggered = ControlStats[i].TriggeredThisFrame;
		ControlStats[i].TriggeredThisFrame = 0;
		ControlStats[i].AverageTriggers += (triggered - ControlStats[i].AverageTriggers) * 0.005f;
	}
}

void CameraHelp::fn_004492F0(int x, int y)
{
	int     i;
	LHColor blue(0, 0, 0xff);
	LHSys::GetDraw().Box(0, y, LHSys::GetScreen().Width() / 2, y, blue, 0);
	y++;
	for (i = 0; i < CAMERA_HELP_ERROR_COUNT; i++, y += ErrorStats[i].GetDrawHeight())
	{
		ErrorStats[i].Draw(fn_00449080(CAMERA_HELP_REASON_GROUP_ERROR + i), x, y);
	}
	LHSys::GetDraw().Box(0, y, LHSys::GetScreen().Width() / 2, y, blue, 0);
	y++;
	for (i = 0; i < CAMERA_HELP_COLLIDE_COUNT; i++, y += CollideStats[i].GetDrawHeight())
	{
		CollideStats[i].Draw(fn_00449080(CAMERA_HELP_REASON_GROUP_COLLIDE + i), x, y);
	}
	LHSys::GetDraw().Box(0, y, LHSys::GetScreen().Width() / 2, y, blue, 0);
	y++;
	LHSys::GetDraw().Box(0, y, LHSys::GetScreen().Width() / 2, y, blue, 0);
	y++;
	for (i = 0; i < CAMERA_HELP_CONTROL_COUNT; i++, y += ControlStats[i].GetDrawHeight())
	{
		ControlStats[i].Draw(fn_004490A0(1 << i), x, y);
	}
	LHSys::GetDraw().Box(0, y, LHSys::GetScreen().Width() / 2, y, blue, 0);
}

void __stdcall camera_editor_callback(unsigned long message, unsigned long param, unsigned long value)
{
	static char    markerName[MARKER_NAME_LENGTH];
	static int     markerColour;
	static bool    markerToggle;
	static float   fov;
	static HWND    window;
	static LHPoint lastMarkerPos;
	static LHPoint markerPos;
	static LHPoint focus;
	static LHPoint position;
	static HWND    dialog;
	int            update = 0;
	switch (message)
	{
	case CAMERA_EDITOR_MESSAGE_INIT:
		window = (HWND)value;
		position = LH3DTech::GetCameraPosition();
		focus = LH3DTech::GetCameraTarget();
		fov = LH3DTech::g_camera.fov;
		dialog = (HWND)value;
		SendMessageA(window, CAMERA_EDITOR_DIALOG_MESSAGE_POSITION_X, 0, *(LPARAM*)&position.x);
		SendMessageA(window, CAMERA_EDITOR_DIALOG_MESSAGE_POSITION_Y, 0, *(LPARAM*)&position.y);
		SendMessageA(window, CAMERA_EDITOR_DIALOG_MESSAGE_POSITION_Z, 0, *(LPARAM*)&position.z);
		SendMessageA(window, CAMERA_EDITOR_DIALOG_MESSAGE_FOCUS_X, 0, *(LPARAM*)&focus.x);
		SendMessageA(window, CAMERA_EDITOR_DIALOG_MESSAGE_FOCUS_Y, 0, *(LPARAM*)&focus.y);
		SendMessageA(window, CAMERA_EDITOR_DIALOG_MESSAGE_FOCUS_Z, 0, *(LPARAM*)&focus.z);
		SendMessageA(window, CAMERA_EDITOR_DIALOG_MESSAGE_FOV, 0, *(LPARAM*)&fov);
		break;
	case CAMERA_EDITOR_MESSAGE_POSITION_X:
		position.x = *(float*)&value;
		break;
	case CAMERA_EDITOR_MESSAGE_POSITION_Y:
		position.y = *(float*)&value;
		break;
	case CAMERA_EDITOR_MESSAGE_POSITION_Z:
		position.z = *(float*)&value;
		update = 1;
		break;
	case CAMERA_EDITOR_MESSAGE_FOCUS_X:
		focus.x = *(float*)&value;
		break;
	case CAMERA_EDITOR_MESSAGE_FOCUS_Y:
		focus.y = *(float*)&value;
		break;
	case CAMERA_EDITOR_MESSAGE_FOCUS_Z:
		focus.z = *(float*)&value;
		update = 2;
		break;
	case CAMERA_EDITOR_MESSAGE_FOV:
		fov = *(float*)&value;
		update = 3;
		break;
	case CAMERA_EDITOR_MESSAGE_MARKER_START:
		markerToggle = param != 0;
		markerColour = value;
		markerName[0] = '\0';
		break;
	case CAMERA_EDITOR_MESSAGE_MARKER_X:
		markerPos.x = *(float*)&value;
		break;
	case CAMERA_EDITOR_MESSAGE_MARKER_Y:
		markerPos.y = *(float*)&value;
		break;
	case CAMERA_EDITOR_MESSAGE_MARKER_Z:
		markerPos.z = *(float*)&value;
		break;
	case CAMERA_EDITOR_MESSAGE_MARKER_NAME:
		strcat(markerName, (char*)&value);
		break;
	case CAMERA_EDITOR_MESSAGE_MARKER_END: {
		CEMarker* marker;
		if (markerToggle)
		{
			marker = CEMarker::FindIt(lastMarkerPos);
			if (marker != NULL)
			{
				delete marker;
			}
		}
		marker = CEMarker::FindIt(markerPos);
		if (marker != NULL)
		{
			delete marker;
		}
		else
		{
			CEMarker::Create(markerPos, markerColour, markerName);
		}
		if (markerToggle)
		{
			lastMarkerPos = markerPos;
		}
		break;
	}
	}
	GCamera* camera = GGame::g_game->GetCamera();
	if (camera != NULL)
	{
		CameraModeNew3* mode = dynamic_cast<CameraModeNew3*>(camera->GetCurrentMode());
		if (GGame::g_game->GetCamera() != NULL)
		{
			if (update == 1)
			{
				GGame::g_game->GetCamera()->CameraOriginZoomer.SetPosition(position);
				if (mode != NULL)
				{
					mode->FlyToPosFoc(position, camera->CameraHeadingZoomer.GetDestination(), 0.3f);
				}
			}
			else if (update == 2)
			{
				GGame::g_game->GetCamera()->CameraHeadingZoomer.SetPosition(focus);
				if (mode != NULL)
				{
					mode->FlyToPosFoc(camera->CameraOriginZoomer.GetDestination(), focus, 0.3f);
				}
			}
			else if (update == 3)
			{
				GGame::g_game->GetCamera()->FovZoomer.SetPosition(fov);
			}
		}
	}
}

void CEMarker::fn_00449EB0()
{
	if (MarkerObject == NULL)
	{
		MarkerObject = LH3DObject::Create(LH3DObject::ANIMATED);
		MarkerObject->SetMesh(LH3DMesh::GetPackedMesh(MSH_O_SQUAREROCK_VOLCANIC), NULL, NULL);
		MarkerObject->SetCurrentAnim(LH3DAnim::GetPackedAnim(ANM_P_STAND_DESPAIR_3));
		MarkerObject->SetCurrentCycleTime(0);
		MarkerObject->SetDynamicLighting(1);
		if (MarkerObject == NULL)
		{
			return;
		}
	}
	float altitude = LH3DIsland::GetAltitude(LH3DMapCoords(Pos.x, Pos.z));
	if (altitude > Pos.y)
	{
		Pos.y = altitude;
	}
	MarkerObject->SetPosition(Pos, ((GetTickCount() >> 3) & 0x1ff) * 0.012271847f, Scale);
	MarkerObject->SetColorSpecular(0xffffffff, 0);
	MarkerObject->Draw();
	GatheringText* font;
	if (Name[0] != '\0' && (font = GatheringText::gamefont) != NULL)
	{
		int   x;
		int   y;
		float depth;
		if (LH3DTech::ProjectPoint(&Pos, &x, &y, &depth))
		{
			float z = depth + 0.1f;
			float textY = (y + 1) - 2000.0f / depth;
			float textX = x - 1;
			font->DrawTextRaw(CHAR2WCHAR(Name), strlen(Name), textX, textY, z, 16.0f, &LH3DColor(0xff000000), 1, NULL,
			                  0.0f, 4096.0f);
			textY = y - 2000.0f / depth;
			textX = x;
			font->DrawTextRaw(CHAR2WCHAR(Name), strlen(Name), textX, textY, depth, 16.0f, &LH3DColor(Colour), 1, NULL,
			                  0.0f, 4096.0f);
		}
	}
}
