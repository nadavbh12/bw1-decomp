#include "GameTimeConstants.h"
#include "PSysHandFX.h"

#include <math.h> /* For log */

#include <Lionhead/LH3DLib/development/LHPoint.h> /* For struct LHPoint */

static LHPoint ZeroPoint(0.0f, 0.0f, 0.0f);

#include "ColourConstants.h" /* For White */

static float OneOverLogHalf = 1.0f / (float)log(0.5);

#include <Lionhead/LH3DLib/development/LH3DAnim.h>
#include <Lionhead/LH3DLib/development/LH3DComplexObject.h>
#include <Lionhead/LH3DLib/development/LH3DMaterial.h>
#include <Lionhead/LH3DLib/development/LH3DMesh.h>
#include <Lionhead/LH3DLib/development/LH3DObject.h>
#include <Lionhead/LH3DLib/development/LH3DTech.h>
#include <Lionhead/LHAudio/ver7.0/LH_SamplePlayOptions.h>

#include <chlasm/LHSample.h>

#include "Audio.h"
#include "Game.h"
#include "GJUtils.h"
#include "Global.h"
#include "Interface.h"
#include "HelpText.h"
#include "Player.h"
#include "Creature3D.h"
#include "ControlHand.h"
#include "PowerSpin.h"

#define P_SYS_HAND_FX_FILE "C:\\dev\\MP\\Black\\PSysHandFX.cpp"

enum
{
	PHAND_FX_MAX_PERM_BANDS = 5,
	PHAND_FX_SPELL_BANDS = 5,
	PHAND_FX_TRIBE_POWER_TEXT = 1240,
};

const float         PUPulseDuration = 2.4f;
const float         PUPulseDelay = 2.1f;
const float         RingBandScale = 1.0f;
const float         RingBandSpacing = 0.1f;
const int           RingBandCount = 10;
const unsigned long RingBandColour = 0x80808;
const float         SpellBandSpacing = 0.1f;
const float         GlowHeadHeight = 1.3f;
const float         GlowSpinSpeed = 1.0f;
const float         GlowScale = 1.0f;

PSysHandFX* PSysHandFX::CreateHandFX()
{
	PHandFX* fx = new (P_SYS_HAND_FX_FILE, 147) PHandFX;
	fx->fn_0068CC70();
	return fx;
}

void PHandFX::Release()
{
	delete this;
}

PHandFX::Band::Band(LH3DMesh* mesh, long index, float param_3, bool permanent, float param_5, uint8_t param_6,
                    uint8_t param_7, bool param_8, unsigned long param_9, unsigned long param_10)
	: Prev(NULL), Next(NULL), Object(NULL), field_0x20(0.0f), field_0x24(0.0f), field_0x28(param_3),
	  field_0x2c(param_5), field_0x30(param_6), field_0x31(param_7), field_0x34(param_9), field_0x38(param_10),
	  Permanent(permanent), Index(index), Finished(false), field_0x45(false), field_0x46(param_8)
{
	Object = LH3DObject::Create(LH3DObject::STATIC);
	Object->SetMesh(mesh, NULL, NULL);
}

PHandFX::Band::~Band()
{
	if (Object != NULL)
	{
		Object->Release();
		Object = NULL;
	}
}

PHandFX::PHandFX()
	: BandMesh(NULL), TribalPowerRing(NULL), field_0xc(10.0f), field_0x10(100.0f), field_0x14(12.0f), field_0x18(10.0f),
	  field_0x1c(40.0f), field_0x20(15), field_0x21(3), field_0x22(50), field_0x23(5), field_0x24(3.5f),
	  field_0x28(1.0f), field_0x2c(6.0f), field_0x30(0.3f), field_0x34(120), field_0x35(20), field_0x36(130),
	  field_0x37(20), field_0x38(0.85f), field_0x6c(0.0f), field_0x70(false), field_0x71(false)
{
	FlowTexture = NULL;
	FlowMaterial = NULL;
	field_0x54 = 0.0f;
	field_0x58 = 0.0f;
	field_0x60 = 32;
	field_0x5c = -20.0f;
	field_0x3c = 4.0f;
	field_0x40 = 0.5f;
}

PHandFX::~PHandFX()
{
	if (BandMesh != NULL)
	{
		BandMesh = NULL;
	}
	PermBands.DeleteAll();
	TempBands.DeleteAll();
}

