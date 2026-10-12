// Deterministic opaque terrain comparison using both production renderers.
#include "App.h"
#include "rendering/TerrainRenderer.h"
#include "rendering/RenderContext.h"
#include "rendering/RendererMode.h"
#include <SDL2/SDL.h>
#include <GL/glu.h>
#include <cstdio>
#include <algorithm>
#include <vector>

int main() {
    App app; SoundTask sound; VideoTask video;
    app.soundTask = &sound;
    if (!video.Start()) return 1;
    int status = 0;
    {
        GraphicsTask graphics;
        app.graphicsTask = &graphics;
        graphics.resourceManager.reset(new ResourceManager);
        auto& resources = graphics.Resources();
        if (!resources.Initialize()) return 2;
        TerrainRenderer renderer(&resources);
        renderer.Initialize();
        for (int frame = 0; frame < 3; ++frame) {
            TerrainRenderData terrain;
            terrain.sizeX = terrain.sizeZ = 8;
            terrain.levelNumber = frame == 0 ? 48 : 50;
            terrain.colors.defaultColor = Vector3(.7f,.5f,.3f);
            terrain.colors.blockColor = Vector3(.2f,.6f,.7f);
            for (int x = 1; x < 6; ++x)
                for (int z = 1; z < 6; ++z)
                    terrain.heightMap[x][z] = x < 3 ? 1 : 2;
            terrain.floatMap[4][4] = 5 + frame;
            if (frame == 2) terrain.heightMap[3][3] = 4; // Same id, edited content.
            glViewport(0, 0, 1280, 720);
            glClearColor(0, 0, .2f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderContext::Current().SetProjection(Matrix4::Perspective(45, 1280.f/720, .1f, 100));
            RenderContext::Current().SetView(Matrix4::LookAt(12, 10, 14, 3, 1, 3, 0, 1, 0));
            if (!RendererMode::IsModern()) {
                glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(45, 1280.f/720, .1f, 100);
                glMatrixMode(GL_MODELVIEW); glLoadIdentity(); gluLookAt(12,10,14,3,1,3,0,1,0);
                // Unlit comparison isolates geometry, winding, colour and UVs.
                // No negative cells: the retained water pass emits no geometry.
                glDisable(GL_LIGHTING);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            }
            renderer.RenderTerrain(terrain);
            if (RendererMode::IsModern()) {
                const auto& first = resources.PrepareTerrain(terrain);
                const auto* gpu = &first[static_cast<std::size_t>(TerrainBatch::BlackSurface)].geometry.ModernGeometry();
                const auto& second = resources.PrepareTerrain(terrain);
                if (gpu != &second[static_cast<std::size_t>(TerrainBatch::BlackSurface)].geometry.ModernGeometry()) return 3;
                std::printf("fixture frame %d: cached GPU identity stable\n", frame);
            }
            glFinish();
            std::vector<unsigned char> pixels(1280*720*4), flipped(pixels.size());
            glReadPixels(0, 0, 1280, 720, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            for (int y = 0; y < 720; ++y)
                std::copy(pixels.begin()+y*1280*4, pixels.begin()+(y+1)*1280*4, flipped.begin()+(719-y)*1280*4);
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(flipped.data(), 1280, 720, 32, 1280*4,
                                                          0xff, 0xff00, 0xff0000, 0xff000000);
            char path[256];
            std::snprintf(path, sizeof(path), "../documentation/terrain-renderer-2026-10-09/fixture-%s-%d.bmp", RendererMode::Name(), frame);
            SDL_SaveBMP(surface, path); SDL_FreeSurface(surface);
            const unsigned error = glGetError();
            std::printf("fixture frame %d: GL error=%u\n", frame, error);
            if (error) status = 4;
        }
        renderer.Cleanup(); resources.Cleanup();
        TTF_CloseFont(graphics.defaultFont);
        app.graphicsTask = nullptr;
    }
    video.Stop();
    return status;
}
