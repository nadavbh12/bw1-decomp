#ifndef BW1_DECOMP_P_SYS_HAND_FX_INCLUDED_H
#define BW1_DECOMP_P_SYS_HAND_FX_INCLUDED_H

#include <assert.h> /* For static_assert */
#include <stddef.h> /* For NULL */
#include <stdint.h> /* For uint32_t, uint8_t */

#include <chlasm/Enum.h> /* For enum TRIBE_TYPE */

#include <Lionhead/LHLib/ver5.0/LHWin.h> /* For operator new(size_t, const char*, uint32_t) */

#include "PSysBase.h" /* For struct PSysBase */

#define P_SYS_HAND_FX_HEADER_FILE "C:\\dev\\MP\\Black\\PSysHandFX.h"

// Forward Declares

class LH3DCreature;
struct LH3DMaterial;
struct LH3DMesh;
class LH3DObject;
struct LH3DTexture;
struct LHMatrix;
struct LHPoint;
class PowerSpinRunner;

enum HAND_GLOW_TYPE
{
	HAND_GLOW_TYPE_0 = 0,
	HAND_GLOW_TYPE_1 = 1,
	HAND_GLOW_TYPE_2 = 2,
	HAND_GLOW_TYPE_3 = 3,
	HAND_GLOW_TYPE_LAST = 4,
};

template <class T> class GJDoubleLinkList
{
public:
	T*   Head;
	long Count;

	// BW1W120 inlined BW1M119 01417b50
	GJDoubleLinkList() : Head(NULL), Count(0) {}
	// BW1W120 inlined BW1M119 01417ba0
	~GJDoubleLinkList() {}

	long GetCount() const { return Count; }
	T*   GetHead() const { return Head; }

	// BW1W120 inlined BW1M119 01417c20
	void InsertIntoList(T* item)
	{
		item->Next = Head;
		item->Prev = NULL;
		if (Head != NULL)
		{
			Head->Prev = item;
		}
		Head = item;
		Count++;
	}
	// BW1W120 inlined BW1M119 01417e20
	void RemoveFromList(T* item)
	{
		if (item->Prev != NULL)
		{
			item->Prev->Next = item->Next;
		}
		else
		{
			Head = item->Next;
		}
		if (item->Next != NULL)
		{
			item->Next->Prev = item->Prev;
		}
		Count--;
		item->Next = NULL;
		item->Prev = NULL;
	}
	// BW1W120 inlined BW1M119 01417cb0
	void DeleteFromList(T* item)
	{
		RemoveFromList(item);
		delete item;
	}
	// BW1W120 inlined BW1M119 01417d60
	void DeleteAll()
	{
		while (Head != NULL)
		{
			DeleteFromList(Head);
		}
	}
};

class PSysHandFX
{
public:
	// Static methods

	// BW1W120 0068c9d0 BW1M119 01417a90
	static PSysHandFX* CreateHandFX();
	// BW1W120 0068def0 BW1M119 014166a0
	static void CreateTribalPowerColumn(TRIBE_TYPE tribe, const LHPoint& pos, unsigned long colour);
	// BW1W120 0068de90 BW1M119 01416770
	static float GetPUPulseDuration();
	// BW1W120 0068dea0 BW1M119 014167c0
	static float GetPUPulseDelay();

	// Virtual methods

	virtual void RemoveAllPermBands() = 0;
	virtual void DoRemoveFromHandVisual() = 0;
	virtual void AddSpellToHandVisuals(bool pulse) = 0;
	virtual void SetPULevel(long level, bool pulse) = 0;
	virtual void DrawHandFX() = 0;
	virtual long GetPULevel() = 0;
	virtual void StartTribalPowerRing(TRIBE_TYPE tribe) = 0;
	virtual void StopTribalPowerRing() = 0;
	virtual void ReleaseOrCreateTribalPowerRing(TRIBE_TYPE tribe) = 0;
	virtual void Release() = 0;
};

