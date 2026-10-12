#include "MapCellConstants.h"
#include "CreatureAttitudeConstants.h"
#include "HandStatePlayAnim.h"

#include "ColourConstants.h" /* For White */

HandStatePlayAnim::HandStatePlayAnim(CHand* hand) : HandState(hand)
{
	Time = 0;
}

void HandStatePlayAnim::Enter()
{
	Time = 0;
}
