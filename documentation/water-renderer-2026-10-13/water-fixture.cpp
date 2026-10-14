// Deterministic water comparison using both production renderers.
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
        for (int frame = 0; frame < 6; ++frame) {
            TerrainRenderData terrain;
            terrain.sizeX = terrain.sizeZ = 8;
            terrain.levelNumber = frame == 0 ? 48 : 50;
            terrain.colors.defaultColor = Vector3(.7f,.5f,.3f);
            terrain.colors.blockColor = Vector3(.2f,.6f,.7f);
            terrain.heightMap[3][3] = -3;
            if (frame >= 2) terrain.heightMap[3][4] = -1;
            if (frame >= 4) terrain.heightMap[4][3] = 2;
            glViewport(0, 0, 1280, 720);
            glClearColor(0, 0, .2f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderContext::Current().SetProjection(Matrix4::Perspective(45, 1280.f/720, .1f, 100));
            RenderContext::Current().SetView(Matrix4::LookAt(frame % 2 ? 0 : 10, 7, frame % 2 ? 0 : 10, 3, 1, 3, 0, 1, 0));
            RenderContext::Current().SetLighting(true);
            if (!RendererMode::IsModern()) {
                glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(45, 1280.f/720, .1f, 100);
                glMatrixMode(GL_MODELVIEW); glLoadIdentity(); gluLookAt(frame % 2 ? 0 : 10, 7, frame % 2 ? 0 : 10, 3, 1, 3, 0,1,0);
                // Match production directional lighting in both renderers.
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
            renderer.RenderTerrain(terrain);
            check("terrain");
            if (RendererMode::IsModern()) {
                std::vector<float> before(1280*720), after(before.size());
                glReadPixels(0, 0, 1280, 720, GL_DEPTH_COMPONENT, GL_FLOAT, before.data());
                renderer.RenderWater(terrain);
                glReadPixels(0, 0, 1280, 720, GL_DEPTH_COMPONENT, GL_FLOAT, after.data());
                if (before != after) return 5;
                GLboolean depthWrite = GL_FALSE;
                GLint source = 0, destination = 0;
                glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
                glGetIntegerv(GL_BLEND_SRC, &source);
                glGetIntegerv(GL_BLEND_DST, &destination);
                if (!depthWrite || glIsEnabled(GL_BLEND) || !glIsEnabled(GL_CULL_FACE) ||
                    !glIsEnabled(GL_DEPTH_TEST) || glIsEnabled(GL_POLYGON_OFFSET_FILL) || source != GL_SRC_ALPHA ||
                    destination != GL_ONE_MINUS_SRC_ALPHA) return 6;
                std::printf("frame %d: water preserves depth and restores pass state\n", frame);
            }
            check("water");
            if (RendererMode::IsModern()) {
                const auto& first = resources.PrepareTerrain(terrain);
                const auto* gpu = &first[static_cast<std::size_t>(TerrainBatch::Water)].geometry.ModernGeometry();
                const auto& second = resources.PrepareTerrain(terrain);
                if (gpu != &second[static_cast<std::size_t>(TerrainBatch::Water)].geometry.ModernGeometry()) return 3;
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
            std::snprintf(path, sizeof(path), "../documentation/water-renderer-2026-10-13/fixture-%s-%d.bmp", RendererMode::Name(), frame);
            SDL_SaveBMP(surface, path); SDL_FreeSurface(surface);
            const unsigned error = glGetError();
            std::printf("fixture frame %d: GL error=%u\n", frame, error);
            if (error) status = 4;
        }
        if (RendererMode::IsModern()) {
            // Two views share one clear. A near opaque cube must fully occlude
            // water in the left view; a far cube must receive visible additive
            // water in the right view. Neither pass may touch the other view.
            TerrainRenderData terrain;
            terrain.sizeX = terrain.sizeZ = 8;
            terrain.heightMap[3][3] = -3;
            terrain.colors.defaultColor = Vector3(.7f,.5f,.3f);
            glDepthMask(GL_TRUE);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            for (int view = 0; view < 2; ++view) {
                glViewport(view*640, 0, 640, 720);
                auto& context = RenderContext::Current();
                context.SetProjection(Matrix4::Perspective(45, 640.f/720, .1f, 100));
                context.SetView(Matrix4::LookAt(3.5f,1,-5,3.5f,1,3,0,1,0));
                glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE); glDisable(GL_BLEND);
                glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
                context.Draw(resources.GetGeometry(GeometryResource::TerrainCube),
                    Matrix4::Translation(3.5f,1,view == 0 ? 2 : 5) * Matrix4::Scale(4,4,.2f),
                    .1f,.15f,.2f);
                std::vector<unsigned char> before(1280*720*4), after(before.size());
                glReadPixels(0,0,1280,720,GL_RGBA,GL_UNSIGNED_BYTE,before.data());
                renderer.RenderWater(terrain);
                glReadPixels(0,0,1280,720,GL_RGBA,GL_UNSIGNED_BYTE,after.data());
                int changed = 0;
                for (int y = 0; y < 720; ++y) {
                    for (int x = 0; x < 1280; ++x) {
                        const int i = (y*1280+x)*4;
                        const bool differs = !std::equal(before.begin()+i,before.begin()+i+3,after.begin()+i);
                        const bool active = x/640 == view;
                        if (differs && !active) return 7;
                        if (differs && active) ++changed;
                    }
                }
                if ((view == 0 && changed != 0) || (view == 1 && changed < 100)) return 8;
                std::printf("occlusion view %d: changed pixels=%d, other view preserved\n",view,changed);
            }
            if (glGetError()) return 9;
        }
        renderer.Cleanup(); resources.Cleanup();
        TTF_CloseFont(graphics.defaultFont);
        app.graphicsTask = nullptr;
    }
    video.Stop();
    return status;
}
