#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#else
// If building on Linux:
#include <GL/gl.h>
#endif

#include "App.h"
#include "FXHandler.h"
#include "LevelHandler.h"

FX::FX()
{
	x = y = z = 0;
	rx = ry = rz = 0;
	dx = dy = dz = 0;
	r = g = b = a = 0;
	time = 0;
	maxTime = 10;
	alive = true;
	type = FxType::TYPE_ZERO;
}

FX::FX(FxType _type, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a)
{
	type = _type;
	x = _x;
	y = _y;
	z = _z;
	rx = _rx;
	ry = _ry;
	rz = _rz;
	dx = dy = dz = 0;
	r = _r;
	g = _g;
	b = _b;
	a = _a;

	alive = true;
	time = 0;
	SetMaxTime();
}

FX::FX(FxType _type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a)
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
	r = _r;
	g = _g;
	b = _b;
	a = _a;

	alive = true;
	time = 0;
	SetMaxTime();
}

void FX::SetMaxTime()
{
	if (type == FxType::TYPE_SMALL_SQUARE)
	{
		maxTime = 0.2;
	}
	else if (type == FxType::TYPE_DEATH)
	{
		maxTime = 0.4;
	}
	else if (type == FxType::TYPE_SMALL_RECTANGLE)
	{
		maxTime = 2.5;
	}
	else
	{
		maxTime = 0.3;
	}
}

void FX::Update()
{
	time += GlobalTimer::dT;

	if (type == FxType::TYPE_SMALL_RECTANGLE)
	{
		a -= 0.2 * GlobalTimer::dT;
	}
	else
	{
		a -= 2 * GlobalTimer::dT;
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
		r -= .5 * GlobalTimer::dT;
		g -= .5 * GlobalTimer::dT;
		b -= .5 * GlobalTimer::dT;
	}
}

FXHandler::FXHandler()
{
	fx.reserve(128);
}

void FXHandler::NextFrame()
{
	for (auto j = fx.begin(); j != fx.end();)
	{
		if (j->time > j->maxTime)
		{
			j->alive = false;
		}

		if (j->alive)
		{
			j->Update();
			++j;
		}
		else
		{
			j = fx.erase(j);
		}
	}
}

void FXHandler::CreateFX(FxType _type, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a)
{
	FX temp(_type, _x, _y, _z, _rx, _ry, _rz, _r, _g, _b, _a);

	fx.push_back(temp);
}

void FXHandler::CreateFX(FxType _type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a)
{
	FX temp(_type, _x, _y, _z, _dx, _dy, _dz, _rx, _ry, _rz, _r, _g, _b, _a);

	fx.push_back(temp);
}

void FXHandler::ClearFX()
{
	fx.clear();
}

// Legacy draw method removed - effects now handled by EffectRenderer
