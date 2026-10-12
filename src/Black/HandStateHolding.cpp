const float HoldingHandMaxOffsetX = 80.0f;
const float HoldingHandMaxOffsetY = 80.0f;

#include "CreatureAttitudeConstants.h"
#include "GameTimeConstants.h"
#include "HandStateHolding.h"

#include "ColourConstants.h" /* For White */

const float HoldingHandPointHeight = 0.2f;
const float HoldingHandMinHeight = 1.9f;

#include <math.h>    /* For sqrt */
#include <stdlib.h>  /* For abs */
#include <windows.h> /* For min */

#include <Lionhead/LH3DLib/development/LH3DAnim.h>   /* For LH3DAnim::FinishTransform */
#include <Lionhead/LH3DLib/development/LH3DIsland.h> /* For LH3DIsland::GetAltitude */
#include <Lionhead/LH3DLib/development/LH3DMath.h>   /* For PI_F, heading_from_direction_vector */
#include <Lionhead/LH3DLib/development/LH3DObject.h> /* For class LH3DObject */
#include <Lionhead/LH3DLib/development/LH3DTech.h>   /* For LH3DTech */
#include <Lionhead/LH3DLib/development/LHMatrix.h>   /* For struct LHMatrix */

#include "Camera.h"          /* For class GCamera */
#include "ControlHand.h"     /* For class CHand */
#include "Creature.h"        /* For class Creature */
#include "Creature3D.h"      /* For class LH3DCreature */
#include "Game.h"            /* For GGame */
#include "HandStateNormal.h" /* For ObtainRequiredHandPosition */
#include "Interface.h"       /* For class GInterface */
#include "InterfaceStatus.h" /* For class GInterfaceStatus */
#include "Landscape.h"       /* For GLandscape */
#include "Living.h"          /* For class Living */
#include "Object.h"          /* For class Object */
#include "Game3DObject.h"    /* For class Game3DObject */
#include "Packet.h"          /* For enum PACKET_TYPE */
#include "PhysicsObject.h"   /* For PhysicsObject::PredictionActive */

#define HOLDING_GIVE_TO_CREATURE_TIME  1100
#define HOLDING_GIVE_TO_CREATURE_DELAY 300
#define HOLDING_CREATURE_PICK_UP_TIME  500
#define HOLDING_CREATURE_PICK_UP_REACH 7.5f
#define HOLDING_SPRING_STEP_MS         10
#define HOLDING_SPRING_STEP            0.01f
#define HOLDING_TILT_TIME              0.4f
#define HOLDING_HAND_TURN_RANGE        0.3f
#define HOLDING_OBJECT_SCALE_RANGE     1.2f
#define HOLDING_VELOCITY_LOOKAHEAD     0.1f
#define HOLDING_ROOTED_HEIGHT_SCALE    0.1f
#define HOLDING_HAND_HEIGHT_SCALE      0.6f

enum
{
	HOLDING_ANIM_HOLD = 0,
	HOLDING_ANIM_POINT = 6,
	HOLDING_ANIM_MULTI_PICK_UP = 9,
	HOLDING_ANIM_HOLD_LARGE = 0x3c
};

enum
{
	HOLDING_STATE_POINT = 1,
	HOLDING_STATE_PINCH = 2,
	HOLDING_STATE_GRAB = 3,
	HOLDING_STATE_NONE = 4,
	HOLDING_STATE_MULTI_GRAB = 7
};

LHPoint  HandStateHolding::Global_00d13f20;
LHPoint  HandStateHolding::Global_00d13f30;
int      HandStateHolding::GiveToCreatureTimeLeft;
float    HandStateHolding::Global_00d13f4c;
uint32_t HandStateHolding::SpringTargetTime;
uint32_t HandStateHolding::SpringTime;
float    HandStateHolding::SpringStiffness = 260.0f;
float    HandStateHolding::SpringDamping = 40.0f;

inline void GCamera::GetPosition(LHPoint& pos)
{
	pos = LH3DTech::g_camera.pos;
}

