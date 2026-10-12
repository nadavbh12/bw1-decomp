#ifndef BW1_DECOMP_CONTROL_HAND_INCLUDED_H
#define BW1_DECOMP_CONTROL_HAND_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stdint.h> /* For uint32_t, uint8_t */

#include <Lionhead/LH3DLib/development/LHCoord.h> /* For struct LHCoord */
#include <Lionhead/LH3DLib/development/LHPoint.h> /* For struct LHPoint */

#include "Morphable.h"       /* For struct Morphable */
#include "PSysProcessInfo.h" /* For struct PSysProcessInfo */
#include "HandState.h"
#include "HandStateInvisible.h"
#include "HandStateNormal.h"
#include "HandStateCamera.h"
#include "HandStateTug.h"
#include "HandStateHolding.h"
#include "HandStateTotem.h"
#include "HandStateMultiPickUp.h"
#include "HandStateCreature.h"
#include "HandStateGrain.h"
#include "HandStatePlayAnim.h"
#include "HandStateCitadel.h"

// Forward Declares

class Creature;
class GInterfaceStatus;
class GameOSFile;
class LH3DCreature;
class LHPlayer;
struct LHSurface;
class Object;
class PSysHandFX;
class PSysInterface;
class SpellSeed;
struct LHMatrix;
class CHand;

class CHand : public Morphable
{
public:
	union State {
		struct Named
		{
			HandStateInvisible*   invisible;
			HandStateNormal*      normal;
			HandStateCamera*      camera;
			HandStateTug*         tug;
			HandStateHolding*     holding;
			HandStateTotem*       totem;
			HandStateMultiPickUp* MultiPickUp;
			HandStateCreature*    creature;
			HandStateGrain*       grain;
			HandStatePlayAnim*    PlayAnim;
			HandStateCitadel*     citadel;
		};
		Named      named;
		HandState* raw[_HAND_STATES_COUNT];
	};
	float             field_0x4834;
	float             MaxDistanceFromUser;
	int               field_0x483c;
	uint32_t          field_0x4840;
	uint32_t          field_0x4844;
	uint32_t          field_0x4848;
	int               field_0x484c;
	GInterfaceStatus* InterfaceStatus; /* 0x4850 */
	float             DistanceFromView;
	bool32_t          field_0x4858;
	LHCoord           ScreenPos;
	LHCoord           PreviousScreenPos; /* 0x4864 */
	LHCoord           field_0x486c;
	uint8_t           field_0x4874;
	uint8_t           field_0x4875;
	uint8_t           field_0x4876;
	uint8_t           field_0x4877;
	uint32_t          CurrentState;
	State             HandStates;
	uint8_t           field_0x48a8;
	uint8_t           field_0x48a9;
	uint8_t           field_0x48aa;
	uint8_t           field_0x48ab;
	uint8_t           field_0x48ac;
	uint8_t           field_0x48ad;
	uint8_t           field_0x48ae;
	uint8_t           field_0x48af;
	uint8_t           field_0x48b0;
	uint8_t           field_0x48b1;
	uint8_t           field_0x48b2;
	uint8_t           field_0x48b3;
	uint32_t          field_0x48b4;
	float             field_0x48b8;
	float             field_0x48bc;
	float             field_0x48c0;
	float             field_0x48c4;
	LHPoint           Point0x48c8;
	LHPoint           Point0x48d4;
	LHPoint           Point0x48e0;
	LHPoint           Point0x48ec;
	int               field_0x48f8;
	Object*           ThrownObject; /* 0x48fc */
	int               PreInteractionTimer;
	Object*           HeldObject; /* 0x4904 */
	bool32_t          field_0x4908;
	uint32_t          field_0x490c;
	uint32_t          field_0x4910;
	uint32_t          field_0x4914;
	int               field_0x4918;
	uint8_t           field_0x491c;
	uint8_t           field_0x491d;
	uint8_t           field_0x491e;
	uint8_t           field_0x491f;
	LHPoint           field_0x4920;
	LHPoint           field_0x492c;
	LHPoint           field_0x4938;
	float             field_0x4944;
	PSysHandFX*       HandFx;
	PSysInterface*    SpellInHand; /* 0x494c */
	SpellSeed*        SpellInHandSeed;
	uint8_t           field_0x4954;
	uint8_t           field_0x4955;
	uint8_t           field_0x4956;
	uint8_t           field_0x4957;
	PSysProcessInfo   SpellProcessInfo; /* 0x4958 */
	uint32_t          field_0x4994;
	uint32_t          field_0x4998;
	LHSurface*        MouseSprite; /* 0x499c */
	bool32_t          field_0x49a0;
	bool32_t          field_0x49a4;
	uint32_t          field_0x49a8;
	LHMatrix*         Matrix0x49ac;
	float             field_0x49b0;
	Creature*         field_0x49b4;
	LH3DCreature*     field_0x49b8;
	Creature*         field_0x49bc;
	bool32_t          field_0x49c0;

	// Override methods

	// BW1W120 0046c4a0 BW1M119 011d0470
	virtual void SetSize(float size);
	// BW1W120 0046bf80 BW1M119 011d0be0
	virtual void MorphTexture();
	// BW1W120 0046c350 BW1M119 011d04f0
	virtual void UpdateTime(int param_1);
	// BW1W120 0046c550 BW1M119 0102c680
	virtual void PrepareForDrawing();
	// BW1W120 0046d0f0 BW1M119 011cfe30
	virtual uint32_t AddForDrawing();
	// BW1W120 0046d2c0 BW1M119 011cf800
	virtual uint32_t LoadBinary(char* filename, int param_1);
	// BW1W120 0046d5f0 BW1M119 011cf660
	virtual uint32_t SaveBinary(char* filename);

	// Constructors

