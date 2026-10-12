#include "MagicHand.h"

#include <Lionhead/LH3DLib/development/LH3DIsland.h>

#include "FireEffect.h"
#include "Game.h"
#include "GameOSFile.h"
#include "Interface.h"
#include "InterfaceStatus.h"
#include "Object.h"

GMagicHand::GMagicHand()
{
	fn_005FAF70();
}

uint32_t GMagicHand::GetSaveType()
{
	return GAME_THING_TYPE_GMAGIC_HAND_1;
}

char* GMagicHand::GetDebugText()
{
	return "MagicHand:";
}

GMagicHand::~GMagicHand() {}

void GMagicHand::fn_005FAF70()
{
	NumObjects = 0;
	Objects[0] = NULL;
}

void GMagicHand::DrawContents() const
{
	Object* object = GetObjectFromHand();
	if (object != NULL && object->IsAvailable())
	{
		GInterfaceStatus* status = GGame::g_game->MyInterface()->status;
		object->DrawInHand(status);
	}
}

bool32_t GMagicHand::AddToHand(Object* object, GInterfaceStatus* status)
{
	if (NumObjects < ARRAY_SIZE(Objects))
	{
		Objects[NumObjects] = object;
		NumObjects++;
		object->Pos.SetX(status->GetHandPos().x);
		object->Pos.SetZ(status->GetHandPos().z);
		object->Pos.SetAltitude(0.0f);
		object->Flags |= GAME_THING_WITH_POS_FLAG_UNAVAILABLE_FOR_STATE_CHANGE;
		object->SetPlayer(status->GetPlayer());
		return true;
	}
	return false;
}

Object* GMagicHand::GetObjectFromHand() const
{
	if (NumObjects > 0)
	{
		return Objects[NumObjects - 1];
	}
	return NULL;
}

Object* GMagicHand::RemoveFromHand(GInterfaceStatus* status)
{
	if (NumObjects > 0)
	{
		NumObjects--;
		Object* object = Objects[NumObjects];
		object->Flags &= ~GAME_THING_WITH_POS_FLAG_UNAVAILABLE_FOR_STATE_CHANGE;
		object->InterfaceSetOutMagicHand(status);
		if (object->GetFireEffect() != NULL)
		{
			object->GetFireEffect()->SetOutMagicHand();
		}
		return object;
	}
	return NULL;
}

bool32_t GMagicHand::RemoveFromHand(Object* object, GInterfaceStatus* status)
{
	for (int i = 0; i < NumObjects; i++)
	{
		if (object == Objects[i])
		{
			for (; i < NumObjects - 1; i++)
			{
				Objects[i] = Objects[i + 1];
			}
			NumObjects--;
			object->Flags &= ~GAME_THING_WITH_POS_FLAG_UNAVAILABLE_FOR_STATE_CHANGE;
			object->InterfaceSetOutMagicHand(status);
			if (object->GetFireEffect() != NULL)
			{
				object->GetFireEffect()->SetOutMagicHand();
			}
			return true;
		}
	}
	return false;
}

bool32_t GMagicHand::Validate(GInterfaceStatus* status)
{
	for (int i = 0; i < NumObjects; i++)
	{
		Object* object = Objects[i];
		if (!object->IsInteractable())
		{
			RemoveFromHand(object, status);
			Validate(status);
			return true;
		}
	}
	return false;
}

bool32_t GMagicHand::Process(GInterfaceStatus* status)
{
	bool32_t result = false;
	for (int i = 0; i < NumObjects; i++)
	{
		Object* object = Objects[i];
		if (object != NULL)
		{
			LHPoint pos = status->GetHandPos();
			object->Pos.SetX(pos.x);
			object->Pos.SetZ(pos.z);
			object->Pos.SetAltitude(0.0f);
			object->Pos.SetAltitude(pos.y - LH3DIsland::GetAltitude(pos) + object->Pos.altitude);
			if (object->ProcessInHand() != true || object->IsAvailable() != true)
			{
				result |= Validate(status);
			}
		}
	}
	return result;
}

void GMagicHand::CleanUpForSerialisation()
{
	for (int i = NumObjects; i < (int)ARRAY_SIZE(Objects); i++)
	{
		Objects[i] = NULL;
	}
}

uint32_t GMagicHand::Save(GameOSFile& file)
{
	CleanUpForSerialisation();
	file.WriteIt(NumObjects);
	file.WritePtrArray((GameThing**)Objects, ARRAY_SIZE(Objects));
	return true;
}

uint32_t GMagicHand::Load(GameOSFile& file)
{
	file.ReadIt(NumObjects);
	file.ReadPtrArray((GameThing**)Objects);
	return true;
}
