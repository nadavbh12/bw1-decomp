#include "Base.h"
#include "Creature3D.h"
#include "GameTimeConstants.h"
#include "ColourConstants.h" /* For White */
#include "Interface.h"       /* For class GInterface */
#include "ControlHand.h"
#include "HandStateCreature.h"

#include <Lionhead/LH3DLib/development/LH3DTech.h> /* For LH3DTech::GetCameraPosition */

#include "Camera.h"           /* For class GCamera */
#include "Creature.h"         /* For class Creature */
#include "Creature3D.h"       /* For class LH3DCreature */
#include "CreaturePhysical.h" /* For class CreaturePhysical */
#include "Game.h"             /* For class GGame */
#include "Network.h"          /* For struct GNetwork */
#include "Packet.h"           /* For enum PACKET_TYPE */
#include "Utils.h"            /* For GUtils */

#include <math.h>   /* For cos, pow, sin, sqrt */
#include <minmax.h> /* For max, min */

#include <Lionhead/LH3DLib/development/LH3DAnim.h>   /* For LH3DAnim::FinishTransform */
#include <Lionhead/LH3DLib/development/LH3DIsland.h> /* For LH3DIsland::GetAltitude */
#include <Lionhead/LH3DLib/development/LH3DMath.h>   /* For PI_F */
#include <Lionhead/LHLib/ver5.0/LHSystem.h>          /* For LHSys */
#include <Lionhead/LHMultiplayer/ver4.0/LHSession.h> /* For LHSession */

inline void GCamera::GetPosition(LHPoint& pos)
{
	pos = *LH3DTech::GetCameraPosition();
}

long     HandStateCreature::BoneActions[HAND_STATE_CREATURE_BONE_COUNT] = {154, 155, 155, 157, 158, 159, 159, 161, 161};
bool32_t HandStateCreature::BoneActionMirrored[HAND_STATE_CREATURE_BONE_COUNT] = {false, false, true,  false, false,
                                                                                  false, true,  false, true};
long     HandStateCreature::BoneFacialActions[HAND_STATE_CREATURE_BONE_COUNT] = {25, 16, 16, 25, 24, 16, 16, 16, 16};

HandStateCreature::HandStateCreature(CHand* hand) : HandState(hand) {}

void HandStateCreature::Enter()
{
	Object*   object = GGame::g_game->MyInterface()->interface_collide.object;
	Creature* creature = dynamic_cast<Creature*>(object);
	if (creature != NULL)
	{
		LH3DCreature* creature3d = creature->physical->Creature3d;
		HandValue = 0;
		field_0xe0 = 0;
		field_0xcc = 0;
		field_0xd0 = 0;
		field_0x110 = 0;
		field_0xd4 = 0;
		field_0xdc = 0;
		field_0x8 = 0;
		field_0x118 = 0;
		field_0x11c = 0;
		Bones[HAND_STATE_CREATURE_BONE_HEAD] = creature3d->HeadBone;
		Bones[HAND_STATE_CREATURE_BONE_RIGHT_ARMPIT] = creature3d->RightArmpit;
		Bones[HAND_STATE_CREATURE_BONE_LEFT_ARMPIT] = creature3d->GetMirrorBone(creature3d->RightArmpit);
		Bones[HAND_STATE_CREATURE_BONE_BELLY] = creature3d->BellyBone;
		Bones[HAND_STATE_CREATURE_BONE_ANUS] = creature3d->Anus;
		Bones[HAND_STATE_CREATURE_BONE_RIGHT_FOOT] = creature3d->RightFoot;
		Bones[HAND_STATE_CREATURE_BONE_LEFT_FOOT] = creature3d->GetMirrorBone(creature3d->RightFoot);
		Bones[HAND_STATE_CREATURE_BONE_RIGHT_HAND] = creature3d->RightHand;
		Bones[HAND_STATE_CREATURE_BONE_LEFT_HAND] = creature3d->GetMirrorBone(creature3d->RightHand);
		LHPoint cameraPos;
		GGame::g_game->GetCamera()->GetPosition(cameraPos);
		LHPoint direction = cameraPos - creature3d->GetPos();
		float   distance = direction.Normalise();
		field_0xc.SetPosition(direction);
		field_0xc.SetDestinationWithTime(direction, 1.0f);
		field_0x114 = 27;
		float viewDistance = distance - (creature3d->GetSize() * 0.825f);
		field_0x9c.SetPosition(viewDistance);
		field_0x9c.SetDestination(viewDistance, 1.0f);
	}
}

