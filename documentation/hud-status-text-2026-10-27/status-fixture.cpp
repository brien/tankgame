#include "App.h"
#include "rendering/TextRenderer.h"
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
    const auto* texture=resources.GetFontTexture().get();
    for(int players=1;players<=2;++players) {
        glDepthMask(GL_TRUE); glClearDepth(.5);
        glClearColor(.1,.15,.2,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        std::vector<float> before(w*h),after(w*h);
        std::vector<unsigned char> initial(w*h*4),pixels(w*h*4),flipped(w*h*4);
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,before.data());
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,initial.data());
        for(int i=0;i<players;++i) {
            const int y=players==1?0:i*h/2, height=players==1?h:(i?h-h/2:h/2);
            glViewport(0,y,w,height);
            HUDRenderData data; data.currentFPS=i?30:60; data.health=i?37:89; data.energy=i?12:75; data.comboNumber=i?7:2; data.special=i?125:40;
            glEnable(GL_DEPTH_TEST); glDisable(GL_BLEND); glEnable(GL_CULL_FACE);
            GLFunctions::BlendFuncSeparate(GL_ONE,GL_ZERO,GL_ZERO,GL_ONE);
            TextRenderer::Render(resources,TextRenderer::PlayerLines(data,w));
            TextRenderer::Render(resources,TextRenderer::StatusLines(data,w,height));
            GLint box[4]; glGetIntegerv(GL_SCISSOR_BOX,box);
            glEnable(GL_SCISSOR_TEST); glScissor(0,y,0,height);
            std::vector<unsigned char> clipped(w*h*4),unclipped(w*h*4);
            glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,clipped.data());
            TextRenderer::Render(resources,TextRenderer::StatusLines(data,w,height));
            glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,unclipped.data());
            GLint restored[4]; glGetIntegerv(GL_SCISSOR_BOX,restored);
            if(clipped!=unclipped || !glIsEnabled(GL_SCISSOR_TEST) || restored[2]!=0 || restored[1]!=y) return 9;
            glDisable(GL_SCISSOR_TEST); glScissor(box[0],box[1],box[2],box[3]);
            GLboolean write; GLint src,dst;
            glGetBooleanv(GL_DEPTH_WRITEMASK,&write);
            glGetIntegerv(GL_BLEND_SRC_ALPHA,&src); glGetIntegerv(GL_BLEND_DST_ALPHA,&dst);
            if(!write || !glIsEnabled(GL_DEPTH_TEST) || glIsEnabled(GL_BLEND) || !glIsEnabled(GL_CULL_FACE) || src!=GL_ZERO || dst!=GL_ONE) return 3;
        }
        glViewport(0,0,w,h);
        DebugRenderData debug; debug.showDebugInfo=debug.showPerformanceStats=debug.showRenderStats=true;
        TextRenderer::Render(resources,TextRenderer::DebugLines(debug));
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,after.data());
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
        if(before!=after || glGetError() || initial==pixels || texture!=resources.GetFontTexture().get()) return 4;
        // Both local FPS regions must contain text, including the lower split.
        for(int i=0;i<players;++i) {
            const int top=players==1?h:(i?h:h/2);
            int changed=0;
            for(int y=top-35;y<top-10;++y) for(int x=w-160;x<w;++x)
                changed += pixels[(y*w+x)*4]!=initial[(y*w+x)*4];
            if(changed<20) return 5;
            const int bottom=players==1?0:(i?h/2:0);
            changed=0;
            for(int y=bottom+12;y<bottom+96;++y) for(int x=12;x<250;++x)
                changed += pixels[(y*w+x)*4]!=initial[(y*w+x)*4];
            if(changed<100) return 10;
        }
        for(int y=0;y<h;++y) std::copy(pixels.begin()+y*w*4,pixels.begin()+(y+1)*w*4,flipped.begin()+(h-1-y)*w*4);
        auto* surface=SDL_CreateRGBSurfaceFrom(flipped.data(),w,h,32,w*4,0xff,0xff00,0xff0000,0xff000000);
        char file[256]; std::snprintf(file,sizeof(file),"../documentation/hud-status-text-2026-10-27/modern-%d.bmp",players);
        SDL_SaveBMP(surface,file); SDL_FreeSurface(surface);
        std::printf("players=%d: visible text, stable atlas, unchanged depth, restored state; GL_NO_ERROR\n",players);
    }
    // Viewport transforms alone allow negative local Y to cross a split seam.
    glViewport(0,0,w,h/2);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    std::vector<unsigned char> seamBefore(w*h*4), seamAfter(w*h*4);
    glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,seamBefore.data());
    TextRenderer::Render(resources,{{"Must stay in lower view",12,-9,Vector3(1,1,1)}});
    glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,seamAfter.data());
    if (!std::equal(seamBefore.begin()+(h/2)*w*4,seamBefore.end(),seamAfter.begin()+(h/2)*w*4)) return 11;
    if (seamBefore==seamAfter || glIsEnabled(GL_SCISSOR_TEST) || glGetError()) return 12;
    std::printf("split seam clipped; scissor restored; GL_NO_ERROR\n");
    resources.Cleanup();
    if(resources.GetFontTexture()) return 6;
    if(!resources.Initialize()) return 7;
    TextRenderer::Render(resources,{{"Recreated atlas",12,12,Vector3(1,1,1)}});
    if(glGetError()) return 8;
    resources.Cleanup(); video.Stop();
}