class PHandFX : public PSysHandFX
{
public:
	class Band : public PSysBase
	{
	public:
		Band*       Prev;       /* 0x14 */
		Band*       Next;       /* 0x18 */
		LH3DObject* Object;     /* 0x1c */
		float       field_0x20; /* 0x20 */
		float       field_0x24; /* 0x24 */
		float       field_0x28; /* 0x28 */
		float       field_0x2c; /* 0x2c */
		uint8_t     field_0x30; /* 0x30 */
		uint8_t     field_0x31; /* 0x31 */
		uint32_t    field_0x34; /* 0x34 */
		uint32_t    field_0x38; /* 0x38 */
		bool        Permanent;  /* 0x3c */
		long        Index;      /* 0x40 */
		bool        Finished;   /* 0x44 */
		bool        field_0x45; /* 0x45 */
		bool        field_0x46; /* 0x46 */

		// Override methods

		// BW1W120 0068cae0 BW1M119 014177e0
		virtual ~Band();

		// Constructors

		// BW1W120 0068ca30 BW1M119 014178a0
		Band(LH3DMesh* mesh, long index, float param_3, bool permanent, float param_5, uint8_t param_6, uint8_t param_7,
		     bool param_8, unsigned long param_9, unsigned long param_10);

		// Non-virtual methods

		// BW1W120 0068d6d0 BW1M119 01416b50
		void Draw(PHandFX* fx, float time, const LHMatrix& matrix);
	};

	LH3DMesh*              BandMesh;        /* 0x4 */
	PowerSpinRunner*       TribalPowerRing; /* 0x8 */
	float                  field_0xc;       /* 0xc */
	float                  field_0x10;      /* 0x10 */
	float                  field_0x14;      /* 0x14 */
	float                  field_0x18;      /* 0x18 */
	float                  field_0x1c;      /* 0x1c */
	uint8_t                field_0x20;      /* 0x20 */
	uint8_t                field_0x21;      /* 0x21 */
	uint8_t                field_0x22;      /* 0x22 */
	uint8_t                field_0x23;      /* 0x23 */
	float                  field_0x24;      /* 0x24 */
	float                  field_0x28;      /* 0x28 */
	float                  field_0x2c;      /* 0x2c */
	float                  field_0x30;      /* 0x30 */
	uint8_t                field_0x34;      /* 0x34 */
	uint8_t                field_0x35;      /* 0x35 */
	uint8_t                field_0x36;      /* 0x36 */
	uint8_t                field_0x37;      /* 0x37 */
	float                  field_0x38;      /* 0x38 */
	float                  field_0x3c;      /* 0x3c */
	float                  field_0x40;      /* 0x40 */
	GJDoubleLinkList<Band> PermBands;       /* 0x44 */
	GJDoubleLinkList<Band> TempBands;       /* 0x4c */
	float                  field_0x54;      /* 0x54 */
	float                  field_0x58;      /* 0x58 */
	float                  field_0x5c;      /* 0x5c */
	uint8_t                field_0x60;      /* 0x60 */
	LH3DTexture*           FlowTexture;     /* 0x64 */
	LH3DMaterial*          FlowMaterial;    /* 0x68 */
	float                  field_0x6c;      /* 0x6c */
	bool                   field_0x70;      /* 0x70 */
	bool                   field_0x71;      /* 0x71 */

	// Override methods

	// BW1W120 0068d060 BW1M119 014174a0
	virtual void RemoveAllPermBands();
	// BW1W120 0068ce90 BW1M119 01417500
	virtual void DoRemoveFromHandVisual();
	// BW1W120 0068de20 BW1M119 01416800
	virtual void AddSpellToHandVisuals(bool pulse);
	// BW1W120 0068dda0 BW1M119 01416960
	virtual void SetPULevel(long level, bool pulse);
	// BW1W120 0068dd60 BW1M119 0101e350
	virtual void DrawHandFX();
	// BW1W120 0068ccb0 BW1M119 01417640
	virtual long GetPULevel();
	// BW1W120 0068df20 BW1M119 01416510
	virtual void StartTribalPowerRing(TRIBE_TYPE tribe);
	// BW1W120 0068dfa0 BW1M119 01416480
	virtual void StopTribalPowerRing();
	// BW1W120 0068dfc0 BW1M119 01416310
	virtual void ReleaseOrCreateTribalPowerRing(TRIBE_TYPE tribe);
	// BW1W120 0068ca10 BW1M119 014179e0
	virtual void Release();