void HandStateCreature::Exit()
{
	GGame::g_game->MyInterface()->StopImmersion(IMMERSION_EFFECT_TYPE_FUR);
}

void HandStateCreature::Update(float dt, LHMatrix* matrices)
{
	LH3DCreature* creature3d = hand->field_0x49b8;
	float         size = creature3d->GetSize() * 7.5f;
	float         sizeRatio = (float)pow(creature3d->GetSize(), 0.5);
	if (creature3d->InHandInteraction == 0)
	{
		return;
	}

	LHPoint direction;
	GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, 1.0f, &direction);
	LHPoint cameraPos;
	GGame::g_game->GetCamera()->GetPosition(cameraPos);
	direction -= cameraPos;
	LHPoint lookDirection = direction;
	field_0xcc += GGame::g_game->TimeInc;
	field_0xd0 += GGame::g_game->TimeInc;

	LHPoint anusPos = *hand->field_0x49b8->GetAnusPos();
	LHPoint viewPos;
	GGame::g_game->GetCamera()->GetPosition(viewPos);
	float viewDistance = viewPos.GetDistance(anusPos) - size * 0.25f;
	if (field_0x9c.GetCurrentValue() > viewDistance)
	{
		field_0x9c.SetPosition(viewDistance);
	}
	hand->SetDistanceFromView(field_0x9c.GetCurrentValue());
	hand->field_0x4834 *= sizeRatio;

	int halfWidth = LHSys::TheSystem.screen.width / 2;
	int lastScreenSide = hand->PreviousScreenPos.x - halfWidth;
	int screenSide = hand->ScreenPos.x - halfWidth;
	field_0xdc += GGame::g_game->TimeInc;

	MeshIntersect intersect;
	bool32_t      hit =
		intersect.GetNearestIntersection(hand->field_0x49b8->GetMesh(), hand->field_0x49b8->GetMatrixBuffer(),
	                                     &cameraPos, &direction, false, NULL, NULL, false);
	bool32_t immersing = GGame::g_game->MyInterface()->IsImmersionPlaying(IMMERSION_EFFECT_TYPE_FUR);
	if (!hit)
	{
		LHCoord middle;
		middle.x = (hand->PreviousScreenPos.x + hand->ScreenPos.x) >> 1;
		middle.y = (hand->ScreenPos.y + hand->PreviousScreenPos.y) >> 1;
		LHPoint middleDirection;
		GUtils::SetPointFromScreenPointAndDistance(&middle, 1.0f, &middleDirection);
		middleDirection -= cameraPos;
		hit = intersect.GetNearestIntersection(hand->field_0x49b8->GetMesh(), hand->field_0x49b8->GetMatrixBuffer(),
		                                       &cameraPos, &middleDirection, false, NULL, NULL, false);
	}
	if (hit && !immersing)
	{
		GGame::g_game->MyInterface()->StartImmersion(IMMERSION_EFFECT_TYPE_FUR, INFINITE);
	}
	else if (immersing)
	{
		GGame::g_game->MyInterface()->StopImmersion(IMMERSION_EFFECT_TYPE_FUR);
	}

	LHPoint normal(0.0f, -1.0f, 0.0f);
	LHPoint handPos;
	GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, hand->DistanceFromView, &handPos);
	LHPoint lastHandPos;
	GUtils::SetPointFromScreenPointAndDistance(&hand->PreviousScreenPos, hand->DistanceFromView, &lastHandPos);
	LHPoint velocity(handPos.x - lastHandPos.x, 0.0f, handPos.z - lastHandPos.z);
	velocity *= 1.0f / dt;
	float speed = velocity.GetNorme();
	float hitSpeed = size * 4.5f;
	float slapSpeed = size * 2.5f;
	if (speed > slapSpeed && field_0x114 != 33)
	{
		field_0x114 = 33;
		hand->BlendToNewPosition();
	}

	if ((hit && !field_0x110) || screenSide * lastScreenSide < 0)
	{
		float height = (hand->position.y - LH3DIsland::GetAltitude(hand->position)) / size;
		if (height > 0.0f && height < 1.1 && field_0xdc > 1000 && speed > slapSpeed)
		{
			long action;
			int  zone;
			if (height < 0.2)
			{
				action = 169;
				zone = 0;
			}
			else if (height < 0.7)
			{
				action = 167;
				zone = 1;
			}
			else
			{
				action = 165;
				zone = 2;
			}
			if (zone != field_0x118)
			{
				field_0x118 = zone;
				field_0x11c = 0;
			}
			int strength;
			if (speed < hitSpeed)
			{
				strength = 1;
			}
			else
			{
				strength = 2;
				field_0x11c++;
				if (hit && field_0x11c > 2)
				{
					field_0x11c = 0;
					int force = (long)((speed - hitSpeed) * 2.5f / hitSpeed);
					force = min(max(force, 0), 7);
					HandStateCreaturePacket packet;
					packet.Intersect = intersect;
					packet.field_0x2c = 7 - force;
					packet.field_0x28 = 0;
					GGame::g_game->fn_00551040(PACKET_TYPE_0x63, &packet, sizeof(packet));
				}
			}
			if (!hand->field_0x49b8->IsPerformingBodyAction() || hand->field_0x49b8->GetBodyActionFraction() > 0.8f)
			{
				bool32_t mirrored = hand->ScreenPos.x - hand->PreviousScreenPos.x < 0;
				field_0xcc = 0;
				field_0xd0 = 1000;
				action += strength == 1 ? 6 : 0;
				field_0xe0 = hand->field_0x49b8->ForceIndividualAction(action, mirrored);
				if (field_0xe0 && GGame::g_game->network.session->Players.count == 1)
				{
					hand->field_0x49b8->InitialiseDestructionBones();
					hand->field_0x49b8->StartDestruction();
				}
				GGame::g_game->MyInterface()->StartImmersion(
					strength == 1 ? IMMERSION_EFFECT_TYPE_SLAP : IMMERSION_EFFECT_TYPE_HIT, INFINITE);
				unsigned long index = hand->field_0x49b4->GetIndexOffset();
				GGame::g_game->SetPacket(PACKET_TYPE_0x27, index, action, mirrored, 0);
				if (HandValue > 0.25)
				{
					strength *= 2;
				}
				HandValue = max(-1.0f, HandValue - strength * 0.1f);
				field_0xd4 = 0;
			}
		}
	}
	else if (hit)
	{
		LHPoint touchPos;
		intersect.GetWorldPositionAndInwardNormal(hand->field_0x49b8->GetMesh(), hand->field_0x49b8->GetMatrixBuffer(),
		                                          &touchPos, &normal);
		float distance = touchPos.GetRange(cameraPos) - size * 0.055f;
		float closest = distance - size * 0.03f;
		if (field_0x9c.GetCurrentValue() > closest)
		{
			field_0x9c.SetPosition(closest);
		}
		field_0x9c.SetDestination(distance, 0.3f);
		hand->SetDistanceFromView(field_0x9c.GetCurrentValue());
		hand->field_0x4834 *= sizeRatio;
		field_0xd4 += GGame::g_game->TimeInc;
		if (field_0xd4 > 1000 && hand->field_0x4858 != 0)
		{
			LHMatrix* bones = hand->field_0x49b8->SafeBuffer0;
			int       nearest = 0;
			float     nearestDistance = 1000.0f;
			for (int i = 0; i < HAND_STATE_CREATURE_BONE_COUNT; i++)
			{
				LHPoint bonePos = bones[Bones[i]].GetPos();
				float   boneDistance = touchPos.GetRange(bonePos);
				if (boneDistance < nearestDistance)
				{
					nearestDistance = boneDistance;
					nearest = i;
				}
			}
			if (nearest == HAND_STATE_CREATURE_BONE_LEFT_ARMPIT || nearest == HAND_STATE_CREATURE_BONE_RIGHT_ARMPIT)
			{
				if (field_0x114 != 27)
				{
					field_0x114 = 27;
					hand->BlendToNewPosition();
				}
			}
			else if (field_0x114 != 27)
			{
				field_0x114 = 27;
				hand->BlendToNewPosition();
			}
			if (field_0xcc >= (long)((1.0f - HandValue) * 0.5f * 2500.0f) && field_0x108 != nearest &&
			    field_0xd0 >= 2000)
			{
				hand->field_0x49b8->StartFacialAction(BoneFacialActions[nearest], 2000);
				if (!hand->field_0x49b8->IsPerformingBodyAction() || hand->field_0x49b8->GetBodyActionFraction() > 0.8)
				{
					field_0x108 = nearest;
					field_0xcc = 0;
					field_0xd0 = 1000;
					long     action = BoneActions[nearest];
					bool32_t mirrored = BoneActionMirrored[nearest];
					field_0xe0 = hand->field_0x49b8->ForceIndividualAction(action, mirrored);
					unsigned long index = hand->field_0x49b4->GetIndexOffset();
					GGame::g_game->SetPacket(PACKET_TYPE_0x27, index, action, mirrored, 0);
					HandValue = min(1.0f, HandValue + 0.1f);
					field_0xdc = 0;
				}
			}
		}
	}
	else
	{
		field_0x108 = HAND_STATE_CREATURE_BONE_COUNT;
	}
	field_0x9c.Update(dt);

	if (hand->field_0x49b8->GetAnusPos()->GetRange(hand->position) > size * 0.6f)
	{
		lookDirection = lookDirection * (size * -0.3f) + hand->position;
		LH3DCreature::AlwaysSetLookPoint = true;
		hand->field_0x49b8->SetLookPoint(&lookDirection);
	}
	else
	{
		LH3DCreature::AlwaysSetLookPoint = true;
		hand->field_0x49b8->SetLookPoint(&cameraPos);
	}
	LH3DCreature::AlwaysSetLookPoint = false;
	GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, hand->DistanceFromView, &hand->position);
	field_0x110 = hit;
	LHMatrix handMatrix;
	hand->SetUpPositionMatrix(handMatrix);
	if (!hit && field_0x114 == 27)
	{
		field_0x114 = 0;
	}
	if (hand->field_0x4858 != 0 && hit)
	{
		normal.FastNormalizeInline();
		LHPoint inward(-normal.x, -normal.y, -normal.z);
		LHPoint offset = viewPos - hand->field_0x49b8->GetPos();
		offset.SetSize(0.4f);
		field_0xc.SetDestinationWithTime(inward + offset, 0.9f);
	}
	if (!hit)
	{
		field_0xc.SetDestinationWithSpeedAndTime(LHPoint(0.0f, 1.0f, 0.0f), 0.0f, 2.5f);
	}
	field_0xc.Update(dt);

	LHPoint up = field_0xc.GetCurrentValue();
	up.FastNormalizeInline();
	float   angle = hand->Heading + PI_F / 2;
	LHPoint forward(-(float)sin(angle), 0.0f, (float)cos(angle));
	LHPoint side = up ^ forward;
	side.Normalise();
	LHPoint  back = up ^ side;
	LHMatrix rotation;
	rotation._11 = side.x;
	rotation._12 = side.y;
	rotation._13 = side.z;
	rotation._21 = -back.x;
	rotation._22 = -back.y;
	rotation._23 = -back.z;
	rotation._31 = -up.x;
	rotation._32 = -up.y;
	rotation._33 = -up.z;
	handMatrix.PostMultiplyMatrixOnly(rotation);
	*(LHPoint*)&handMatrix.m[9] = hand->GetPos();

	CAnim* anim = hand->GetAnim(field_0x114, 0);
	field_0x8 = (field_0x8 + LH3DTech::g_delta_time) % anim->Duration;
	anim->FillBuffer(hand->TransformedMatrices, hand->GetMesh(), hand->field_0x47f4, hand->field_0x47f8, field_0x8,
	                 hand->frame, NULL, 1);
	LH3DAnim::FinishTransform(hand->TransformedMatrices, hand->GetMesh(), handMatrix);
	hand->field_0x486c.x = hand->ScreenPos.x;
	hand->field_0x486c.y = hand->ScreenPos.y;
}
