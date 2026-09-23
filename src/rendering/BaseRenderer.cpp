#include "BaseRenderer.h"
#include "../Logger.h"
#include <iostream>

bool BaseRenderer::Initialize()
{
    if (isInitialized) {
        Logger::Get().Write("BaseRenderer: Already initialized\n");
        return true;
    }
    
    // Base initialization - derived classes should extend this
    isInitialized = true;
    stateStored = false;
    
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
    stateStored = false;
    
    CheckGLError("BaseRenderer::Cleanup");
}

void BaseRenderer::SetupRenderState()
{
    if (!isInitialized) {
        Logger::Get().Write("BaseRenderer: Attempting to setup render state on uninitialized renderer!\n");
        return;
    }
    
    // Store current state for restoration
    StoreRenderState();
    
    // Set up common 3D rendering state
    Setup3DRenderState();
    
    CheckGLError("BaseRenderer::SetupRenderState");
}

void BaseRenderer::CleanupRenderState()
{
    if (!isInitialized) {
        return;
    }
    
    // Restore previous state
    RestoreRenderState();
    
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
    
    // Smooth shading is fixed-function desktop state; browser shaders define
    // interpolation explicitly.
#ifndef __EMSCRIPTEN__
    glShadeModel(GL_SMOOTH);
#endif
    
    // Disable blending by default (can be enabled per-object)
    glDisable(GL_BLEND);
    
    CheckGLError("BaseRenderer::Setup3DRenderState");
}

void BaseRenderer::Setup2DRenderState()
{
    // Disable depth testing for UI rendering
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for transparency
    SetBlending(true);
    
    // Disable face culling for 2D quads
    glDisable(GL_CULL_FACE);
    
    CheckGLError("BaseRenderer::Setup2DRenderState");
}

void BaseRenderer::SetLighting(bool enable)
{
#ifdef __EMSCRIPTEN__
    (void)enable;
#else
    if (enable) {
        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE); // Normalize normals after scaling
    } else {
        glDisable(GL_LIGHTING);
    }
#endif
    
    CheckGLError("BaseRenderer::SetLighting");
}

void BaseRenderer::SetBlending(bool enable)
{
    if (enable) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
    
    CheckGLError("BaseRenderer::SetBlending");
}

void BaseRenderer::PushMatrix()
{
#ifndef __EMSCRIPTEN__
    glPushMatrix();
#endif
    CheckGLError("BaseRenderer::PushMatrix");
}

void BaseRenderer::PopMatrix()
{
#ifndef __EMSCRIPTEN__
    glPopMatrix();
#endif
    CheckGLError("BaseRenderer::PopMatrix");
}

void BaseRenderer::StoreRenderState()
{
#ifdef __EMSCRIPTEN__
    // WebGL state used by migrated draws is established explicitly. Do not
    // query desktop-only lighting or matrix-stack state.
    stateStored = false;
#else
    // Store current OpenGL state
    previousState.depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    previousState.lightingEnabled = glIsEnabled(GL_LIGHTING);
    previousState.blendEnabled = glIsEnabled(GL_BLEND);
    previousState.cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    glGetIntegerv(GL_MATRIX_MODE, &previousState.matrixMode);
    
    stateStored = true;
#endif
    CheckGLError("BaseRenderer::StoreRenderState");
}

void BaseRenderer::RestoreRenderState()
{
#ifdef __EMSCRIPTEN__
    stateStored = false;
    return;
#else
    if (!stateStored) {
        return;
    }
    
    // Restore previous OpenGL state
    if (previousState.depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    
    if (previousState.lightingEnabled) {
        glEnable(GL_LIGHTING);
    } else {
        glDisable(GL_LIGHTING);
    }
    
    if (previousState.blendEnabled) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
    
    if (previousState.cullFaceEnabled) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    
    glMatrixMode(previousState.matrixMode);
    
    stateStored = false;
    CheckGLError("BaseRenderer::RestoreRenderState");
#endif
}
