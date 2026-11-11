// SoundTask.cpp: implementation of the SoundTask class.
//
//////////////////////////////////////////////////////////////////////

#include "SoundTask.h"
#include "Logger.h"
#include <iostream>
#include <SDL2/SDL.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

unsigned int SoundTask::samplingRate = 44100;

SoundTask::SoundTask()
{
    disable = false;
}

bool SoundTask::Start()
{
    if (!disable)
    {
        music = NULL;
        energy = NULL;
        cowbell = NULL;
        xphone = NULL;
        fire = NULL;

        jump = NULL;
        land = NULL;

        death = NULL;
        death2 = NULL;

        hit = NULL;
        bounce = NULL;

        cowbellChannel = 0;
        xphoneChannel = 1;
        fireChannel = 2;
        energyChannel = 3;

        jumpChannel = 4;
        landChannel = 5;

        deathChannel = 6;
        death2Channel = 7;
        hitChannel = 8;
        bounceChannel = 9;

        audio_rate = 22050;
        audio_format = AUDIO_S16;
        audio_channels = 2;
        audio_buffers = 2048;

        if (-1 == SDL_InitSubSystem(SDL_INIT_AUDIO))
        {
            Logger::Get().Write("Unable to initialize SDL audio: %.512s\n", SDL_GetError());
            disable = true;
            return true;
        }

        if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
        {
            Logger::Get().Write("Unable to open audio! Running without sound: %.512s\n", Mix_GetError());
            disable = true;
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return true;
        }

        opened = true;
        Mix_AllocateChannels(16);
        Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

        SDL_version compiled;
        SDL_MIXER_VERSION(&compiled);
        const SDL_version* linked = Mix_Linked_Version();
        Logger::Get().Write("SDL_mixer compiled %d.%d.%d, linked %d.%d.%d; audio %d Hz, %d channels, format 0x%x.\n",
                            compiled.major, compiled.minor, compiled.patch,
                            linked->major, linked->minor, linked->patch,
                            audio_rate, audio_channels, static_cast<unsigned int>(audio_format));

        music = Mix_LoadMUS("sounds/TGTest3.ogg");
        if (!music)
            Logger::Get().Write("Unable to load music 'sounds/TGTest3.ogg' (relative to working directory): %.512s\n", Mix_GetError());
        else
            Logger::Get().Write("Loaded music 'sounds/TGTest3.ogg'.\n");

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
    SetActive(false);
}

void SoundTask::Update()
{
    // we don't need to do anything, SDL_mixer does the work
}

void SoundTask::OnResume()
{
    SetActive(true);
}

void SoundTask::PlayChannel(int ID)
{
    if (!disable && opened && active)
    {
        if (ID == cowbellChannel)
        {
            Mix_PlayChannel(-1, cowbell, 0);
        }
        if (ID == fireChannel)
        {
            Mix_PlayChannel(ID, fire, 0);
        }
        if (ID == xphoneChannel)
        {
            Mix_PlayChannel(-1, xphone, 0);
        }
        if (ID == energyChannel)
        {
            Mix_PlayChannel(-1, energy, 0);
        }
        if (ID == jumpChannel)
        {
            Mix_PlayChannel(-1, jump, 0);
        }
        if (ID == landChannel)
        {
            Mix_PlayChannel(-1, land, 0);
        }
        if (ID == deathChannel)
        {
            Mix_PlayChannel(-1, death, 0);
        }
        if (ID == death2Channel)
        {
            Mix_PlayChannel(-1, death2, 0);
        }
        if (ID == hitChannel)
        {
            Mix_PlayChannel(-1, hit, 0);
        }
        if (ID == bounceChannel)
        {
            Mix_PlayChannel(ID, bounce, 0);
        }
    }
}

void SoundTask::PlayMusic(int ID)
{
    if (ID < 0) ID = musicID + 1;
    if (ID > 7) ID = 0;
    if (disable || !opened || ID != 0) return;
    musicID = ID;
    if (music && !Mix_PlayingMusic())
    {
        if (Mix_PlayMusic(music, -1) == -1)
            Logger::Get().Write("Unable to start music 'sounds/TGTest3.ogg': %.512s\n", Mix_GetError());
        else
            Logger::Get().Write("Music playback started (active=%d, musicPaused=%d).\n", active, musicPaused);
    }
    if (!active || musicPaused) Mix_PauseMusic();
}

void SoundTask::PauseMusic()
{
    if (disable || !opened) return;
    musicPaused = !musicPaused;
    if (musicPaused || !active) Mix_PauseMusic();
    else Mix_ResumeMusic();
    Logger::Get().Write("Music toggle: loaded=%d, active=%d, requestedPause=%d, playing=%d, paused=%d.\n",
                        music != nullptr, active, musicPaused, Mix_PlayingMusic(), Mix_PausedMusic());
}

void SoundTask::SetActive(bool value)
{
    if (active == value) return;
    active = value;
    if (!opened) return;
    if (!active)
    {
        Mix_HaltChannel(-1);
        Mix_PauseMusic();
    }
    else if (!musicPaused) Mix_ResumeMusic();
}

void SoundTask::Stop()
{
    if (!opened) return;
    Mix_HaltChannel(-1);
    Mix_HaltMusic();
    for (Mix_Chunk* chunk : {cowbell, fire, xphone, energy, jump, land, death, death2, hit, bounce})
        Mix_FreeChunk(chunk);
    cowbell = fire = xphone = energy = jump = land = death = death2 = hit = bounce = nullptr;
    Mix_FreeMusic(music);
    music = nullptr;
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    opened = false;
}
