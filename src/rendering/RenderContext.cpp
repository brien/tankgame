#include "RenderContext.h"
#include "../DisplayList.h"
#include "ModernRenderer.h"

#include <stdexcept>

RenderContext& RenderContext::Current()
{
    static RenderContext context;
    return context;
}

void RenderContext::Draw(const DisplayList& resource, const Matrix4& model,
                         float red, float green, float blue, float alpha, bool lit) const
{
    BasicMaterial material;
    material.lit = lit;
    material.red = red;
    material.green = green;
    material.blue = blue;
    material.alpha = alpha;
    Draw(resource, model, material);
}

void RenderContext::Draw(const DisplayList& resource, const Matrix4& model,
                         const BasicMaterial& material) const
{
    const Matrix4 mvp = Mvp(model);
    if (!renderer) throw std::logic_error("RenderContext has no modern renderer");
    renderer->Draw(resource.ModernGeometry(), mvp.Data(), material,
                   model.NormalTransform().Data(), lighting);
}
