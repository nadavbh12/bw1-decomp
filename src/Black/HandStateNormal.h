#ifndef BW1_DECOMP_HAND_STATE_NORMAL_INCLUDED_H
#define BW1_DECOMP_HAND_STATE_NORMAL_INCLUDED_H

#include <assert.h> /* For static_assert */

#include <Lionhead/LH3DLib/development/LHPoint.h> /* For struct LHPoint */
#include <Lionhead/LH3DLib/development/Zoomer.h>  /* For struct Zoomer, struct Zoomer3d */

#include "HandState.h" /* For struct HandState */

enum HAND_ANIM
{
	HAND_ANIM_POINT = 0x0,
	HAND_ANIM_PICK_UP = 0xc,
	HAND_ANIM_HOLD = 0xf,
	HAND_ANIM_OVER_OBJECT = 0x18,
	HAND_ANIM_CAMERA_GRAB = 0x2a,
	HAND_ANIM_CAMERA_ROTATE = 0x2d,
	HAND_ANIM_CAMERA_PITCH = 0x30,
	HAND_ANIM_CAMERA_ZOOM = 0x33,
};

enum
{
	HAND_ANIM_LEAN_X = 1,
	HAND_ANIM_LEAN_Y = 2,
	HAND_TRICON_ROTATE = 0x1,
	HAND_TRICON_PITCH = 0x2,
	HAND_TRICON_ZOOM = 0x8,
	HAND_PLAYBACK_TRICON_PITCH = 0x1,
	HAND_PLAYBACK_TRICON_ROTATE = 0x2,
	HAND_PLAYBACK_TRICON_ZOOM = 0x4,
	INTERFACE_ACTION_STATE_0x15 = 0x15,
};

// Forward Declares

class CHand;
struct LHMatrix;

class HandStateNormal : public HandState
{
public:
	long AnimationTime; /* 0x8 */
	long Anim;          /* 0xc */

	// BW1W120 00d13f60
	static LHPoint Global_00d13f60;
	// BW1W120 00d13f70
	static LHPoint Global_00d13f70;
	// BW1W120 00d13f80
	static Zoomer Global_00d13f80;
	// BW1W120 00d13fb0
	static Zoomer3d Global_00d13fb0;
	// BW1W120 00d14048
	static int Global_00d14048;
	// BW1W120 00d1404c
	static int Global_00d1404c;
	// BW1W120 00bf01cc
	static int Global_00bf01cc;
	// BW1W120 00bf01d0
	static float Global_00bf01d0;
	// BW1W120 00bf01d4
	static float Global_00bf01d4;
	// BW1W120 00bf01d8
	static float Global_00bf01d8;

	// Override methods

	// BW1W120 005b5d00 BW1M119 01344a80
	virtual void Enter();
	// BW1W120 005b71a0 BW1M119 0107b890
	virtual void Update(float param_1, LHMatrix* param_2);
	// BW1W120 005b5c90 BW1M119 010977d0
	virtual bool AllowCameraTricons() { return true; }

	// Constructors

	// BW1W120 005b5c70 BW1M119 01344b80
	HandStateNormal(CHand* hand);
};
static_assert(sizeof(HandStateNormal) == 0x10, "Data type is of wrong size");

// BW1W120 005b5e70 BW1M119 01065620
void ObtainRequiredHandPosition(CHand* hand, float param_2, float param_3, float& param_4, LHPoint& param_5,
                                LHPoint& param_6, int& param_7, bool param_8, bool param_9);

#endif /* BW1_DECOMP_HAND_STATE_NORMAL_INCLUDED_H */
