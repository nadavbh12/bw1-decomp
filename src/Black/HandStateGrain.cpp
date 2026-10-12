#include "HandStateHolding.h"
#include "GameTimeConstants.h"
#include "ColourConstants.h" /* For White */
#include "ControlHand.h"
#include "HandStateGrain.h"

#include "Creature.h"    /* For class Creature */
#include "Creature3D.h"  /* For class LH3DCreature */
#include "Game.h"        /* For class GGame */
#include "GJBaseUtils.h" /* For class GJArray */
#include "Interface.h"   /* For class GInterface */

#include <math.h> /* For acos, exp, log, sqrt */

float   HandStateGrain::PositionFilterFactor = 0.8f;
float   HandStateGrain::AngleFilterFactor = 0.2f;
LHPoint HandStateGrain::field_0x00d13f00;
bool    HandStateGrain::field_0x00d13f1c;

template <class T> inline void GJLowPassFilterVariableStep<T>::Init(const T& value, float time_constant)
{
	Value = value;
	TimeConstant = (float)log(1.0f - time_constant) * -10.0f;
	Initialised = true;
}

template <class T> inline void GJLowPassFilterVariableStep<T>::Update(float time, const T& value)
{
	Value = (value - Value) * (1.0f - (float)exp(-(time * TimeConstant))) + Value;
}

inline void GJLowPassFilterVariableStep<LHPoint>::Update(float time, const LHPoint& value)
{
	Value.Add((value - Value) * (1.0f - (float)exp(-(time * TimeConstant))));
}

template <class T>
void t_spline(GJArray<KPSI_Element<T> >& elements, long count, float first_derivative, float last_derivative)
{
	GJArray<float> u;
	u.SetSize(count - 1);
	if (first_derivative > 0.99e30)
	{
		u[0] = elements[0].SecondDerivative = 0.0f;
	}
	else
	{
		elements[0].SecondDerivative = -0.5f;
		u[0] = (3.0f / (elements[1].T - elements[0].T)) *
		       ((elements[1].Value - elements[0].Value) * (1.0f / (elements[1].T - elements[0].T)) - first_derivative);
	}
	for (long i = 1; i < count - 1; i++)
	{
		float sig = (elements[i].T - elements[i - 1].T) / (elements[i + 1].T - elements[i - 1].T);
		float p = sig * elements[i - 1].SecondDerivative + 2.0f;
		elements[i].SecondDerivative = (1.0f / (sig - 1.0f)) * p;
		elements[i].SecondDerivative = 1.0f / elements[i].SecondDerivative;
		u[i] = (elements[i + 1].Value - elements[i].Value) * (1.0f / (elements[i + 1].T - elements[i].T)) -
		       (elements[i].Value - elements[i - 1].Value) * (1.0f / (elements[i].T - elements[i - 1].T));
		u[i] = 6.0f / (elements[i + 1].T - elements[i - 1].T) * u[i] - sig * u[i - 1];
		u[i] = 1.0f / p * u[i];
	}
	float qn;
	float un;
	if (last_derivative > 0.99e30)
	{
		qn = 0.0f;
		un = 0.0f;
	}
	else
	{
		qn = 0.5f;
		un = (3.0f / (elements[count - 1].T - elements[count - 2].T)) *
		     (last_derivative - (elements[count - 1].Value - elements[count - 2].Value) *
		                            (1.0f / (elements[count - 1].T - elements[count - 2].T)));
	}
	elements[count - 1].SecondDerivative = qn * elements[count - 2].SecondDerivative + 1.0f;
	elements[count - 1].SecondDerivative = 1.0f / elements[count - 1].SecondDerivative;
	elements[count - 1].SecondDerivative = (un - qn * u[count - 2]) * elements[count - 1].SecondDerivative;
	for (long k = count - 2; k >= 0; k--)
	{
		elements[k].SecondDerivative = elements[k + 1].SecondDerivative * elements[k].SecondDerivative + u[k];
	}
}

