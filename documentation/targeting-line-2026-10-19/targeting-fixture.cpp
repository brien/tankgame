#include "App.h"
#include "rendering/TargetingLineRenderer.h"
#include "rendering/RenderContext.h"
#include "rendering/GLFunctions.h"
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
    App app; SoundTask sound; VideoTask video;
    app.soundTask=&sound; app.videoTask=&video;
    if (!video.Start()) return 1;
    SDL_SetWindowFullscreen(SDL_GL_GetCurrentWindow(),0);
    SDL_SetWindowSize(SDL_GL_GetCurrentWindow(),801,601);
    SDL_PumpEvents(); video.RefreshDrawableSize();
    const int w=VideoTask::scrWidth,h=VideoTask::scrHeight;
    ResourceManager resources;
    if (!resources.Initialize()) return 2;
    for(int players=1;players<=2;++players) for(int scenario=0;scenario<5;++scenario) {
        glDepthMask(GL_TRUE); glClearDepth(scenario==3 ? 0 : 1);
        glClearColor(.1,.15,.2,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        std::vector<float> before(w*h),after(w*h);
        std::vector<unsigned char> initial(w*h*4),pixels(w*h*4),flipped(w*h*4);
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,before.data());
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,initial.data());
        for(int i=0;i<players;++i) {
            const int y=players==1?0:i*h/2, height=players==1?h:(i?h-h/2:h/2);
            glViewport(0,y,w,height);
            RenderContext::Current().SetProjection(Matrix4::Perspective(60,float(w)/height,.1,100));
            RenderContext::Current().SetView(Matrix4::Translation(-8,0,-20));
            HUDRenderData data; data.playerRotation.y=i?20:0;
            data.targetingAnimationTime=scenario==1?0x5555:65535;
            data.targetingColor=Vector3(1,.3,.1);
            if(scenario==2) data.showTargeting=false;
            if(scenario==4) data.playerPosition=Vector3(-30,0,0);
            glDisable(GL_DEPTH_TEST); glEnable(GL_BLEND); glDepthFunc(GL_GREATER);
            glLineWidth(2);
            TargetingLineRenderer::Render(resources,data);
            GLboolean write; GLint func; GLfloat width;
            glGetBooleanv(GL_DEPTH_WRITEMASK,&write); glGetIntegerv(GL_DEPTH_FUNC,&func); glGetFloatv(GL_LINE_WIDTH,&width);
            if(!write || glIsEnabled(GL_DEPTH_TEST) || !glIsEnabled(GL_BLEND) || func!=GL_GREATER || width!=2) return 3;
        }
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,after.data());
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
        if(before!=after || glGetError()) return 4;
        if((scenario==2 || scenario==3)!=(initial==pixels)) return 5;
        for(int y=0;y<h;++y) std::copy(pixels.begin()+y*w*4,pixels.begin()+(y+1)*w*4,flipped.begin()+(h-1-y)*w*4);
        auto* surface=SDL_CreateRGBSurfaceFrom(flipped.data(),w,h,32,w*4,0xff,0xff00,0xff0000,0xff000000);
        char file[256]; std::snprintf(file,sizeof(file),"../documentation/targeting-line-2026-10-19/modern-%d-%d.bmp",players,scenario);
        SDL_SaveBMP(surface,file); SDL_FreeSurface(surface);
        std::printf("players=%d scenario=%d: visibility, occlusion, depth and state pass; GL_NO_ERROR\n",players,scenario);
    }
    resources.Cleanup(); video.Stop();
}
