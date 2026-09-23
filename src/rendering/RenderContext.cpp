#include "RenderContext.h"
#include "../DisplayList.h"

RenderContext& RenderContext::Current()
{
    static RenderContext context;
    return context;
}

void RenderContext::Draw(DisplayList& resource, const Matrix4& model,
                         float red, float green, float blue, float alpha) const
{
    const Matrix4 mvp = Mvp(model);
    resource.SetMvpMatrix(mvp.Data());
    resource.SetDefaultColor(red, green, blue, alpha);
    resource.Call(0);
}
