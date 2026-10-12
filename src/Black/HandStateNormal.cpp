#include "MapCellConstants.h"
#include "CreatureAttitudeConstants.h"
#include "GameTimeConstants.h"
#include "HandStateNormal.h"

#include <Lionhead/LH3DLib/development/LH3DAnim.h>          /* For struct CAnim */
#include <Lionhead/LH3DLib/development/LH3DIsland.h>        /* For LH3DIsland */
#include <Lionhead/LH3DLib/development/LH3DMeshIntersect.h> /* For struct MeshIntersect */
#include <Lionhead/LH3DLib/development/LH3DTech.h>          /* For LH3DTech */
#include <Lionhead/LH3DLib/development/LHMatrix.h>          /* For struct LHMatrix */

#include "AnimatedStatic.h"   /* For class AnimatedStatic */
#include "Camera.h"           /* For class GCamera */
#include "CameraHelp.h"       /* For CameraHelp::EnabledFeatures */
#include "CameraModeNew3.h"   /* For class CameraModeNew3 */
#include "ColourConstants.h"  /* For White */
#include "ControlHand.h"      /* For class CHand */
#include "Creature.h"         /* For class Creature */
#include "Creature3D.h"       /* For class LH3DCreature */
#include "CreaturePhysical.h" /* For class CreaturePhysical */
#include "Game.h"             /* For GGame::g_game */
#include "Game3DObject.h"     /* For class Game3DObject */
#include "Interface.h"        /* For class GInterface */
#include "Landscape.h"        /* For GLandscape */
#include "Living.h"           /* For class Living */
#include "MagicVortex.h"      /* For class LandscapeVortex */
#include "SpellIcon.h"        /* For class SpellIcon */
#include "Utils.h"            /* For GUtils */

int   HandStateNormal::Global_00bf01cc = 1;
float HandStateNormal::Global_00bf01d0 = 0.28f;
float HandStateNormal::Global_00bf01d4 = 2.0f;
float HandStateNormal::Global_00bf01d8 = 0.1f;
int   HandStateNormal::Global_00d14048;
int   HandStateNormal::Global_00d1404c;

HandStateNormal::HandStateNormal(CHand* hand) : HandState(hand)
{
	AnimationTime = 0;
}

LHPoint  HandStateNormal::Global_00d13f60;
Zoomer   HandStateNormal::Global_00d13f80;
Zoomer3d HandStateNormal::Global_00d13fb0;

void HandStateNormal::Enter()
{
	Global_00d1404c = 0;
	Global_00d13fb0.SetPosition(LHPoint(0.0f, 1.0f, 0.0f));
	AnimationTime = 0;
	Anim = -1;
	Global_00d13f80.SetPosition(hand->DistanceFromView);
}

LHPoint HandStateNormal::Global_00d13f70;

