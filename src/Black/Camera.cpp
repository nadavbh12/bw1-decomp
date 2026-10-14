#include "Camera.h"

#include <float.h> /* For _isnan */
#include <math.h>
#include <stdio.h> /* For sprintf */

#include <Lionhead/LH3DLib/development/LH3DAtmos.h>
#include <Lionhead/LH3DLib/development/LH3DIsland.h>
#include <Lionhead/LH3DLib/development/LH3DMath.h>
#include <Lionhead/LH3DLib/development/LH3DTech.h>
#include <Lionhead/LHLib/ver5.0/LHWin.h> /* For operator new(size_t, const char*, uint32_t) */

#include "CameraModeFollow.h"
#include "CameraModeFollowHeading.h"
#include "CameraModeFree.h"
#include "CameraModeNew3.h"
#include "CameraModePath.h"
#include "ColourConstants.h" /* For White */
#include "Game.h"
#include "GameOSFile.h"
#include "Interface.h"
#include "LandAlignement.h"
#include "LandFeature.h"
#include "MapCellConstants.h"
#include "Landscape.h"
#include "Utils.h"

#define FILEPATH "C:\\dev\\MP\\Black\\Camera.cpp"

enum
{
	CAMERA_MODE_SAVE_ID_FOLLOW = 70,
	CAMERA_MODE_SAVE_ID_FOLLOW_HEADING = 72,
	CAMERA_MODE_SAVE_ID_FREE = 101,
	CAMERA_DEBUG_TEXT_LENGTH = 100,
	MAP_SIZE_IN_CELLS = 512,
};

const float  StartDistance = 120.0f;
const float  StartPitch = 0.22439948f;
const float  ViewingDistanceScale = 8.0f;
const float  InverseCameraRadius = 0.00028579595f;
const float  CameraRadiusSq = 12250000.0f;
const float  MapCentreInMetres = 2560.0f;
const double MinPointDelta = 0.01;
const float  CameraClipMapSize = 1536.0f;
const float  DefaultFov = 1.2217305f;

inline void GCamera::GetFocus(LHPoint& focus)
{
	focus = LHPoint(LH3DTech::GetCameraTarget());
}

inline void GCamera::GetPosition(LHPoint& pos)
{
	pos = LH3DTech::g_camera.pos;
}

// BW1W120 00441650 BW1M119 null
static CameraMode* fn_00441650(int save_id, GCamera* camera);

static CameraMode* fn_00441650(int save_id, GCamera* camera)
{
	switch (save_id)
	{
	case CAMERA_MODE_SAVE_ID_FOLLOW:
		return new (FILEPATH, 32) CameraModeFollow(camera, NULL, 1.0f, false, false);
	case CAMERA_MODE_SAVE_ID_FOLLOW_HEADING:
		return new (FILEPATH, 33) CameraModeFollowHeading(camera, NULL, 1.0f, 1.0f, 1.0f, 1.0f);
	case CAMERA_MODE_SAVE_ID_FREE:
		return new (FILEPATH, 34) CameraModeFree(camera);
	}
	return NULL;
}

bool32_t CameraMode::Arrived()
{
	float originDistanceSq = GGame::g_game->GetCamera()->CameraOriginZoomer.GetCurrentValue().GetDistanceSq(
		GGame::g_game->GetCamera()->CameraOriginZoomer.GetDestination());
	float focusDistanceSq = GGame::g_game->GetCamera()->CameraHeadingZoomer.GetCurrentValue().GetDistanceSq(
		GGame::g_game->GetCamera()->CameraHeadingZoomer.GetDestination());
	if (originDistanceSq < 0.001f && focusDistanceSq < 0.001f)
	{
		return true;
	}
	return false;
}

void CameraStore::Set(LHPoint& position, LHPoint& focus)
{
	Position.Set(position);
	Focus.Set(focus);
}

GCamera::GCamera(MapCoords& coords)
{
	field_0x78 = false;
	TimeDelta = 0.0f;
	GLandscape::ConvertMapCoordToLandscapePoint(coords, pos);
	LHPoint origin;
	SetPointFromPointDistanceHeadingAndPitch(&origin, pos, 70.0f, 3.5343983f, 0.5235988f);
	CameraOriginZoomer.SetPosition(origin);
	CameraHeadingZoomer.SetPosition(pos);
	SetPointFromPointDistanceHeadingAndPitch(&origin, pos, StartDistance, 2.749f, StartPitch);
	CameraOriginZoomer.SetPosition(origin);
	CameraOriginZoomer.SetDestinationWithTime(origin, 2.5f);
	ModeCurrentIndex = -1;
	ScriptSlomoControl = 0;
	SetCameraFov(GetDefaultFov(), 0.0f);
	new (FILEPATH, 116) CameraModeNew3(this);
}

