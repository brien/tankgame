#include <gtest/gtest.h>

#include "rendering/ResourceManager.h"

TEST(GeometryAssetTest, EmptyCopiesFailExplicitlyWithoutAGraphicsContext)
{
    GeometryAsset asset;
    GeometryAsset copy = asset;
    EXPECT_THROW(asset.GetGpuGeometry(), std::logic_error);
    EXPECT_THROW(copy.GetGpuGeometry(), std::logic_error);
}

TEST(RendererResourceCatalogueTest, RepeatedGeometryLookupReturnsSameResource)
{
    ResourceManager resources;

    GeometryAsset& first = resources.GetGeometry(GeometryResource::TankBody);
    GeometryAsset& second = resources.GetGeometry(GeometryResource::TankBody);

    EXPECT_EQ(&first, &second);
    EXPECT_NE(&first, &resources.GetGeometry(GeometryResource::TankTurret));
}

TEST(RendererResourceCatalogueTest, UnknownTypedLookupsFailExplicitly)
{
    ResourceManager resources;

    EXPECT_THROW(resources.GetGeometry(static_cast<GeometryResource>(999)),
                 std::out_of_range);
    EXPECT_THROW(resources.GetMaterial(static_cast<MaterialResource>(999)),
                 std::out_of_range);
}
