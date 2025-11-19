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
#include "GameWorld.h"

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

	// Check if FX has expired
	if (time >= maxTime || a <= 0.0f) {
		alive = false;
		return;
	}

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
	if (gameWorld)
	{
		// No-op - GameWorld handles all FX updates through EntityManager
		// Legacy fx.clear() to ensure no accumulation during transition
		fx.clear();
		return;
	}
	
	// Legacy update loop for fallback
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
	// Legacy method - no longer used, effects created directly through GameWorld events
	// This method exists only for interface compatibility during transition
}

void FXHandler::CreateFX(FxType _type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a)
{
	// Legacy method - no longer used, effects created directly through GameWorld events
	// This method exists only for interface compatibility during transition
}void FXHandler::ClearFX()
{
	fx.clear();
	if (gameWorld)
	{
		gameWorld->Clear();
	}
}

const std::vector<FX>& FXHandler::GetAllFX() const
{
	if (gameWorld)
	{
		// During transition: return empty vector since rendering should use GameWorld directly
		// TODO: Remove this method entirely when rendering is updated
		static const std::vector<FX> emptyFX;
		return emptyFX;
	}
	else
	{
		// Fallback to old system (should not be used in practice)
		return fx;
	}
}

// Legacy draw method removed - effects now handled by EffectRenderer
