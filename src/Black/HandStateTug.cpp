#include <Lionhead/LH3DLib/development/LHMatrix.h> /* For struct LHMatrix */

#include "GameTimeConstants.h"
#include "ColourConstants.h" /* For White */
#include "EditorPhysics.h"   /* For PHYSICS_CONSTANTS_TYPE */
#include "Tree.h"            /* For class Tree */
#include "ControlHand.h"
#include "HandStateTug.h"

#include <math.h>   /* For sqrt */
#include <minmax.h> /* For max, min */

#include <Lionhead/LH3DLib/development/LH3DAnim.h>   /* For LH3DAnim::FinishTransform */
#include <Lionhead/LH3DLib/development/LH3DIsland.h> /* For LH3DIsland::GetNormal */
#include <Lionhead/LH3DLib/development/LH3DTech.h>   /* For LH3DTech::GetCameraPosition */

#include "Camera.h"       /* For class GCamera */
#include "Game.h"         /* For class GGame */
#include "Game3DObject.h" /* For class Game3DObject */
#include "Landscape.h"    /* For GLandscape::DrawListRebuildCount */
#include "Living.h"       /* For class Living */
#include "Object.h"       /* For class Object */
#include "RoutePlan.h"    /* For range_2d */
#include "Utils.h"        /* For GUtils */

const float TugStartThreshold = 0.13f;
const float TugSpringStrength = 1000.0f;
const float TugAngularInertia = 1000.0f;
const float TugAngularDamping = 4000.0f;

inline void GCamera::GetPosition(LHPoint& pos)
{
	pos = *LH3DTech::GetCameraPosition();
}

HandStateTug::HandStateTug(CHand* hand) : HandState(hand)
{
	field_0x8 = 0;
}

void HandStateTug::Enter()
{
	field_0x14.SetPosition(LHPoint(0.0f, 1.0f, 0.0f));
	field_0x8 = 0;
	field_0x10 = false;
	LHMatrix* matrix = &hand->HeldObject->Game3dObject->matrix;
	hand->HeldObject->GetWorldMatrix(matrix);
	hand->field_0x492c = matrix->GetPos();
	LH3DIsland::GetNormal(hand->field_0x492c, &field_0xa4);
	LHPoint cameraPos;
	GGame::g_game->GetCamera()->GetPosition(cameraPos);
	LHPoint direction;
	GUtils::SetUnitDirectionVectorFromScreenPoint(hand->ScreenPos, &direction);
	float   handRange = range_2d(cameraPos, hand->position);
	float   heldRange = range_2d(cameraPos, hand->field_0x492c);
	LHPoint pos = hand->field_0x492c;
	pos.y = cameraPos.y - (cameraPos.y - hand->position.y) * (heldRange / handRange);
	float   minHeight = CHand::GetStandardSize() * hand->field_0x4834 * 0.3f;
	Object* object = hand->HeldObject;
	float   lowering = object->GetHoldLoweringMultiplier();
	hand->field_0x4944 = object->GetHeight() * lowering;
	hand->field_0x4944 = max(minHeight, hand->field_0x4944);
	field_0xc = true;
	field_0xb0 = pos;
	hand->field_0x4920.SetNull();
	field_0x11c.SetPosition(1.0f);
	field_0xec = *matrix;
}

