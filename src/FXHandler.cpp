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

FXHandler::FXHandler()
{
	fx.reserve(128);
}

void FXHandler::NextFrame()
{
	if (gameWorld)
	{
		// GameWorld handles updates, we just need to sync for rendering
		// The GameWorld's EntityManager will handle the update/cleanup
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

void FXHandler::CreateFX(FxType _type, float _x, float _y, float _z, float _rx, float _ry, float _rz, const Color& _color)
{
	if (gameWorld)
	{
		// Delegate to GameWorld
		gameWorld->CreateFX(_type, _x, _y, _z, _rx, _ry, _rz, _color.r, _color.g, _color.b, _color.a);
	}
	else
	{
		// Fallback to old system during transition
		FX temp(_type, _x, _y, _z, _rx, _ry, _rz, _color);
		fx.push_back(temp);
	}
}

void FXHandler::CreateFX(FxType _type, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a)
{
	CreateFX(_type, _x, _y, _z, _rx, _ry, _rz, Color(_r, _g, _b, _a));
}

void FXHandler::CreateFX(FxType _type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, const Color& _color)
{
	if (gameWorld)
	{
		// Delegate to GameWorld  
		gameWorld->CreateFX(_type, _x, _y, _z, _dx, _dy, _dz, _rx, _ry, _rz, _color.r, _color.g, _color.b, _color.a);
	}
	else
	{
		// Fallback to old system during transition
		FX temp(_type, _x, _y, _z, _dx, _dy, _dz, _rx, _ry, _rz, _color);
		fx.push_back(temp);
	}
}

void FXHandler::CreateFX(FxType _type, float _x, float _y, float _z, float _dx, float _dy, float _dz, float _rx, float _ry, float _rz, float _r, float _g, float _b, float _a)
{
	CreateFX(_type, _x, _y, _z, _dx, _dy, _dz, _rx, _ry, _rz, Color(_r, _g, _b, _a));
}

void FXHandler::ClearFX()
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
		// Create unified view: old FX + GameWorld FX
		unifiedFXView.clear();
		
		// Add old system FX
		unifiedFXView.insert(unifiedFXView.end(), fx.begin(), fx.end());
		
		// Add GameWorld FX (dereference the unique_ptrs)
		const auto& worldFX = gameWorld->GetFX();
		for (const auto& fxPtr : worldFX)
		{
			if (fxPtr && fxPtr->IsAlive())
			{
				unifiedFXView.push_back(*fxPtr);
			}
		}
		
		return unifiedFXView;
	}
	else
	{
		// No GameWorld, just return old system FX
		return fx;
	}
}

// Legacy draw method removed - effects now handled by EffectRenderer
