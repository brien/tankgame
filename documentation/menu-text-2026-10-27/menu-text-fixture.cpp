#include <GL/glu.h>
#include "App.h"
#include "rendering/RenderingPipeline.h"
#include "rendering/RenderContext.h"
#include "rendering/RendererMode.h"
#include "rendering/HUDDataExtractor.h"
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
    GraphicsTask graphics; app.graphicsTask=&graphics;
    graphics.resourceManager.reset(new ResourceManager);
    auto& resources=graphics.Resources();
    if (!resources.Initialize()) return 2;
    ViewportManager views; CameraManager cameras;
    RenderingPipeline pipeline(views,cameras,resources);
    if (!pipeline.Initialize()) return 3;
    MenuRenderer menu(&resources);
    for (int players=1;players<=2;++players) for (int option=0;option<3;++option) {
        pipeline.ConfigureViewports(players,w,h);
        SceneData scene;
        scene.numPlayers=players;
        scene.terrain.sizeX=scene.terrain.sizeZ=2;
        scene.terrain.drawFloor=scene.terrain.drawWalls=scene.terrain.drawTop=false;
        scene.cameras.resize(players);
        for (auto& camera:scene.cameras) { camera.position=Vector3(0,3,5); camera.focus=Vector3(0,0,0); }
        auto ui=std::make_shared<UIRenderData>();
        ui->menu=HUDDataExtractor::ExtractMenuData(option,true);
        scene.uiData=ui;
        std::vector<unsigned char> actual(w*h*4),expected(w*h*4),flipped(w*h*4);
        std::vector<float> depth(w*h),referenceDepth(w*h);
        glDisable(GL_SCISSOR_TEST); glDepthMask(GL_TRUE);
        glClearColor(.1,.15,.2,1);
        pipeline.RenderAllPlayerViews(scene);
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,actual.data());
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,depth.data());
        GLint viewport[4]; glGetIntegerv(GL_VIEWPORT,viewport);
        const auto& last=views.GetViewport(players-1);
        if(viewport[0]!=last.x || viewport[1]!=last.y || viewport[2]!=last.width || viewport[3]!=last.height) return 4;
        ui->menu.isVisible=false;
        pipeline.RenderAllPlayerViews(scene);
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,referenceDepth.data());
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,expected.data());
        if(actual==expected) return 7; // Menu must visibly change the frame.
        glViewport(0,0,w,h);
        if(RendererMode::IsModern()) RenderContext::Current().SetProjection(Matrix4::Perspective(45,float(w)/h,.1,1000));
        else { glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(45,double(w)/h,.1,1000); glMatrixMode(GL_MODELVIEW); }
        ui->menu.isVisible=true;
        menu.RenderMenu(ui->menu);
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,expected.data());
        if(actual!=expected || depth!=referenceDepth || glGetError()) return 5;
        if (RendererMode::IsModern()) {
            int glyphPixels=0;
            for(int y=h-70;y<h-12;++y) for(int x=12;x<200;++x)
                glyphPixels+=actual[(y*w+x)*4+1]>100;
            if(glyphPixels<100) return 8;
        }
        int visible=0; for(int i=0;i<w*h;++i) visible+=actual[i*4+1]>80;
        if(visible<100) return 6;
        for(int y=0;y<h;++y) std::copy(actual.begin()+y*w*4,actual.begin()+(y+1)*w*4,flipped.begin()+(h-1-y)*w*4);
        auto* surface=SDL_CreateRGBSurfaceFrom(flipped.data(),w,h,32,w*4,0xff,0xff00,0xff0000,0xff000000);
        char path[256]; std::snprintf(path,sizeof(path),"../documentation/menu-text-2026-10-27/%s-%d-%d.bmp",RendererMode::IsModern()?"modern":"compatibility",players,option);
        SDL_SaveBMP(surface,path); SDL_FreeSurface(surface);
        std::printf("players=%d option=%d: full-drawable reference matches, depth unchanged, viewport restored, GL_NO_ERROR\n",players,option);
    }
    pipeline.Cleanup(); resources.Cleanup(); video.Stop();
}
