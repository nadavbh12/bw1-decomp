#ifndef BW1_DECOMP_LH3D_Z_SORTER_INCLUDED_H
#define BW1_DECOMP_LH3D_Z_SORTER_INCLUDED_H

#include <stdint.h> /* For uint32_t */

#include "DrawingObject.h"

// Declaration-only interface; no instance allocation or guessed object layout.
class LH3DZSorter
{
public:
	typedef void (DrawingObject::*DrawCallback)();

	struct ZObject
	{
		uint32_t     field_0x0;
		void*        Object; /* 0x4 */
		DrawCallback Draw;
		uint32_t     Flags;
		float        Distance;
		ZObject*     Next; /* 0x14 */
	};

	// BW1W120 00ee9d30 BW1M119 01356448 (LHCombined Release)
	static uint32_t UserData;
	// BW1W120 00ee9d38
	static ZObject* Dat_00EE9D38;

	// ECX=this, EDX=object; three stack arguments, RET 0x0c.
	// BW1W120 0083f310 BW1M119 0101bf70 (LHCombined Release)
	void __fastcall NewZObject(void* object, DrawCallback draw, float distance, unsigned long flags);
	// BW1W120 0083f2b0 BW1M119 010d83f0 (LHCombined Release)
	static LH3DZSorter* __fastcall Create(long count);
	// BW1W120 0083f2f0
	void fn_0083F2F0();
	// BW1W120 0083f3b0 BW1M119 0100ad50 (LHCombined Release)
	void OpenSort();
};

#endif /* BW1_DECOMP_LH3D_Z_SORTER_INCLUDED_H */
