#ifndef BW1_DECOMP_HAND_STATE_CITADEL_INCLUDED_H
#define BW1_DECOMP_HAND_STATE_CITADEL_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stdint.h> /* For uint32_t */

#include <Lionhead/LH3DLib/development/Zoomer.h> /* For struct Zoomer, struct Zoomer3d */

#include "HandState.h" /* For struct HandState */

// Forward Declares

class CHand;
struct LHMatrix;

class HandStateCitadel : public HandState
{
public:
	long     Frame;
	uint32_t field_0xc;
	Zoomer3d Direction; /* 0x10 */
	Zoomer   Distance;  /* 0xa0 */

	// Override methods

	// BW1W120 005b0c10 BW1M119 0133fbc0
	virtual void Enter();
	// BW1W120 005b0d00 BW1M119 0133f400
	virtual void Update(float param_1, LHMatrix* param_2);

	// Constructors

	// BW1W120 005b0bf0 BW1M119 0133fc90
	HandStateCitadel(CHand* hand);
};

static_assert(sizeof(HandStateCitadel) == 0xd0, "Data type is of wrong size");

#endif /* BW1_DECOMP_HAND_STATE_CITADEL_INCLUDED_H */
