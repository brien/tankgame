#include "TargetingLineRenderer.h"
#include "ResourceManager.h"
#include "RenderContext.h"
#include "PlatformGL.h"
#include <algorithm>
#include <cmath>

Matrix4 TargetingLineRenderer::BuildModel(const HUDRenderData& data) {
    const float angle = data.playerRotation.y * 0.017453292519943295f;
    return Matrix4::Translation(data.playerPosition.x, data.playerPosition.y + .1f, data.playerPosition.z) *
        Matrix4::Scale(32 * std::cos(angle), 0, 32 * std::sin(angle));
}

bool TargetingLineRenderer::BuildMaterial(const HUDRenderData& data, const Matrix4& mvp,
                                          const int viewport[4], BasicMaterial& material) {
    if (!data.showTargeting || viewport[2] <= 0 || viewport[3] <= 0 ||
        !std::isfinite(data.targetingAnimationTime)) return false;
    float a[4], b[4];
    for (int r = 0; r < 4; ++r) {
        a[r] = mvp(r, 3);
        b[r] = a[r] + mvp(r, 0) + mvp(r, 1) + mvp(r, 2);
        if (!std::isfinite(a[r]) || !std::isfinite(b[r])) return false;
    }
    // Clip in homogeneous space before dividing by w. This also keeps the
    // stipple origin well-defined when the player is behind the camera.
    float first = 0, last = 1;
    for (int axis = 0; axis < 3; ++axis) {
        for (int sign : {-1, 1}) {
            const float start = a[3] + sign * a[axis], end = b[3] + sign * b[axis];
            if (start < 0 && end < 0) return false;
            if (start < 0) first = std::max(first, start / (start - end));
            if (end < 0) last = std::min(last, start / (start - end));
        }
    }
    if (first >= last) return false;
    float start[4], end[4];
    for (int r = 0; r < 4; ++r) {
        start[r] = a[r] + first * (b[r] - a[r]);
        end[r] = a[r] + last * (b[r] - a[r]);
    }
    if (start[3] <= 0 || end[3] <= 0) return false;
    float screen[2], delta[2];
    for (int axis = 0; axis < 2; ++axis) {
        screen[axis] = viewport[axis] + (start[axis] / start[3] + 1) * viewport[axis + 2] * .5f;
        delta[axis] = (end[axis] / end[3] - start[axis] / start[3]) * viewport[axis + 2] * .5f;
    }
    if (std::max(std::abs(delta[0]), std::abs(delta[1])) < .001f) return false;
    material = BasicMaterial{};
    material.targeting = true;
    material.red = data.targetingColor.x; material.green = data.targetingColor.y; material.blue = data.targetingColor.z;
    material.stippleFrame[0] = screen[0]; material.stippleFrame[1] = screen[1];
    // Legacy stipple counts fragments along the major raster axis, not world units.
    const int axis = std::abs(delta[0]) >= std::abs(delta[1]) ? 0 : 1;
    material.stippleFrame[axis + 2] = delta[axis] > 0 ? 1 : -1;
    float wrapped = std::fmod(std::trunc(data.targetingAnimationTime), 65536.0f);
    if (wrapped < 0) wrapped += 65536;
    const unsigned pattern = static_cast<unsigned>(wrapped);
    for (int i = 0; i < 4; ++i) material.stippleBits[i] = (pattern >> (4 * i)) & 15;
    return pattern != 0;
}

void TargetingLineRenderer::Render(ResourceManager& resources, const HUDRenderData& data) {
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    const Matrix4 model = BuildModel(data);
    BasicMaterial material;
    if (!BuildMaterial(data, RenderContext::Current().Mvp(model), viewport, material)) return;
    const GLboolean depth = glIsEnabled(GL_DEPTH_TEST), blend = glIsEnabled(GL_BLEND);
    GLboolean write; GLint function; GLfloat width;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &write);
    glGetIntegerv(GL_DEPTH_FUNC, &function); glGetFloatv(GL_LINE_WIDTH, &width);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDepthMask(GL_FALSE);
    glDisable(GL_BLEND); glLineWidth(1);
    RenderContext::Current().Draw(resources.GetGeometry(GeometryResource::DebugLine), model, material);
    glLineWidth(width); glDepthFunc(function); glDepthMask(write);
    if (!depth) glDisable(GL_DEPTH_TEST);
    if (blend) glEnable(GL_BLEND);
}