	// BW1W120 0046ba80 BW1M119 011d0ce0
	CHand(LHPoint point, GInterfaceStatus* status);

	// Destructor

	// BW1W120 0046bfd0 BW1M119 011d0a50
	~CHand();

	// Non-virtual methods

	// BW1W120 inlined
	float GetViewDistanceScale() { return field_0x4834; }
	// BW1W120 0046bef0
	void fn_0046BEF0();
	// BW1W120 0046bf20 BW1M119 011d0c80
	void SetMaxDistanceFromUser(float distance);
	// BW1W120 0046bf50 BW1M119 010a0aa0
	float GetMaxDistanceFromUser();
	// BW1W120 0046bf60
	void fn_0046BF60(uint32_t param_1);
	// BW1W120 0046bfa0
	void fn_0046BFA0();
	// BW1W120 0046bfb0 BW1M119 011d0b90
	float GetMaxForce();
	// BW1W120 00c5e640
	static LHPoint HandPosition;
	// BW1W120 00c5e650
	static LHPoint PreviousHandPosition;
	// BW1W120 0046c040 BW1M119 01085c70
	static float GetStandardSize();
	// BW1W120 0046c050 BW1M119 011d0860
	void StartFixedPosAnimation(LHPoint& pos, long anim);
	// BW1W120 0046c080 BW1M119 011d07e0
	void GiveToCreature(Creature* creature);
	// BW1W120 0046c0b0 BW1M119 011d0790
	void EndGiveToCreature();
	// BW1W120 0046c0d0 BW1M119 010812d0
	void SetDistanceFromView(float distance);
	// BW1W120 0046c1b0 BW1M119 011d0600
	void Show(bool32_t param_1);
	// BW1W120 0046c250
	void fn_0046C250();
	// BW1W120 0046c260 BW1M119 011d0550
	void ToggleLeftRight();
	// BW1W120 0046c2e0
	void fn_0046C2E0(bool32_t param_1);
	// BW1W120 0046c380
	void fn_0046C380();
	// BW1W120 0046c500 BW1M119 011d0400
	void BlendToNewPosition();
	// BW1W120 0046cb60 BW1M119 010211c0
	void OperateInteractionCamera(long hand_state);
	// BW1W120 0046cd10 BW1M119 010240f0
	long GetRequiredState();
	// BW1W120 0046cf80 BW1M119 01023510
	long GetActualState(long state);
	// BW1W120 0046d080 BW1M119 0101cde0
	void SetUpPositionMatrix(LHMatrix& matrix);
	// BW1W120 0046d100 BW1M119 01024a70
	void AddDrawing();
	// BW1W120 0046d210 BW1M119 01029500
	void Draw();
	// BW1W120 0046d740
	CAnim* fn_0046D740(long anim);
	// BW1W120 0046d760 BW1M119 011cf200
	void DoPassiveModeDraw();
	// BW1W120 0046daf0 BW1M119 011cf0d0
	void PrepareMouseSprite();
	// BW1W120 0046db70 BW1M119 011cefc0
	float GetDistanceFromObject(Object* object);
	// BW1W120 0046dc30 BW1M119 011ced50
	void PickUpObject(Object* object, bool32_t param_2);
	// BW1W120 0046ddd0 BW1M119 011cec70
	void ThrowObject();
	// BW1W120 0046de40 BW1M119 011ceb10
	void SwitchObject(Object* object);
	// BW1W120 0046df00 BW1M119 0108f950
	void DrawTheHeldObject();
	// BW1W120 0046df60 BW1M119 01030120
	float WorkOutStandardDistanceFromView(bool32_t param_1);
	// BW1W120 0046e160
	void fn_0046E160(LHMatrix& matrix, const LHPoint& pos, const LHPoint& dir);
	// BW1W120 0046e2f0
	void fn_0046E2F0(LHMatrix& matrix, const LHPoint& pos, const LHPoint& dir);
	// BW1W120 0046e480 BW1M119 011ce710
	bool32_t HasPluckedObject() const;
	// BW1W120 0046e490 BW1M119 01016be0
	void UpdateHeldObject();
	// BW1W120 0046e4b0 BW1M119 010953b0
	Creature* GetCreatureToGiveTo();
	// BW1W120 0046e4e0 BW1M119 010841c0
	void GameTurnUpdate();
	// BW1W120 0046e590 BW1M119 011ce2e0
	void SetHeldObject(Object* object);
	// BW1W120 0046e5d0 BW1M119 0108e800
	HandStateGrain* GetHandStateGrain();
	// BW1W120 0046e5f0 BW1M119 011cdda0
	void UpdateLeftRightFromPlayer(LHPlayer* player);
	// BW1W120 0046e630 BW1M119 011cdd60
	void ReadRegistrySettings();
	// BW1W120 0046e640
	void fn_0046E640();
	// BW1W120 0046e680 BW1M119 01014280
	void DrawSpellInHand();
	// BW1W120 0046e780 BW1M119 011cd4e0
	void SpellInHandStop();
	// BW1W120 0046e7b0 BW1M119 011cd560
	void SpellInHandStart(const SpellSeed* seed);
	// BW1W120 0046e890
	void fn_0046E890();
	// BW1W120 0046e8c0 BW1M119 011cd410
	void OnClearMap();
	// BW1W120 0046e930 BW1M119 011cd140
	uint32_t Load(GameOSFile& file);
	// BW1W120 0046eb10 BW1M119 011cce70
	uint32_t Save(GameOSFile& file);
	// BW1W120 0046ecf0 BW1M119 011ccd60
	void ResolveLoad();
};
static_assert(sizeof(CHand) == 0x49c4, "Data type is of wrong size");

#endif /* BW1_DECOMP_CONTROL_HAND_INCLUDED_H */