void PHandFX::fn_0068CC70()
{
	if (BandMesh == NULL)
	{
		MaterialProperties props;
		props.field_0x3 = true;
		props.field_0x4 = true;
		props.field_0x0 = true;
		props.cull = true;
		props.field_0x1 = false;
		BandMesh = GJUtils::GetSharedMesh(".\\data\\spells\\meshes\\Power_Up_Band.L3d", props);
	}
}

long PHandFX::GetPULevel()
{
	return PermBands.Count;
}

void PHandFX::fn_0068CCC0(float param_1)
{
	PermBands.InsertIntoList(new (P_SYS_HAND_FX_FILE, 269) Band(BandMesh, PermBands.GetCount() + 1, param_1, true,
	                                                            field_0x38, field_0x37, field_0x36, false, 0, 0));
}

void PHandFX::fn_0068CD30(float param_1)
{
	TempBands.InsertIntoList(new (P_SYS_HAND_FX_FILE, 274) Band(BandMesh, TempBands.GetCount(), param_1, false,
	                                                            field_0x38, field_0x35, field_0x34, false, 0, 0));
}

void PHandFX::fn_0068CDA0(float param_1)
{
	float scale = GJUtils::Linterp(field_0x24, field_0x28, param_1);
	float endA = field_0x23;
	float startA = field_0x21;
	float endB = field_0x22;
	float startB = field_0x20;
	TempBands.InsertIntoList(new (P_SYS_HAND_FX_FILE, 283) Band(
		BandMesh, TempBands.GetCount(), 0.0f, false, scale, (uint8_t)GJUtils::Linterp(startA, endA, param_1),
		(uint8_t)GJUtils::Linterp(startB, endB, param_1), false, 0, 0));
}

void PHandFX::DoRemoveFromHandVisual()
{
	float                endScale = field_0x28;
	uint8_t              endAlpha = field_0x23;
	uint8_t              startAlpha = field_0x22;
	LH_SamplePlayOptions options;
	options.Bank = GGlobal::Global.audio->AudioBanks[AUDIO_SFX_BANK_TYPE_IN_GAME];
	options.SampleNumber = LH_SAMPLE_G_SHAKEHAND_01;
	options.AttachedObject = (Base*)this;
	options.Positional = 0;
	GGlobal::Global.audio->PlaySoundEffect(&options);
	TempBands.InsertIntoList(new (P_SYS_HAND_FX_FILE, 304) Band(BandMesh, TempBands.GetCount(), 0.0f, false, endScale,
	                                                            endAlpha, startAlpha, true, 0, 0));
}

void PHandFX::fn_0068CF60()
{
	float time = 0.0f;
	for (int i = 0; i < RingBandCount; i++)
	{
		TempBands.InsertIntoList(new (P_SYS_HAND_FX_FILE, 322)
		                             Band(BandMesh, TempBands.GetCount() * RingBandCount, time, false, RingBandScale,
		                                  20, 100, true, RingBandColour, RingBandColour));
		time += RingBandSpacing;
	}
}

void PHandFX::fn_0068D000()
{
	if (PermBands.Count != 0)
	{
		PermBands.DeleteFromList(PermBands.Head);
	}
}

void PHandFX::RemoveAllPermBands()
{
	PermBands.DeleteAll();
}

void PHandFX::DrawHandFX()
{
	Draw((GGame::g_game->GameFlags & GAME_FLAG_PAUSED) ? 0.0f : LH3DTech::GetGameTimeInc() * 0.001f);
}

void PHandFX::SetPULevel(long level, bool pulse)
{
	long change = level - GetPULevel();
	if (change > 0)
	{
		for (long i = 0; i < change && PermBands.Count != PHAND_FX_MAX_PERM_BANDS; i++)
		{
			fn_0068CCC0(pulse ? GetPUPulseDuration() : 0.0f);
		}
	}
	else if (change < 0)
	{
		for (long i = 0; i < -change && PermBands.Count != 0; i++)
		{
			fn_0068D000();
		}
	}
}

void PHandFX::AddSpellToHandVisuals(bool pulse)
{
	float start = pulse ? GetPUPulseDuration() : 0.0f;
	for (long i = 0; i < PHAND_FX_SPELL_BANDS; i++)
	{
		fn_0068CD30((i + 1) * SpellBandSpacing + start);
	}
	GGlobal::Global.audio->PlaySoundEffect(NULL, LH_SAMPLE_G_SPELLPOWERUPBAND, 3, 0, 0, 0, AUDIO_SFX_BANK_TYPE_IN_GAME);
}

