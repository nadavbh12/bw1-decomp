#ifndef BW1_DECOMP_HAND_STATE_CAMERA_INCLUDED_H
#define BW1_DECOMP_HAND_STATE_CAMERA_INCLUDED_H

#include <assert.h> /* For static_assert */

#include <Lionhead/LH3DLib/development/Zoomer.h> /* For struct Zoomer, struct Zoomer3d */

#include "HandState.h" /* For struct HandState */

// Forward Declares

class CHand;
struct LHMatrix;

class HandStateCamera : public HandState
{
public:
	long     AnimationTime; /* 0x8 */
	int      field_0xc;
	int      field_0x10;
	long     Anim;       /* 0x14 */
	Zoomer3d field_0x18; /* 0x18 */
	Zoomer   field_0xa8; /* 0xa8 */

	// Override methods

	// BW1W120 005b02f0 BW1M119 0133f280
	virtual void Enter();
	// BW1W120 005b03e0 BW1M119 0133e8f0
	virtual void Update(float param_1, LHMatrix* param_2);
	// BW1W120 005b02e0 BW1M119 0133e8a0
	virtual bool AllowCameraTricons() { return true; }

	// Constructors

	// BW1W120 005b02b0 BW1M119 0133f360
	HandStateCamera(CHand* hand);
};
static_assert(sizeof(HandStateCamera) == 0xd8, "Data type is of wrong size");

#endif /* BW1_DECOMP_HAND_STATE_CAMERA_INCLUDED_H */
