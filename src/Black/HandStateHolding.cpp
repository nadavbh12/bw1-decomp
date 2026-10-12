#include "MapCellConstants.h"
#include "CreatureAttitudeConstants.h"
#include "HandStateHolding.h"

#include "ColourConstants.h" /* For White */

HandStateHolding::HandStateHolding(CHand* hand) : HandState(hand)
{
	GiveToCreatureTimeLeft = 0;
	AnimationTime = 0;
	UnusedClearedOnCreate = 0;
}