float PSysHandFX::GetPUPulseDuration()
{
	return PUPulseDuration;
}

float PSysHandFX::GetPUPulseDelay()
{
	return PUPulseDelay;
}

static char16_t* GetTribePowerText(TRIBE_TYPE tribe)
{
	if (tribe <= TRIBE_TYPE_NONE)
	{
		return NULL;
	}
	if (tribe >= TRIBE_TYPE_LAST)
	{
		return NULL;
	}
	return HelpTextData::GetTextL(PHAND_FX_TRIBE_POWER_TEXT + tribe);
}

void PSysHandFX::CreateTribalPowerColumn(TRIBE_TYPE tribe, const LHPoint& pos, unsigned long colour)
{
	PowerSpinRunner::Create(GetTribePowerText(tribe), pos, LH3DColor(colour), false);
}

void PHandFX::StartTribalPowerRing(TRIBE_TYPE tribe)
{
	StopTribalPowerRing();
	LHPoint   pos = LH3DTech::g_camera_to_world.GetPos();
	LH3DColor colour(GGame::g_game->MyPlayer()->GetPlayerColour() | 0xff000000);
	TribalPowerRing = PowerSpinRunner::Create(GetTribePowerText(tribe), pos, colour, true);
}

void PHandFX::ReleaseOrCreateTribalPowerRing(TRIBE_TYPE tribe)
{
	LHPoint pos = GGame::g_game->MyInterface()->hand.Get()->position;
	if (TribalPowerRing != NULL)
	{
		TribalPowerRing->Detach();
		TribalPowerRing->field_0x9c0 = pos;
		TribalPowerRing = NULL;
	}
	else
	{
		CreateTribalPowerColumn(tribe, pos, GGame::g_game->MyPlayer()->GetPlayerColour());
	}
}

void PHandFX::StopTribalPowerRing()
{
	if (TribalPowerRing != NULL)
	{
		delete TribalPowerRing;
		TribalPowerRing = NULL;
	}
}

void HandGlow::SetPower(float power)
{
	Power = power;
}

void HandGlow::SetScale(float scale)
{
	Scale = scale;
}

float HandGlow::GetPower() const
{
	return Power;
}

float HandGlow::GetScale() const
{
	return Scale;
}

HandGlows* HandGlows::Create()
{
	return new (P_SYS_HAND_FX_FILE, 925) HandGlows;
}

void HandGlows::Release()
{
	delete this;
}

HandGlows::HandGlows()
{
	for (int i = 0; i < HAND_GLOW_TYPE_LAST; i++)
	{
		Glows[i] = NULL;
	}
}

HandGlows::~HandGlows()
{
	for (int i = 0; i < HAND_GLOW_TYPE_LAST; i++)
	{
		if (Glows[i] != NULL)
		{
			delete Glows[i];
			Glows[i] = NULL;
		}
	}
}

void HandGlows::Stop(HAND_GLOW_TYPE type)
{
	if (Glows[type] != NULL)
	{
		delete Glows[type];
		Glows[type] = NULL;
	}
}

void HandGlows::Start(HAND_GLOW_TYPE type)
{
	if (Glows[type] == NULL)
	{
		Glows[type] = HandGlow::Create();
	}
}

void HandGlows::SetScalePowerTime(HAND_GLOW_TYPE type, float scale, float power, float time)
{
	if (power == 0.0f)
	{
		Stop(type);
		return;
	}
	if (Glows[type] == NULL)
	{
		Start(type);
	}
	Glows[type]->SetPower(power);
	Glows[type]->SetScale(scale);
}

void HandGlows::GetScalePower(HAND_GLOW_TYPE type, float* scale, float* power)
{
	if (type >= 0 && type < HAND_GLOW_TYPE_LAST && Glows[type] != NULL)
	{
		*scale = Glows[type]->GetScale();
		*power = Glows[type]->GetPower();
	}
	else
	{
		*power = 0.0f;
		*scale = 0.0f;
	}
}

bool HandGlows::IsAnyActive()
{
	for (int i = 0; i < HAND_GLOW_TYPE_LAST; i++)
	{
		if (Glows[i] != NULL && Glows[i]->GetPower() > 0.0f)
		{
			return true;
		}
	}
	return false;
}