const char* GCamera::GetText()
{
	return "Camera";
}

bool32_t GCamera::IsMoving() const
{
	return Moving;
}

float GCamera::GetHeight()
{
	return Pos.altitude;
}

uint32_t GCamera::GetSaveType()
{
	return GAME_THING_TYPE_GCAMERA;
}

bool32_t GCamera::CantExitCurrentMode()
{
	if (ModeCurrentIndex == -1)
	{
		return false;
	}
	return !modes[ModeCurrentIndex]->CanExit();
}

void GCamera::fn_00441B90(CameraStore& store)
{
	store.Position = CameraOriginZoomer.GetDestination();
	store.Focus = CameraHeadingZoomer.GetDestination();
}

void GCamera::fn_00441C10(const CameraStore& store)
{
	CameraModeNew3* mode = dynamic_cast<CameraModeNew3*>(modes[ModeCurrentIndex]);
	if (mode != NULL)
	{
		mode->Store = store;
		mode->Restart();
	}
}

void GCamera::PopViewMode()
{
	modes[ModeCurrentIndex]->Cleanup();
	delete modes[ModeCurrentIndex];
	if (ModeCurrentIndex > 0)
	{
		ModeCurrentIndex--;
		modes[ModeCurrentIndex]->Restart();
	}
	else
	{
		ModeCurrentIndex = -1;
		new (FILEPATH, 159) CameraModeNew3(this);
	}
	time = 0.0f;
}

void GCamera::SwitchToViewMode(CameraMode* mode)
{
	if (ModeCurrentIndex >= 0)
	{
		modes[ModeCurrentIndex]->Cleanup();
	}
	ModeCurrentIndex++;
	if (ModeCurrentIndex == ARRAY_SIZE(modes))
	{
		ModeCurrentIndex = ARRAY_SIZE(modes) - 1;
		delete modes[0];
		for (int i = 0; i < ModeCurrentIndex; i++)
		{
			modes[i] = modes[i + 1];
		}
	}
	modes[ModeCurrentIndex] = mode;
	time = 0.0f;
}

void GCamera::CheckStackedModesForValidity()
{
	bool32_t restart = false;
	for (int i = 0; i <= ModeCurrentIndex;)
	{
		CameraMode* mode = modes[i];
		if (mode->IsStillValid())
		{
			i++;
			continue;
		}
		if (i == ModeCurrentIndex)
		{
			restart = true;
			mode->Cleanup();
		}
		delete modes[i];
		for (int j = i; j < ModeCurrentIndex; j++)
		{
			modes[j] = modes[j + 1];
		}
		ModeCurrentIndex--;
	}
	if (ModeCurrentIndex < 0)
	{
		new (FILEPATH, 219) CameraModeNew3(this);
	}
	else if (restart)
	{
		modes[ModeCurrentIndex]->Restart();
	}
}

void GCamera::InvalidatePathMode(Particle3DAnimWithCamera* anim)
{
	for (int i = 0; i <= ModeCurrentIndex; i++)
	{
		CameraModePath* mode = dynamic_cast<CameraModePath*>(modes[i]);
		if (mode != NULL && mode->Anim == anim)
		{
			mode->Valid = false;
		}
	}
}

float GCamera::CalculateRotationAngleY()
{
	return GetHeadingFromPoints(CameraOriginZoomer.GetCurrentValue(), CameraHeadingZoomer.GetCurrentValue());
}

void GCamera::fn_00441EC0(LHPoint& point)
{
	point = pos;
}

void GCamera::ToBeDeleted(int param_1)
{
	for (int i = 0; i <= ModeCurrentIndex; i++)
	{
		delete modes[i];
	}
	GameThingWithPos::ToBeDeleted(param_1);
}

float GCamera::GetThingViewingDistance(GameThingWithPos* target)
{
	return target->GetHeight() * ViewingDistanceScale;
}

void GCamera::SetScriptSlomoControl(int control)
{
	ScriptSlomoControl = control;
}

void GCamera::Validate()
{
	for (int i = 0; i <= ModeCurrentIndex;)
	{
		modes[i++]->Validate();
	}
}

