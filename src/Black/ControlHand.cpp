#include "ControlHand.h"

#include <math.h>   /* For cos, pow, sin */
#include <stdio.h>  /* For sprintf */
#include <string.h> /* For memcpy */

#include <Lionhead/LH3DLib/development/LH3DAnim.h>          /* For struct LH3DAnim */
#include <Lionhead/LH3DLib/development/LH3DComplexObject.h> /* For struct LH3DComplexObject */
#include <Lionhead/LH3DLib/development/LH3DMath.h>          /* For PI_F */
#include <Lionhead/LH3DLib/development/LH3DIsland.h>        /* For struct LH3DIsland */
#include <Lionhead/LH3DLib/development/LH3DMesh.h>          /* For struct LH3DMesh */
#include <Lionhead/LH3DLib/development/LH3DPrimitive.h>     /* For struct LH3DPrimitive */
#include <Lionhead/LH3DLib/development/LH3DRender.h>        /* For struct LH3DRender */
#include <Lionhead/LH3DLib/development/LH3DSubMesh.h>       /* For struct LH3DSubMesh */
#include <Lionhead/LH3DLib/development/LH3DTech.h>          /* For struct LH3DTech */
#include <Lionhead/LH3DLib/development/LHMatrix.h>          /* For struct LHMatrix */
#include <Lionhead/LHLib/ver5.0/LHMouse.h>                  /* For struct LHMouse */
#include <Lionhead/LHLib/ver5.0/LHSystem.h>                 /* For struct LHSys */
#include <Lionhead/LHLib/ver5.0/LHWin.h>                    /* For operator new(size_t, const char*, uint32_t) */

#include "ColourConstants.h"
#include "GameTimeConstants.h"
#include "alexmfc.h"               /* For struct SetupBox */
#include "Camera.h"                /* For class GCamera */
#include "CameraModeCtrInteract.h" /* For class CameraModeCtrInteract */
#include "CameraModeNew3.h"        /* For class CameraModeNew3 */
#include "Creature.h"              /* For class Creature */
#include "FrontEnd.h"              /* For struct FrontEnd */
#include "Game.h"                  /* For class GGame */
#include "Game3DObject.h"          /* For class Game3DObject */
#include "GameInfo.h"              /* For class GGameInfo */
#include "GameOSFile.h"            /* For class GameOSFile */
#include "Global.h"                /* For struct GGlobal */
#include "Utils.h"                 /* For struct GUtils */
#include "Interface.h"             /* For class GInterface */
#include "InterfaceStatus.h"       /* For class GInterfaceStatus */
#include "Landscape.h"             /* For class GLandscape */
#include "LH3DZSorter.h"           /* For class LH3DZSorter */
#include "Object.h"                /* For class Object */
#include "PhysicsObject.h"         /* For class PhysicsObject */
#include "Player.h"                /* For class GPlayer */
#include "PSysHandFX.h"            /* For class PSysHandFX */
#include "PSysInterface.h"         /* For class PSysInterface */
#include "Spell.h"                 /* For class Spell */
#include "SpellSeed.h"             /* For class SpellSeed */
#include "VirtualInfluence.h"      /* For class GVirtualInfluence */

#define FILEPATH "C:\\dev\\MP\\Black\\ControlHand.cpp"

inline void GCamera::GetPosition(LHPoint& pos)
{
	pos = LH3DTech::g_camera.pos;
}

inline float GCamera::GetDistance(const LHPoint& point)
{
	LHPoint pos;
	GetPosition(pos);
	return pos.GetDistance(point);
}

const float MinDistanceFromView = 20.0f;
const float HandSpringDamping = 0.03f;
const float DefaultBlend = 0.13f;
const float DefaultDistanceFromView = 10.0f;
const float MinViewDistance = 2.0f;
const float FarDistanceFromView = 150.0f;
const float DefaultMaxDistanceFromUser = 1800.0f;

const float HandMaxLagX = 80.0f;
const float HandMaxLagY = 80.0f;

enum
{
	ScreenEdgeWidth = 256,
	HandDataVersion = 0,
	HandSpecVersion = 5,
	HandBinaryWriteParam = 0x45,
	MaxPreInteractionTime = 450,
	ThrowPacketDelay = 180,
};

static char* HandLagMessages[] = {"hand lag ON", "hand lag OFF"};
static float HandBrightness = 1.5f;

static bool    HiddenAtScreenEdge;
static LHPoint BlendEndPosition;
static LHPoint BlendStartPosition;
static LHPoint BlendedHandPosition;
LHPoint        CHand::PreviousHandPosition;
LHPoint        CHand::HandPosition;

