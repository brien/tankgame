#include "FX.h"
#include "App.h"
#include "LevelHandler.h"

FX::FX()
{
	x = y = z = 0;
	rx = ry = rz = 0;
	dx = dy = dz = 0;
	color = Color(0.0f, 0.0f, 0.0f, 0.0f);
	time = 0;
	maxTime = 10;
	alive = true;
	type = FxType::TYPE_ZERO;
}

FX::FX(FxType _type, float _x, float _y, float _z, float _rx, float _ry, float _rz, const Color& _color)
{
	type = _type;
	x = _x;
	y = _y;
	z = _z;
	rx = _rx;
	ry = _ry;
	rz = _rz;
	dx = dy = dz = 0;
	color = _color;

	alive = true;
	time = 0;
	SetMaxTime();
}

FX::FX(FxType _type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, const Color& _color)
{
	type = _type;
	x = _x;
	y = _y;
	z = _z;
	dx = _dx;
	dy = _dy;
	dz = _dz;
	rx = _rx;
	ry = _ry;
	rz = _rz;
	color = _color;

	alive = true;
	time = 0;
	SetMaxTime();
}

void FX::SetMaxTime()
{
	if (type == FxType::TYPE_SMALL_SQUARE)
	{
		maxTime = 0.2f;
	}
	else if (type == FxType::TYPE_DEATH)
	{
		maxTime = 0.4f;
	}
	else if (type == FxType::TYPE_SMALL_RECTANGLE)
	{
		maxTime = 2.5f;
	}
	else
	{
		maxTime = 0.3f;
	}
}

void FX::Update()
{
	time += GlobalTimer::dT;

	// Check if effect has timed out
	if (time > maxTime)
	{
		alive = false;
		return;
	}

	if (type == FxType::TYPE_SMALL_RECTANGLE)
	{
		color.a -= 0.2 * GlobalTimer::dT;
	}
	else
	{
		color.a -= 2 * GlobalTimer::dT;
	}

	x += dx * GlobalTimer::dT;
	y += dy;
	z += dz * GlobalTimer::dT;

	if (type == FxType::TYPE_DEATH)
	{
		if ((int)ry % 2 == 0)
		{
			ry += 120.0 * GlobalTimer::dT;
			rx += 300.0 * GlobalTimer::dT;
		}

		rz += 150.0 * GlobalTimer::dT;
	}

	if (type == FxType::TYPE_THREE)
		rz += 300 * GlobalTimer::dT;

	if (type == FxType::TYPE_JUMP && !LevelHandler::GetSingleton().PointCollision(x, y, z))
	{
		y -= 5 * GlobalTimer::dT;
		color.r -= .5 * GlobalTimer::dT;
		color.g -= .5 * GlobalTimer::dT;
		color.b -= .5 * GlobalTimer::dT;
	}
}
