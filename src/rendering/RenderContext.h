#pragma once

#include "Matrix4.h"

class DisplayList;

class RenderContext
{
public:
    static RenderContext& Current();
    void SetProjection(const Matrix4& value) { projection = value; }
    void SetView(const Matrix4& value) { view = value; }
    Matrix4 Mvp(const Matrix4& model) const { return projection * view * model; }
    void Draw(DisplayList& resource, const Matrix4& model,
              float red, float green, float blue, float alpha = 1.0f) const;

private:
    Matrix4 projection;
    Matrix4 view;
};
