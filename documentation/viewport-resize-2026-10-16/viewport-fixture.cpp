// Render each view alone, then prove the full pipeline preserves both views.
#include "App.h"
#include "rendering/RenderingPipeline.h"
#include "rendering/RendererMode.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cstdio>
#include <vector>

std::vector<unsigned char> ReadPixels(int width, int height)
{
    std::vector<unsigned char> pixels(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    return pixels;
}

int main()
{
    App app; SoundTask sound; VideoTask video;
    app.soundTask = &sound;
    app.videoTask = &video;
    if (!video.Start()) return 1;
    SDL_Window* window = SDL_GL_GetCurrentWindow();
    SDL_SetWindowFullscreen(window, 0);
    {
        GraphicsTask graphics;
        app.graphicsTask = &graphics;
        graphics.resourceManager.reset(new ResourceManager);
        auto& resources = graphics.Resources();
        if (!resources.Initialize()) return 2;
        ViewportManager views;
        CameraManager cameras;
        RenderingPipeline pipeline(views, cameras, resources);
        if (!pipeline.Initialize()) return 3;
        SceneData scene;
        scene.terrain.sizeX = scene.terrain.sizeZ = 8;
        scene.terrain.heightMap[3][3] = -3;
        scene.terrain.heightMap[4][3] = 2;
        scene.terrain.colors.defaultColor = Vector3(.7f, .5f, .3f);
        TankRenderData enemy;
        enemy.position = Vector3(2, 0, 3);
        enemy.primaryColor = Color(.2f, .7f, .8f, 1);
        scene.tanks.push_back(enemy);
        EffectRenderData effect;
        effect.type = FxType::TYPE_THREE;
        effect.position = Vector3(4, 1, 4);
        effect.alpha = .6f;
        scene.effects.push_back(effect);
        scene.cameras.resize(2);
        scene.cameras[0].position = Vector3(10, 9, 10);
        scene.cameras[1].position = Vector3(-4, 7, 9);
        for (auto& camera : scene.cameras) camera.focus = Vector3(3, 0, 3);
        const int sizes[][2] = {{801,601}, {997,503}, {801,601}};
        for (int frame = 0; frame < 3; ++frame)
        {
            SDL_SetWindowSize(window, sizes[frame][0], sizes[frame][1]);
            SDL_PumpEvents();
            if (!video.RefreshDrawableSize()) return 4;
            const int width = VideoTask::scrWidth, height = VideoTask::scrHeight;
            for (int players = 1; players <= 2; ++players)
            {
                scene.numPlayers = players;
                pipeline.ConfigureViewports(players, width, height);
                std::vector<std::vector<unsigned char>> references;
                for (int view = 0; view < players; ++view)
                {
                    pipeline.SetupRenderState();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    pipeline.RenderScene(scene, view);
                    references.push_back(ReadPixels(width, height));
                    pipeline.CleanupRenderState();
                }
                pipeline.SetupRenderState();
                pipeline.RenderAllPlayerViews(scene);
                const auto combined = ReadPixels(width, height);
                for (int view = 0; view < players; ++view)
                {
                    const auto& viewport = views.GetViewport(view);
                    int visible = 0;
                    for (int y = viewport.y; y < viewport.y + viewport.height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            const int i = (y * width + x) * 4;
                            if (!std::equal(combined.begin()+i, combined.begin()+i+4,
                                            references[view].begin()+i)) return 5;
                            if (combined[i] || combined[i+1]) ++visible;
                        }
                    }
                    if (visible < 100) return 6;
                    std::printf("%s %dx%d players=%d view=%d: %d visible pixels, exact isolated-view match\n",
                                RendererMode::Name(), width, height, players, view, visible);
                }
                const unsigned error = glGetError();
                std::printf("frame=%d players=%d GL error=%u\n", frame, players, error);
                if (error) return 7;
                if (frame < 2)
                {
                    std::vector<unsigned char> flipped(combined.size());
                    for (int y = 0; y < height; ++y)
                        std::copy(combined.begin()+y*width*4, combined.begin()+(y+1)*width*4,
                                  flipped.begin()+(height-1-y)*width*4);
                    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(flipped.data(), width, height,
                        32, width*4, 0xff, 0xff00, 0xff0000, 0xff000000);
                    char path[256];
                    std::snprintf(path, sizeof(path), "../documentation/viewport-resize-2026-10-16/%s-%d-%d.bmp",
                                  RendererMode::Name(), frame, players);
                    SDL_SaveBMP(surface, path);
                    SDL_FreeSurface(surface);
                }
                pipeline.CleanupRenderState();
            }
        }
        pipeline.Cleanup();
        resources.Cleanup();
        TTF_CloseFont(graphics.defaultFont);
    }
    video.Stop();
    return 0;
}