CHand::CHand(LHPoint point, GInterfaceStatus* status) : HandFx(PSysHandFX::CreateHandFX())
{
	field_0x4874 = 0;
	field_0x486c.x = 0;
	field_0x486c.y = 0;
	field_0x4844 = 0;
	field_0x484c = 0;
	MorphInit(point, 1, NULL);
	field_0x483c = 0;
	field_0x4848 = 0;
	InterfaceStatus = status;
	DistanceFromView = DefaultDistanceFromView;
	field_0x48b4 = 0;
	CurrentState = HAND_STATES_NORMAL;
	field_0x4840 = 1;
	HeldObject = NULL;
	ThrownObject = NULL;
	field_0x4908 = false;
	field_0x4918 = 0;
	field_0x48f8 = 0;
	field_0x49c0 = false;
	field_0x49b4 = NULL;
	field_0x49b8 = NULL;
	field_0x49bc = NULL;
	field_0x491c = 0;
	DynamicShadow->CreateDynamicShadow();
	field_0x4834 = 1.0f;
	MouseSprite = NULL;
	field_0x49a0 = false;
	field_0x49a4 = false;
	field_0x49b0 = DefaultBlend;
	field_0x4998 = 1;
	MaxDistanceFromUser = DefaultMaxDistanceFromUser;
	field_0x49a8 = 0;
	Matrix0x49ac = NULL;
	SpellInHand = NULL;
	SpellInHandSeed = NULL;
	field_0x4954 = 0;
	HandStates.named.invisible = new (FILEPATH, 162) HandStateInvisible(this);
	HandStates.named.normal = new (FILEPATH, 163) HandStateNormal(this);
	HandStates.named.camera = new (FILEPATH, 164) HandStateCamera(this);
	HandStates.named.tug = new (FILEPATH, 165) HandStateTug(this);
	HandStates.named.holding = new (FILEPATH, 166) HandStateHolding(this);
	HandStates.named.totem = new (FILEPATH, 167) HandStateTotem(this);
	HandStates.named.MultiPickUp = new (FILEPATH, 168) HandStateMultiPickUp(this);
	HandStates.named.creature = new (FILEPATH, 169) HandStateCreature(this);
	HandStates.named.grain = new (FILEPATH, 170) HandStateGrain(this);
	HandStates.named.PlayAnim = new (FILEPATH, 171) HandStatePlayAnim(this);
	HandStates.named.citadel = new (FILEPATH, 172) HandStateCitadel(this);
	HandStates.raw[CurrentState]->Enter();
	field_0x4994 = 0;
	PreInteractionTimer = 0;
}

void HandStateInvisible::Enter()
{
	hand->fn_0046C2E0(false);
	memcpy(hand->Matrix0x49ac, hand->TransformedMatrices, hand->field_0x47b8 * sizeof(LHMatrix));
}

void HandStateInvisible::Exit()
{
	hand->Show(false);
}

void HandStateInvisible::Update(float param_1, LHMatrix* param_2) {}

void CHand::fn_0046BEF0()
{
	field_0x4918 = (field_0x4918 + 1) % 2;
	GGlobal::Global.AddPlayerTextMessage(-1, HandLagMessages[field_0x4918]);
}

void CHand::SetMaxDistanceFromUser(float distance)
{
	MaxDistanceFromUser = distance > DefaultMaxDistanceFromUser ? DefaultMaxDistanceFromUser : distance;
}

float CHand::GetMaxDistanceFromUser()
{
	return MaxDistanceFromUser;
}

void CHand::fn_0046BF60(uint32_t param_1)
{
	if (param_1 != field_0x4840)
	{
		field_0x4840 = param_1;
		MorphTexture();
	}
}

void CHand::MorphTexture()
{
	Morphable::MorphTexture();
	if (field_0x4840 == 0)
	{
		fn_0046BFA0();
	}
}

void CHand::fn_0046BFA0() {}

float CHand::GetMaxForce()
{
	return (WeakStrong + 1.0f) * 600000.0f;
}

CHand::~CHand()
{
	if (Matrix0x49ac != NULL)
	{
		delete Matrix0x49ac;
	}
	if (file.opened)
	{
		file.CloseSegment();
		file.Close();
	}
	for (int i = 0; i < _HAND_STATES_COUNT; i++)
	{
		delete HandStates.raw[i];
	}
	SpellInHandStop();
	HandFx->Release();
}

float CHand::GetStandardSize()
{
	return 3.2f;
}

void CHand::StartFixedPosAnimation(LHPoint& pos, long anim)
{
	field_0x491c = 1;
	HandStatePlayAnim* playAnim = HandStates.named.PlayAnim;
	playAnim->Position = pos;
	playAnim->Anim = anim;
}

void CHand::GiveToCreature(Creature* creature)
{
	if (HeldObject != NULL)
	{
		field_0x49b4 = field_0x49bc = creature;
		field_0x49b8 = creature->GetCreature3D();
	}
}

void CHand::EndGiveToCreature()
{
	field_0x49bc = NULL;
	field_0x49b4 = NULL;
	field_0x49b8 = NULL;
}

void CHand::SetDistanceFromView(float distance)
{
	float viewDistance = distance > MinViewDistance ? min(distance, MaxDistanceFromUser) : MinViewDistance;
	DistanceFromView = viewDistance;
	if (viewDistance < DefaultDistanceFromView)
	{
		field_0x4834 = (float)pow(viewDistance / DefaultDistanceFromView, 0.8f);
	}
	else
	{
		field_0x4834 = 1.0f;
	}
	if (viewDistance > FarDistanceFromView)
	{
		field_0x4834 *=
			(1.0f - (viewDistance - FarDistanceFromView) / (DefaultMaxDistanceFromUser - FarDistanceFromView) * 0.3f) *
			(viewDistance / FarDistanceFromView);
	}
}

void CHand::Show(bool32_t param_1)
{
	DynamicShadow->field_0xac = 0;
	if (DynamicShadow->field_0x84)
	{
		DynamicShadow->field_0x84[2] = 1;
	}
	if (param_1)
	{
		field_0x4848 = 0;
	}
	if (!field_0x483c || field_0x4848)
	{
		return;
	}
	if (!field_0x49a0)
	{
		LHSys::TheSystem.mouse.SetCursor(NULL, LH_MOUSE_IMAGE_TYPE_0x01, 0);
		FrontEnd::SetCursorOff();
	}
	else
	{
		LHSys::TheSystem.mouse.SetCursor(MouseSprite, LH_MOUSE_IMAGE_TYPE_0x08, 1);
	}
	field_0x483c = 0;
}

void CHand::fn_0046C250()
{
	field_0x49a4 ^= 1;
}

