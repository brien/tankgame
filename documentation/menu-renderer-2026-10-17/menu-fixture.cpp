#include "App.h"
#include "rendering/MenuRenderer.h"
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
    ResourceManager resources;
    if (!resources.Initialize()) return 2;
    MenuRenderer menu(&resources);
    menu.Initialize();
    const bool modern = RendererMode::IsModern();
    for (int players = 1; players <= 2; ++players) {
        for (int option = -1; option <= 3; ++option) {
            glClearColor(.1f,.1f,.1f,1);
            glDepthMask(GL_TRUE); glClearDepth(.25);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            std::vector<float> before(w*h), after(w*h);
            glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,before.data());
            std::vector<unsigned char> initial(w*h*4), pixels(w*h*4), flipped(w*h*4);
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
                glDisable(GL_CULL_FACE); glDisable(GL_LIGHTING); glDisable(GL_TEXTURE_2D);
                glEnable(GL_DEPTH_TEST); glDisable(GL_BLEND);
                glLineWidth(1);
                if (modern) {
                    glEnable(GL_CULL_FACE);
                    GLFunctions::BlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ZERO);
                    GLFunctions::BlendEquationSeparate(GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT);
                }
                MenuRenderData data;
                data.isVisible = option != -1; data.numOptions = 3; data.selectedOption = option;
                menu.RenderMenu(data);
                if (modern) {
                    GLboolean mask; GLint src, dst, eq;
                    glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);
                    glGetIntegerv(GL_BLEND_SRC_RGB,&src); glGetIntegerv(GL_BLEND_DST_ALPHA,&dst);
                    glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&eq);
                    if (!mask || !glIsEnabled(GL_DEPTH_TEST) || glIsEnabled(GL_BLEND) || !glIsEnabled(GL_CULL_FACE) ||
                        src != GL_SRC_ALPHA || dst != GL_ZERO || eq != GL_FUNC_REVERSE_SUBTRACT) return 3;
                    const Matrix4 actual=RenderContext::Current().Mvp(Matrix4::Identity()), expected=projection*view;
                    for(int r=0;r<4;++r) for(int c=0;c<4;++c) if(actual(r,c)!=expected(r,c)) return 4;
                }
            }
            glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,after.data());
            glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
            if (before != after || glGetError()) return 5;
            if ((option == -1 || option == 3) != (initial == pixels)) return 6;
            for (int y=0;y<h;++y)
                std::copy(pixels.begin()+y*w*4,pixels.begin()+(y+1)*w*4,flipped.begin()+(h-1-y)*w*4);
            auto* surface=SDL_CreateRGBSurfaceFrom(flipped.data(),w,h,32,w*4,0xff,0xff00,0xff0000,0xff000000);
            char file[256]; std::snprintf(file,sizeof(file),"../documentation/menu-renderer-2026-10-17/%s-%d-%d.bmp",modern?"modern":"compatibility",players,option);
            SDL_SaveBMP(surface,file); SDL_FreeSurface(surface);
            std::printf("players=%d option=%d depth unchanged; GL_NO_ERROR%s\n",players,option,modern?"; state/context restored":"");
        }
    }
    menu.Cleanup(); resources.Cleanup(); video.Stop();
}
