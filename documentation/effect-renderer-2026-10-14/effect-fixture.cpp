// Deterministic effect comparison using both production renderers.
#include "App.h"
#include "rendering/EffectRenderer.h"
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
        EffectRenderer renderer(&resources);
        renderer.Initialize();
        for (int frame = 0; frame < 6; ++frame) {
            std::vector<EffectRenderData> effects;
            for (int i = 0; i < 8; ++i) {
                EffectRenderData effect;
                effect.type = static_cast<FxType>(i);
                effect.position = Vector3((i % 4)*1.5f, 0, (i / 4)*2.f);
                effect.rotation = Vector3(frame >= 4 ? 40 : 0, frame*15.f, frame >= 4 ? 25 : 0);
                effect.scale = .6f;
                effect.r = .3f; effect.g = .6f; effect.b = .8f;
                effect.alpha = .15f + frame*.15f;
                effects.push_back(effect);
            }
            glViewport(0, 0, 1280, 720);
            glClearColor(0, 0, .2f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderContext::Current().SetProjection(Matrix4::Perspective(45, 1280.f/720, .1f, 100));
            RenderContext::Current().SetView(Matrix4::LookAt(frame % 2 ? 0 : 7, 7, frame % 2 ? 0 : 7, 2, 0, 1, 0, 1, 0));
            RenderContext::Current().SetLighting(true);
            if (!RendererMode::IsModern()) {
                glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(45, 1280.f/720, .1f, 100);
                glMatrixMode(GL_MODELVIEW); glLoadIdentity(); gluLookAt(frame % 2 ? 0 : 7, 7, frame % 2 ? 0 : 7, 2, 0, 1, 0,1,0);
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
            renderer.RenderEffects(effects);
            check("effects");
            if (RendererMode::IsModern()) {
                GLboolean depthWrite = GL_FALSE;
                GLint source = 0, destination = 0;
                glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
                glGetIntegerv(GL_BLEND_SRC, &source);
                glGetIntegerv(GL_BLEND_DST, &destination);
                if (!depthWrite || glIsEnabled(GL_BLEND) || !glIsEnabled(GL_CULL_FACE) ||
                    !glIsEnabled(GL_DEPTH_TEST) || source != GL_SRC_ALPHA ||
                    destination != GL_ONE_MINUS_SRC_ALPHA) return 6;
                // Exclude the intentionally depth-writing outlines here.
                const auto original = effects;
                effects.erase(effects.begin()+6);
                effects.erase(effects.begin());
                std::vector<float> before(1280*720), after(before.size());
                glReadPixels(0,0,1280,720,GL_DEPTH_COMPONENT,GL_FLOAT,before.data());
                renderer.RenderEffects(effects);
                glReadPixels(0,0,1280,720,GL_DEPTH_COMPONENT,GL_FLOAT,after.data());
                if (before != after) return 5;
                // Restore a single draw for the comparison capture.
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                effects = original;
                renderer.RenderEffects(effects);
                std::printf("frame %d: additive quads preserve depth, outgoing state valid\n", frame);
            }
            glFinish();
            std::vector<unsigned char> pixels(1280*720*4), flipped(pixels.size());
            glReadPixels(0, 0, 1280, 720, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            for (int y = 0; y < 720; ++y)
                std::copy(pixels.begin()+y*1280*4, pixels.begin()+(y+1)*1280*4, flipped.begin()+(719-y)*1280*4);
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(flipped.data(), 1280, 720, 32, 1280*4,
                                                          0xff, 0xff00, 0xff0000, 0xff000000);
            char path[256];
            std::snprintf(path, sizeof(path), "../documentation/effect-renderer-2026-10-14/fixture-%s-%d.bmp", RendererMode::Name(), frame);
            SDL_SaveBMP(surface, path); SDL_FreeSurface(surface);
            const unsigned error = glGetError();
            std::printf("fixture frame %d: GL error=%u\n", frame, error);
            if (error) status = 4;
        }
        if (RendererMode::IsModern()) {
            // Two views share one clear. A near opaque cube must fully occlude
            // effects in the left view; a far cube must receive visible additive
            // effects in the right view. Neither pass may touch the other view.
            EffectRenderData effect;
            effect.type = FxType::TYPE_THREE;
            effect.position = Vector3(3.5f,.8f,3);
            effect.rotation.x = 90;
            effect.alpha = .5f;
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
                renderer.RenderEffects({effect});
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