void CHand::ToggleLeftRight()
{
	field_0x484c ^= 1;
	LH3DSubMesh* submesh = meshes[0]->submeshes[0];
	for (int i = 0; i < submesh->NumPrimitives; i++)
	{
		LH3DPrimitive* primitive = submesh->primitives_[i];
		for (int j = 0; j < primitive->NumTriangles; j++)
		{
			uint32_t index = ((uint16_t*)primitive->Triangles)[j * 3];
			((uint16_t*)primitive->Triangles)[j * 3] = ((uint16_t*)primitive->Triangles)[j * 3 + 1];
			((uint16_t*)primitive->Triangles)[j * 3 + 1] = index;
		}
	}
}

void CHand::fn_0046C2E0(bool32_t param_1)
{
	DynamicShadow->field_0xac = 1;
	if (DynamicShadow->field_0x84 != NULL)
	{
		DynamicShadow->field_0x84[2] = 0;
	}
	if (param_1)
	{
		field_0x4848 = 1;
	}
	if (!field_0x483c)
	{
		if (!field_0x4844)
		{
			FrontEnd::SetCursorOn();
		}
		field_0x483c = 1;
	}
}

void CHand::UpdateTime(int param_1)
{
	if (field_0x47bc && CycleAnim[0] != NULL)
	{
		CycleTime[0] = (CycleTime[0] + param_1) % CycleAnim[0]->Duration;
	}
}

void CHand::SetSize(float size)
{
	Size1 = size > 0.05f ? (size < 2.0f ? size : 2.0f) : 0.05f;
	Size2 = GetStandardSize() / field_0x8c * field_0x4834 * Size1;
}

void CHand::BlendToNewPosition()
{
	LHMatrix* matrices = TransformedMatrices;
	field_0x49b0 = 0.0f;
	TransformedMatrices = Matrix0x49ac;
	Matrix0x49ac = matrices;
	BlendStartPosition = BlendEndPosition;
}

void CHand::PrepareForDrawing()
{
	if (GGame::g_game->field_0x205d50 || GGame::g_game->field_0x205d54)
	{
		LHCoord mouse = LHSys::TheSystem.mouse.Pos();
		if (mouse.x < ScreenEdgeWidth)
		{
			if (!field_0x483c)
			{
				HiddenAtScreenEdge = true;
				fn_0046C2E0(true);
			}
		}
		else if (HiddenAtScreenEdge)
		{
			HiddenAtScreenEdge = false;
			Show(true);
		}
	}
	else if (HiddenAtScreenEdge)
	{
		HiddenAtScreenEdge = false;
		Show(true);
	}
	long timeInc = GGame::g_game->GetCameraTimeInc();
	if (GGame::g_game->ViewMode == GAME_VIEW_MODE_INSIDE_CITADEL)
	{
		timeInc = LH3DTech::g_delta_time;
	}
	PreInteractionTimer += timeInc;
	float time = timeInc / 1000.0f;
	fn_0046BF60(GGame::g_game->MyInterface()->field_0x48);
	float alignment = GGame::g_game->MyPlayer()->GetAlignmentValue();
	EvilGood = alignment > -1.0f ? min(alignment, 1.0f) : -1.0f;
	UpdateMorphing();
	if (field_0x47bc)
	{
		DoPassiveModeDraw();
		return;
	}
	PreviousScreenPos = ScreenPos;
	if (field_0x4874)
	{
		ScreenPos.x = field_0x486c.x;
		ScreenPos.y = field_0x486c.y;
	}
	else
	{
		ScreenPos = LHSys::TheSystem.mouse.Pos();
	}
	field_0x4858 = PreviousScreenPos.x != ScreenPos.x;
	LHPoint direction;
	GUtils::SetUnitDirectionVectorFromScreenPoint(ScreenPos, &direction);
	if (fabs(direction.x) > 0.01f || fabs(direction.z) > 0.01f)
	{
		Heading = heading_from_direction_vector(direction);
	}
	long state = GetActualState(GetRequiredState());
	if (state != CurrentState)
	{
		HandStates.raw[CurrentState]->Exit();
		CurrentState = state;
		HandStates.raw[state]->Enter();
		BlendToNewPosition();
	}
	if (field_0x483c || state == HAND_STATES_INVISIBLE || SetupBox::Dat_00C4CCEE ||
	    !InterfaceStatus->GetInterface()->IsActive())
	{
		return;
	}
	if (field_0x48b4)
	{
		float x = field_0x48b8 = field_0x48b8 + time * field_0x48c0;
		float targetX = (float)ScreenPos.x;
		field_0x48b8 = x > targetX - HandMaxLagX ? min(x, targetX + HandMaxLagX) : targetX - HandMaxLagX;
		float velocity = field_0x48c0 = (targetX - field_0x48b8) * MinDistanceFromView * time + field_0x48c0;
		field_0x48c0 = (float)pow(HandSpringDamping, time) * velocity;
		float y = field_0x48bc = field_0x48bc + time * field_0x48c4;
		field_0x48bc = y > ScreenPos.y - HandMaxLagY ? min(y, ScreenPos.y + HandMaxLagY) : ScreenPos.y - HandMaxLagY;
		velocity = field_0x48c4 = (ScreenPos.y - field_0x48bc) * MinDistanceFromView * time + field_0x48c4;
		field_0x48c4 = (float)pow(HandSpringDamping, time) * velocity;
	}
	else
	{
		field_0x48b8 = (float)ScreenPos.x;
		field_0x48c4 = 0.0f;
		field_0x48c0 = 0.0f;
		field_0x48b4 = 1;
		field_0x48bc = (float)ScreenPos.y;
	}
	HandStates.raw[CurrentState]->Update(time, TransformedMatrices);
	if (field_0x4874)
	{
		field_0x4874 = 0;
	}
	if (field_0x49b0 < DefaultBlend)
	{
		field_0x49b0 += time;
		if (field_0x49b0 < DefaultBlend)
		{
			float  blend = field_0x49b0 / DefaultBlend;
			int    count = field_0x47b8 * sizeof(LHMatrix) / sizeof(float);
			float* to = TransformedMatrices->m;
			float* from = Matrix0x49ac->m;
			for (int i = 0; i < count; i++)
			{
				to[i] = (to[i] - from[i]) * blend + from[i];
			}
			LHPoint offset = (BlendEndPosition - BlendStartPosition) * blend;
			BlendedHandPosition = offset + BlendStartPosition;
			if (CurrentState == HAND_STATES_HOLDING &&
			    (!PhysicsObject::PredictionActive || HeldObject != PhysicsObject::PredictionObject))
			{
				HeldObject->Game3dObject->matrix.SetTranslateOnly(BlendedHandPosition +
				                                                  HandStateHolding::Global_00d13f20);
			}
		}
	}
	else
	{
		BlendedHandPosition = BlendEndPosition;
	}
	DynamicShadow->SetMesh(meshes[CurrentMesh], NULL, NULL);
	DynamicShadow->matrix = TransformedMatrices[0];
	DynamicShadow->scale = Size2;
	DynamicShadow->Matrix0x80 = TransformedMatrices;
	LH3DMesh::g_current_list_matrix = TransformedMatrices;
	DynamicShadow->field_0xbc = 1;
	DynamicShadow->PrepareShadow();
	LH3DMesh::g_current_list_matrix = LH3DMesh::g_my_list_matrix;
	if (field_0x49a0)
	{
		PrepareMouseSprite();
	}
}