template <class E, class T> inline void t_splint(const GJArray<E>& elements, long count, float x, T* y)
{
	long low = 0;
	long high = count - 1;
	while (high - low > 1)
	{
		long mid = (high + low) / 2;
		if (elements.Entry(mid).T > x)
		{
			high = mid;
		}
		else
		{
			low = mid;
		}
	}
	const E& hi = elements.Entry(high);
	const E& lo = elements.Entry(low);
	float    h = hi.T - lo.T;
	if (h != 0.0)
	{
		float a = (hi.T - x) / h;
		float b = (x - lo.T) / h;
		*y = ((b * b * b - b) * hi.SecondDerivative + (a * a * a - a) * lo.SecondDerivative) * (h * h * (1.0f / 6.0f)) +
		     a * lo.Value + b * hi.Value;
	}
}

HandStateGrain::HandStateGrain(CHand* hand) : HandStateHolding(hand)
{
	field_0x14c = 0;
	Tilt = 0.0f;
	Raise = 0.0f;
	PreviousTilt = 0.0f;
	PreviousRaise = 0.0f;
	PourCreature = NULL;
	field_0x1f8 = false;
	Pouring = false;
	PourTime = 0.0f;
	PourDuration = 1.0f;
	RaiseScale = 0.0f;
	TiltScale = 0.0f;
	GJArray<float> curve;
	curve.SetSize(8);
	curve[0] = 0.0f;
	curve[1] = 0.0f;
	curve[2] = 0.2f;
	curve[3] = 1.0f;
	curve[4] = 0.8f;
	curve[5] = 1.0f;
	curve[6] = 1.0f;
	curve[7] = 0.0f;
	PourCurve.SetFromArray(curve);
}

float HandStateGrain::GetRaiseForRender()
{
	return (Raise - PreviousRaise) * GGame::g_game->TurnFraction + PreviousRaise;
}

float HandStateGrain::GetTiltForRender()
{
	return (Tilt - PreviousTilt) * GGame::g_game->TurnFraction + PreviousTilt;
}

void HandStateGrain::Process(float dt)
{
	GInterface*     iface = GGame::g_game->MyInterface();
	HandStateGrain* grain = iface->hand.Get()->GetHandStateGrain();
	if (grain != NULL)
	{
		grain->PProcess(dt);
	}
}

void HandStateGrain::PProcess(float dt)
{
	PreviousRaise = Raise;
	PreviousTilt = Tilt;
	if (PourCreature != NULL && PourCreature->IsAvailable() != true)
	{
		PourCreature = NULL;
	}
	if (Pouring)
	{
		PourTime += dt / PourDuration;
		if (PourTime > 1.0f)
		{
			if (PourType == POUR_TYPE_REPEAT)
			{
				PourTime = 0.0f;
			}
			else
			{
				PEndTimedPour();
			}
		}
		if (Pouring)
		{
			float value = PourTime;
			t_splint(PourCurve.Elements, PourCurve.Elements.Size, PourTime, &value);
			Raise = value * RaiseScale;
			Tilt = value * TiltScale;
		}
	}
}

void HandStateGrain::EndTimedPour()
{
	GInterface*     iface = GGame::g_game->MyInterface();
	HandStateGrain* grain = iface->hand.Get()->GetHandStateGrain();
	if (grain != NULL)
	{
		grain->PEndTimedPour();
	}
}

void HandStateGrain::PEndTimedPour()
{
	PourCreature = NULL;
	field_0x1f8 = false;
	Raise = 0.0f;
	Tilt = 0.0f;
	Pouring = false;
}

void HandStateGrain::StartTimedPour(bool param_1, float duration, float raise_scale, float tilt_scale, POUR_TYPE type)
{
	GInterface*     iface = GGame::g_game->MyInterface();
	HandStateGrain* grain = iface->hand.Get()->GetHandStateGrain();
	if (grain != NULL)
	{
		grain->PStartTimedPour(param_1, duration, raise_scale, tilt_scale, type);
	}
}

void HandStateGrain::StartTimedPour(Creature* creature, float duration, float raise_scale, float tilt_scale,
                                    POUR_TYPE type)
{
	GInterface*     iface = GGame::g_game->MyInterface();
	HandStateGrain* grain = iface->hand.Get()->GetHandStateGrain();
	if (grain != NULL)
	{
		grain->PSetCtr(creature);
		grain->PStartTimedPour(true, duration, raise_scale, tilt_scale, type);
	}
}

