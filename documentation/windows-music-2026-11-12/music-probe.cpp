#include <SDL.h>
#include <SDL_mixer.h>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <cstring>
std::atomic<bool> finished{false};
char finishError[512] = {};
void OnFinished() {
    SDL_strlcpy(finishError, Mix_GetError(), sizeof(finishError));
    finished.store(true);
}
int main(int argc, char** argv) {
    if (argc != 4) {
        std::fprintf(stderr, "Usage: music-probe RATE BUFFER_FRAMES MUSIC_PATH\n");
        return 3;
    }
    SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
    SDL_Init(SDL_INIT_AUDIO);
    int rate = argc > 1 ? std::atoi(argv[1]) : 22050;
    if (Mix_OpenAudio(rate, AUDIO_S16SYS, 2, argc > 2 ? std::atoi(argv[2]) : 2048) < 0) return 1;
    Mix_Music* music = Mix_LoadMUS(argv[3]);
    if (!music) { std::printf("load error: %s\n", Mix_GetError()); return 2; }
    Mix_HookMusicFinished(OnFinished);
    std::printf("rate=%d start=%d duration=%.3f\n", rate, Mix_PlayMusic(music, -1), Mix_MusicDuration(music));
    SDL_Delay(1200);
    std::printf("playing=%d pos=%.3f finished=%d\n", Mix_PlayingMusic(), Mix_GetMusicPosition(music), int(finished.load()));
    if (finished.load()) std::printf("audio thread error: %s\n", finishError);
    Mix_HookMusicFinished(nullptr);
    Mix_FreeMusic(music); Mix_CloseAudio(); SDL_Quit();
}
