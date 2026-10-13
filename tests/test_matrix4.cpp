#include <gtest/gtest.h>

#include "rendering/Matrix4.h"
#include "rendering/RenderContext.h"

namespace
{
void ExpectNear(const Matrix4& matrix, int row, int column, float value)
{
    EXPECT_NEAR(matrix(row, column), value, 0.0001f);
}
}

TEST(Matrix4Test, IdentityLeavesTranslationUnchanged)
{
    const Matrix4 translation = Matrix4::Translation(2.0f, -3.0f, 4.0f);
    const Matrix4 result = Matrix4::Identity() * translation;
    ExpectNear(result, 0, 3, 2.0f);
    ExpectNear(result, 1, 3, -3.0f);
    ExpectNear(result, 2, 3, 4.0f);
}

TEST(Matrix4Test, PostMultiplicationPreservesLegacyTransformOrder)
{
    const Matrix4 model = Matrix4::Translation(5, 0, 0) *
                          Matrix4::Rotation(90, 0, 0, 1);
    // Translation * rotation rotates the object locally, then translates it.
    ExpectNear(model, 0, 0, 0.0f);
    ExpectNear(model, 1, 0, 1.0f);
    ExpectNear(model, 0, 3, 5.0f);
}

TEST(Matrix4Test, PerspectiveMatchesOpenGLConvention)
{
    const Matrix4 perspective = Matrix4::Perspective(90, 2, 1, 11);
    ExpectNear(perspective, 0, 0, 0.5f);
    ExpectNear(perspective, 1, 1, 1.0f);
    ExpectNear(perspective, 2, 2, -1.2f);
    ExpectNear(perspective, 2, 3, -2.2f);
    ExpectNear(perspective, 3, 2, -1.0f);
}

TEST(Matrix4Test, LookAtMovesEyeToOrigin)
{
    const Matrix4 view = Matrix4::LookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
    ExpectNear(view, 0, 3, 0.0f);
    ExpectNear(view, 1, 3, 0.0f);
    ExpectNear(view, 2, 3, -5.0f);
}

TEST(Matrix4Test, RepresentativeTankCompositionKeepsLocalTurretOffset)
{
    const Matrix4 body = Matrix4::Translation(10, 2, 3) *
                         Matrix4::Rotation(90, 0, 1, 0);
    const Matrix4 turret = body * Matrix4::Translation(0, 0.1f, 0);
    ExpectNear(turret, 0, 3, 10.0f);
    ExpectNear(turret, 1, 3, 2.1f);
    ExpectNear(turret, 2, 3, 3.0f);
}

TEST(RenderContextTest, ComposesProjectionViewModelInThatOrder)
{
    RenderContext& context = RenderContext::Current();
    context.SetProjection(Matrix4::Scale(2, 3, 4));
    context.SetView(Matrix4::Translation(1, 0, 0));
    const Matrix4 mvp = context.Mvp(Matrix4::Translation(0, 2, 0));
    ExpectNear(mvp, 0, 3, 2.0f);
    ExpectNear(mvp, 1, 3, 6.0f);
}

TEST(Matrix4Test, NormalTransformPreservesPerpendicularityUnderNonuniformScale)
{
    const Matrix4 model = Matrix4::Translation(7, 2, -4) *
        Matrix4::Rotation(37, 1, 2, 3) * Matrix4::Scale(2, 3, .5f);
    const Matrix4 normal = model.NormalTransform();
    // Original tangent (1,1,0) and normal (1,-1,0) are perpendicular.
    float dot = 0;
    for (int row = 0; row < 3; ++row)
        dot += (model(row,0) + model(row,1)) * (normal(row,0) - normal(row,1));
    EXPECT_NEAR(dot, 0, .00001f);
    ExpectNear(normal, 0, 3, 0);
    ExpectNear(normal, 1, 3, 0);
    ExpectNear(normal, 2, 3, 0);
}

TEST(Matrix4Test, NormalTransformHandlesReflectionAndSingularScale)
{
    const Matrix4 reflection = Matrix4::Scale(-2, 4, .5f).NormalTransform();
    ExpectNear(reflection, 0, 0, -.5f);
    ExpectNear(reflection, 1, 1, .25f);
    ExpectNear(reflection, 2, 2, 2);
    const Matrix4 singular = Matrix4::Scale(0, 1, 1).NormalTransform();
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col) ExpectNear(singular, row, col, 0);
}
