// SoundTask.cpp: implementation of the SoundTask class.
//
//////////////////////////////////////////////////////////////////////


#include "SoundTask.h"
#include <SDL2/sdl.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

unsigned int SoundTask::samplingRate = 44100;

SoundTask::SoundTask()
{
    disable=false;
}

SoundTask::~SoundTask()
{
    
}

bool SoundTask::Start()
{
    if(!disable)
    {
        music=NULL;
        energy=NULL;
        cowbell=NULL;
        xphone = NULL;
        fire=NULL;
        
        jump=NULL;
        land=NULL;
        
        death=NULL;
        death2=NULL;
        
        hit=NULL;
        bounce=NULL;
        
        cowbellChannel = 0;
        xphoneChannel = 1;
        fireChannel=2;
        energyChannel=3;
        
        jumpChannel=4;
        landChannel=5;
        
        deathChannel = 6;
        death2Channel = 7;
        hitChannel = 8;
        bounceChannel = 9;
        
        audio_rate = 22050;
        audio_format = AUDIO_S16;
        audio_channels = 2;
        audio_buffers = 2048;
        
        if(-1==SDL_InitSubSystem(SDL_INIT_AUDIO))
        {
            return false;
        }
        
        if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
        {
            printf("Unable to open audio!\n");
            exit(1);
        }
        
        Mix_AllocateChannels(16);
        Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
        
        music = Mix_LoadMUS("sounds/TGTest3.ogg");
        
        cowbell = Mix_LoadWAV("sounds/cowbell.wav");
        
        death = Mix_LoadWAV("sounds/shatter.wav");
        death2 = Mix_LoadWAV("sounds/shatter2.wav");
        hit = Mix_LoadWAV("sounds/hit3.wav");
        
        fire = Mix_LoadWAV("sounds/shot.wav");
        xphone = Mix_LoadWAV("sounds/xphone.wav");
        energy = Mix_LoadWAV("sounds/gain.wav");
        
        jump = Mix_LoadWAV("sounds/jump.wav");
        land = Mix_LoadWAV("sounds/land.wav");
        bounce = Mix_LoadWAV("sounds/elastik.wav");
        
        Mix_Volume(-1, 50);
        
    }
    
    return true;
}

void SoundTask::OnSuspend()
{
    
}

void SoundTask::Update()
{
    //we don't need to do anything, SDL_mixer does the work
}

void SoundTask::OnResume()
{
    
}

void SoundTask::PlaySound(int ID)
{
    if(!disable)
    {
        if(ID==cowbellChannel)
        {
            Mix_PlayChannel(-1, cowbell, 0);
        }
        if(ID==fireChannel)
        {
            Mix_PlayChannel(ID, fire, 0);
        }
        if(ID==xphoneChannel)
        {
            Mix_PlayChannel(-1, xphone, 0);
        }
        if(ID==energyChannel)
        {
            Mix_PlayChannel(-1, energy, 0);
        }
        if(ID==jumpChannel)
        {
            Mix_PlayChannel(-1, jump, 0);
        }
        if(ID==landChannel)
        {
            Mix_PlayChannel(-1, land, 0);
        }
        if(ID==deathChannel)
        {
            Mix_PlayChannel(-1, death, 0);
        }
        if(ID==death2Channel)
        {
            Mix_PlayChannel(-1, death2, 0);
        }
        if(ID==hitChannel)
        {
            Mix_PlayChannel(-1, hit, 0);
        }
        if(ID==bounceChannel)
        {
            Mix_PlayChannel(ID, bounce, 0);
        }
    }
    
}

void SoundTask::PlayMusic(int ID)
{
    if(ID<0)
    {
        ID=musicID+1;
    }
    if(!disable && Mix_PausedMusic() == 0)
    {
        if(ID>7)
            ID=0;
        if(ID == 0)
        {
            Mix_FreeMusic(music);
            music = Mix_LoadMUS("sounds/TGTest3.ogg");
            Mix_PlayMusic(music, -1);
            musicID=ID;
        }
    }
}

void SoundTask::PauseMusic()
{
    if(!disable)
    {
        if(Mix_PausedMusic() == 0)
        {
            Mix_PauseMusic();
        }
        else
        {
            Mix_ResumeMusic();
        }
    }
}


void SoundTask::Stop()
{
    if(!disable)
    {
        Mix_FreeChunk(cowbell);
        Mix_FreeChunk(fire);
        Mix_FreeChunk(xphone);
        Mix_FreeChunk(energy);
        Mix_FreeChunk(jump);
        Mix_FreeChunk(death);
        Mix_FreeChunk(death2);
        Mix_FreeChunk(hit);
        Mix_FreeChunk(bounce);
        
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
        
        Mix_CloseAudio();
    }
}