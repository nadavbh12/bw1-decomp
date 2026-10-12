#ifndef BW1_DECOMP_BOOKMARK_INCLUDED_H
#define BW1_DECOMP_BOOKMARK_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stdint.h> /* For uint32_t */

#include <chlasm/CreatureEnum.h> /* For enum CREATURE_TYPE */

#include <re_common.h> /* For bool32_t */

// Forward Declares

struct LH3DSprite;
struct MapCoords;
class GameOSFile;

class Bookmark
{
public:
	// BW1W120 00c58b30
	static Bookmark Bookmarks[];
	// BW1W120 009cd384
	static bool32_t Dat_009CD384;
	// BW1W120 00439d40 BW1M119 010b9f40
	static void Clear(unsigned long index);
	// BW1W120 00439d60 BW1M119 010b9ea0
	static void ClearAll();
	// BW1W120 00439dd0 BW1M119 01085110
	static void ProcessAll();
	// TODO: Instance layout is not recovered; only the static serialization interface is declared.
	// BW1W120 0043a3f0 BW1M119 010b8990
	static unsigned int SaveAll(GameOSFile& file);
	// BW1W120 0043a5e0 BW1M119 010b8600
	static unsigned int LoadAll(GameOSFile& file);
	// BW1W120 00439e50 BW1M119 010b99c0
	static void SetBookMark(unsigned long index, const MapCoords& coords);
	// BW1W120 0043a070 BW1M119 010b8fa0
	static void SetCameraToBookMark(unsigned long index);
	// BW1W120 0043a2c0 BW1M119 010b8d20
	static void SetFocusNextBookmark();
	// BW1W120 0043a320 BW1M119 010b8e60
	static void SetFocusPreviousBookmark();
	// BW1W120 0043a380 BW1M119 010b8cc0
	static void SetFocusBeforeJumpBookmark();
	// BW1W120 0043a390 BW1M119 010b8b60
	static void SetFollow(long index);
	// BW1W120 0043aa60 BW1M119 010b7b00
	static void SetFocusToOwnedTown();

	uint32_t field_0x0;
	uint32_t Flags; /* 0x4 */
	uint8_t  field_0x8[0x1c];
};

struct BookmarkGraphic
{
	LH3DSprite*   sprite; /* 0x0 */
	CREATURE_TYPE type;

	// Static methods

	// BW1W120 0043a7d0 BW1M119 010b8360
	static BookmarkGraphic* Create(int param_1, CREATURE_TYPE type);
};

#endif /* BW1_DECOMP_BOOKMARK_INCLUDED_H */