bool HandStateHolding::AllowCameraTricons()
{
	Object* interact = GGame::g_game->MyInterface()->status->field_0x3c;
	return interact == NULL || !interact->IsLockedInInteract();
}

HandStateHolding::HandStateHolding(CHand* hand) : HandState(hand)
{
	GiveToCreatureTimeLeft = 0;
	AnimationTime = 0;
	UnusedClearedOnCreate = 0;
}

float HandStateHolding::GetTiltForRender()
{
	return 0.0f;
}

float HandStateHolding::GetRaiseForRender()
{
	return 0.0f;
}

void HandStateHolding::OverrideHandPos(LHPoint* pos) {}

void HandStateHolding::Enter()
{
	UpZoomer.SetPosition(LHPoint(0.0f, 1.0f, 0.0f));
	ThrowVelocity.x = ThrowVelocity.y = ThrowVelocity.z = 0.0f;
	InThrow = false;
	AnimationTime = 0;
	DistanceFromViewZoomer.SetPosition(hand->DistanceFromView);
	RotationZoomer.SetPosition(0.0f);
	GiveToCreatureTimeLeft = 0;
	Global_00d13f20.x = Global_00d13f20.y = Global_00d13f20.z = 0.0f;

	if (hand->HeldObject != NULL)
		hand->field_0x4944 = hand->HeldObject->GetHoldLoweringMultiplier() * hand->HeldObject->GetHeight();
}

bool32_t HandStateHolding::CanCurrentlyGiveToCreature()
{
	return GiveToCreatureTimeLeft > 0;
}

