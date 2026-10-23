#include "App.h"
#include "rendering/HUDMeterRenderer.h"
#include "rendering/HUDRenderer.h"
#include "rendering/HUDLayout.h"
#include "rendering/RenderContext.h"
#include "rendering/RendererMode.h"
#include "rendering/GLFunctions.h"
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
    App app; SoundTask sound; VideoTask video;
    app.soundTask = &sound; app.videoTask = &video;
    if (!video.Start()) return 1;
    SDL_SetWindowFullscreen(SDL_GL_GetCurrentWindow(), 0);
    SDL_SetWindowSize(SDL_GL_GetCurrentWindow(), 801, 601);
    SDL_PumpEvents(); video.RefreshDrawableSize();
    const int w = VideoTask::scrWidth, h = VideoTask::scrHeight;
    GraphicsTask graphics; app.graphicsTask = &graphics;
    graphics.resourceManager.reset(new ResourceManager);
    auto& resources = graphics.Resources();
    if (!resources.Initialize()) return 2;
    HUDRenderer reference(resources);
    if (!reference.Initialize()) return 3;
    const bool modern = RendererMode::IsModern();
    for (int players = 1; players <= 2; ++players) {
        for (int scenario = 0; scenario < 5; ++scenario) {
            glClearColor(.1f,.15f,.2f,1);
            glDepthMask(GL_TRUE); glClearDepth(.25);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            std::vector<float> before(w*h), after(w*h);
            std::vector<unsigned char> initial(w*h*4), pixels(w*h*4), flipped(w*h*4);
            glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,before.data());
            glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,initial.data());
            for (int i=0; i<players; ++i) {
                const int y = players == 1 ? 0 : i*h/2;
                const int height = players == 1 ? h : (i == 0 ? h/2 : h-h/2);
                glViewport(0,y,w,height);
                const Matrix4 projection = Matrix4::Perspective(60, float(w)/height, .1f, 100);
                const Matrix4 view = Matrix4::Translation(2,3,-4);
                RenderContext::Current().SetProjection(projection);
                RenderContext::Current().SetView(view);
                glMatrixMode(GL_PROJECTION); glLoadMatrixf(projection.Data());
                glMatrixMode(GL_MODELVIEW); glLoadMatrixf(view.Data());
                glDisable(GL_LIGHTING); glDisable(GL_TEXTURE_2D); glDisable(GL_LINE_STIPPLE);
                glEnable(GL_CULL_FACE); glEnable(GL_DEPTH_TEST); glDisable(GL_BLEND);
                glLineWidth(1);
                GLFunctions::BlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                GLuint texture = resources.GetTextureHandler().GetTexture(TEXTURE_CHECKER)->CompatibilityHandle();
                if (modern) {
                    GLFunctions::BlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ZERO);
                    GLFunctions::BlendEquationSeparate(GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT);
                    GLFunctions::ActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texture);
                    GLFunctions::ActiveTexture(GL_TEXTURE1);
                }
                HUDRenderData data;
                data.playerId = i; data.showTargeting = false; data.showFPSCounter = false;
                data.health = i ? 30 : 75; data.energy = i ? 25 : 60;
                data.combo = i ? 20 : 55; data.special = i ? 30 : 65;
                data.fireCost = 50; data.jumpCost = i ? 50 : 70;
                data.canFire = !i; data.hasSpecialAvailable = !i;
                data.showDebugInfo = true;
                if (scenario == 1) { data.health = 150; data.energy = -30; data.hasSpecialAvailable = false; }
                if (scenario == 2) { data.maxHealth = data.maxEnergy = 0; data.fireCost = 0; }
                if (scenario == 3) { data.showHealthBar = data.showEnergyBar = false; data.special = 0; }
                if (scenario == 4) {
                    data.showHealthBar = data.showEnergyBar = data.showComboMeter = data.showSpecialMeter = data.showDebugInfo = false;
                }
                if (modern) HUDMeterRenderer::Render(resources, data);
                else reference.RenderPlayerHUD(data);
                if (modern) {
                    GLboolean mask; GLint src, dst, eq, unit, binding;
                    glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);
                    glGetIntegerv(GL_BLEND_SRC_RGB,&src); glGetIntegerv(GL_BLEND_DST_ALPHA,&dst);
                    glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&eq); glGetIntegerv(GL_ACTIVE_TEXTURE,&unit);
                    GLFunctions::ActiveTexture(GL_TEXTURE0); glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);
                    if (!mask || !glIsEnabled(GL_DEPTH_TEST) || glIsEnabled(GL_BLEND) || !glIsEnabled(GL_CULL_FACE) ||
                        src != GL_SRC_ALPHA || dst != GL_ZERO || eq != GL_FUNC_REVERSE_SUBTRACT ||
                        unit != GL_TEXTURE1 || binding != static_cast<GLint>(texture)) return 4;
                    const Matrix4 actual=RenderContext::Current().Mvp(Matrix4::Identity()), expected=projection*view;
                    for(int r=0;r<4;++r) for(int c=0;c<4;++c) if(actual(r,c)!=expected(r,c)) return 5;
                }
            }
            glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,after.data());
            glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
            if (before != after || glGetError()) return 6;
            if ((scenario == 4) != (initial == pixels)) return 7;
            for (int y=0;y<h;++y)
                std::copy(pixels.begin()+y*w*4,pixels.begin()+(y+1)*w*4,flipped.begin()+(h-1-y)*w*4);
            auto* surface=SDL_CreateRGBSurfaceFrom(flipped.data(),w,h,32,w*4,0xff,0xff00,0xff0000,0xff000000);
            char file[256]; std::snprintf(file,sizeof(file),"../documentation/hud-meters-2026-10-19/%s-%d-%d.bmp",modern?"modern":"compatibility",players,scenario);
            SDL_SaveBMP(surface,file); SDL_FreeSurface(surface);
            std::printf("players=%d scenario=%d depth unchanged; GL_NO_ERROR%s\n",players,scenario,modern?"; state/context restored":"");
        }
    }
    reference.Cleanup(); resources.Cleanup(); video.Stop();
}
