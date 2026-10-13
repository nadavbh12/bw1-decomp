#ifndef BW1_DECOMP_LH3D_VB_INCLUDED_H
#define BW1_DECOMP_LH3D_VB_INCLUDED_H

#include <re_common.h> /* For bool32_t */

struct LH3DVB
{
	// BW1W120 008845b0 BW1M119 010cacb0 (LHCombined Release)
	static bool32_t Init();
	// BW1W120 00884680 BW1M119 010cabf0 (LHCombined Release)
	static void Close();
};

#endif /* BW1_DECOMP_LH3D_VB_INCLUDED_H */