void LH3DCreature::UpdateHandGlows(LHSoundAction action)
{
	HAND_GLOW_TYPE type = HAND_GLOW_TYPE_0;
	switch (action)
	{
	case LH_SOUND_ACTION_0x5f:
	case LH_SOUND_ACTION_0x60:
	case LH_SOUND_ACTION_0x61:
	case LH_SOUND_ACTION_0x62:
	case LH_SOUND_ACTION_0x63:
		type = HAND_GLOW_TYPE_0;
		break;
	case LH_SOUND_ACTION_0x64:
	case LH_SOUND_ACTION_0x65:
	case LH_SOUND_ACTION_0x66:
	case LH_SOUND_ACTION_0x67:
	case LH_SOUND_ACTION_0x68:
		type = HAND_GLOW_TYPE_2;
		break;
	case LH_SOUND_ACTION_0x69:
	case LH_SOUND_ACTION_0x6a:
	case LH_SOUND_ACTION_0x6b:
	case LH_SOUND_ACTION_0x6c:
	case LH_SOUND_ACTION_0x6d:
		type = HAND_GLOW_TYPE_1;
		break;
	}

	float power = 0.0f;
	switch (action)
	{
	case LH_SOUND_ACTION_0x5f:
	case LH_SOUND_ACTION_0x64:
	case LH_SOUND_ACTION_0x69:
		power = 0.0f;
		break;
	case LH_SOUND_ACTION_0x60:
	case LH_SOUND_ACTION_0x65:
	case LH_SOUND_ACTION_0x6a:
		power = 0.25f;
		break;
	case LH_SOUND_ACTION_0x61:
	case LH_SOUND_ACTION_0x66:
	case LH_SOUND_ACTION_0x6b:
		power = 0.5f;
		break;
	case LH_SOUND_ACTION_0x62:
	case LH_SOUND_ACTION_0x67:
	case LH_SOUND_ACTION_0x6c:
		power = 0.75f;
		break;
	case LH_SOUND_ACTION_0x63:
	case LH_SOUND_ACTION_0x68:
	case LH_SOUND_ACTION_0x6d:
		power = 1.0f;
		break;
	}

	if (Glows == NULL)
	{
		Glows = HandGlows::Create();
	}
	if (power == 0.0f)
	{
		Glows->Stop(type);
	}
	else
	{
		Glows->Start(type);
		Glows->SetScalePowerTime(type, 1.0f, power * 2, 1.0f);
	}
}

void HandGlows::Draw(LH3DCreature* creature)
{
	for (int i = 0; i < HAND_GLOW_TYPE_LAST; i++)
	{
		HandGlow* glow = Glows[i];
		if (glow == NULL)
		{
			continue;
		}
		LHMatrix* matrices = creature->TransformedMatrices;
		long      bone;
		LHPoint   grasp;
		switch (i)
		{
		case HAND_GLOW_TYPE_0:
		case HAND_GLOW_TYPE_2:
			bone = i == HAND_GLOW_TYPE_0 ? creature->GetMirrorBone(creature->RightHand) : creature->RightHand;
			LH3DAnim::GetGraspPoint(matrices, creature->meshes[creature->CurrentMesh], grasp, bone);
			break;
		case HAND_GLOW_TYPE_1:
			bone = creature->BellyBone;
			break;
		case HAND_GLOW_TYPE_3:
			bone = creature->HeadBone;
			break;
		}
		LHMatrix matrix = matrices[bone];
		matrix.NormaliseMatrixOnly();
		if (i == HAND_GLOW_TYPE_0 || i == HAND_GLOW_TYPE_2)
		{
			matrix.SetTranslateOnly(grasp);
		}
		else if (i == HAND_GLOW_TYPE_3)
		{
			glow->field_0x8 += LH3DTech::GetGameTimeInc() * GlowSpinSpeed * 0.001f;
			matrix.SetRotationY(glow->field_0x8);
			matrix.SetTranslateOnly(creature->DynamicShadow->matrix.GetPos());
			matrix._42 += GlowHeadHeight * creature->Size1 * 15.0f;
		}
		float scale = GlowScale * creature->DynamicShadow->scale;
		matrix.ScaleMatrixOnly(scale);
		matrix.GetVectorZ() *= -1.0f;
		glow->DrawAt(matrix, scale);
	}
}
