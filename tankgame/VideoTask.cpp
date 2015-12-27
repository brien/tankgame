#include "VideoTask.h"
#include "TankHandler.h"
#include "App.h"

#include <SDL2/SDL.h>


int VideoTask::scrWidth=800;
int VideoTask::scrHeight=600;
int VideoTask::scrBPP=32;

VideoTask::VideoTask()
{

}

VideoTask::~VideoTask()
{

}


bool VideoTask::Start()
{
	
	if(-1==SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK))
	{
		return false;
	}
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1 );
	SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 1 );

   char line[64];
   FILE *filein;
	filein=fopen("settings.txt", "rt");

   fgets(line, 64, filein);
   fgets(line, 64, filein);

   if(line[0]=='0')
   {
      flags = SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_OPENGL;
      //hide the mouse cursor
	   //SDL_ShowCursor(SDL_DISABLE);
   }
   else
   {
      flags = SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_OPENGL|SDL_FULLSCREEN;
      //hide the mouse cursor
	   SDL_ShowCursor(SDL_DISABLE);
   }

   fgets(line, 64, filein);
   fgets(line, 64, filein);

   scrWidth=(line[0]-48)*100;
   scrWidth+=(line[1]-48)*10;
   scrWidth+=(line[2]-48);

   if(line[3]>=48 && line[3]<=57)
   {
      scrWidth*=10;
      scrWidth+=(line[3]-48);
   }

   fgets(line, 64, filein);
   fgets(line, 64, filein);

   scrHeight=(line[0]-48)*100;
   scrHeight+=(line[1]-48)*10;
   scrHeight+=(line[2]-48);

   if(line[3]>=48 && line[3]<=57)
   {
      scrHeight*=10;
      scrHeight+=(line[3]-48);
   }

   fgets(line, 64, filein);
   fgets(line, 64, filein);

   TankHandler::GetSingleton().diff=(line[0]-48);

   fgets(line, 64, filein);
   fgets(line, 64, filein);

   App::GetSingleton().soundTask->disable=(bool)(line[0]-48);

   fclose(filein);
	

	if(!SDL_SetVideoMode(scrWidth, scrHeight, scrBPP, flags) )
	{
		return false;
	}




	SDL_WM_SetCaption("Tank!Game 2", 0);

	return true;
}

void VideoTask::Update()
{
	
	SDL_GL_SwapBuffers();
}

void VideoTask::Stop()
{

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