void HandStateGrain::PStartTimedPour(bool param_1, float duration, float raise_scale, float tilt_scale, POUR_TYPE type)
{
	PourType = type;
	PourDuration = duration;
	RaiseScale = raise_scale;
	TiltScale = tilt_scale;
	Pouring = true;
	PourTime = 0.0f;
	field_0x1f8 = param_1;
	field_0x1ec = hand->position;
}

void HandStateGrain::PSetCtr(Creature* creature)
{
	PourCreature = creature;
}

void HandStateGrain::Enter()
{
	HandStateHolding::Enter();
	field_0x1e0 = LHPoint(0.0f, 0.0f, 0.0f);
	field_0x1fc.Init(LHPoint(0.0f, 0.0f, 0.0f), PositionFilterFactor);
	field_0x210.Init(0.0f, AngleFilterFactor);
	field_0x21c.Init(0.0f, AngleFilterFactor);
	field_0x150.SetPosition(LHPoint(0.0f, 1.0f, 0.0f));
	field_0x14c = 0;
	Pouring = false;
	field_0x1f8 = false;
	PourCreature = NULL;
	Tilt = 0.0f;
	Raise = 0.0f;
	PreviousTilt = 0.0f;
	PreviousRaise = 0.0f;
}

void HandStateGrain::Exit()
{
	Pouring = false;
	PourCreature = NULL;
}

void HandStateGrain::OverrideHandPos(LHPoint* pos)
{
	if (PourCreature == NULL)
	{
		if (field_0x1f8)
		{
			*pos = field_0x1ec;
		}
	}
	else
	{
		LH3DCreature* creature3d = PourCreature->GetCreature3D();
		*pos = creature3d->TransformedMatrices[creature3d->HeadBone].GetPos();
	}
}

void HandStateGrain::Update(float dt, LHMatrix* matrices)
{
	LHPoint oldPos = hand->position;
	LHPoint oldMove = field_0x1e0;
	if (hand->HeldObject != NULL)
	{
		HandStateHolding::Update(dt, matrices);
		hand->Point0x48e0 = hand->position;
		field_0x00d13f00 = hand->position;
		field_0x1e0 = hand->position - oldPos;
		float   invDt = 1.0f / dt;
		LHPoint velocity = (hand->position - oldPos) * invDt;
		field_0x1fc.Update(dt, velocity);
		hand->Point0x48c8 = field_0x1fc.GetValue();

		float   lateral = 0.0f;
		LHPoint side(oldMove.z, 0.0f, -oldMove.x);
		if (side.Normalise() != 0.0f)
		{
			lateral = side.DotProductInline(field_0x1e0);
		}
		field_0x210.Update(dt, lateral * invDt * invDt);

		float angle = 0.0f;
		float oldLengthSq = oldMove.z * oldMove.z + oldMove.x * oldMove.x;
		if (oldLengthSq > 0.0001f)
		{
			float lengthSq = field_0x1e0.z * field_0x1e0.z + field_0x1e0.x * field_0x1e0.x;
			if (lengthSq > 0.0001f)
			{
				angle = (float)acos((field_0x1e0.z * oldMove.z + field_0x1e0.x * oldMove.x) /
				                    ((float)sqrt(lengthSq) * (float)sqrt(oldLengthSq)));
			}
		}
		field_0x21c.Update(dt, angle * invDt);

		float   speed = field_0x1fc.GetValue().GetNorme();
		LHPoint tilt(0.0f, 0.0f, 0.0f);
		tilt.x = field_0x21c.GetValue();
		if (field_0x00d13f1c)
		{
			tilt.y = -field_0x21c.GetValue();
		}
		else if (speed > 0.0001f)
		{
			tilt.y = -(field_0x210.GetValue() / speed);
		}
		else
		{
			tilt.y = 0.0f;
		}
		hand->Point0x48d4 = tilt;
	}
}
