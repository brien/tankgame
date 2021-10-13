
#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#else
//if not:
#include <OpenGL/gl.h>
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
	type = 0;
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
	if (type == 4)
	{
		maxTime = 0.2;
	}
	else if (_type == TYPE_SMALL_RECTANGLE)
	{
		maxTime = 10;
	}
	else
	{
		maxTime = 0.3;
	}
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
	if (type == 4)
	{
		maxTime = .2;
	}
	else if (type == 6)
	{
		maxTime = .4;
	}
	else if (_type == TYPE_SMALL_RECTANGLE)
	{
		maxTime = 10;
	}
	else
	{
		maxTime = .3;
	}
}

FX::~FX()
{

}


void FX::Update()
{
	a -= 2 * GlobalTimer::dT;
	time += GlobalTimer::dT;

	x += dx * GlobalTimer::dT;
	y += dy;
	z += dz * GlobalTimer::dT;

	if (type == TYPE_DEATH)
	{
		if ((int)ry % 2 == 0)
		{
			ry += 120.0 * GlobalTimer::dT;
			rx += 300.0 * GlobalTimer::dT;
		}

		rz += 150.0 * GlobalTimer::dT;
	}


	if (type == TYPE_THREE)
		rz += 300 * GlobalTimer::dT;

	if (type == TYPE_JUMP && !LevelHandler::GetSingleton().PointCollision(x, y, z))
	{
		y -= 5 * GlobalTimer::dT;
		r -= .5 * GlobalTimer::dT;
		g -= .5 * GlobalTimer::dT;
		b -= .5 * GlobalTimer::dT;
	}

}


void FX::Draw()
{

	glColor3f(r, g, b);

	glPushMatrix();

	glTranslatef(x, y + .2, z);
	glRotatef(rx, 1, 0, 0);
	glRotatef(-ry, 0, 1, 0);
	glRotatef(rz, 0, 0, 1);

	if (type == TYPE_THREE)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[16]);
	}
	if (type == TYPE_STAR)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[19]);
	}

	glDisable(GL_CULL_FACE);

	if (type == TYPE_ZERO)
	{
		glScalef(5 * time / maxTime, 1, 5 * time / maxTime);
	}

	if (type == TYPE_JUMP)
	{
		glScalef(1 * time / maxTime, 1, 1 * time / maxTime);
	}

	if (type == TYPE_SMOKE)
	{
		glScalef(.25 * time / maxTime, .25, .25 * time / maxTime);
	}

	if (type == TYPE_SMALL_SQUARE)
	{
		glScalef(.5 * time / maxTime, .5, .5 * time / maxTime);
	}

	if (type == TYPE_STAR)
	{
		glScalef(.3 * time / maxTime, .3, .3 * time / maxTime);
	}

	if (type == TYPE_DEATH)
	{
		glCallList(App::GetSingleton().graphicsTask->squarelist2);
	}

	if (type == TYPE_ZERO)
	{
		glCallList(App::GetSingleton().graphicsTask->squarelist2);
	}

	if (type == TYPE_SMALL_RECTANGLE)
	{
		//a = time / maxTime;
		glScalef(0.02, 1, 0.2);
		glCallList(App::GetSingleton().graphicsTask->squarelist2);
	}

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);

	glColor4f(r, g, b, a);

	glCallList(App::GetSingleton().graphicsTask->squarelist);

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	glPopMatrix();

}


FXHandler::FXHandler()
{
	fx.reserve(128);
}

FXHandler::~FXHandler()
{
	fx.clear();
}


void FXHandler::NextFrame()
{
	for (vector<FX>::iterator j = fx.begin(); j != fx.end();)
	{
		if (j->time > j->maxTime)
		{
			j->alive = false;
		}

		if (j->alive)
		{
			j->Update();
			j++;
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



void FXHandler::Draw()
{
	if (!fx.empty())
	{
		vector< FX >::iterator it;
		for (it = fx.begin();it != fx.end();)
		{
			if (it->alive)
			{
				it->Draw();
				++it;
			}
		}
	}
}