void HandStateTug::Update(float dt, LHMatrix* matrices)
{
	LH3DObject* heldObject3d = hand->HeldObject->Game3dObject;
	if (field_0xc)
	{
		if (hand->field_0x49b0 >= TugStartThreshold)
		{
			field_0xc = false;
		}
	}
	else
	{
		LHPoint cameraPos;
		GGame::g_game->GetCamera()->GetPosition(cameraPos);
		LHPoint direction;
		GUtils::SetUnitDirectionVectorFromScreenPoint(hand->ScreenPos, &direction);
		float distance = (field_0xb0 - cameraPos).DotProductInline(field_0xa4) / direction.DotProductInline(field_0xa4);
		hand->position = cameraPos + direction * distance;

		LHMatrix heldMatrix = field_0xec;
		LHPoint  top(0.0f, hand->field_0x4944, 0.0f);
		heldMatrix.NormaliseMatrixOnly();
		heldMatrix.TransformPoint(top);
		LHPoint pull = hand->position - top;
		float   pullLength = sqrt(pull.x * pull.x + pull.z * pull.z + pull.y * pull.y);
		float   force = TugSpringStrength * pullLength;
		if (hand->GetMaxForce() > force)
		{
			pull *= TugSpringStrength;
		}
		else
		{
			pull *= hand->GetMaxForce() / pullLength;
		}

		float weight = hand->HeldObject->GetWeight();
		float pullOut = weight * 9.81f;
		float stretch = min(1.3f, (pullLength + hand->field_0x4944) / hand->field_0x4944);
		field_0x11c.SetDestination(stretch, 0.3f);
		field_0x11c.Update(dt);

		bool isRock = hand->HeldObject->GetPhysicsConstantsType() == PHYSICS_CONSTANTS_TYPE_ROCK;
		if (!hand->HeldObject->IsTree())
		{
			hand->field_0x4908 = false;
		}
		else if (hand->GetMaxForce() >= pullOut && sqrt(pull.x * pull.x + pull.z * pull.z + pull.y * pull.y) > weight)
		{
			Tree* tree = dynamic_cast<Tree*>(hand->HeldObject);
			if (tree != NULL && !(tree->Flags & GAME_THING_WITH_POS_FLAG_IN_PHYSICS))
			{
				tree->AddPermanentFootPrint();
			}
			hand->field_0x4908 = false;
			if (dynamic_cast<Living*>(hand->HeldObject) != NULL)
			{
				GLandscape::DrawListRebuildCount = 1;
			}
		}

		LHPoint torque = pull ^ (top - hand->field_0x492c);
		if (isRock)
		{
			torque.SetNull();
		}
		else
		{
			torque *= 1.0f / TugAngularInertia;
		}
		LHPoint spin = torque * dt;
		float   damping = hand->field_0x4920.GetNorme() * TugAngularDamping * dt / TugAngularInertia;
		spin -= hand->field_0x4920 * damping;
		hand->field_0x4920 += spin;

		LHPoint  axis = hand->field_0x4920 * dt;
		float    angle = axis.Normalise();
		LHMatrix rotation;
		rotation.SetFromVectorAndAngle(axis, angle);

		float   centreHeight = hand->HeldObject->GetHeight() * 0.5f;
		LHPoint centre(0.0f, centreHeight, 0.0f);
		field_0xec.TransformPoint(centre);
		field_0xec.PostMultiplyMatrixOnly(rotation);
		LHPoint newCentre(0.0f, centreHeight, 0.0f);
		field_0xec.TransformPoint(newCentre);
		heldObject3d->matrix = field_0xec;
		if (isRock)
		{
			hand->field_0x4938.SetNull();
		}
		else
		{
			heldObject3d->matrix.GetVectorY() *= field_0x11c.GetCurrentValue();
			hand->field_0x4938 = newCentre - centre;
			hand->field_0x4938 *= 1.0f / dt;
		}
		field_0xbc = heldObject3d->matrix;
		field_0x10 = true;
	}

	float standardSize = CHand::GetStandardSize();
	if (hand->HeldObject != NULL)
	{
		CAnim*  anim;
		Object* object;
		float   ratio;
		switch (hand->field_0x490c)
		{
		case 1:
			ratio = min(1.0f, hand->HeldObject->GetHoldRadius() * 1.2f / (standardSize * hand->field_0x4834 * 1.2f));
			object = hand->HeldObject;
			hand->field_0x4944 = hand->HeldObject->GetHeight() * hand->HeldObject->GetHoldLoweringMultiplier();
			anim = hand->GetAnim(6, 0);
			field_0x8 = anim->Duration * 0.5f * (1.0f - ratio);
			break;
		case 2:
			object = hand->HeldObject;
			hand->field_0x4944 = hand->HeldObject->GetHeight() * hand->HeldObject->GetHoldLoweringMultiplier();
			anim = hand->GetAnim(0, 0);
			field_0x8 = anim->Duration >> 1;
			break;
		case 3:
			ratio = min(1.0f, hand->HeldObject->GetHoldRadius() / (standardSize * hand->field_0x4834));
			object = hand->HeldObject;
			hand->field_0x4944 = hand->HeldObject->GetHeight() * hand->HeldObject->GetHoldLoweringMultiplier();
			anim = hand->GetAnim(60, 0);
			field_0x8 = (anim->Duration >> 1) * ratio;
			break;
		case 5:
		case 6:
		case 7:
			ratio = min(1.0f, hand->HeldObject->GetHoldRadius() / (standardSize * hand->field_0x4834));
			object = hand->HeldObject;
			hand->field_0x4944 = hand->HeldObject->GetHeight() * hand->HeldObject->GetHoldLoweringMultiplier();
			anim = hand->GetAnim(9, 0);
			field_0x8 = (anim->Duration >> 1) * ratio;
			break;
		}

		LHMatrix handMatrix = field_0xec;
		handMatrix.NormaliseMatrixOnly();
		handMatrix.GetVectorY() *= field_0x11c.GetCurrentValue();
		hand->position.Set(0.0f, hand->field_0x4944, 0.0f);
		handMatrix.TransformPoint(hand->position);
		LHPoint up = heldObject3d->matrix.GetVectorY();
		up.FastNormalizeInline();
		CHand::GetStandardSize();
		LHPoint  pos = hand->position;
		LHMatrix transform;
		hand->fn_0046E160(transform, pos, up);
		anim->FillBuffer(matrices, hand->GetMesh(), hand->field_0x47f4, hand->field_0x47f8, field_0x8, hand->frame,
		                 NULL, 1);
		LH3DAnim::FinishTransform(matrices, hand->GetMesh(), transform);
		hand->field_0x486c.x = hand->ScreenPos.x;
		hand->field_0x486c.y = hand->ScreenPos.y;
	}
}

void HandStateTug::DrawTheHeldObject()
{
	hand->HeldObject->DrawOutOfMap(false);
}

void HandStateTug::UpdateHeldObject()
{
	if (field_0x10)
	{
		hand->HeldObject->Game3dObject->matrix = field_0xbc;
	}
}
