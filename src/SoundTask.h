#pragma once

#include <SDL2/SDL_mixer.h>
#include "ITask.h"

class SoundTask : public ITask
{
public:
    SoundTask();
    ~SoundTask() = default;

    bool Start();
    void OnSuspend();
    void Update();
    void OnResume();
    void Stop();

    void PlayChannel(int ID);
    void PlayMusic(int ID);
    void PauseMusic();
    void SetActive(bool active);

private:
    bool opened = false;
    bool active = true;
    bool musicPaused = false;
public:

    Mix_Music *music = nullptr;

    Mix_Chunk *cowbell = nullptr;
    Mix_Chunk *fire = nullptr;
    Mix_Chunk *xphone = nullptr;
    Mix_Chunk *energy = nullptr;

    Mix_Chunk *jump = nullptr;
    Mix_Chunk *land = nullptr;

    Mix_Chunk *death = nullptr;
    Mix_Chunk *death2 = nullptr;
    Mix_Chunk *hit = nullptr;
    Mix_Chunk *bounce = nullptr;

    int audio_rate;
    Uint16 audio_format;
    int audio_channels;
    int audio_buffers;

    int cowbellChannel = 0;
    int fireChannel = 2;
    int xphoneChannel = 1;
    int energyChannel = 3;

    int jumpChannel = 4;
    int landChannel = 5;

    int deathChannel = 6;
    int death2Channel = 7;
    int hitChannel = 8;
    int bounceChannel = 9;

    static unsigned int samplingRate;

    bool disable;

    int musicID = 0;
};
