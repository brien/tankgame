#include "BaseRenderer.h"
#include "../Logger.h"

bool BaseRenderer::Initialize()
{
    if (isInitialized) {
        Logger::Get().Write("BaseRenderer: Already initialized\n");
        return true;
    }
    
    // Base initialization - derived classes should extend this
    isInitialized = true;
    
    CheckGLError("BaseRenderer::Initialize");
    return true;
}

void BaseRenderer::Cleanup()
{
    if (!isInitialized) {
        return;
    }
    
    // Base cleanup - derived classes should extend this
    isInitialized = false;
    
    CheckGLError("BaseRenderer::Cleanup");
}

void BaseRenderer::SetupRenderState()
{
    if (!isInitialized) {
        Logger::Get().Write("BaseRenderer: Attempting to setup render state on uninitialized renderer!\n");
        return;
    }
    
    // Set up common 3D rendering state
    Setup3DRenderState();
    
    CheckGLError("BaseRenderer::SetupRenderState");
}

void BaseRenderer::CleanupRenderState()
{
    if (!isInitialized) {
        return;
    }
    
    CheckGLError("BaseRenderer::CleanupRenderState");
}

void BaseRenderer::Setup3DRenderState()
{
    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable face culling for performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    
    // Disable blending by default (can be enabled per-object)
    glDisable(GL_BLEND);
    
    CheckGLError("BaseRenderer::Setup3DRenderState");
}