void ObtainRequiredHandPosition(CHand* hand, float param_2, float param_3, float& param_4, LHPoint& param_5,
                                LHPoint& param_6, int& param_7, bool param_8, bool param_9)
{
	if (HandStateNormal::Global_00bf01cc)
	{
		HandStateNormal::Global_00d13fb0.SetPosition(LHPoint(0.0f, 1.0f, 0.0f));
		HandStateNormal::Global_00d13f80.SetPosition(0.0f);
		HandStateNormal::Global_00bf01cc = 0;
	}
	CameraModeNew3* mode = dynamic_cast<CameraModeNew3*>(GGame::g_game->GetCamera()->GetCurrentMode());
	bool            fight = mode != NULL && mode->HasFight;
	bool            canGive = hand->GetCreatureToGiveTo() != NULL;
	bool            isCreature = false;
	GGame::g_game->GetCamera();
	LHPoint cameraPos = *LH3DTech::GetCameraPosition();
	float   standardDistance = hand->WorkOutStandardDistanceFromView(param_9);
	hand->SetDistanceFromView(standardDistance);
	LHPoint point;
	if (HandStateNormal::Global_00d1404c)
	{
		point = HandStateNormal::Global_00d13f70;
	}
	else
	{
		GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, standardDistance, &point);
	}
	GInterface* in = GGame::g_game->MyInterface();
	Object*     object = in->interface_collide.object;
	Bubble*     bubble = in->interface_collide.bubble;
	bool32_t    useDefault =
		in->interface_collide.leash != NULL || GGame::g_game->MyInterface()->ActionState == INTERFACE_ACTION_STATE_0x15;
	param_7 = useDefault ? HAND_ANIM_HOLD : HAND_ANIM_POINT;
	bool32_t        noFeel = false;
	bool32_t        useObjectPosition = false;
	float           raise = 0.0f;
	LHPoint         dir;
	LH3DCreature*   creature3d;
	const LHMatrix* matrices;
	if (object != NULL && dynamic_cast<LandscapeVortex*>(object) != NULL)
	{
		object = NULL;
	}
	if (object == hand->HeldObject)
	{
		object = NULL;
	}
	if (object != NULL && object->Game3dObject != NULL)
	{
		param_7 = in->HandState.GetState() == HAND_STATE_CAN_PICK_UP ? HAND_ANIM_PICK_UP : HAND_ANIM_OVER_OBJECT;
		if (dynamic_cast<Living*>(object) != NULL)
		{
			Creature* creature = dynamic_cast<Creature*>(object);
			if (creature != NULL)
			{
				creature3d = creature->physical->Creature3d;
				matrices = creature3d->GetMatrixBuffer();
				isCreature = true;
				if (hand->HeldObject != NULL && creature->InterfaceValidToGiveObject(in->status, hand->HeldObject) == 1)
				{
					if (hand->field_0x49b4 != creature)
					{
						hand->field_0x49b4 = creature;
						hand->field_0x49b8 = creature->GetCreature3D();
					}
					HandStateHolding::GiveToCreatureTimeLeft = HOLDING_GIVE_TO_CREATURE_TIME;
				}
			}
			else
			{
				matrices = &object->Game3dObject->matrix;
				useObjectPosition = true;
			}
		}
		else
		{
			matrices = &object->Game3dObject->matrix;
			if (object->Game3dObject->IsAnimated() && dynamic_cast<AnimatedStatic*>(object) == NULL)
			{
				useObjectPosition = true;
			}
		}
		if (dynamic_cast<SpellIcon*>(object) != NULL)
		{
			GLandscape::ConvertMapCoordToLandscapePoint(object->Pos, point);
			param_7 = HAND_ANIM_PICK_UP;
			noFeel = true;
			raise = CHand::GetStandardSize();
		}
		else
		{
			LH3DMesh*       mesh;
			AnimatedStatic* animatedStatic = dynamic_cast<AnimatedStatic*>(object);
			if (animatedStatic != NULL)
			{
				mesh = animatedStatic->GetCurrentPhysicsMesh();
				useObjectPosition = false;
			}
			else
			{
				mesh = object->Game3dObject->GetMesh();
			}
			LHPoint ray = point - cameraPos;
			ray.Normalise();
			LHPoint heldRay;
			if (hand->HeldObject != NULL)
			{
				heldRay = ray;
				ray = HandStateHolding::Global_00d13f30 - cameraPos;
				ray.y += HandStateHolding::Global_00d13f4c;
				ray.Normalise();
			}
			if (useObjectPosition)
			{
				LHPoint objectPos;
				GLandscape::ConvertMapCoordToLandscapePoint(object->Pos, objectPos);
				float distance = objectPos.GetRange(cameraPos) - object->Get2DRadius();
				GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, distance, &point);
				GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, (point.y - objectPos.y) * ray.y + distance,
				                                           &point);
				noFeel = true;
			}
			else
			{
				MeshIntersect intersect;
				if (intersect.GetNearestIntersection(mesh, matrices, &cameraPos, &ray, false, &point, &dir, false))
				{
					dir.Normalise();
					LHPoint toCamera = cameraPos - point;
					float   dot = -toCamera.DotProductInline(ray);
					if (hand->HeldObject != NULL)
					{
						ray = heldRay;
						point = cameraPos + ray * dot;
						toCamera = cameraPos - point;
					}
					if (dot < 1.0f)
					{
						point = cameraPos + ray;
						toCamera = LHPoint(-ray.x, -ray.y, -ray.z);
					}
					else
					{
						toCamera.Normalise();
					}
					if (hand->HeldObject != NULL)
					{
						float inside = 0.0f;
						if (isCreature)
						{
							float distance2D = point.GetDistance2D(creature3d->position);
							if (distance2D < creature3d->field_0x5228)
							{
								inside = creature3d->field_0x5228 - distance2D;
							}
						}
						point += toCamera * (hand->HeldObject->Get2DRadius() * 0.5f + inside);
					}
					dir = toCamera * 0.25f - dir;
					dir.y += 0.5f;
					if (!object->HandShouldFeelWithMeshIntersect())
					{
						noFeel = true;
					}
				}
				else
				{
					noFeel = true;
				}
			}
		}
	}
	else if (bubble != NULL || useDefault)
	{
		dir = LHPoint(0.0f, 1.0f, 0.0f);
	}
	else
	{
		mode = dynamic_cast<CameraModeNew3*>(GGame::g_game->GetCamera()->GetCurrentMode());
		uint32_t triconFlags = mode != NULL && param_8 ? mode->MouseTriconFlags : 0;
		if (GGame::g_game->MyInterface()->IsPlayBack(0))
		{
			if (CameraModeNew3::Global_00c5b0f4 & HAND_PLAYBACK_TRICON_ROTATE)
			{
				triconFlags |= HAND_TRICON_ROTATE;
			}
			if (CameraModeNew3::Global_00c5b0f4 & HAND_PLAYBACK_TRICON_PITCH)
			{
				triconFlags |= HAND_TRICON_PITCH;
			}
			if (CameraModeNew3::Global_00c5b0f4 & HAND_PLAYBACK_TRICON_ZOOM)
			{
				triconFlags |= HAND_TRICON_ZOOM;
			}
			hand->field_0x4858 = true;
		}
		if (triconFlags & HAND_TRICON_ROTATE)
		{
			if (CameraHelp::EnabledFeatures & CAMERA_FEATURE_ROTATE)
			{
				param_7 = HAND_ANIM_CAMERA_ROTATE;
				dir.Set(*LH3DTech::GetCameraTarget() - point);
				dir.y += dir.GetNorme() * 0.3f;
				dir.Normalise();
			}
			else
			{
				noFeel = true;
			}
		}
		else
		{
			if ((triconFlags & HAND_TRICON_PITCH) && (CameraHelp::EnabledFeatures & CAMERA_FEATURE_PITCH))
			{
				param_7 = HAND_ANIM_CAMERA_PITCH;
			}
			noFeel = true;
		}
	}
	if (hand->field_0x4874)
	{
		param_7 = HAND_ANIM_CAMERA_ROTATE;
		noFeel = false;
		dir.Set(*LH3DTech::GetCameraTarget() - point);
		dir.y += dir.GetNorme() * 0.3f;
		dir.Normalise();
	}
	float distance =
		!useDefault && bubble != NULL ? in->interface_collide.BubbleDistance : cameraPos.GetRange(point) - raise;
	if (distance < 1.0f)
	{
		distance = 1.0f;
	}
	float time;
	if (distance < HandStateNormal::Global_00d13f80.GetCurrentValue())
	{
		time = HandStateNormal::Global_00bf01d8;
	}
	else if ((fight || canGive) && HandStateNormal::Global_00d13f80.CurrentSpeed > -1.0f)
	{
		time = HandStateNormal::Global_00bf01d4;
	}
	else
	{
		time = HandStateNormal::Global_00bf01d0;
	}
	HandStateNormal::Global_00d13f80.SetDestination(distance, time);
	HandStateNormal::Global_00d13f80.Update(param_2);
	if (HandStateNormal::Global_00d13f80.GetCurrentValue() < 1.0f)
	{
		HandStateNormal::Global_00d13f80.SetPosition(1.0f);
	}
	distance = min(standardDistance, HandStateNormal::Global_00d13f80.GetCurrentValue());
	hand->SetDistanceFromView(distance);
	distance = hand->DistanceFromView;
	LH3DMapCoords coords(param_5.x, param_5.z);
	float         height = param_5.y - LH3DIsland::GetAltitude(coords);
	float         halfSize = CHand::GetStandardSize() * hand->field_0x4834 * 0.5f;
	if (param_3 != 0.0f)
	{
		dir = LHPoint(0.0f, 1.0f, 0.0f);
		GGame::g_game->GetCamera();
		LHPoint axis = *LH3DTech::GetCameraPosition();
		axis -= param_5;
		axis.FastNormalizeInline();
		LHMatrix rotation;
		rotation.SetFromVectorAndAngle(axis, param_3);
		rotation.TransformPoint(dir);
	}
	else if (noFeel || (object != NULL && height < halfSize))
	{
		LH3DIsland::GetNormal(param_5, &dir);
	}
	dir.Normalise();
	CHand::GetStandardSize();
	if (hand->field_0x4858 || param_3 != 0.0f)
	{
		HandStateNormal::Global_00d13fb0.SetDestinationWithTime(dir, 0.4f);
	}
	HandStateNormal::Global_00d13fb0.Update(LH3DTech::g_delta_time / 1000.0f);
	param_6 = HandStateNormal::Global_00d13fb0.GetCurrentValue();
	param_6.Normalise();
	HandStateNormal::Global_00d13f60 = param_6;
	if (HandStateNormal::Global_00d1404c)
	{
		HandStateNormal::Global_00d1404c = 0;
		param_5 = HandStateNormal::Global_00d13f70;
	}
	else
	{
		GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, distance, &param_5);
	}
}