void CHand::OperateInteractionCamera(long hand_state)
{
	GCamera*    camera = GGame::g_game->GetCamera();
	GInterface* playerInterface = GGame::g_game->MyInterface();
	if (hand_state == HAND_STATE_CREATURE_INTERACTION)
	{
		Object* object = GGame::g_game->MyInterface()->interface_collide.object;
		field_0x49b4 = dynamic_cast<Creature*>(object);
		if (field_0x49b4 == NULL)
		{
			return;
		}
		field_0x49b8 = field_0x49b4->GetCreature3D();
		if (playerInterface->interface_collide.object == NULL)
		{
			fn_0046C2E0(false);
			return;
		}
		if (playerInterface->interface_collide.object != field_0x49b4)
		{
			fn_0046C2E0(false);
			return;
		}
		if (!field_0x49c0)
		{
			new (FILEPATH, 746) CameraModeCtrInteract(camera, field_0x49b4);
			field_0x49c0 = true;
			PreInteractionTimer = 0;
		}
	}
	else if (field_0x49c0)
	{
		camera->PopViewMode();
		field_0x49c0 = false;
		GGlobal::Global.debug.SetMessage("PreInteractionTimer: %d", PreInteractionTimer);
		if (PreInteractionTimer < MaxPreInteractionTime && field_0x49b4->GetPlayer() == GGame::g_game->MyPlayer())
		{
			GGame::g_game->SetPacket(PACKET_TYPE_0x5f);
		}
	}
}

long CHand::GetRequiredState()
{
	if (GGame::g_game->ViewMode == GAME_VIEW_MODE_INSIDE_CITADEL)
	{
		return HAND_STATES_CITADEL;
	}
	GInterface* playerInterface = GGame::g_game->MyInterface();
	if (HeldObject != NULL && playerInterface->HandState.hand_state == HAND_STATE_OFF)
	{
		field_0x4844 = 1;
		return HAND_STATES_INVISIBLE;
	}
	Object* statusObject = playerInterface->status->field_0x3c;
	if (statusObject != NULL && statusObject->IsPileResource() && HeldObject != NULL)
	{
		if (playerInterface->HandState.hand_state == HAND_STATE_OFF)
		{
			field_0x4844 = 1;
			return HAND_STATES_INVISIBLE;
		}
		return HAND_STATES_HOLDING;
	}
	if (HeldObject != NULL)
	{
		if (playerInterface->HandState.hand_state == HAND_STATE_OFF)
		{
			field_0x4844 = 1;
			return HAND_STATES_INVISIBLE;
		}
		if (HeldObject->IsSpellSeed())
		{
			return HAND_STATES_GRAIN;
		}
		if (field_0x4908)
		{
			return HAND_STATES_TUG;
		}
		field_0x48f8 = ThrowPacketDelay;
		return HAND_STATES_HOLDING;
	}
	if (field_0x48f8 > 0)
	{
		field_0x48f8 -= GGame::g_game->TimeInc;
		if (field_0x48f8 < 0 && ThrownObject != NULL)
		{
			field_0x48f8 = 0;
			HandStateHolding* holding = HandStates.named.holding;
			LHPoint           velocity(HandPosition.x - PreviousHandPosition.x, HandPosition.y - PreviousHandPosition.y,
			                           HandPosition.z - PreviousHandPosition.z);
			GGame::g_game->SetPacket(PACKET_TYPE_0x40, ThrownObject->GetIndexOffset(), &velocity,
			                         &holding->SpringVelocity, 0.0f);
			ThrownObject = NULL;
		}
	}
	long handState = playerInterface->HandState.hand_state;
	OperateInteractionCamera(handState);
	field_0x4844 = 0;
	if (field_0x491c)
	{
		return HAND_STATES_PLAY_ANIM;
	}
	switch (handState)
	{
	case HAND_STATE_POINTER_NORMAL:
	case HAND_STATE_POINTER_SELECT:
	case HAND_STATE_NORMAL:
	case HAND_STATE_CAN_SELECT:
		return HAND_STATES_TOTEM;
	case HAND_STATE_CREATURE_INTERACTION:
		return field_0x49b8 != NULL ? HAND_STATES_CREATURE : HAND_STATES_NORMAL;
	case HAND_STATE_OFF:
		return HAND_STATES_INVISIBLE;
	}
	CameraModeNew3* mode = dynamic_cast<CameraModeNew3*>(GGame::g_game->GetCamera()->GetCurrentMode());
	if (mode != NULL && (mode->MouseButtons & CAMERA_MODE_MOUSE_STATUS_LEFT))
	{
		return HAND_STATES_CAMERA;
	}
	return HAND_STATES_NORMAL;
}