	// Constructors

	// BW1W120 0068cb10 BW1M119 01417680
	PHandFX();
	// BW1W120 0068cbc0 BW1M119 inlined
	~PHandFX();

	// Non-virtual methods

	// BW1W120 0068cc70 BW1M119 inlined
	void fn_0068CC70();
	// BW1W120 0068ccc0 BW1M119 inlined
	void fn_0068CCC0(float param_1);
	// BW1W120 0068cd30 BW1M119 inlined
	void fn_0068CD30(float param_1);
	// BW1W120 0068cda0 BW1M119 inlined
	void fn_0068CDA0(float param_1);
	// BW1W120 0068cf60 BW1M119 inlined
	void fn_0068CF60();
	// BW1W120 0068d000 BW1M119 inlined
	void fn_0068D000();
	// BW1W120 0068d0c0 BW1M119 0107cc90
	void Draw(float time);
};

class HandGlow
{
public:
	float       Power;      /* 0x0 */
	float       Scale;      /* 0x4 */
	float       field_0x8;  /* 0x8 */
	LH3DObject* Object;     /* 0xc */
	void*       field_0x10; /* 0x10 */

	// Static methods

	// BW1W120 inlined BW1M119 inlined
	static HandGlow* Create() { return new (P_SYS_HAND_FX_HEADER_FILE, 62) HandGlow; }

	// Constructors

	// BW1W120 0068e070 BW1M119 01416210
	HandGlow();
	// BW1W120 inlined BW1M119 01416150
	~HandGlow();

	// Non-virtual methods

	// BW1W120 0068e120 BW1M119 01415fa0
	void DrawAt(const LHMatrix& matrix, float scale);
	// BW1W120 0068e1f0 BW1M119 inlined
	void SetPower(float power);
	// BW1W120 0068e200 BW1M119 01415f70
	void SetScale(float scale);
	// BW1W120 0068e210 BW1M119 inlined
	float GetPower() const;
	// BW1W120 0068e220 BW1M119 inlined
	float GetScale() const;
};

struct HandGlows
{
	HandGlow* Glows[HAND_GLOW_TYPE_LAST];

	// Static methods

	// BW1W120 0068e230 BW1M119 01415f00
	static HandGlows* Create();

	// Constructors

	// BW1W120 0068e280 BW1M119 inlined
	HandGlows();
	// BW1W120 0068e2a0 BW1M119 inlined
	~HandGlows();

	// Non-virtual methods

	// BW1W120 0068e260 BW1M119 01415e00
	void Release();
	// BW1W120 0068e300 BW1M119 01415cd0
	void Start(HAND_GLOW_TYPE type);
	// BW1W120 0068e2d0 BW1M119 inlined
	void Stop(HAND_GLOW_TYPE type);
	// BW1W120 0068e340 BW1M119 01415af0
	void SetScalePowerTime(HAND_GLOW_TYPE type, float scale, float power, float time);
	// BW1W120 0068e3a0 BW1M119 01415a60
	void GetScalePower(HAND_GLOW_TYPE type, float* scale, float* power);
	// BW1W120 0068e3f0 BW1M119 01415990
	bool IsAnyActive();
	// BW1W120 0068e430 BW1M119 01415510
	void Draw(LH3DCreature* creature);
};

#endif /* BW1_DECOMP_P_SYS_HAND_FX_INCLUDED_H */
