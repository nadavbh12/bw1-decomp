#ifndef BW1_DECOMP_HAND_STATE_CREATURE_INCLUDED_H
#define BW1_DECOMP_HAND_STATE_CREATURE_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stdint.h> /* For uint8_t */

#include <re_common.h>                                      /* For bool32_t */
#include <Lionhead/LH3DLib/development/LH3DMeshIntersect.h> /* For struct MeshIntersect */
#include <Lionhead/LH3DLib/development/Zoomer.h>            /* For struct Zoomer, struct Zoomer3d */

#include "HandState.h" /* For struct HandState */

// Forward Declares

class CHand;
struct LHMatrix;

enum HAND_STATE_CREATURE_BONE
{
	HAND_STATE_CREATURE_BONE_HEAD,
	HAND_STATE_CREATURE_BONE_RIGHT_ARMPIT,
	HAND_STATE_CREATURE_BONE_LEFT_ARMPIT,
	HAND_STATE_CREATURE_BONE_BELLY,
	HAND_STATE_CREATURE_BONE_ANUS,
	HAND_STATE_CREATURE_BONE_RIGHT_FOOT,
	HAND_STATE_CREATURE_BONE_LEFT_FOOT,
	HAND_STATE_CREATURE_BONE_RIGHT_HAND,
	HAND_STATE_CREATURE_BONE_LEFT_HAND,
	HAND_STATE_CREATURE_BONE_COUNT
};

struct HandStateCreaturePacket
{
	MeshIntersect Intersect;
	int           field_0x28;
	int           field_0x2c;
};

class HandStateCreature : public HandState
{
public:
	int      field_0x8;
	Zoomer3d field_0xc;
	Zoomer   field_0x9c;
	int      field_0xcc;
	int      field_0xd0;
	int      field_0xd4;
	float    HandValue; /* 0xd8 */
	int      field_0xdc;
	int      field_0xe0;
	long     Bones[HAND_STATE_CREATURE_BONE_COUNT];
	int      field_0x108;
	int      field_0x10c;
	int      field_0x110;
	int      field_0x114;
	int      field_0x118;
	int      field_0x11c;

	// Static data

	// BW1W120 00bf00e0
	static long BoneActions[HAND_STATE_CREATURE_BONE_COUNT];
	// BW1W120 00bf0104
	static bool32_t BoneActionMirrored[HAND_STATE_CREATURE_BONE_COUNT];
	// BW1W120 00bf0128
	static long BoneFacialActions[HAND_STATE_CREATURE_BONE_COUNT];

	// Override methods

	// BW1W120 005b13f0 BW1M119 013411c0
	virtual void Enter();
	// BW1W120 005b2a80 BW1M119 0133fd30
	virtual void Exit();
	// BW1W120 005b17b0 BW1M119 0133fd90
	virtual void Update(float dt, LHMatrix* matrices);

	// Constructors

	// BW1W120 005b13d0 BW1M119 01341490
	HandStateCreature(CHand* hand);
};
static_assert(sizeof(HandStateCreature) == 0x120, "Data type is of wrong size");

#endif /* BW1_DECOMP_HAND_STATE_CREATURE_INCLUDED_H */
