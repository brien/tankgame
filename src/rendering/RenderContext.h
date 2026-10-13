#pragma once

#include "Matrix4.h"
#include "BasicMaterial.h"

class DisplayList;
class ModernRenderer;

class RenderContext
{
public:
    static RenderContext& Current();
    void SetModernRenderer(ModernRenderer* value) { renderer = value; }
    void SetProjection(const Matrix4& value) { projection = value; }
    void SetView(const Matrix4& value) { view = value; }
    void SetLighting(bool value) { lighting = value; }
    Matrix4 Mvp(const Matrix4& model) const { return projection * view * model; }
    void Draw(const DisplayList& resource, const Matrix4& model,
              float red, float green, float blue, float alpha = 1.0f, bool lit = true) const;
    void Draw(const DisplayList& resource, const Matrix4& model,
              const BasicMaterial& material) const;

private:
    bool lighting = false;
    ModernRenderer* renderer = nullptr;
    Matrix4 projection;
    Matrix4 view;
};
