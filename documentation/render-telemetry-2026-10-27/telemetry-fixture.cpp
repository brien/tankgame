#include "App.h"
#include "rendering/RenderingPipeline.h"
#include "rendering/RenderContext.h"
#include "rendering/TextRenderer.h"
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
    if (!resources.Initialize() || !resources.GetModernRenderer()) return 2;
    ViewportManager views; CameraManager cameras;
    RenderingPipeline pipeline(views,cameras,resources);
    if (!pipeline.Initialize()) return 3;
    ModernRenderer::SubmissionStats single;
    for (int players=1;players<=2;++players) {
        pipeline.ConfigureViewports(players,w,h);
        SceneData scene;
        scene.numPlayers=players;
        scene.terrain.sizeX=scene.terrain.sizeZ=2;
        scene.terrain.drawFloor=scene.terrain.drawWalls=scene.terrain.drawTop=false;
        scene.cameras.resize(players);
        for (auto& camera:scene.cameras) { camera.position=Vector3(0,3,5); camera.focus=Vector3(0,0,0); }
        scene.debugMode=true;
        scene.debugLines.push_back({Vector3(0,0,0),Vector3(1,0,0),Vector3(1,0,0)});
        auto ui=std::make_shared<UIRenderData>();
        ui->menu.isVisible=false;
        ui->debug.showPerformanceStats=false;
        ui->debug.showRenderStats=true;
        ui->debug.renderCallCount=99999; // Immutable input must never be changed or displayed.
        scene.uiData=ui;
        glDisable(GL_SCISSOR_TEST); glDepthMask(GL_TRUE);
        glClearColor(.1,.15,.2,1);
        pipeline.RenderAllPlayerViews(scene);
        const auto baseline=pipeline.GetRenderStats().submissions;
        if(players==1) single=baseline;
        if(baseline.drawCalls!=single.drawCalls*players || baseline.triangles!=single.triangles*players) return 11;
        for (int i=0;i<2;++i) {
            pipeline.RenderAllPlayerViews(scene);
            const auto stats=pipeline.GetRenderStats().submissions;
            if(stats.drawCalls!=baseline.drawCalls || stats.triangles!=baseline.triangles) return 4;
        }
        ui->debug.showDebugInfo=true;
        pipeline.RenderAllPlayerViews(scene);
        const auto actualStats=pipeline.GetRenderStats().submissions;
        std::vector<unsigned char> actual(w*h*4),expected(w*h*4),flipped(w*h*4);
        std::vector<float> depth(w*h),referenceDepth(w*h);
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,actual.data());
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,depth.data());
        if(ui->debug.renderCallCount!=99999) return 5;
        ui->debug.showDebugInfo=false;
        pipeline.RenderAllPlayerViews(scene);
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,referenceDepth.data());
        glViewport(0,0,w,h);
        auto reference=ui->debug;
        reference.showDebugInfo=true;
        reference.renderCallCount=baseline.drawCalls;
        reference.triangleCount=baseline.triangles;
        const auto before=resources.GetModernRenderer()->GetSubmissionStats();
        TextRenderer::Render(resources,TextRenderer::DebugLines(reference));
        const auto after=resources.GetModernRenderer()->GetSubmissionStats();
        if(actualStats.drawCalls!=after.drawCalls-before.drawCalls+baseline.drawCalls ||
           actualStats.triangles!=after.triangles-before.triangles+baseline.triangles || !actualStats.triangles) return 6;
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,expected.data());
        if(actual!=expected || depth!=referenceDepth || glGetError()) return 7;
        // Out-of-frame draws must not leak into the next frame's totals.
        pipeline.RenderAllPlayerViews(scene);
        if(pipeline.GetRenderStats().submissions.drawCalls!=baseline.drawCalls) return 8;
        for(int y=0;y<h;++y) std::copy(actual.begin()+y*w*4,actual.begin()+(y+1)*w*4,flipped.begin()+(h-1-y)*w*4);
        auto* surface=SDL_CreateRGBSurfaceFrom(flipped.data(),w,h,32,w*4,0xff,0xff00,0xff0000,0xff000000);
        char path[256]; std::snprintf(path,sizeof(path),"../documentation/render-telemetry-2026-10-27/modern-%d.bmp",players);
        SDL_SaveBMP(surface,path); SDL_FreeSurface(surface);
        std::printf("players=%d: frame totals, global text reference, immutable DTO, depth and GL checks passed\n",players);
    }
    pipeline.ConfigureViewports(1,0,0);
    SceneData empty;
    pipeline.RenderAllPlayerViews(empty);
    if(pipeline.GetRenderStats().submissions.drawCalls || pipeline.GetRenderStats().submissions.triangles) return 9;
    pipeline.Cleanup();
    if(pipeline.GetRenderStats().submissions.drawCalls) return 10;
    resources.Cleanup(); video.Stop();
}
