const float CitadelHandMaxOffsetX = 80.0f;
const float CitadelHandMaxOffsetY = 80.0f;

#include "CreatureAttitudeConstants.h"
#include "GameTimeConstants.h"
#include "HandStateCitadel.h"

#include "ColourConstants.h" /* For White */

#include <math.h>    /* For fabs */
#include <windows.h> /* For min */

#include <Lionhead/LH3DLib/development/LH3DIsland.h> /* For LH3DIsland::GetAltitude */
#include <Lionhead/LH3DLib/development/LH3DTech.h>   /* For LH3DTech */
#include <Lionhead/LH3DLib/development/LHMatrix.h>   /* For struct LHMatrix */

#include "Camera.h"      /* For class GCamera */
#include "ControlHand.h" /* For class CHand */
#include "Game.h"        /* For GGame */
#include "TempleRoom.h"  /* For class TempleRoom */
#include "Utils.h"       /* For GUtils */

#define CITADEL_HAND_FOCUS_OFFSET  5.0f
#define CITADEL_HAND_MIN_DISTANCE  1.0f
#define CITADEL_HAND_NEAR_DISTANCE 4.0f
#define CITADEL_HAND_FAR_DISTANCE  300.0f
#define CITADEL_HAND_ZOOM_IN_TIME  0.2f
#define CITADEL_HAND_ZOOM_OUT_TIME 0.4f
#define CITADEL_HAND_TURN_TIME     0.4f
#define CITADEL_HAND_MIN_OFFSET    0.0001f

enum
{
	CITADEL_HAND_ANIM_IDLE = 0,
	CITADEL_HAND_ANIM_HORIZONTAL = 1,
	CITADEL_HAND_ANIM_VERTICAL = 2
};

inline void GCamera::GetPosition(LHPoint& pos)
{
	pos = LH3DTech::g_camera.pos;
}

HandStateCitadel::HandStateCitadel(CHand* hand) : HandState(hand)
{
	Frame = 0;
}

void HandStateCitadel::Enter()
{
	Direction.SetPosition(LHPoint(0.0f, 1.0f, 0.0f));
	Frame = 0;
	field_0xc = 0;
	Distance.SetPosition(hand->DistanceFromView);
}

void HandStateCitadel::Update(float param_1, LHMatrix* param_2)
{
	LHPoint cameraPos;
	GGame::g_game->GetCamera()->GetPosition(cameraPos);
	LHPoint direction = TempleRoom::HandDirection;
	LHPoint focus = TempleRoom::HandFocus;

	float distance = focus.GetDistance(cameraPos) - CITADEL_HAND_FOCUS_OFFSET;
	if (distance < CITADEL_HAND_MIN_DISTANCE)
		distance = CITADEL_HAND_MIN_DISTANCE;

	LHPoint handPos;
	GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, distance, &handPos);
	GGame::g_game->MyInterface();

	distance = cameraPos.GetDistance(handPos);
	if (distance < CITADEL_HAND_NEAR_DISTANCE)
		distance = CITADEL_HAND_NEAR_DISTANCE;
	else if (distance > CITADEL_HAND_FAR_DISTANCE)
		distance = CITADEL_HAND_FAR_DISTANCE;

	if (Distance.GetDestination() > distance)
		Distance.SetDestination(distance, CITADEL_HAND_ZOOM_IN_TIME);
	else
		Distance.SetDestination(distance, CITADEL_HAND_ZOOM_OUT_TIME);
	Distance.Update(param_1);
	distance = Distance.GetCurrentValue();
	hand->SetDistanceFromView(distance);

	LH3DIsland::GetAltitude(hand->position);
	CHand::GetStandardSize();
	direction.Normalise();
	CHand::GetStandardSize();

	Direction.SetDestinationWithTime(direction, CITADEL_HAND_TURN_TIME);
	Direction.Update(LH3DTech::g_delta_time / 1000.0f);
	LHPoint current = Direction.GetCurrentValue();
	current.Normalise();

	GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, distance, &hand->position);

	CAnim* anim = hand->GetAnim(CITADEL_HAND_ANIM_IDLE, 0);
	Frame = (LH3DTech::g_delta_time + Frame) % anim->Duration;

	LHMatrix* buffer;
	if (field_0xc)
	{
		field_0xc = 0;
		hand->BlendToNewPosition();
		buffer = hand->TransformedMatrices;
	}
	else
	{
		buffer = param_2;
	}

	LHMatrix matrix;
	hand->fn_0046E160(matrix, hand->position, current);
	anim->FillBuffer(buffer, hand->GetMesh(), hand->field_0x47f4, hand->field_0x47f8, Frame, hand->frame, NULL, 1);

	CAnim* horizontal = hand->GetAnim(CITADEL_HAND_ANIM_HORIZONTAL, 0);
	float  dx = hand->field_0x48b8 - hand->ScreenPos.x;
	float  offsetX = dx > -CitadelHandMaxOffsetX ? (dx < CitadelHandMaxOffsetX ? dx : CitadelHandMaxOffsetX)
	                                             : -CitadelHandMaxOffsetX;
	if (horizontal != NULL && fabs(offsetX) > CITADEL_HAND_MIN_OFFSET)
	{
		horizontal->ModifyBufferByDifferenceFromFrame(
			buffer, horizontal->frames[(horizontal->FrameCount - 1) / 2], hand->GetMesh(), hand->field_0x47f4,
			hand->field_0x47f8,
			(offsetX + CitadelHandMaxOffsetX) / (CitadelHandMaxOffsetX + CitadelHandMaxOffsetX) * horizontal->Duration,
			NULL, NULL);
	}

	CAnim* vertical = hand->GetAnim(CITADEL_HAND_ANIM_VERTICAL, 0);
	float  dy = hand->ScreenPos.y - hand->field_0x48bc;
	float  offsetY = dy > -CitadelHandMaxOffsetY ? (dy < CitadelHandMaxOffsetY ? dy : CitadelHandMaxOffsetY)
	                                             : -CitadelHandMaxOffsetY;
	if (vertical != NULL && fabs(hand->field_0x48bc) > CITADEL_HAND_MIN_OFFSET)
	{
		vertical->ModifyBufferByDifferenceFromFrame(
			buffer, vertical->frames[(vertical->FrameCount - 1) / 2], hand->GetMesh(), hand->field_0x47f4,
			hand->field_0x47f8,
			(offsetY + CitadelHandMaxOffsetY) / (CitadelHandMaxOffsetY + CitadelHandMaxOffsetY) * vertical->Duration,
			NULL, NULL);
	}

	LH3DAnim::FinishTransform(buffer, hand->GetMesh(), matrix);
	hand->field_0x486c = hand->ScreenPos;
}
