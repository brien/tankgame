#include "App.h"
#include "rendering/EnemyTankRendererImpl.h"
#include "rendering/RenderContext.h"
#include "rendering/RendererMode.h"
#include <SDL2/SDL.h>
#include <GL/glu.h>
#include <cstdio>
#include <algorithm>
#include <vector>

int main() {
    App app;
    SoundTask sound;
    VideoTask video;
    app.soundTask = &sound;
    if (!video.Start()) return 1;
    {
        ResourceManager resources;
        if (!resources.Initialize()) return 2;
        EnemyTankRendererImpl renderer(&resources);
        renderer.Initialize();
        for (int frame = 0; frame < 2; ++frame) {
            glViewport(0, 0, 1280, 720);
            glClearColor(0, 0, .2f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderContext::Current().SetProjection(Matrix4::Perspective(45, 1280.f/720, .1f, 100));
            RenderContext::Current().SetView(Matrix4::LookAt(0, 2, 4, 0, .1f, 0, 0, 1, 0));
            if (!RendererMode::IsModern()) {
                glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(45, 1280.f/720, .1f, 100);
                glMatrixMode(GL_MODELVIEW); glLoadIdentity(); gluLookAt(0, 2, 4, 0, .1f, 0, 0, 1, 0);
                GLfloat light[] = {.5f, 1, .5f, 0};
                GLfloat ambient[] = {.2f, .2f, .2f, 1};
                GLfloat diffuse[] = {.8f, .8f, .8f, 1};
                glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL); glEnable(GL_NORMALIZE);
                glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
                glLightfv(GL_LIGHT0, GL_POSITION, light); glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
            }
            renderer.SetupRenderState();
            for (int i = 0; i < 3; ++i) {
                TankRenderData tank;
                tank.position = Vector3((i-1)*1.2f + frame*.15f, 0, 0);
                tank.bodyRotation = Vector3(i*5.f, i*35.f, i*3.f);
                tank.turretRotation.y = i*70.f + frame*50.f;
                tank.primaryColor = Color(.2f, .05f, .1f);
                tank.secondaryColor = Color(.05f, .15f, .2f);
                renderer.Render(tank);
            }
            renderer.CleanupRenderState();
            glFinish();
            std::vector<unsigned char> pixels(1280*720*4), flipped(pixels.size());
            glReadPixels(0, 0, 1280, 720, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            for (int y = 0; y < 720; ++y)
                std::copy(pixels.begin()+y*1280*4, pixels.begin()+(y+1)*1280*4,
                          flipped.begin()+(719-y)*1280*4);
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(flipped.data(), 1280, 720, 32, 1280*4,
                                                          0xff, 0xff00, 0xff0000, 0xff000000);
            char path[200];
            std::snprintf(path, sizeof(path), "../work/enemy-fixture-%s-%d.bmp", RendererMode::Name(), frame);
            SDL_SaveBMP(surface, path); SDL_FreeSurface(surface);
            std::printf("fixture frame %d: GL error=%u\n", frame, glGetError());
        }
        renderer.Cleanup(); resources.Cleanup();
    }
    video.Stop();
    return 0;
}
