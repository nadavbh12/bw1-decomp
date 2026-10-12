#include "MapCellConstants.h"
#include "CreatureAttitudeConstants.h"
#include "HandStateHolding.h"

#include "ColourConstants.h" /* For White */

HandStateHolding::HandStateHolding(CHand* hand) : HandState(hand)
{
	GiveToCreatureTime = 0;
	Frame = 0;
	field_0x10 = 0;
}