void GCamera::Update()
{
	float timeDelta = GGame::g_game->GetCameraTimeInc() / 1000.0f;
	if (0.1f < timeDelta)
	{
		timeDelta = 0.1f;
	}
	TimeDelta = timeDelta;
	time += timeDelta;
	modes[ModeCurrentIndex]->Update();
	CameraOriginZoomer.Update(TimeDelta);
	CameraHeadingZoomer.Update(TimeDelta);

	LHPoint        origin = CameraOriginZoomer.GetCurrentValue();
	LHPoint        focus = CameraHeadingZoomer.GetCurrentValue();
	static LHPoint lastGoodOrigin(1000.0f, 0.0f, 1000.0f);
	static LHPoint lastGoodFocus(1000.0f, 0.0f, 1000.0f);
	if (_isnan(origin.x) || _isnan(origin.y) || _isnan(origin.z))
	{
		origin = lastGoodOrigin;
	}
	else
	{
		lastGoodOrigin = origin;
	}
	if (_isnan(focus.x) || _isnan(focus.y) || _isnan(focus.z))
	{
		focus = lastGoodFocus;
	}
	else
	{
		lastGoodFocus = focus;
	}

	if ((origin - focus).GetNormeSq() < 0.001f)
	{
		origin.x -= 1.0f;
		origin.y += 1.0f;
	}

	LHPoint destination = CameraOriginZoomer.GetDestination();
	float   dx = destination.x - MapCentreInMetres;
	float   dy = destination.y;
	float   dz = destination.z - MapCentreInMetres;
	destination.Set(dx, dy, dz);
	if (destination.GetNormeSq() > CameraRadiusSq)
	{
		destination *= 1.0f / (destination.GetNorme() * InverseCameraRadius);
		destination.x += MapCentreInMetres;
		destination.z += MapCentreInMetres;
		CameraOriginZoomer.SetDestinationWithTime(destination, 3.0f);
	}

	if (GGame::g_game->ViewMode == GAME_VIEW_MODE_INSIDE_CITADEL)
	{
		origin = LHPoint(LH3DTech::GetCameraPosition());
		focus = LHPoint(LH3DTech::GetCameraTarget());
	}
	else if (GGame::g_game->ViewMode == GAME_VIEW_MODE_FALLING_SPELL_VIDEO)
	{
		origin = LHPoint(LH3DTech::GetCameraPosition());
		focus = LHPoint(LH3DTech::GetCameraTarget());
	}
	else if (LH3DIsland::RayCast(origin, focus, &pos.x, &pos.z))
	{
		pos.y = LH3DIsland::GetAltitude(pos);
	}

	float heightAboveLand = LH3DIsland::GetAltitude(origin) + 1.0f - origin.y;
	if (dynamic_cast<CameraModeFree*>(GetCurrentMode()) == NULL && !field_0x78 && heightAboveLand > 0.0f)
	{
		origin.y += heightAboveLand;
		focus.y += heightAboveLand;
	}

	LH3DTech::SetNearClipping(LandFeature::GetNearClipping());
	if (GGame::g_game->ViewMode == GAME_VIEW_MODE_WORLD)
	{
		FovZoomer.Update(LH3DTech::GetGameTimeInc() / 1000.0f);
		LH3DTech::ChangeFov(FovZoomer.GetCurrentValue());
	}

	LHPoint previous;
	GetPosition(previous);
	if (GGame::g_game->ViewMode == GAME_VIEW_MODE_WORLD)
	{
		if (!GGame::g_game->MyInterface()->IsPlayBack(0))
		{
			LH3DTech::UpdateCamera(origin, focus);
			Moving = !(previous == origin);
		}
	}
	else
	{
		Moving = !(previous == origin);
	}

	Weather = LH3DAtmos::GetWeatherSmooth(origin, true);
	GLandAlignement::Dat_00D1A26C = Weather.field_0x3 * 0.01f;
	UpdateGameThingWithPosData();
}

void GCamera::SetPointFromPointDistanceHeadingAndPitch(LHPoint* param_1, const LHPoint& point, float distance,
                                                       float heading, float pitch)
{
	float cosPitch = (float)cos(pitch);
	param_1->Set(point.x + (float)sin(heading) * cosPitch * distance, point.y + (float)sin(pitch) * distance,
	             point.z + (float)cos(heading) * cosPitch * distance);
}

float GCamera::GetHeadingFromPoints(const LHPoint& origin, const LHPoint& focus)
{
	LHPoint direction = origin - focus;
	if (fabs(direction.x) < MinPointDelta && fabs(direction.z) < MinPointDelta)
	{
		return 0.0f;
	}
	return PI_F - heading_from_direction_vector(direction);
}