long CHand::GetActualState(long state)
{
	GInterface* playerInterface = GGame::g_game->MyInterface();
	uint32_t    currentState = CurrentState;
	if (state == HAND_STATES_CREATURE)
	{
		if (currentState != state && (field_0x49b4->Flags & 0x10) && field_0x49b4->field_0x3cc == NULL)
		{
			return state;
		}
		return currentState;
	}
	if (currentState == HAND_STATES_CREATURE)
	{
		HandStateCreature* creatureState = HandStates.named.creature;
		if (field_0x49b4 != NULL && field_0x49b4->IsReadyForNetworkUnfriendlyEndLockedSelect() != 1)
		{
			float value = creatureState->HandValue > -1.0f ? min(creatureState->HandValue, 1.0f) : -1.0f;
			GGame::g_game->SetPacket(PACKET_TYPE_0x59, field_0x49b4->GetIndexOffset(), value);
			field_0x49b4 = NULL;
			field_0x49b8 = NULL;
		}
	}
	return state;
}

void CHand::SetUpPositionMatrix(LHMatrix& matrix)
{
	float scale = Size2 = GetStandardSize() / field_0x8c * field_0x4834 * Size1;
	matrix.SetScale(scale);
	if (field_0x484c)
	{
		matrix._12 = 0.0f;
		matrix._11 = -scale;
		matrix._13 = 0.0f;
		matrix._41 = 0.0f;
	}
}

uint32_t CHand::AddForDrawing()
{
	AddDrawing();
	return 1;
}

void CHand::AddDrawing()
{
	LH3DIsland::GetColorAndSpecular(&DynamicShadow->matrix.GetPos(), &DynamicShadow->color, &DynamicShadow->specular);
	uint8_t* colour = (uint8_t*)&DynamicShadow->color;
	float    value = colour[0] * HandBrightness;
	colour[0] = (uint8_t)(value > 255.0f ? 255.0f : value);
	value = colour[1] * HandBrightness;
	colour[1] = (uint8_t)(value > 255.0f ? 255.0f : value);
	value = colour[2] * HandBrightness;
	colour[2] = (uint8_t)(value > 255.0f ? 255.0f : value);
	LH3DRender::g_zsorter->NewZObject(this, (void (DrawingObject::*)())&CHand::Draw,
	                                  LH3DTech::GetValueForZSorter(DynamicShadow->matrix.GetPos()), 0);
}

void CHand::Draw()
{
	if (field_0x49a0 == true || SetupBox::Dat_00C4CCEE)
	{
		return;
	}
	if (!InterfaceStatus->GetInterface()->IsActive())
	{
		return;
	}
	if (field_0x47bc)
	{
		DynamicShadow->Draw();
		return;
	}
	if (CurrentState != HAND_STATES_INVISIBLE && !field_0x483c)
	{
		DynamicShadow->Draw();
		if (GGame::g_game->ViewMode != GAME_VIEW_MODE_INSIDE_CITADEL)
		{
			HandFx->DrawHandFX();
			InterfaceStatus->influence->Draw();
			DrawSpellInHand();
		}
	}
}

uint32_t CHand::LoadBinary(char* filename, int param_1)
{
	AnimInfo* oldInfo = NULL;
	LHFile*   handFile = &file;
	handFile->SetName(filename);
	if (handFile->Open(LH_FILE_MODE_READ_ONLY))
	{
		morphable_error("Can't open hand : ", filename);
		return false;
	}
	if (handFile->OpenSegment("Hand"))
	{
		morphable_error("Can't open segment Hand : ", filename);
		handFile->Close();
		return false;
	}
	long version;
	handFile->GetSegmentData(&version, sizeof(version), -1);
	if (version > HandDataVersion)
	{
		morphable_error("Unknown hand version : ", filename);
		handFile->Close();
		return false;
	}
	handFile->GetSegmentData(&version, sizeof(version), -1);
	if (version > HandSpecVersion)
	{
		morphable_error("Unknown data version : ", filename);
		handFile->Close();
		return false;
	}
	char specFilename[0x40];
	if (version < HandSpecVersion)
	{
		sprintf(specFilename, "data\\hndspec%d.txt", version);
		oldInfo = new (FILEPATH, 1139) AnimInfo;
		if (!oldInfo->Read(specFilename))
		{
			morphable_error("Can't read : ", specFilename);
			delete oldInfo;
			handFile->Close();
			return false;
		}
	}
	sprintf(specFilename, "data\\hndspec%d.txt", HandSpecVersion);
	AnimInfo* info = new (FILEPATH, 1150) AnimInfo;
	if (!info->Read(specFilename))
	{
		morphable_error("Can't read : ", specFilename);
		delete oldInfo;
		delete info;
		handFile->Close();
		return false;
	}
	if (!ReadBinary(handFile, oldInfo ? oldInfo : info, info))
	{
		delete oldInfo;
		delete info;
		handFile->Close();
		return false;
	}
	GetSetAnim(0, 1, 0);
	GetSetAnim(0, 2, 0);
	GetSetAnim(0, 3, 0);
	GetSetAnim(0, 4, 0);
	GetSetAnim(0, 5, 0);
	SelectMesh(0);
	ToggleLeftRight();
	delete oldInfo;
	delete info;
	SetSize(1.0f);
	if (Matrix0x49ac != NULL)
	{
		delete Matrix0x49ac;
	}
	Matrix0x49ac = new (FILEPATH, 1185) LHMatrix[field_0x47b8];
	ReadRegistrySettings();
	return true;
}

