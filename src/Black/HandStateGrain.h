#ifndef BW1_DECOMP_HAND_STATE_GRAIN_INCLUDED_H
#define BW1_DECOMP_HAND_STATE_GRAIN_INCLUDED_H

#include <assert.h> /* For static_assert */

#include <re_common.h>                            /* For bool32_t */
#include <Lionhead/LH3DLib/development/LHPoint.h> /* For struct LHPoint */
#include <Lionhead/LH3DLib/development/Zoomer.h>  /* For struct Zoomer3d */

#include "HandStateHolding.h"     /* For struct HandStateHolding */
#include "GJBaseUtils.h"          /* For class GJLowPassFilterVariableStep */
#include "KPSplineInterpolator.h" /* For class KPSplineInterpolator */

// Forward Declares

class CHand;
class Creature;
struct LHMatrix;

enum POUR_TYPE
{
	POUR_TYPE_REPEAT = 1
};

class HandStateGrain : public HandStateHolding
{
public:
	float                                Tilt; /* 0x114 */
	float                                Raise;
	float                                PreviousTilt;
	float                                PreviousRaise;
	bool                                 Pouring; /* 0x124 */
	POUR_TYPE                            PourType;
	float                                PourTime;
	float                                PourDuration;
	float                                RaiseScale; /* 0x134 */
	float                                TiltScale;
	Creature*                            PourCreature;
	KPSplineInterpolator<float>          PourCurve; /* 0x140 */
	int                                  field_0x14c;
	Zoomer3d                             field_0x150;
	LHPoint                              field_0x1e0;
	LHPoint                              field_0x1ec;
	bool32_t                             field_0x1f8;
	GJLowPassFilterVariableStep<LHPoint> field_0x1fc;
	GJLowPassFilterVariableStep<float>   field_0x210;
	GJLowPassFilterVariableStep<float>   field_0x21c;

	// Static data

	// BW1W120 00bf014c
	static float PositionFilterFactor;
	// BW1W120 00bf0150
	static float AngleFilterFactor;
	// BW1W120 00d13f00
	static LHPoint field_0x00d13f00;
	// BW1W120 00d13f1c
	static bool field_0x00d13f1c;

	// Override methods

	// BW1W120 005b3080 BW1M119 01341a90
	virtual void Enter();
	// BW1W120 005b3290 BW1M119 01341a50
	virtual void Exit();
	// BW1W120 005b3310 BW1M119 01341520
	virtual void Update(float dt, LHMatrix* matrices);
	// BW1W120 005b2d50 BW1M119 013420f0
	virtual float GetTiltForRender();
	// BW1W120 005b2d30 BW1M119 01342150
	virtual float GetRaiseForRender();
	// BW1W120 005b32a0 BW1M119 01341930
	virtual void OverrideHandPos(LHPoint* pos);

	// Static methods

	// BW1W120 005b2d70 BW1M119 01084140
	static void Process(float dt);
	// BW1W120 005b2f70 BW1M119 01341e30
	static void StartTimedPour(bool param_1, float duration, float raise_scale, float tilt_scale, POUR_TYPE type);
	// BW1W120 005b2fb0 BW1M119 01341d40
	static void StartTimedPour(Creature* creature, float duration, float raise_scale, float tilt_scale, POUR_TYPE type);
	// BW1W120 005b2f10 BW1M119 01341fb0
	static void EndTimedPour();

	// Constructors

	// BW1W120 005b2b80 BW1M119 013421b0
	HandStateGrain(CHand* hand);

	// Non-virtual methods

	// BW1W120 005b2da0 BW1M119 0108e430
	void PProcess(float dt);
	// BW1W120 005b3000 BW1M119 01341cb0
	void PStartTimedPour(bool param_1, float duration, float raise_scale, float tilt_scale, POUR_TYPE type);
	// BW1W120 005b2f40 BW1M119 01341f10
	void PEndTimedPour();
	// BW1W120 005b3070 BW1M119 01341be0
	void PSetCtr(Creature* creature);
};
static_assert(sizeof(HandStateGrain) == 0x228, "Data type is of wrong size");

#endif /* BW1_DECOMP_HAND_STATE_GRAIN_INCLUDED_H */
