#ifndef BW1_DECOMP_HAND_STATE_PLAY_ANIM_INCLUDED_H
#define BW1_DECOMP_HAND_STATE_PLAY_ANIM_INCLUDED_H

#include <assert.h> /* For static_assert */

#include <Lionhead/LH3DLib/development/LHPoint.h> /* For struct LHPoint */

#include "HandState.h" /* For struct HandState */

// Forward Declares

class CHand;
struct LHMatrix;

class HandStatePlayAnim : public HandState
{
public:
	LHPoint Position; /* 0x8 */
	long    Anim;     /* 0x14 */
	long    Time;     /* 0x18 */
	int     field_0x1c;

	// Override methods

	// BW1W120 005b7510 BW1M119 01344df0
	virtual void Enter();
	// BW1W120 005b7520 BW1M119 01344c50
	virtual void Update(float param_1, LHMatrix* param_2);

	// Constructors

	// BW1W120 005b74f0 BW1M119 01344e30
	HandStatePlayAnim(CHand* hand);
};
static_assert(sizeof(HandStatePlayAnim) == 0x20, "Data type is of wrong size");

#endif /* BW1_DECOMP_HAND_STATE_PLAY_ANIM_INCLUDED_H */
