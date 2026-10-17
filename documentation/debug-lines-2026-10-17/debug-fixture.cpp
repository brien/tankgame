#include "App.h"
#include "rendering/DebugLineRenderer.h"
#include "rendering/RenderContext.h"
#include "rendering/RendererMode.h"
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
    App app; SoundTask sound; VideoTask video;
    app.soundTask = &sound; app.videoTask = &video;
    if (!video.Start()) return 1;
    SDL_SetWindowFullscreen(SDL_GL_GetCurrentWindow(), 0);
    SDL_SetWindowSize(SDL_GL_GetCurrentWindow(), 601, 501);
    SDL_PumpEvents(); video.RefreshDrawableSize();
    const int w = VideoTask::scrWidth, h = VideoTask::scrHeight;
    ResourceManager resources;
    if (!resources.Initialize()) return 2;
    const bool modern = RendererMode::IsModern();
    RenderContext::Current().SetProjection(Matrix4::Identity());
    RenderContext::Current().SetView(Matrix4::Identity());
    if (!modern) {
        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    }
    const std::vector<DebugLineData> lines = {
        {Vector3(-.8f, -.3f, -.5f), Vector3(.8f, .3f, -.5f), Vector3(1,0,0)},
        {Vector3(-.8f, .3f, .5f), Vector3(.8f, -.3f, .5f), Vector3(0,1,0)},
        {Vector3(0, -.8f, -.5f), Vector3(0, .8f, -.5f), Vector3(0,0,1)}};
    for (int players = 1; players <= 2; ++players) {
        glClearColor(0,0,0,1);
        glDepthMask(GL_TRUE);
        // A depth plane at .5 hides the green line and reveals red/blue.
        glClearDepth(.5);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        std::vector<float> before(w*h), after(w*h);
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,before.data());
        for (int i=0; i<players; ++i) {
            const int y = players == 1 ? 0 : i*h/2;
            const int height = players == 1 ? h : (i == 0 ? h/2 : h-h/2);
            glViewport(0,y,w,height);
            glDisable(GL_DEPTH_TEST); glDepthFunc(GL_GREATER);
            glEnable(GL_BLEND); glBlendFunc(GL_ONE,GL_ONE);
            DebugLineRenderer::Render(resources, lines);
            GLboolean mask; GLint func;
            glGetBooleanv(GL_DEPTH_WRITEMASK,&mask); glGetIntegerv(GL_DEPTH_FUNC,&func);
            if (!mask || func != GL_GREATER || glIsEnabled(GL_DEPTH_TEST) || !glIsEnabled(GL_BLEND)) return 3;
        }
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,after.data());
        if (before != after || glGetError()) return 4;
        std::vector<unsigned char> pixels(w*h*4), flipped(w*h*4);
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
        int red=0, green=0, blue=0;
        for (int y=0;y<h;++y) {
            for(int x=0;x<w;++x) {
                const auto n=(y*w+x)*4;
                red += pixels[n]>0; green += pixels[n+1]>0; blue += pixels[n+2]>0;
            }
            std::copy(pixels.begin()+y*w*4,pixels.begin()+(y+1)*w*4,flipped.begin()+(h-1-y)*w*4);
        }
        if (red<100 || blue<100 || green) return 5;
        auto* surface=SDL_CreateRGBSurfaceFrom(flipped.data(),w,h,32,w*4,0xff,0xff00,0xff0000,0xff000000);
        char file[256]; std::snprintf(file,sizeof(file),"../documentation/debug-lines-2026-10-17/%s-%d.bmp",modern?"modern":"compatibility",players);
        SDL_SaveBMP(surface,file); SDL_FreeSurface(surface);
        std::printf("players=%d red=%d blue=%d hiddenGreen=%d depth unchanged; state restored; GL_NO_ERROR\n",players,red,blue,green);
    }
    resources.Cleanup(); video.Stop();
}