void GCamera::GetHeadingAndPitchFromPoints(const LHPoint& origin, const LHPoint& focus, float* heading, float* pitch)
{
	LHPoint direction = origin - focus;
	if (fabs(direction.x) < MinPointDelta && fabs(direction.z) < MinPointDelta)
	{
		*heading = 0.0f;
		*pitch = 1.5393804f;
		return;
	}
	*heading = PI_F - heading_from_direction_vector(direction);
	float horizontal = sqrt(direction.z * direction.z + direction.x * direction.x);
	*pitch = atan360(horizontal, direction.y);
}

void GCamera::fn_00442980(LHPoint* point, const LHPoint& centre, float heading, float pitch)
{
	LHPoint offset = centre - *point;
	float   newHeading;
	float   newPitch;
	GetHeadingAndPitchFromPoints(*point, centre, &newHeading, &newPitch);
	newHeading += heading;
	newPitch += pitch;
	SetPointFromPointDistanceHeadingAndPitch(point, centre, offset.GetNorme(), newHeading, newPitch);
}

void GCamera::fn_00442A20(LHPoint* point, const LHPoint& centre)
{
	LHPoint offset(point->x - centre.x, point->y - centre.y, point->z - centre.z);
	offset.FastNormalizeInline();
	*point = LHPoint(offset.x + centre.x, offset.y + centre.y, offset.z + centre.z);
}

float GCamera::fn_00442AB0()
{
	return pos.GetDistance(CameraOriginZoomer.GetCurrentValue());
}

void GCamera::ClipToCameraRadius(LHPoint* point)
{
	float   centre = (MetresPerMapCell * MAP_SIZE_IN_CELLS) * 0.5f;
	LHPoint offset(point->x - centre, point->y, point->z - centre);
	float   distance = sqrt(offset.z * offset.z + offset.y * offset.y + offset.x * offset.x);
	if (distance > MetresPerMapCell * CameraClipMapSize * 0.5f)
	{
		offset.SetSize(MetresPerMapCell * CameraClipMapSize * 0.5f);
		*point = LHPoint(offset.x + centre, offset.y, offset.z + centre);
	}
}

bool32_t GCamera::TranslatePointToScreen(float* x, float* y, LHPoint* point)
{
	const LHMatrix& matrix = LH3DTech::g_clipping_to_world;
	*x = matrix._31 * point->z + matrix._11 * point->x + matrix._21 * point->y + matrix._41;
	*y = matrix._32 * point->z + matrix._12 * point->x + matrix._22 * point->y + matrix._42;
	float z = matrix._33 * point->z + matrix._13 * point->x + matrix._23 * point->y + matrix._43;
	if (z < LH3DTech::g_info_transform.NearClip)
	{
		return false;
	}
	float inverse = 1.0f / z;
	*x = (inverse * *x + 1.0f) * LH3DTech::g_info_transform.HalfRes.x;
	*y = LH3DTech::g_info_transform.HalfRes.y - LH3DTech::g_info_transform.HalfRes.y * *y * inverse;
	return true;
}

void GCamera::GetFocusVector(LHPoint& vector)
{
	vector.Set(LH3DTech::g_camera.LookAtVector);
}

float GCamera::GetDistanceSq(const LHPoint& point)
{
	return LH3DTech::GetCameraPosition()->GetDistanceSq(point);
}

float GCamera::GetDistance(const LHPoint& point)
{
	LHPoint position;
	GetPosition(position);
	return point.GetDistance(position);
}

void GCamera::fn_00442DB0(float heading, float pitch)
{
	CameraModeNew3* mode = dynamic_cast<CameraModeNew3*>(modes[ModeCurrentIndex]);
	if (mode != NULL)
	{
		mode->SetHeadingAndPitch(heading, pitch);
	}
}

uint32_t GCamera::fn_00442DF0()
{
	return GUtils::ConvertAngle3DToGame(CalculateRotationAngleY());
}

void GCamera::fn_00442E10(const LHPoint& focus, int param_2)
{
	CameraModeNew3* mode = dynamic_cast<CameraModeNew3*>(modes[ModeCurrentIndex]);
	if (mode != NULL)
	{
		mode->SetFocus(focus);
	}
}

void GCamera::fn_00442E50(const MapCoords& coords, int param_2)
{
	LHPoint focus;
	GLandscape::ConvertMapCoordToLandscapePoint(coords, focus);
	fn_00442E10(focus, param_2);
}

bool32_t GCamera::IsFollowing(GameThingWithPos* thing)
{
	CameraModeFollow* mode = dynamic_cast<CameraModeFollow*>(GetCurrentMode());
	if (mode != NULL && mode->Target == thing)
	{
		return true;
	}
	return false;
}