uint32_t CHand::SaveBinary(char* filename)
{
	LHReleasedFile file;
	file.SetName(filename);
	LoadAllAndClose();
	if (file.Open(LH_FILE_MODE_READ_WRITE))
	{
		morphable_error("Can't create file : ", filename);
		return false;
	}
	if (file.OpenSegment("Hand"))
	{
		morphable_error("Can't open segment Hand : ", filename);
		file.Close();
		return false;
	}
	long version = HandDataVersion;
	file.WriteSegmentData(&version, sizeof(version));
	version = HandSpecVersion;
	file.WriteSegmentData(&version, sizeof(version));
	WriteBinary(&file, HandBinaryWriteParam);
	file.CloseSegment();
	file.Close();
	return true;
}

CAnim* CHand::fn_0046D740(long anim)
{
	CAnim* result = GetAnim(anim, 0);
	if (result == NULL)
	{
		result = GetAnim(0, 0);
	}
	return result;
}

void CHand::DoPassiveModeDraw()
{
	LHMatrix matrix;
	SetUpPositionMatrix(matrix);
	float   angle = PI_F - Heading;
	LHPoint heading((float)sin(angle), 0.0f, -(float)cos(angle));
	LHPoint facing(-heading.z, heading.y, heading.x);
	facing.SetSize(1.0f);
	LHPoint  up(0.0f, 1.0f, 0.0f);
	LHPoint  side = up ^ facing;
	LHMatrix rotation;
	rotation.GetVectorX().Set(facing.x, facing.y, facing.z);
	rotation.GetVectorY().Set(-side.x, -side.y, -side.z);
	rotation.GetVectorZ().Set(0.0f, -1.0f, 0.0f);
	matrix.PostMultiplyMatrixOnly(rotation);
	matrix.SetTranslateOnly(position);
	matrix._42 += GetStandardSize();
	if (CycleAnim[0] != NULL)
	{
		CycleAnim[0]->FillBuffer(TransformedMatrices, meshes[CurrentMesh], field_0x47f4, field_0x47f8, CycleTime[0],
		                         frame, NULL, 1);
		LH3DAnim::FinishTransform(TransformedMatrices, meshes[CurrentMesh], matrix);
	}
	else
	{
		LH3DAnim::SetTransform(TransformedMatrices, meshes[CurrentMesh], matrix);
	}
	DynamicShadow->SetMesh(meshes[CurrentMesh], NULL, NULL);
	DynamicShadow->matrix = matrix;
	DynamicShadow->Matrix0x80 = TransformedMatrices;
	LH3DMesh::g_current_list_matrix = TransformedMatrices;
	DynamicShadow->field_0xbc = 1;
	DynamicShadow->PrepareShadow();
	LH3DMesh::g_current_list_matrix = LH3DMesh::g_my_list_matrix;
}

void __fastcall LHMatrix::PostMultiplyMatrixOnly(const LHMatrix& r)
{
	LHMatrix m;
	m._11 = _11;
	m._12 = _12;
	m._13 = _13;
	m._21 = _21;
	m._22 = _22;
	m._23 = _23;
	m._31 = _31;
	m._32 = _32;
	m._33 = _33;
	_11 = m._11 * r._11 + m._12 * r._21 + m._13 * r._31;
	_12 = m._11 * r._12 + m._12 * r._22 + m._13 * r._32;
	_13 = m._11 * r._13 + m._12 * r._23 + m._13 * r._33;
	_21 = m._21 * r._11 + m._22 * r._21 + m._23 * r._31;
	_22 = m._21 * r._12 + m._22 * r._22 + m._23 * r._32;
	_23 = m._21 * r._13 + m._22 * r._23 + m._23 * r._33;
	_31 = m._31 * r._11 + m._32 * r._21 + m._33 * r._31;
	_32 = m._31 * r._12 + m._32 * r._22 + m._33 * r._32;
	_33 = m._31 * r._13 + m._32 * r._23 + m._33 * r._33;
}

float CHand::GetDistanceFromObject(Object* object)
{
	float radius = 0.0f;
	if (object->Game3dObject != NULL)
	{
		radius = object->Get2DRadius();
	}
	LHPoint pos;
	GLandscape::ConvertMapCoordToLandscapePoint(object->Pos, pos);
	return GGame::g_game->GetCamera()->GetDistance(pos) - radius;
}

void CHand::PickUpObject(Object* object, bool32_t param_2)
{
	HeldObject = object;
	if (object == NULL)
	{
		field_0x4908 = false;
		return;
	}
	Game3DObject* game3dObject = object->Game3dObject;
	if (game3dObject == NULL)
	{
		field_0x4908 = false;
		return;
	}
	if (object->IsSpellSeed())
	{
		param_2 = false;
	}
	if (HeldObject != NULL && HeldObject->IsG3DObjectDrawnInHand())
	{
		DynamicShadow->SetCarriedObject(game3dObject);
	}
	else
	{
		DynamicShadow->SetCarriedObject(NULL);
	}
	field_0x4910 = game3dObject->IsNeedSorting();
	field_0x4914 = game3dObject->IsCastDynamicShadow();
	game3dObject->SetNeedSorting(true);
	game3dObject->SetCastDynamicShadow(false);
	game3dObject->specular = 0;
	field_0x490c = object->GetHoldType();
	bool fixed = (object->Flags & 0x40) && !(object->GameThing::Flags & 0x10);
	if (param_2 && !fixed)
	{
		field_0x4908 = true;
		return;
	}
	field_0x4908 = false;
	field_0x492c.Set(object->Game3dObject->matrix.GetPos());
	float height = object->GetHoldLoweringMultiplier() * object->GetHeight();
	field_0x4944 = height;
	float limit = GetStandardSize() * field_0x4834 * 0.3f;
	field_0x4944 = max(limit, field_0x4944);
	field_0x4938.SetNull();
	field_0x4920.SetNull();
}

