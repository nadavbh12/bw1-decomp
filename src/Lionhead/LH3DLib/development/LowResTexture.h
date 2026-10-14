#ifndef BW1_DECOMP_LOW_RES_TEXTURE_INCLUDED_H
#define BW1_DECOMP_LOW_RES_TEXTURE_INCLUDED_H

#include <stdint.h> /* For uint32_t */

#include "LH3DTexture.h" /* For LH3DTexture::g_b_use_low_res */

enum
{
	LOW_RES_TEXTURE_COUNT = 0x100,
	LOW_RES_TEXTURE_COUNT_LOW_RES = 0x80,
};

struct LowResTexture
{
	uint32_t field_0x0;
	uint8_t  field_0x4[0x18];

	// BW1W120 00fa76d0
	static LowResTexture Dat_00FA76D0[LOW_RES_TEXTURE_COUNT];
	// BW1W120 00c398ec
	static int g_count;
	// BW1W120 00c398f0
	static float Dat_00C398F0;

	// BW1W120 inlined BW1M119 010459f0 (LHCombined Release)
	static void Open()
	{
		g_count = LH3DTexture::g_b_use_low_res ? LOW_RES_TEXTURE_COUNT_LOW_RES : LOW_RES_TEXTURE_COUNT;
		for (int i = 0; i < g_count; i++)
		{
			Dat_00FA76D0[i].field_0x0 = 0;
		}
		Dat_00C398F0 = (float)(g_count - 1);
	}
};

#endif /* BW1_DECOMP_LOW_RES_TEXTURE_INCLUDED_H */
