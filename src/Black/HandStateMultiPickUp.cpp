#include "HandStateMultiPickUp.h"

#include <limits.h>  /* For SHRT_MIN */
#include <math.h>    /* For atan */
#include <windows.h> /* For min */

#include <Lionhead/LH3DLib/development/LH3DIsland.h> /* For LH3DIsland::GetAltitude */
#include <Lionhead/LH3DLib/development/LH3DMath.h>   /* For PI_F */
#include <Lionhead/LH3DLib/development/LHMatrix.h>   /* For struct LHMatrix */
#include <Lionhead/LHLib/ver5.0/LHSystem.h>          /* For LHSys */

#include "ControlHand.h"     /* For class CHand */
#include "Game.h"            /* For GGame */
#include "Interface.h"       /* For class GInterface */
#include "InterfaceStatus.h" /* For class GInterfaceStatus */
#include "Object.h"          /* For class Object */
#include "Utils.h"           /* For GUtils */

#define MULTI_PICK_UP_HAND_ANGLE  ((PI_F * 7) / 16)
#define MULTI_PICK_UP_HAND_HEIGHT 2.5f

int HandStateMultiPickUp::HeightAboveObject = 3;

HandStateMultiPickUp::HandStateMultiPickUp(CHand* hand) : HandState(hand) {}

void HandStateMultiPickUp::Enter()
{
	Object* object = GGame::g_game->MyInterface()->interface_collide.object;
	hand->SetDistanceFromView(hand->GetDistanceFromObject(object));
	GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, hand->DistanceFromView, &hand->position);
	LH3DIsland::GetAltitude((const LH3DMapCoords&)object->Pos);
	LHSys::GetMouse().PinImage(SHRT_MIN, SHRT_MIN);
}

void HandStateMultiPickUp::Exit()
{
	LHSys::GetMouse().Pinned = false;
}

void HandStateMultiPickUp::Update(float param_1, LHMatrix* param_2)
{
	GInterface* iface = GGame::g_game->MyInterface();
	Object*     object = iface->interface_collide.object;
	Object*     interact = iface->status->field_0x3c;
	float       angle = MULTI_PICK_UP_HAND_ANGLE;

	if (interact != NULL)
	{
		LHPoint& pos = hand->position;
		pos.y = LH3DIsland::GetAltitude(pos);
		hand->position.y += interact->GetHeight() + HeightAboveObject;
	}

	if (hand->position.y < MULTI_PICK_UP_HAND_HEIGHT)
	{
		angle =
			atan((MULTI_PICK_UP_HAND_HEIGHT - hand->position.y) / CHand::GetStandardSize()) + MULTI_PICK_UP_HAND_ANGLE;
	}

	LHMatrix rotation;
	rotation.SetRotationX(-angle);
	rotation.PostRotateY(hand->GetHeading() + PI_F);

	LHMatrix matrix;
	hand->SetUpPositionMatrix(matrix);
	matrix.PostMultiplyMatrixOnly(rotation);
	*(LHPoint*)&matrix._41 = hand->position;

	float radius = object->GetHoldRadius();
	float ratio = radius / (CHand::GetStandardSize() * 0.5f);
	float scale = min(1.0f, ratio);

	CAnim* anim = hand->GetAnim(HAND_STATE_MULTI_PICK_UP_ANIM, 0);
	anim->FillBuffer(param_2, hand->GetMesh(), hand->field_0x47f4, hand->field_0x47f8,
	                 (anim->Duration >> 1) * (scale * 0.5f), hand->frame, NULL, 1);
	LH3DAnim::FinishTransform(param_2, hand->GetMesh(), matrix);
}

void HandStateMultiPickUp::DrawTheHeldObject()
{
	hand->HeldObject->DrawOutOfMap(false);
}
