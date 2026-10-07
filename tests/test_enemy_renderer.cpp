#include <gtest/gtest.h>
#include <algorithm>

#include "rendering/EnemyTankRendererImpl.h"
#include "rendering/TankRendererFactory.h"
#include "TankTypeManager.h"

namespace {
void ExpectPoint(const Matrix4& model, float x, float y, float z,
                 float expectedX, float expectedY, float expectedZ) {
    EXPECT_NEAR(model(0, 0)*x + model(0, 1)*y + model(0, 2)*z + model(0, 3), expectedX, .00001f);
    EXPECT_NEAR(model(1, 0)*x + model(1, 1)*y + model(1, 2)*z + model(1, 3), expectedY, .00001f);
    EXPECT_NEAR(model(2, 0)*x + model(2, 1)*y + model(2, 2)*z + model(2, 3), expectedZ, .00001f);
}
}

TEST(EnemyRendererTest, AllTypePairsUseLegacyEnemyGeometryAndExtractedColors) {
    const TankType types[] = {TankType::TYPE_GREY, TankType::TYPE_RED, TankType::TYPE_BLUE,
                              TankType::TYPE_YELLOW, TankType::TYPE_PURPLE};
    for (TankType primary : types) {
        for (TankType secondary : types) {
            TankRenderData tank;
            tank.type1 = primary;
            tank.type2 = secondary;
            tank.primaryColor = TankTypeManager::GetTankTypeColor(primary);
            tank.secondaryColor = TankTypeManager::GetTankTypeColor(secondary);
            const auto draws = EnemyTankRendererImpl::BuildDraws(tank);
            EXPECT_EQ(draws[0].geometry, GeometryResource::EnemyBody);
            EXPECT_EQ(draws[1].geometry, GeometryResource::EnemyBarrel);
            EXPECT_EQ(draws[2].geometry, GeometryResource::EnemyTurret);
            const Color colors[] = {tank.secondaryColor, tank.primaryColor, tank.primaryColor};
            for (size_t i = 0; i < draws.size(); ++i) {
                EXPECT_FLOAT_EQ(draws[i].material.red, (4*colors[i].r + 1)/2);
                EXPECT_FLOAT_EQ(draws[i].material.green, (4*colors[i].g + 1)/2);
                EXPECT_FLOAT_EQ(draws[i].material.blue, (4*colors[i].b + 1)/2);
                EXPECT_FLOAT_EQ(draws[i].material.alpha, 1);
                EXPECT_FALSE(draws[i].material.HasTexture());
            }
        }
    }
}

TEST(EnemyRendererTest, ComponentScalesAndLocalCannonOffsetMatchCompatibility) {
    TankRenderData tank;
    tank.position = Vector3(10, 2, -3);
    const auto draws = EnemyTankRendererImpl::BuildDraws(tank);
    ExpectPoint(draws[0].model, 5, 2, 3, 10.3f, 2.12f, -2.82f);
    ExpectPoint(draws[1].model, 3, 4, 2, 10.3f, 2.4f, -2.8f);
    ExpectPoint(draws[2].model, 5, 4, .25f, 10.6f, 2.4f, -2.975f);
}

TEST(EnemyRendererTest, TurretAimIsRelativeToBodyAndOffsetRotatesWithAim) {
    TankRenderData tank;
    tank.position = Vector3(10, 2, -3);
    tank.bodyRotation.y = 90;
    tank.turretRotation.y = 90;
    const auto draws = EnemyTankRendererImpl::BuildDraws(tank);
    ExpectPoint(draws[0].model, 5, 2, 3, 9.82f, 2.12f, -2.7f);
    ExpectPoint(draws[1].model, 3, 4, 2, 9.7f, 2.4f, -3.2f);
    ExpectPoint(draws[2].model, 5, 4, .25f, 9.4f, 2.4f, -3.025f);
    tank.turretRotation.y = 0;
    const auto aimed = EnemyTankRendererImpl::BuildDraws(tank);
    ExpectPoint(aimed[0].model, 5, 2, 3, 9.82f, 2.12f, -2.7f);
    ExpectPoint(aimed[2].model, 5, 4, .25f, 9.975f, 2.4f, -2.4f);
}

TEST(EnemyRendererTest, PitchRollAndYawComposeInLegacyOrder) {
    TankRenderData tank;
    tank.position = Vector3(10, 2, -3);
    tank.bodyRotation = Vector3(90, 90, 90);
    tank.turretRotation = Vector3(90, 90, 0);
    const auto draws = EnemyTankRendererImpl::BuildDraws(tank);
    // Body Rx * Ry(-yaw) * Rz maps (x,y,z) to (-z,y,x).
    ExpectPoint(draws[0].model, 5, 2, 3, 9.82f, 2.12f, -2.7f);
    // Relative turret Rx * Ry(-yaw) maps (x,y,z) to (-z,-x,y).
    ExpectPoint(draws[1].model, 3, 4, 2, 9.6f, 1.7f, -3.2f);
    ExpectPoint(draws[2].model, 5, 4, .25f, 9.6f, 1.4f, -3.025f);
}

