#ifndef BW1_DECOMP_LH3D_VB_INCLUDED_H
#define BW1_DECOMP_LH3D_VB_INCLUDED_H

#include <re_common.h> /* For bool32_t */

struct LH3DVB
{
	// BW1W120 00faa808 BW1M119 0134216c (LHCombined Release)
	static int curvmt;
	// BW1W120 00faa80c BW1M119 0134217c (LHCombined Release)
	static int curv;
	// BW1W120 00faa810 BW1M119 01342170 (LHCombined Release)
	static bool32_t newframemt;
	// BW1W120 00faa814 BW1M119 01342180 (LHCombined Release)
	static bool32_t newframe;

	// BW1W120 008845b0 BW1M119 010cacb0 (LHCombined Release)
	static bool32_t Init();
	// BW1W120 00884680 BW1M119 010cabf0 (LHCombined Release)
	static void Close();
};

#endif /* BW1_DECOMP_LH3D_VB_INCLUDED_H */
