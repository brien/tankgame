#include "RenderContext.h"
#include "../DisplayList.h"
#include "ModernRenderer.h"

#include <stdexcept>

RenderContext& RenderContext::Current()
{
    static RenderContext context;
    return context;
}

void RenderContext::Draw(DisplayList& resource, const Matrix4& model,
                         float red, float green, float blue, float alpha) const
{
    const Matrix4 mvp = Mvp(model);
    BasicMaterial material;
    material.red = red;
    material.green = green;
    material.blue = blue;
    material.alpha = alpha;
    if (!renderer) throw std::logic_error("RenderContext has no modern renderer");
    renderer->Draw(resource.ModernGeometry(), mvp.Data(), material);
}

void RenderContext::Draw(DisplayList& resource, const Matrix4& model,
                         const BasicMaterial& material) const
{
    const Matrix4 mvp = Mvp(model);
    if (!renderer) throw std::logic_error("RenderContext has no modern renderer");
    renderer->Draw(resource.ModernGeometry(), mvp.Data(), material);
}
