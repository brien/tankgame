// Deterministic order regression: compare pipeline with explicit opaque/overlay
// submission, and require the previous interleaved order to differ visibly.
#include "App.h"
#include "rendering/RenderingPipeline.h"
#include "rendering/PlayerTankRendererImpl.h"
#include "rendering/RendererMode.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cstdio>
#include <vector>

std::vector<unsigned char> Pixels(int w, int h) {
    std::vector<unsigned char> data(w*h*4);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    return data;
}

int main() {
    App app; SoundTask sound; VideoTask video;
    app.soundTask = &sound; app.videoTask = &video;
    if (!video.Start() || !RendererMode::IsModern()) return 1;
    SDL_SetWindowFullscreen(SDL_GL_GetCurrentWindow(), 0);
    SDL_SetWindowSize(SDL_GL_GetCurrentWindow(), 601, 501);
    SDL_PumpEvents(); video.RefreshDrawableSize();
    const int w = VideoTask::scrWidth, h = VideoTask::scrHeight;
    {
        GraphicsTask graphics; app.graphicsTask = &graphics;
        graphics.resourceManager.reset(new ResourceManager);
        auto& resources = graphics.Resources();
        if (!resources.Initialize()) return 2;
        ViewportManager views; CameraManager cameras;
        RenderingPipeline pipeline(views, cameras, resources);
        PlayerTankRendererImpl playerRenderer;
        EnemyTankRendererImpl enemyRenderer(&resources);
        if (!pipeline.Initialize() || !playerRenderer.Initialize() || !enemyRenderer.Initialize()) return 3;
        SceneData scene;
        scene.cameras.resize(2);
        scene.cameras[0].position = Vector3(0, 4, 2);
        scene.cameras[1].position = Vector3(0, 4, -2);
        TankRenderData player;
        player.isPlayer = true;
        player.primaryColor = Color(.3f, .6f, .8f, 1);
        TankRenderData enemy;
        enemy.position = Vector3(.4f, 0, 0);
        enemy.primaryColor = Color(.2f, .3f, .4f, 1);
        enemy.secondaryColor = Color(.2f, .3f, .4f, 1);
        for (int players = 1; players <= 2; ++players) {
            scene.numPlayers = players;
            pipeline.ConfigureViewports(players, w, h);
            std::vector<std::vector<unsigned char>> captures;
            for (int order = 0; order < 3; ++order) {
                pipeline.SetupRenderState();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                for (int view = 0; view < players; ++view) {
                    scene.tanks = order == 0 ? std::vector<TankRenderData>{player, enemy}
                                            : std::vector<TankRenderData>{};
                    pipeline.RenderScene(scene, view);
                    if (order != 0) {
                        playerRenderer.RenderOpaque(player);
                        if (order == 2) playerRenderer.RenderOverlays(player);
                        enemyRenderer.SetupRenderState();
                        enemyRenderer.Render(enemy);
                        enemyRenderer.CleanupRenderState();
                        if (order == 1) {
                            std::vector<float> before(w*h), after(w*h);
                            glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, before.data());
                            playerRenderer.RenderOverlays(player);
                            glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, after.data());
                            if (before != after) return 4;
                            GLboolean writes = GL_FALSE;
                            glGetBooleanv(GL_DEPTH_WRITEMASK, &writes);
                            GLint depth = 0; glGetIntegerv(GL_DEPTH_FUNC, &depth);
                            if (!writes || depth != GL_LESS || glIsEnabled(GL_BLEND)) return 5;
                        }
                    }
                }
                captures.push_back(Pixels(w,h));
                if (glGetError()) return 6;
                pipeline.CleanupRenderState();
            }
            if (captures[0] != captures[1]) return 7;
            int changed = 0;
            for (size_t i = 0; i < captures[0].size(); i += 4)
                if (!std::equal(captures[0].begin()+i, captures[0].begin()+i+4, captures[2].begin()+i)) ++changed;
            std::printf("players=%d: exact ordered reference, unchanged overlay depth, old order differs at %d pixels, GL_NO_ERROR\n", players, changed);
            if (changed < 10) return 8;
            auto pixels = captures[0];
            for (int y = 0; y < h; ++y)
                std::copy(captures[0].begin()+y*w*4, captures[0].begin()+(y+1)*w*4, pixels.begin()+(h-1-y)*w*4);
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels.data(), w,h,32,w*4,0xff,0xff00,0xff0000,0xff000000);
            char path[256];
            std::snprintf(path,sizeof(path),"../documentation/player-overlays-2026-10-16/modern-%d.bmp",players);
            SDL_SaveBMP(surface,path); SDL_FreeSurface(surface);
        }
        enemyRenderer.Cleanup(); playerRenderer.Cleanup(); pipeline.Cleanup(); resources.Cleanup();
        TTF_CloseFont(graphics.defaultFont);
    }
    video.Stop();
}
