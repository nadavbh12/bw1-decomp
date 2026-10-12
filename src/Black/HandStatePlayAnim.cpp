#include "MapCellConstants.h"
#include "CreatureAttitudeConstants.h"
#include "GameTimeConstants.h"
#include "HandStatePlayAnim.h"

#include <Lionhead/LH3DLib/development/LH3DAnim.h> /* For struct CAnim */
#include <Lionhead/LH3DLib/development/LH3DTech.h> /* For LH3DTech::g_delta_time */
#include <Lionhead/LH3DLib/development/LHMatrix.h> /* For struct LHMatrix */

#include "ColourConstants.h" /* For White */
#include "ControlHand.h"     /* For class CHand */
#include "Game.h"            /* For GGame::g_game */

HandStatePlayAnim::HandStatePlayAnim(CHand* hand) : HandState(hand)
{
	AnimationTime = 0;
}

void HandStatePlayAnim::Enter()
{
	AnimationTime = 0;
}

void HandStatePlayAnim::Update(float param_1, LHMatrix* param_2)
{
	if (field_0x1C == 0)
	{
		field_0x1C = 1;
	}
	GGame::g_game->MyInterface();
	CAnim*  anim = hand->GetAnim(Anim, 0);
	LHPoint up(0.0f, 1.0f, 0.0f);
	hand->position = Position;
	LHMatrix matrix;
	hand->fn_0046E160(matrix, hand->position, up);
	anim->FillBuffer(param_2, hand->GetMesh(), hand->field_0x47f4, hand->field_0x47f8, AnimationTime, hand->frame, NULL,
	                 1);
	LH3DAnim::FinishTransform(param_2, hand->GetMesh(), matrix);
	hand->field_0x486c = hand->ScreenPos;
	AnimationTime += LH3DTech::g_delta_time;
	if (AnimationTime >= anim->Duration)
	{
		hand->field_0x491c = false;
	}
}