void HandStateNormal::Update(float param_1, LHMatrix* param_2)
{
	float   distance;
	LHPoint dir;
	int     anim;
	ObtainRequiredHandPosition(hand, param_1, 0.0f, distance, hand->position, dir, anim, true, true);
	CAnim* animation = hand->GetAnim(anim, 0);
	AnimationTime = (LH3DTech::g_delta_time + AnimationTime) % animation->Duration;
	if (Anim == -1)
	{
		Anim = anim;
	}
	else if (anim != Anim)
	{
		Anim = anim;
		hand->BlendToNewPosition();
		param_2 = hand->GetMatrixBuffer();
	}
	if (anim == HAND_ANIM_HOLD)
	{
		if (GGame::g_game->MyInterface()->ActionState == INTERFACE_ACTION_STATE_0x15)
		{
			AnimationTime = animation->Duration / 2;
		}
		else
		{
			AnimationTime = 0;
		}
	}
	LHMatrix matrix;
	hand->fn_0046E160(matrix, hand->position, dir);
	animation->FillBuffer(param_2, hand->GetMesh(), hand->field_0x47f4, hand->field_0x47f8, AnimationTime, hand->frame,
	                      NULL, 1);

	animation = hand->GetAnim(anim + HAND_ANIM_LEAN_X, 0);
	float offset = hand->field_0x48b8 - hand->ScreenPos.x;
	float lean =
		offset <= -HAND_LEAN_RANGE_X ? -HAND_LEAN_RANGE_X : (offset < HAND_LEAN_RANGE_X ? offset : HAND_LEAN_RANGE_X);
	if (animation != NULL && fabs(lean) > 0.0001f)
	{
		animation->ModifyBufferByDifferenceFromFrame(
			param_2, animation->frames[(animation->FrameCount - 1) / 2], hand->GetMesh(), hand->field_0x47f4,
			hand->field_0x47f8, (long)((HAND_LEAN_RANGE_X + lean) / (HAND_LEAN_RANGE_X * 2) * animation->Duration),
			NULL, NULL);
	}

	animation = hand->GetAnim(anim + HAND_ANIM_LEAN_Y, 0);
	offset = hand->ScreenPos.y - hand->field_0x48bc;
	lean =
		offset <= -HAND_LEAN_RANGE_Y ? -HAND_LEAN_RANGE_Y : (offset < HAND_LEAN_RANGE_Y ? offset : HAND_LEAN_RANGE_Y);
	if (animation != NULL && fabs(hand->field_0x48bc) > 0.0001f)
	{
		animation->ModifyBufferByDifferenceFromFrame(
			param_2, animation->frames[(animation->FrameCount - 1) / 2], hand->GetMesh(), hand->field_0x47f4,
			hand->field_0x47f8, (long)((HAND_LEAN_RANGE_Y + lean) / (HAND_LEAN_RANGE_Y * 2) * animation->Duration),
			NULL, NULL);
	}
	LH3DAnim::FinishTransform(param_2, hand->GetMesh(), matrix);
	hand->field_0x486c = hand->ScreenPos;
}
