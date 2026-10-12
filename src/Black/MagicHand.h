#ifndef BW1_DECOMP_MAGIC_HAND_INCLUDED_H
#define BW1_DECOMP_MAGIC_HAND_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stdint.h> /* For uint32_t, uint8_t */

#include <re_common.h> /* For bool32_t */

#include "GameThing.h" /* For struct GameThing */

// Forward Declares

class Base;
class GameOSFile;
class GInterfaceStatus;
class Object;

class GMagicHand : public GameThing
{
public:
	uint8_t NumObjects; /* 0x14 */
	Object* Objects[1]; /* 0x18 */

	// Override methods

	// BW1W120 005faf60 BW1M119 013b55b0
	virtual ~GMagicHand();
	// BW1W120 005fb2a0 BW1M119 013b4e50
	virtual void CleanUpForSerialisation();
	// BW1W120 005faf30 BW1M119 013b4c50
	virtual char* GetDebugText();
	// BW1W120 005fb320 BW1M119 013b4c90
	virtual uint32_t Load(GameOSFile& file);
	// BW1W120 005fb2c0 BW1M119 013b4d60
	virtual uint32_t Save(GameOSFile& file);
	// BW1W120 005faf20 BW1M119 013b4c10
	virtual uint32_t GetSaveType();

	// Constructors

	// BW1W120 005faf00 BW1M119 013b5640
	GMagicHand();

	// Non-virtual methods

	// BW1W120 005faf70 BW1M119 inlined
	void fn_005FAF70();
	// BW1W120 005fb040 BW1M119 01077ae0
	Object* GetObjectFromHand() const;
	// BW1W120 005faf80 BW1M119 01019fb0
	void DrawContents() const;
	// BW1W120 005fafc0 BW1M119 013b53f0
	bool32_t AddToHand(Object* object, GInterfaceStatus* status);
	// BW1W120 005fb060 BW1M119 013b5280
	Object* RemoveFromHand(GInterfaceStatus* status);
	// BW1W120 005fb0b0 BW1M119 013b5150
	bool32_t RemoveFromHand(Object* object, GInterfaceStatus* status);
	// BW1W120 005fb130 BW1M119 010963a0
	bool32_t Validate(GInterfaceStatus* status);
	// BW1W120 005fb190 BW1M119 010835c0
	bool32_t Process(GInterfaceStatus* status);
};

#endif /* BW1_DECOMP_MAGIC_HAND_INCLUDED_H */