void GCamera::UpdateGameThingWithPosData()
{
	LHPoint position;
	GetPosition(position);
	Pos.x = (long)(position.x * (float)0x10000 / 10.0f);
	Pos.z = (long)(position.z * (float)0x10000 / 10.0f);
	Pos.SetAltitude(0.0f);
	Pos.SetAltitude(position.y - LH3DIsland::GetHeightAsFloat(Pos.MapX(), Pos.MapZ()));
}

void GCamera::ClearCameraStack()
{
	while (ModeCurrentIndex >= 0)
	{
		CameraMode* mode = modes[0];
		if (ModeCurrentIndex == 0)
		{
			mode->Cleanup();
		}
		delete modes[0];
		for (int i = 0; i < ModeCurrentIndex; i++)
		{
			modes[i] = modes[i + 1];
		}
		ModeCurrentIndex--;
	}
	new (FILEPATH, 647) CameraModeNew3(this);
}

bool32_t GCamera::Arrived()
{
	CameraMode* mode = GetCurrentMode();
	if (mode != NULL)
	{
		return mode->Arrived();
	}
	return true;
}

float GCamera::CalculatePitch()
{
	LHPoint position;
	LHPoint focus;
	GetPosition(position);
	GetFocus(focus);
	float heading;
	float pitch;
	GetHeadingAndPitchFromPoints(position, focus, &heading, &pitch);
	return pitch;
}

uint32_t GCamera::Save(GameOSFile& file)
{
	GCamera* camera = GGame::g_game->GetCamera();
	if (GameThingWithPos::Save(file))
	{
		file.WriteIt(pos);
		file.WriteIt(time);
		file.WriteIt(TimeDelta);
		file.WriteIt(ScriptSlomoControl);
		file.WriteIt(Moving);
		file.WriteIt(Weather);
		file.WriteIt(CameraHeadingZoomer);
		file.WriteIt(CameraOriginZoomer);
		file.WriteIt(FovZoomer);
		file.WriteIt(field_0x78);
		uint32_t saveId = modes[ModeCurrentIndex]->GetSaveID();
		file.WriteIt(saveId);
		modes[ModeCurrentIndex]->Save(&file);
		return true;
	}
	return false;
}

uint32_t GCamera::Load(GameOSFile& file)
{
	if (GameThingWithPos::Load(file))
	{
		file.ReadIt(pos);
		file.ReadIt(time);
		file.ReadIt(TimeDelta);
		file.ReadIt(ScriptSlomoControl);
		file.ReadIt(Moving);
		file.ReadIt(Weather);
		file.ReadIt(CameraHeadingZoomer);
		file.ReadIt(CameraOriginZoomer);
		file.ReadIt(FovZoomer);
		file.ReadIt(field_0x78);
		ClearCameraStack();
		uint32_t saveId;
		file.ReadIt(saveId);
		if (saveId != 0)
		{
			CameraMode* mode = fn_00441650(saveId, this);
			if (mode != NULL)
			{
				mode->Load(&file);
			}
		}
		return true;
	}
	return false;
}

float GCamera::GetDefaultFov()
{
	return DefaultFov;
}

void GCamera::SetCameraFov(float fov, float time)
{
	if (time != 0.0f)
	{
		FovZoomer.SetDestination(fov, time);
	}
	else
	{
		FovZoomer.SetPosition(fov);
	}
}

char* GCamera::GetDebugText()
{
	static char text[CAMERA_DEBUG_TEXT_LENGTH];
	sprintf(text, "Camera Pos(%.2f,%.2f,%.2f), Foc(%.2f,%.2f,%.2f)", Pos.x * MetresPerMapCell / 65536.0f, Pos.altitude,
	        Pos.z * MetresPerMapCell / 65536.0f, CameraHeadingZoomer.x.GetCurrentValue(),
	        CameraHeadingZoomer.y.GetCurrentValue(), CameraHeadingZoomer.z.GetCurrentValue());
	return text;
}

void GCamera::SetPositionAndFocus(const LHPoint& position, const LHPoint& focus)
{
	LHPoint cameraPos;
	LHPoint cameraFocus;
	GetPosition(cameraPos);
	GetFocus(cameraFocus);
	if (cameraPos == position && focus == cameraFocus)
	{
		Moving = false;
		return;
	}
	CameraOriginZoomer.SetPosition(position);
	CameraHeadingZoomer.SetPosition(focus);
	Moving = true;
}