void HandStateHolding::Update(float param_1, LHMatrix* param_2)
{
	LHPoint cameraPos;
	GGame::g_game->GetCamera()->GetPosition(cameraPos);

	LH3DObject* object3d = hand->HeldObject != NULL ? hand->HeldObject->Game3dObject : NULL;

	LHPoint pickUpPos(0.0f, 0.0f, 0.0f);
	float   distance;
	if ((GiveToCreatureTimeLeft > HOLDING_GIVE_TO_CREATURE_DELAY || hand->field_0x49bc != NULL) &&
	    hand->field_0x49b8 != NULL)
	{
		LHPoint direction = cameraPos - hand->field_0x49b8->position;
		hand->field_0x49b8->GetPickUpFromHandPosition(&pickUpPos, true, heading_from_direction_vector(direction));
		distance = pickUpPos.GetDistance(cameraPos);
	}
	else
	{
		distance = hand->WorkOutStandardDistanceFromView(false);
	}
	hand->SetDistanceFromView(distance);

	GInterface* iface = GGame::g_game->MyInterface();
	Object*     collideObject = iface->interface_collide.object;
	Object*     interact = iface->status->field_0x3c;
	GLandscape::Global_00d20178 = hand->field_0x490c == HOLDING_STATE_POINT;

	float height;
	switch (hand->field_0x490c)
	{
	case HOLDING_STATE_POINT:
		height = HoldingHandPointHeight;
		break;
	case HOLDING_STATE_PINCH:
		height = CHand::GetStandardSize() * hand->field_0x4834;
		break;
	case HOLDING_STATE_GRAB:
	case 5:
	case 6:
	case HOLDING_STATE_MULTI_GRAB:
		height = hand->field_0x4944 > HoldingHandMinHeight ? hand->field_0x4944 : HoldingHandMinHeight;
		break;
	}

	float holdHeight = height;
	if (hand->HeldObject != NULL && hand->HeldObject->IsARootedObject())
		holdHeight = hand->HeldObject->GetHeight() * HOLDING_ROOTED_HEIGHT_SCALE + height;
	Global_00d13f4c = holdHeight * HOLDING_HAND_HEIGHT_SCALE;

	LHPoint pos;
	float   raise;
	if (interact != NULL && interact->IsLockedInInteract())
	{
		pos = hand->position;
		CHand::PreviousHandPosition = CHand::HandPosition;
		pos.y = LH3DIsland::GetAltitude(pos);
		raise = interact->GetHeightForHandAboveInteractObject();
	}
	else
	{
		bool    lockedToCreature = hand->field_0x490c == HOLDING_STATE_PINCH || hand->field_0x49b4 != NULL;
		LHPoint handDirection;
		int     anim;
		ObtainRequiredHandPosition(hand, param_1, GetTiltForRender(), height, pos, handDirection, anim, false,
		                           lockedToCreature);
		CHand::PreviousHandPosition = handDirection;
		OverrideHandPos(&pos);
		raise = GetRaiseForRender();
	}
	pos.y += raise;

	if (hand->field_0x49bc != NULL)
	{
		LH3DCreature* creature3d = hand->field_0x49bc->GetCreature3D();
		if (creature3d->MoveState == 15)
		{
			int time = abs((long)(creature3d->AnimTimeInc * GGame::g_game->TurnFraction + creature3d->CycleTime[0] -
			                      creature3d->field_0x4a38));
			if (time < HOLDING_CREATURE_PICK_UP_TIME)
			{
				float fraction = time / (float)HOLDING_CREATURE_PICK_UP_TIME;
				pos = pickUpPos + (pos - pickUpPos) * fraction;
				pos.y -= (1.0f - fraction) * hand->field_0x4944;
			}
		}
	}

	if (dynamic_cast<Living*>(collideObject) != NULL)
	{
		Creature*         creature = dynamic_cast<Creature*>(collideObject);
		GInterfaceStatus* status = GGame::g_game->MyInterface()->status;
		if (creature != NULL && hand->HeldObject != NULL &&
		    creature->InterfaceValidToGiveObject(status, hand->HeldObject) == 1)
		{
			if (hand->field_0x49b4 != creature)
			{
				hand->field_0x49b4 = creature;
				hand->field_0x49b8 = hand->field_0x49b4->GetCreature3D();
			}
			GiveToCreatureTimeLeft = HOLDING_GIVE_TO_CREATURE_TIME;
		}
	}

	if (GiveToCreatureTimeLeft > 0 && hand->field_0x49bc == NULL)
	{
		GiveToCreatureTimeLeft -= LH3DTech::g_delta_time;
		if (GiveToCreatureTimeLeft < 0)
		{
			GiveToCreatureTimeLeft = 0;
			hand->field_0x49b4 = NULL;
			hand->field_0x49b8 = NULL;
		}
	}

	if (hand->field_0x49bc != NULL && hand->field_0x49b8 != NULL)
	{
		if (hand->field_0x49b8->GetSize() * HOLDING_CREATURE_PICK_UP_REACH < pickUpPos.GetDistance(hand->position))
		{
			GGame::g_game->SetPacket(PACKET_TYPE_0x52, hand->field_0x49b8->creature->GetIndexOffset());
		}
	}

	if (GiveToCreatureTimeLeft < HOLDING_GIVE_TO_CREATURE_DELAY && hand->field_0x49bc == NULL)
	{
		RotationZoomer.SetDestination(0.0f, HOLDING_TILT_TIME);
	}
	else
	{
		switch (hand->field_0x490c)
		{
		case HOLDING_STATE_POINT:
			RotationZoomer.SetDestination(hand->field_0x484c ? -PI_F : PI_F, HOLDING_TILT_TIME);
			break;
		case 5:
		case 6:
		case HOLDING_STATE_MULTI_GRAB:
			RotationZoomer.SetDestination((hand->field_0x484c ? -PI_F : PI_F) * 0.5f, HOLDING_TILT_TIME);
			break;
		}
	}
	RotationZoomer.Update(param_1);

	Global_00d13f30.y += Global_00d13f4c;
	pos.y += holdHeight;
	SpringTargetTime += GGame::g_game->TimeInc;

	if (InThrow && !(hand->HeldObject != NULL && hand->HeldObject->IsSpellSeed()))
	{
		do
		{
			SpringTime += HOLDING_SPRING_STEP_MS;
			LHPoint stretch = (pos - hand->position) * SpringStiffness;
			LHPoint damping = ThrowVelocity * SpringDamping;
			ThrowVelocity += (stretch - damping) * HOLDING_SPRING_STEP;
			float speed = ThrowVelocity.GetNorme();
			if (speed > PhysicsObject::MaxSpeed)
				ThrowVelocity *= PhysicsObject::MaxSpeed / speed;
			hand->position += ThrowVelocity * HOLDING_SPRING_STEP;
		} while (SpringTime < SpringTargetTime);
	}
	else
	{
		hand->position = pos;
		if (GGame::g_game->MyInterface()->HandState.hand_state == HAND_STATE_IN_THROW &&
		    !(hand->HeldObject != NULL && hand->HeldObject->IsSpellSeed()))
		{
			InThrow = true;
			hand->position = pos;
			ThrowVelocity.SetNull();
		}
	}

	Global_00d13f20.Set(0.0f, -hand->field_0x4944, 0.0f);
	hand->field_0x492c = LHPoint(pos.x, pos.y + Global_00d13f20.y, pos.z);
	if (InThrow)
	{
		hand->Point0x48c8 = ThrowVelocity;
		hand->Point0x48d4.SetNull();
	}

	if (hand->HeldObject == NULL)
	{
		LHPoint toCamera = cameraPos - hand->position;
		hand->SetDistanceFromView(toCamera.Normalise());

		CAnim*   anim = hand->GetAnim(HOLDING_ANIM_HOLD, 0);
		LHMatrix matrix;
		hand->fn_0046E160(matrix, hand->position, LHPoint(0.0f, 1.0f, 0.0f));
		anim->FillBuffer(param_2, hand->GetMesh(), hand->field_0x47f4, hand->field_0x47f8, AnimationTime, hand->frame,
		                 NULL, 1);
		LH3DAnim::FinishTransform(param_2, hand->GetMesh(), matrix);
		hand->field_0x486c = hand->ScreenPos;
		return;
	}

	Global_00d13f20.Set(0.0f, -1.0f, 0.0f);
	object3d->matrix.GetVectorY().Set(0.0f, 1.0f, 0.0f);

	float dx = hand->field_0x48b8 - hand->ScreenPos.x;
	float offsetX = dx > -HoldingHandMaxOffsetX ? (dx < HoldingHandMaxOffsetX ? dx : HoldingHandMaxOffsetX)
	                                            : -HoldingHandMaxOffsetX;
	float turnX = (HOLDING_HAND_TURN_RANGE / HoldingHandMaxOffsetX) * offsetX;

	LHPoint toCamera = cameraPos - hand->position;
	hand->SetDistanceFromView(toCamera.Normalise());

	LHMatrix rotation;
	rotation.SetFromVectorAndAngle(toCamera, GetTiltForRender() + RotationZoomer.GetCurrentValue() + turnX);

	float dy = hand->ScreenPos.y - hand->field_0x48bc;
	float offsetY = dy > -HoldingHandMaxOffsetY ? (dy < HoldingHandMaxOffsetY ? dy : HoldingHandMaxOffsetY)
	                                            : -HoldingHandMaxOffsetY;
	float turnY = (HOLDING_HAND_TURN_RANGE / HoldingHandMaxOffsetY) * offsetY;

	LHPoint side;
	side.CrossProduct(toCamera, LHPoint(0.0f, 1.0f, 0.0f));
	side.FastNormalizeInline();
	LHMatrix pitch;
	pitch.SetFromVectorAndAngle(side, turnY);
	rotation.PostMultiply(pitch);

	LHMatrix& objectMatrix = object3d->matrix;
	objectMatrix.GetVectorY() = rotation * objectMatrix.GetVectorY();
	LHPoint up = objectMatrix.GetVectorY();
	LHPoint handPos = hand->position;
	hand->fn_0046E2F0(objectMatrix, handPos, up);
	objectMatrix.RotateY(hand->HeldObject->GetHoldYRotate());

	rotation = objectMatrix;
	rotation.NormaliseMatrixOnly();
	rotation._41 = 0.0f;
	rotation._42 = 0.0f;
	rotation._43 = 0.0f;

	float scale = object3d->scale;
	objectMatrix.GetVectorX().SetSize(scale);
	objectMatrix.GetVectorY().SetSize(scale);
	objectMatrix.GetVectorZ().SetSize(scale);

	LHPoint handUp = objectMatrix.GetVectorY();
	handUp.FastNormalizeInline();

	float  standardSize = CHand::GetStandardSize();
	float  fraction;
	CAnim* anim;
	switch (hand->field_0x490c)
	{
	case HOLDING_STATE_POINT:
		fraction = min(1.0f, hand->HeldObject->GetHoldRadius() /
		                         (standardSize * hand->field_0x4834 * HOLDING_OBJECT_SCALE_RANGE));
		hand->field_0x4944 = hand->HeldObject->GetHoldLoweringMultiplier() * hand->HeldObject->GetHeight();
		anim = hand->GetAnim(HOLDING_ANIM_POINT, 0);
		Global_00d13f20.SetSize(hand->field_0x4944);
		Global_00d13f20 = rotation * Global_00d13f20;
		objectMatrix.SetTranslateOnly(Global_00d13f20 + hand->position);
		AnimationTime = (long)(anim->Duration * 0.5f * (1.0f - fraction));
		break;
	case HOLDING_STATE_PINCH:
		hand->field_0x4944 = hand->HeldObject->GetHoldLoweringMultiplier() * hand->HeldObject->GetHeight();
		Global_00d13f20.SetNull();
		objectMatrix.SetTranslateOnly(hand->position);
		anim = hand->GetAnim(HOLDING_ANIM_HOLD, 0);
		AnimationTime = anim->Duration >> 1;
		break;
	case HOLDING_STATE_GRAB:
		fraction = min(1.0f, hand->HeldObject->GetHoldRadius() / (standardSize * hand->field_0x4834));
		hand->field_0x4944 = hand->HeldObject->GetHoldLoweringMultiplier() * hand->HeldObject->GetHeight();
		anim = hand->GetAnim(HOLDING_ANIM_HOLD_LARGE, 0);
		Global_00d13f20.SetSize(hand->field_0x4944);
		Global_00d13f20 = rotation * Global_00d13f20;
		objectMatrix.SetTranslateOnly(Global_00d13f20 + hand->position);
		AnimationTime = (long)((anim->Duration >> 1) * fraction);
		break;
	case 5:
	case 6:
	case HOLDING_STATE_MULTI_GRAB:
		fraction = min(1.0f, hand->HeldObject->GetHoldRadius() / (standardSize * hand->field_0x4834));
		hand->field_0x4944 = hand->HeldObject->GetHoldLoweringMultiplier() * hand->HeldObject->GetHeight();
		anim = hand->GetAnim(HOLDING_ANIM_MULTI_PICK_UP, 0);
		Global_00d13f20.SetSize(hand->field_0x4944);
		Global_00d13f20 = rotation * Global_00d13f20;
		objectMatrix.SetTranslateOnly(Global_00d13f20 + hand->position);
		AnimationTime = (long)((anim->Duration >> 1) * fraction);
		break;
	}

	hand->Point0x48e0 = ThrowVelocity * HOLDING_VELOCITY_LOOKAHEAD + objectMatrix.GetPos();
	objectMatrix.GetYXZ(&hand->Point0x48ec.y, &hand->Point0x48ec.x, &hand->Point0x48ec.z);

	LHPoint  position = hand->position;
	LHMatrix matrix;
	hand->fn_0046E160(matrix, position, handUp);
	anim->FillBuffer(hand->TransformedMatrices, hand->GetMesh(), hand->field_0x47f4, hand->field_0x47f8, AnimationTime,
	                 hand->frame, NULL, 1);
	LH3DAnim::FinishTransform(hand->TransformedMatrices, hand->GetMesh(), matrix);
	hand->field_0x486c = hand->ScreenPos;
}

void HandStateHolding::DrawTheHeldObject()
{
	if (!PhysicsObject::PredictionActive || hand->HeldObject != PhysicsObject::PredictionObject)
		hand->HeldObject->DrawOutOfMap(false);
}
