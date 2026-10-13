#ifndef BW1_DECOMP_LH3D_LANDSCAPE_INCLUDED_H
#define BW1_DECOMP_LH3D_LANDSCAPE_INCLUDED_H

class LH3DLandscape
{
public:
	static void TextureUpdateThread(); // 00871f00
	// BW1W120 00871220 BW1M119 010535d0 (LHCombined Release)
	static void Release();
	// BW1W120 008713f0 BW1M119 01052c90 (LHCombined Release)
	static void Open();
	// BW1W120 00871440 BW1M119 01052c20 (LHCombined Release)
	static void Close();
};

#endif /* BW1_DECOMP_LH3D_LANDSCAPE_INCLUDED_H */
