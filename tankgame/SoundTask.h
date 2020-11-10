// SoundTask.h: interface for the SoundTask class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SOUND_TASK_H
#define SOUND_TASK_H

#include <SDL2_mixer/SDL_mixer.h>
#include "ITask.h"

class SoundTask : public ITask
{
public:
    SoundTask();
    virtual ~SoundTask();
    
    bool Start();
    void OnSuspend();
    void Update();
    void OnResume();
    void Stop();
    
    void PlayChannel(int ID);
    void PlayMusic(int ID);
    void PauseMusic();
    
    Mix_Music *music;
    
    Mix_Chunk *cowbell;
    Mix_Chunk *fire;
    Mix_Chunk *xphone;
    Mix_Chunk *energy;
    
    Mix_Chunk *jump;
    Mix_Chunk *land;
    
    Mix_Chunk *death;
    Mix_Chunk *death2;
    Mix_Chunk *hit;
    Mix_Chunk *bounce;
    
    int audio_rate;
    Uint16 audio_format;
    int audio_channels;
    int audio_buffers;
    
    int cowbellChannel;
    int fireChannel;
    int xphoneChannel;
    int energyChannel;
    
    int jumpChannel;
    int landChannel;
    
    int deathChannel;
    int death2Channel;
    int hitChannel;
    int bounceChannel;
    
    static unsigned int samplingRate;
    
    bool disable;
    
    int musicID;
    
};

#endif
