#include "DebugLineRenderer.h"
#include "ResourceManager.h"
#include "RenderContext.h"
#include "RendererMode.h"
#include "PlatformGL.h"

Matrix4 DebugLineRenderer::BuildModel(const DebugLineData& line) {
    return Matrix4::Translation(line.start.x, line.start.y, line.start.z) *
        Matrix4::Scale(line.end.x - line.start.x, line.end.y - line.start.y,
                       line.end.z - line.start.z);
}

std::vector<DebugLineData> DebugLineRenderer::BuildTankMarkers(const std::vector<TankRenderData>& tanks) {
    std::vector<DebugLineData> lines;
    lines.reserve(tanks.size() * 3);
    for (const auto& tank : tanks) {
        if (!tank.alive) continue;
        const Vector3& p = tank.position;
        // World X/Y/Z axes identify the simulation origin independently of
        // mesh offsets and rotations. Hidden portions remain occluded.
        lines.push_back({p, Vector3(p.x + 0.75f, p.y, p.z), Vector3(1, 0, 0)});
        lines.push_back({p, Vector3(p.x, p.y + 0.75f, p.z), Vector3(0, 1, 0)});
        lines.push_back({p, Vector3(p.x, p.y, p.z + 0.75f), Vector3(0, 0, 1)});
    }
    return lines;
}

void DebugLineRenderer::Render(ResourceManager& resources, const std::vector<DebugLineData>& lines) {
    if (lines.empty()) return;
    // Save only state this pass changes. Preserve the caller's scene state in
    // both backends, including depth writes and line width, across viewports.
    const GLboolean depth = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blend = glIsEnabled(GL_BLEND);
    GLboolean depthWrite;
    GLint depthFunc;
    GLfloat width;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    glGetFloatv(GL_LINE_WIDTH, &width);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    glLineWidth(1.0f);
#ifndef __EMSCRIPTEN__
    if (!RendererMode::IsModern()) {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LINE_STIPPLE);
    }
#endif
    for (const auto& line : lines) {
        const Matrix4 model = BuildModel(line);
        if (RendererMode::IsModern()) {
            RenderContext::Current().Draw(resources.GetGeometry(GeometryResource::DebugLine), model,
                line.color.x, line.color.y, line.color.z, 1.0f, false);
        }
#ifndef __EMSCRIPTEN__
        else {
            glPushMatrix();
            glMultMatrixf(model.Data());
            glColor3f(line.color.x, line.color.y, line.color.z);
            resources.GetGeometry(GeometryResource::DebugLine).Call(0);
            glPopMatrix();
        }
#endif
    }
#ifndef __EMSCRIPTEN__
    if (!RendererMode::IsModern()) glPopAttrib();
#endif
    glLineWidth(width);
    glDepthMask(depthWrite);
    glDepthFunc(depthFunc);
    if (!depth) glDisable(GL_DEPTH_TEST);
    if (blend) glEnable(GL_BLEND);
}