void CHand::ThrowObject()
{
	if (HeldObject != NULL)
	{
		Game3DObject* object = HeldObject->Game3dObject;
		if (object != NULL)
		{
			object->SetNeedSorting(field_0x4910);
			object->SetCastDynamicShadow(field_0x4914);
		}
		DynamicShadow->SetCarriedObject(NULL);
		ThrownObject = HeldObject;
		HeldObject = NULL;
		GLandscape::DrawListRebuildCount = 2;
	}
}

void CHand::SwitchObject(Object* object)
{
	Game3DObject* game3dObject = HeldObject->Game3dObject;
	if (game3dObject != NULL)
	{
		HeldObject->GetWorldMatrix(&game3dObject->matrix);
		game3dObject->SetNeedSorting(field_0x4910);
	}
	HeldObject = object;
	field_0x490c = object->GetHoldType();
	game3dObject = HeldObject->Game3dObject;
	field_0x4910 = game3dObject->IsNeedSorting();
	game3dObject->SetNeedSorting(true);
	if (HeldObject != NULL && HeldObject->IsG3DObjectDrawnInHand())
	{
		DynamicShadow->SetCarriedObject(game3dObject);
	}
	else
	{
		DynamicShadow->SetCarriedObject(NULL);
	}
	BlendToNewPosition();
}

void CHand::DrawTheHeldObject()
{
	if (SetupBox::Dat_00C4CCEE)
	{
		return;
	}
	if (!InterfaceStatus->GetInterface()->IsActive())
	{
		return;
	}
	switch (CurrentState)
	{
	case HAND_STATES_TUG:
	case HAND_STATES_HOLDING:
	case HAND_STATES_MULTI_PICK_UP:
	case HAND_STATES_CREATURE:
	case HAND_STATES_GRAIN:
		HandStates.raw[CurrentState]->DrawTheHeldObject();
		break;
	}
}

float CHand::WorkOutStandardDistanceFromView(bool32_t param_1)
{
	LHPoint  point;
	bool32_t found =
		LH3DIsland::RayCastFrom2DPoint(ScreenPos, &point.x, &point.z, true, LH3DTech::GetNearClipping() + 1.0f);
	if (found)
	{
		point.y = LH3DIsland::GetAltitude(point);
	}
	else
	{
		point = position;
		if (CurrentState == HAND_STATES_HOLDING && HandStates.named.holding->SpringActive)
		{
			found = true;
			GUtils::SetPointFromScreenPointAndDistance(&ScreenPos, MaxDistanceFromUser, &point);
		}
	}
	HandStateHolding::Global_00d13f30 = point;
	HandPosition = point;
	bool32_t subtractHand = point.y < 0.1f ? false : param_1;
	LHPoint  cameraPos;
	GGame::g_game->GetCamera()->GetPosition(cameraPos);
	float distance;
	if (found)
	{
		distance = cameraPos.GetDistance(point);
		if (subtractHand)
		{
			distance -= GetStandardSize() * field_0x4834 * Size1;
		}
	}
	else
	{
		distance = cameraPos.GetDistance(position);
	}
	return distance > MinViewDistance ? min(distance, MaxDistanceFromUser) : MinViewDistance;
}

void CHand::fn_0046E160(LHMatrix& matrix, const LHPoint& pos, const LHPoint& dir)
{
	BlendEndPosition = pos;
	SetUpPositionMatrix(matrix);
	float   angle = Heading + PI_F;
	LHPoint facing((float)sin(angle), 0.0f, -(float)cos(angle));
	LHPoint side = facing ^ dir;
	side.SetSize(1.0f);
	LHPoint  up = dir ^ side;
	LHMatrix rotation;
	rotation.GetVectorX().Set(side.x, side.y, side.z);
	rotation.GetVectorY().Set(-up.x, -up.y, -up.z);
	rotation.GetVectorZ().Set(-dir.x, -dir.y, -dir.z);
	matrix.PostMultiplyMatrixOnly(rotation);
	matrix.SetTranslateOnly(pos);
}

void CHand::fn_0046E2F0(LHMatrix& matrix, const LHPoint& pos, const LHPoint& dir)
{
	SetUpPositionMatrix(matrix);
	float   angle = Heading + PI_F;
	LHPoint facing((float)sin(angle), 0.0f, -(float)cos(angle));
	LHPoint side = facing ^ dir;
	side.SetSize(1.0f);
	LHPoint  up = dir ^ side;
	LHMatrix rotation;
	rotation.GetVectorX().Set(side.x, side.y, side.z);
	rotation.GetVectorY().Set(dir.x, dir.y, dir.z);
	rotation.GetVectorZ().Set(up.x, up.y, up.z);
	if (!field_0x484c)
	{
		rotation._31 *= -1.0f;
		rotation._32 *= -1.0f;
		rotation._33 *= -1.0f;
	}
	matrix.PostMultiplyMatrixOnly(rotation);
	matrix.SetTranslateOnly(pos);
}

bool32_t CHand::HasPluckedObject() const
{
	return field_0x4908 == false;
}

void CHand::UpdateHeldObject()
{
	if (CurrentState == HAND_STATES_TUG)
	{
		HandStates.named.tug->UpdateHeldObject();
	}
}

Creature* CHand::GetCreatureToGiveTo()
{
	if (CurrentState == HAND_STATES_HOLDING && HandStates.named.holding->CanCurrentlyGiveToCreature() &&
	    field_0x49b4 != NULL)
	{
		return field_0x49b4;
	}
	return NULL;
}

