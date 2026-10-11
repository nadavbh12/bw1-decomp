#include "HandStateCamera.h"
#include "CreatureAttitudeConstants.h"
#include "GameTimeConstants.h"

#include <Lionhead/LH3DLib/development/LH3DAnim.h> /* For struct CAnim */
#include <Lionhead/LH3DLib/development/LH3DTech.h> /* For LH3DTech */
#include <Lionhead/LH3DLib/development/LHMatrix.h> /* For struct LHMatrix */

#include "Camera.h"          /* For class GCamera */
#include "CameraHelp.h"      /* For CameraHelp::EnabledFeatures */
#include "CameraModeNew3.h"  /* For class CameraModeNew3 */
#include "ColourConstants.h" /* For White */
#include "ControlHand.h"     /* For class CHand */
#include "Game.h"            /* For GGame::g_game */
#include "Interface.h"       /* For class GInterface */
#include "Utils.h"           /* For GUtils */

HandStateCamera::HandStateCamera(CHand* hand) : HandState(hand)
{
	AnimationTime = 0;
}

void HandStateCamera::Enter()
{
	field_0x18.SetPosition(LHPoint(0.0f, 1.0f, 0.0f));
	AnimationTime = 0;
	field_0xc = 0;
	field_0xa8.SetPosition(hand->DistanceFromView);
	field_0x10 = 0;
	Anim = 0;
}

void HandStateCamera::Update(float param_1, LHMatrix* param_2)
{
	LHPoint dir = HandStateNormal::Global_00d13f60;
	long    anim = HAND_ANIM_POINT;
	GGame::g_game->GetCamera();
	LHPoint cameraPos = *LH3DTech::GetCameraPosition();
	float   standardDistance = hand->WorkOutStandardDistanceFromView(false);
	GGame::g_game->MyInterface();
	CameraModeNew3* mode = dynamic_cast<CameraModeNew3*>(GGame::g_game->GetCamera()->GetCurrentMode());
	hand->SetDistanceFromView(hand->position.GetRange(cameraPos));
	CHand::GetStandardSize();
	if (field_0x10 == 0)
	{
		field_0xa8.SetDestination(hand->position.GetRange(cameraPos), 0.4f);
		field_0xa8.Update(param_1);
		param_1 = min(standardDistance, field_0xa8.GetCurrentValue());
		hand->SetDistanceFromView(param_1);
		GUtils::SetPointFromScreenPointAndDistance(&hand->ScreenPos, param_1, &hand->position);
	}
	uint32_t triconFlags = mode != NULL ? mode->MouseTriconFlags : 0;
	param_1 = 0.0f;
	if (mode->HandStatus == CAMERA_MODE_HAND_STATUS_GRABBING_LAND ||
	    CameraModeNew3::Global_00c5b100.GetCurrentValue() > 0.01f)
	{
		anim = HAND_ANIM_CAMERA_GRAB;
	}
	else if (triconFlags & HAND_TRICON_ROTATE)
	{
		if (CameraHelp::EnabledFeatures & CAMERA_FEATURE_ROTATE)
		{
			dir.Set(*LH3DTech::GetCameraTarget() - hand->position);
			dir.y += dir.GetNorme() * 0.3f;
			dir.Normalise();
			anim = HAND_ANIM_CAMERA_ROTATE;
			param_1 = hand->GetViewDistanceScale() * CHand::GetStandardSize() * 0.33f;
		}
	}
	else if (triconFlags & HAND_TRICON_ZOOM)
	{
		if (CameraHelp::EnabledFeatures & CAMERA_FEATURE_ZOOM)
		{
			anim = HAND_ANIM_CAMERA_ZOOM;
		}
	}
	else if (triconFlags & HAND_TRICON_PITCH)
	{
		if (CameraHelp::EnabledFeatures & CAMERA_FEATURE_PITCH)
		{
			anim = HAND_ANIM_CAMERA_PITCH;
		}
	}
	if (Anim != anim)
	{
		if (anim == HAND_ANIM_CAMERA_GRAB)
		{
			field_0x10 = 1;
			GInterface* in = GGame::g_game->MyInterface();
			if (in->interface_collide.object == NULL && in->interface_collide.bubble == NULL &&
			    in->interface_collide.leash == NULL)
			{
				hand->BlendToNewPosition();
				hand->position = HandStateHolding::Global_00d13f30;
			}
		}
		else if (Anim == HAND_ANIM_CAMERA_GRAB)
		{
			field_0x10 = 0;
		}
		Anim = anim;
		hand->BlendToNewPosition();
	}
	CAnim* animation = hand->GetAnim(anim, 0);
	AnimationTime = (LH3DTech::g_delta_time + AnimationTime) % animation->Duration;
	if (field_0xc)
	{
		field_0x18.SetDestinationWithTime(dir, 0.8f);
	}
	else
	{
		field_0x18.SetPosition(dir);
		field_0xc = 0;
	}
	field_0x18.Update(LH3DTech::g_delta_time / 1000.0f);
	LHPoint direction = field_0x18.GetCurrentValue();
	direction.Normalise();
	LHPoint position = hand->position;
	position.y += param_1;
	LHMatrix matrix;
	hand->fn_0046E160(matrix, position, direction);
	animation->FillBuffer(param_2, hand->GetMesh(), hand->field_0x47f4, hand->field_0x47f8, AnimationTime, hand->frame,
	                      NULL, 1);

	animation = hand->GetAnim(anim + HAND_ANIM_LEAN_X, 0);
	float offset = hand->ScreenPos.x - hand->field_0x48b8;
	float lean =
		offset <= -HAND_LEAN_RANGE_X ? -HAND_LEAN_RANGE_X : (offset < HAND_LEAN_RANGE_X ? offset : HAND_LEAN_RANGE_X);
	if (animation != NULL && fabs(lean) > 0.0001f)
	{
		animation->ModifyBufferByDifferenceFromFrame(
			param_2, animation->frames[(animation->FrameCount - 1) / 2], hand->GetMesh(), hand->field_0x47f4,
			hand->field_0x47f8, (long)((lean + HAND_LEAN_RANGE_X) / (HAND_LEAN_RANGE_X * 2) * animation->Duration),
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
			hand->field_0x47f8, (long)((lean + HAND_LEAN_RANGE_Y) / (HAND_LEAN_RANGE_Y * 2) * animation->Duration),
			NULL, NULL);
	}
	LH3DAnim::FinishTransform(param_2, hand->GetMesh(), matrix);
	hand->field_0x486c = hand->ScreenPos;
}