TEST(EnemyRendererTest, HealthMultiplierPreservesOverbrightColorsAndZeroHealthIsFinite) {
    TankRenderData tank;
    tank.primaryColor = Color(.2f, .4f, .8f, .1f);
    tank.secondaryColor = Color(.1f, .3f, .5f, .2f);
    tank.health = 25;
    tank.maxHealth = 100;
    auto draws = EnemyTankRendererImpl::BuildDraws(tank);
    EXPECT_FLOAT_EQ(draws[0].material.red, 2.2f);
    EXPECT_FLOAT_EQ(draws[1].material.blue, 3.6f);
    EXPECT_FLOAT_EQ(draws[2].material.green, 2.8f);
    for (float health : {0.0f, -1.0f}) {
        tank.health = health;
        draws = EnemyTankRendererImpl::BuildDraws(tank);
        EXPECT_FLOAT_EQ(draws[0].material.red, .1f);
        EXPECT_FLOAT_EQ(draws[1].material.blue, .8f);
    }
    tank.health = 25;
    tank.maxHealth = 0;
    EXPECT_FLOAT_EQ(EnemyTankRendererImpl::BuildDraws(tank)[0].material.green, .3f);
}

TEST(EnemyRendererTest, FactorySelectsEnemyRendererAndRejectsDeadOrPlayerDataBeforeGL) {
    auto renderer = TankRendererFactory::CreateRenderer(false);
    ASSERT_NE(dynamic_cast<EnemyTankRendererImpl*>(renderer.get()), nullptr);
    TankRenderData tank;
    tank.alive = false;
    EXPECT_NO_THROW(renderer->Render(tank));
    tank.alive = true;
    tank.isPlayer = true;
    EXPECT_NO_THROW(renderer->Render(tank));
}

TEST(EnemyRendererTest, GeometryRetainsTriangleCountsBoundsAttributesAndWinding) {
    const Geometry body = SimpleGeometry::CreateEnemyBody();
    const Geometry barrel = SimpleGeometry::CreateEnemyBarrel();
    const Geometry turret = SimpleGeometry::CreateEnemyTurret();
    const Geometry meshes[] = {body, barrel, turret};
    const size_t counts[] = {36, 36, 24};
    const float bounds[][6] = {{-5, 5, 0, 2, -5, 5}, {-3, 3, 2, 4, -3, 3}, {-3, 5, 3, 4, -.25f, .25f}};
    for (size_t i = 0; i < 3; ++i) {
        const auto& mesh = meshes[i];
        EXPECT_EQ(mesh.topology, PrimitiveTopology::TRIANGLES);
        ASSERT_EQ(mesh.vertices.size(), counts[i]);
        EXPECT_TRUE(mesh.hasNormals);
        EXPECT_TRUE(mesh.hasTextureCoordinates);
        EXPECT_FALSE(mesh.hasColors);
        float limits[] = {100, -100, 100, -100, 100, -100};
        for (const auto& v : mesh.vertices) {
            const float point[] = {v.x, v.y, v.z};
            for (int axis = 0; axis < 3; ++axis) {
                limits[axis*2] = (std::min)(limits[axis*2], point[axis]);
                limits[axis*2+1] = (std::max)(limits[axis*2+1], point[axis]);
            }
        }
        for (int j = 0; j < 6; ++j) EXPECT_FLOAT_EQ(limits[j], bounds[i][j]);
        for (size_t j = 0; j < mesh.vertices.size(); j += 3) {
            const auto &a = mesh.vertices[j], &b = mesh.vertices[j+1], &c = mesh.vertices[j+2];
            const float ux = b.x-a.x, uy = b.y-a.y, uz = b.z-a.z;
            const float vx = c.x-a.x, vy = c.y-a.y, vz = c.z-a.z;
            // Legacy triangles wind clockwise when viewed from their outward normal.
            EXPECT_LT((uy*vz-uz*vy)*a.normalX + (uz*vx-ux*vz)*a.normalY +
                      (ux*vy-uy*vx)*a.normalZ, 0);
        }
    }
    EXPECT_FLOAT_EQ(body.vertices[0].x, 4);
    EXPECT_FLOAT_EQ(body.vertices[1].z, 4);
    EXPECT_FLOAT_EQ(body.vertices[2].x, 5);
    EXPECT_FLOAT_EQ(body.vertices[0].normalX, .894427f);
    EXPECT_FLOAT_EQ(turret.vertices[0].v, -.707107f);
}

TEST(EnemyRendererTest, CatalogueOwnsDistinctStableEnemyResources) {
    ResourceManager resources;
    const GeometryResource ids[] = {GeometryResource::EnemyBody, GeometryResource::EnemyBarrel,
                                    GeometryResource::EnemyTurret};
    for (GeometryResource id : ids) {
        EXPECT_EQ(&resources.GetGeometry(id), &resources.GetGeometry(id));
        EXPECT_NE(&resources.GetGeometry(id), &resources.GetGeometry(GeometryResource::TankBody));
    }
    EXPECT_NE(&resources.GetGeometry(ids[0]), &resources.GetGeometry(ids[1]));
    EXPECT_NE(&resources.GetGeometry(ids[1]), &resources.GetGeometry(ids[2]));
}
