// Deterministic opaque terrain comparison using both production renderers.
#include "App.h"
#include "rendering/TerrainRenderer.h"
#include "rendering/EnemyTankRendererImpl.h"
#include "rendering/PlayerTankRenderer.h"
#include "rendering/ItemRenderer.h"
#include "rendering/BulletRenderer.h"
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
        EnemyTankRendererImpl enemies(&resources); enemies.Initialize();
        ItemRenderer items; items.Initialize();
        BulletRenderer bullets; bullets.Initialize();
        for (int frame = 0; frame < 6; ++frame) {
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
            RenderContext::Current().SetView(Matrix4::LookAt(frame < 3 ? 12 : 8, frame < 3 ? 10 : 7, frame < 3 ? 14 : 10, 3, frame < 3 ? 1 : 3, 3, 0, 1, 0));
            RenderContext::Current().SetLighting(true);
            if (!RendererMode::IsModern()) {
                glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(45, 1280.f/720, .1f, 100);
                glMatrixMode(GL_MODELVIEW); glLoadIdentity(); gluLookAt(frame < 3 ? 12 : 8, frame < 3 ? 10 : 7, frame < 3 ? 14 : 10, 3, frame < 3 ? 1 : 3, 3, 0,1,0);
                // Match production directional lighting in both renderers.
                // No negative cells: the retained water pass emits no geometry.
                GLfloat light[] = {.5f,1,.5f,0}, ambient[] = {.2f,.2f,.2f,1}, diffuse[] = {.8f,.8f,.8f,1};
                glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_NORMALIZE);
                glEnable(GL_COLOR_MATERIAL); glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
                glLightfv(GL_LIGHT0, GL_POSITION, light); glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            }
            const auto check = [&](const char* pass) {
                const unsigned error = glGetError();
                std::printf("frame %d %s GL error=%u\n", frame, pass, error);
                if (error) status = 4;
            };
            if (frame < 3) renderer.RenderTerrain(terrain);
            check("terrain");
            TankRenderData tank;
            tank.position = Vector3(4, 3, 5);
            tank.bodyRotation.y = frame * 35;
            tank.turretRotation.y = frame * 65;
            tank.primaryColor = Color(.2f,.1f,.3f);
            tank.secondaryColor = Color(.1f,.3f,.2f);
            enemies.SetupRenderState(); enemies.Render(tank); enemies.CleanupRenderState(); check("enemy");
            tank.position = Vector3(5, 3, 3); tank.isPlayer = true;
            PlayerTankRenderer::RenderPlayerTank(tank, 0); check("player");
            ItemRenderData item;
            item.position = Vector3(2, 3, 3); item.visible = true; item.rotationY = frame*40;
            items.RenderItems({item}); check("item");
            BulletRenderData bullet;
            bullet.position = Vector3(3, 3, 5); bullet.power = 100;
            bullet.primaryColor = Color(1,.2f,.1f); bullet.secondaryColor = Color(.2f,1,.1f);
            bullets.RenderBullets({bullet}); check("bullet");
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
            std::snprintf(path, sizeof(path), "../documentation/opaque-lighting-2026-10-13/fixture-%s-%d.bmp", RendererMode::Name(), frame);
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
