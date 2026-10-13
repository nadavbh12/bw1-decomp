#ifndef BW1_DECOMP_LH3D_Z_SORTER_INCLUDED_H
#define BW1_DECOMP_LH3D_Z_SORTER_INCLUDED_H

#include "DrawingObject.h"

// Declaration-only interface; no instance allocation or guessed object layout.
class LH3DZSorter
{
public:
	typedef void (DrawingObject::*DrawCallback)();
	// ECX=this, EDX=object; three stack arguments, RET 0x0c.
	// BW1W120 0083f310 BW1M119 0101bf70 (LHCombined Release)
	void __fastcall NewZObject(void* object, DrawCallback draw, float distance, unsigned long flags);
	// BW1W120 0083f2b0 BW1M119 010d83f0 (LHCombined Release)
	static LH3DZSorter* __fastcall Create(long count);
	// BW1W120 0083f2f0
	void fn_0083F2F0();
};

#endif /* BW1_DECOMP_LH3D_Z_SORTER_INCLUDED_H */