void CHand::GameTurnUpdate()
{
	HandStateGrain::Process(GGameInfo::Info.MillisecondsPerGameTurn * 0.001f);
	if (HeldObject != NULL)
	{
		if (HeldObject->IsAvailable() != true)
		{
			ThrowObject();
		}
		else
		{
			field_0x490c = HeldObject->GetHoldType();
		}
	}
	if (ThrownObject != NULL && ThrownObject->IsAvailable() != true)
	{
		ThrownObject = NULL;
		field_0x48f8 = 0;
	}
	if (field_0x49b4 != NULL && !field_0x49b4->IsAvailable())
	{
		field_0x49b4 = NULL;
		field_0x49b8 = NULL;
		field_0x49bc = NULL;
	}
	fn_0046E640();
}

void CHand::SetHeldObject(Object* object)
{
	if (object == HeldObject)
	{
		return;
	}
	if (HeldObject == NULL)
	{
		PickUpObject(object, false);
	}
	else if (object != NULL)
	{
		SwitchObject(object);
	}
	else
	{
		ThrowObject();
	}
}

HandStateGrain* CHand::GetHandStateGrain()
{
	return HandStates.named.grain;
}

void CHand::ReadRegistrySettings() {}

void CHand::SpellInHandStop()
{
	if (SpellInHand != NULL)
	{
		delete SpellInHand;
	}
	SpellInHand = NULL;
	SpellInHandSeed = NULL;
}

void CHand::SpellInHandStart(const SpellSeed* seed)
{
	SpellInHandStop();
	PARTICLE_TYPE particleType = seed->fn_007285E0();
	if (particleType != PARTICLE_TYPE_NONE)
	{
		SpellInHand = PSysInterface::Create(NULL, particleType, LHPoint(0.0f, 0.0f, 0.0f), LHPoint(0.0f, 0.0f, 0.0f),
		                                    1.0f, PSysInterface::NET_GAME_TYPE_0);
		if (SpellInHand != NULL)
		{
			SpellInHandSeed = (SpellSeed*)seed;
			field_0x4954 = ((SpellSeed*)seed)->IsDrawAtGraspPos();
			SpellInHand->SetPlayer(GGame::g_game->MyPlayer());
			SpellInHand->SetOrigin(GGame::g_game->MyInterface()->GetRenderHand()->GetPos());
		}
	}
}

void CHand::fn_0046E890()
{
	if (SpellInHand != NULL)
	{
		SpellInHand->CloseDown();
	}
	SpellInHandSeed = NULL;
}

void CHand::fn_0046E640()
{
	if (SpellInHandSeed != NULL && SpellInHandSeed->IsAvailable() != true)
	{
		SpellInHandSeed = NULL;
		SpellInHand->CloseDown();
	}
}

void CHand::DrawSpellInHand()
{
	int timeInc = LH3DTech::g_game_time_inc;
	if (timeInc <= 1)
	{
		timeInc = 1;
	}
	if (SpellInHand != NULL)
	{
		if (field_0x4954)
		{
			LH3DAnim::GetGraspPoint(TransformedMatrices, meshes[CurrentMesh], SpellProcessInfo.field_0xc, 0);
		}
		else
		{
			SpellProcessInfo.field_0xc = position;
		}
		if (SpellInHandSeed != NULL)
		{
			SpellProcessInfo.field_0x30 = SpellInHandSeed->GetPSysPower();
		}
		SpellInHand->SetMagnitude(GetViewDistanceScale());
		if (timeInc > 0 && SpellInHand->Process(SpellProcessInfo, timeInc) == 5)
		{
			SpellInHandStop();
		}
	}
	if (SpellInHand != NULL && (SpellInHandSeed == NULL || SpellInHandSeed->DrawnInHand))
	{
		SpellInHand->Draw(1.0f, false);
	}
}

void CHand::OnClearMap()
{
	SpellInHandStop();
	DynamicShadow->SetCarriedObject(NULL);
	HeldObject = NULL;
	ThrownObject = NULL;
	field_0x49b4 = NULL;
	field_0x49b8 = NULL;
	field_0x49bc = NULL;
	field_0x48f8 = 0;
	PreInteractionTimer = 0;
	field_0x4840 = CurrentState = HAND_STATES_NORMAL;
	MouseSprite = NULL;
	field_0x49c0 = false;
}

uint32_t CHand::Load(GameOSFile& file)
{
	SpellInHandStop();
	file.ReadIt(CurrentState);
	file.ReadPtr((GameThing**)&HeldObject);
	file.ReadIt(field_0x490c);
	file.ReadIt(field_0x4910);
	file.ReadIt(field_0x4914);
	file.ReadIt(field_0x4944);
	file.ReadIt(position);
	file.ReadSafe((PSysBase*&)SpellInHand);
	file.ReadPtr((GameThing**)&SpellInHandSeed);
	file.ReadIt(field_0x4954);
	return true;
}

uint32_t CHand::Save(GameOSFile& file)
{
	file.WriteIt(CurrentState);
	file.WritePtr(HeldObject);
	file.WriteIt(field_0x490c);
	file.WriteIt(field_0x4910);
	file.WriteIt(field_0x4914);
	file.WriteIt(field_0x4944);
	file.WriteIt(position);
	file.WriteSafe((PSysBase*&)SpellInHand);
	file.WritePtr(SpellInHandSeed);
	file.WriteIt(field_0x4954);
	return true;
}

void CHand::ResolveLoad()
{
	if (CurrentState == HAND_STATES_HOLDING && HeldObject == NULL)
	{
		CurrentState = HAND_STATES_NORMAL;
	}
	switch (CurrentState)
	{
	case HAND_STATES_HOLDING:
		GLandscape::ConvertMapCoordToLandscapePoint(HeldObject->Pos, field_0x492c);
		field_0x4938.SetNull();
		field_0x4920.SetNull();
		HandStates.raw[CurrentState]->Enter();
		break;
	case HAND_STATES_GRAIN:
		HandStates.named.grain->Enter();
		break;
	}
}
