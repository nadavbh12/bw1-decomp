#ifndef BW1_DECOMP_HAND_STATE_TUG_INCLUDED_H
#define BW1_DECOMP_HAND_STATE_TUG_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stdint.h> /* For uintptr_t */

#include <re_common.h>                             /* For bool32_t */
#include <Lionhead/LH3DLib/development/LHMatrix.h> /* For struct LHMatrix */
#include <Lionhead/LH3DLib/development/LHPoint.h>  /* For struct LHPoint */
#include <Lionhead/LH3DLib/development/Zoomer.h>   /* For struct Zoomer */

#include "HandState.h" /* For struct HandState */

// Forward Declares

class CHand;

class HandStateTug : public HandState
{
public:
	uintptr_t TugPosition;
	bool32_t  field_0xc;
	bool32_t  field_0x10;
	Zoomer3d  field_0x14;
	LHPoint   field_0xa4;
	LHPoint   field_0xb0;
	LHMatrix  field_0xbc;
	LHMatrix  field_0xec;
	Zoomer    field_0x11c;

	// Override methods

	// BW1W120 005b7df0 BW1M119 01346a30
	virtual void Enter();
	// BW1W120 005b8e40 BW1M119 013458c0
	virtual void DrawTheHeldObject();
	// BW1W120 005b8070 BW1M119 01345930
	virtual void Update(float dt, LHMatrix* matrices);

	// Constructors

	// BW1W120 005b7dd0 BW1M119 01346cd0
	HandStateTug(CHand* hand);

	// Non-virtual methods

	// BW1W120 005b8e60 BW1M119 01345830
	void UpdateHeldObject();
};
static_assert(sizeof(HandStateTug) == 0x14c, "Data type is of wrong size");

#endif /* BW1_DECOMP_HAND_STATE_TUG_INCLUDED_H */
