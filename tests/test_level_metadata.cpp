#include <gtest/gtest.h>
#include "App.h"
#include "LevelHandler.h"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <limits>

class LevelMetadataTest : public ::testing::Test {
protected:
    App app;
    GameTask task;
    LevelHandler level;
    std::string mapPath;
    std::string metadataPath;

    void SetUp() override {
        app.gameTask = &task;
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto prefix = ::testing::TempDir() + "tankgame-metadata-" +
            ::testing::UnitTest::GetInstance()->current_test_info()->name() + "-" + std::to_string(stamp);
        mapPath = prefix + ".txt";
        metadataPath = prefix + ".json";
        std::ofstream map(mapPath);
        ASSERT_TRUE(map.is_open());
        // Match the legacy 128-character map format without special tiles.
        for (int i = 0; i < 128; ++i) map << std::string(128, 'd') << '\n';
        ASSERT_TRUE(map.good());
    }

    void TearDown() override {
        std::remove(mapPath.c_str());
        std::remove(metadataPath.c_str());
    }

    void Metadata(const std::string& json) {
        std::ofstream file(metadataPath);
        ASSERT_TRUE(file.is_open());
        file << json;
        ASSERT_TRUE(file.good());
    }

    void LoadCustom() {
        Metadata(R"({"name":"Custom","author":"Designer","difficulty":7,
            "theme":{"defaultColor":[0.2,0.3,0.4],"blockColor":[0.5,0.6,0.7],
                     "useSpecialColoring":true,"themeId":4},
            "gameplay":{"baseEnemyCount":12,"enemyMultiplier":2}})");
        ASSERT_TRUE(level.Load(mapPath.c_str()));
        ASSERT_EQ(level.GetMetadata().name, "Custom");
        ASSERT_EQ(level.GetEnemyCountForLevel(50), 24);
    }

    void ExpectDefaults() {
        const auto& metadata = level.GetMetadata();
        EXPECT_EQ(metadata.name, "Unnamed Level");
        EXPECT_EQ(metadata.author, "Unknown");
        EXPECT_EQ(metadata.difficulty, 1);
        EXPECT_FALSE(metadata.theme.useSpecialColoring);
        EXPECT_EQ(metadata.theme.themeId, 0);
        EXPECT_FLOAT_EQ(metadata.theme.defaultColor.x, 1);
        EXPECT_FLOAT_EQ(metadata.theme.defaultColor.y, 1);
        EXPECT_FLOAT_EQ(metadata.theme.defaultColor.z, 1);
        EXPECT_FLOAT_EQ(metadata.theme.blockColor.x, 0);
        EXPECT_FLOAT_EQ(metadata.theme.blockColor.y, 1);
        EXPECT_FLOAT_EQ(metadata.theme.blockColor.z, 0);
        EXPECT_EQ(metadata.gameplay.baseEnemyCount, 5);
        EXPECT_FLOAT_EQ(metadata.gameplay.enemyMultiplier, 1);
        EXPECT_EQ(level.GetEnemyCountForLevel(50), 11);
    }
};

TEST_F(LevelMetadataTest, ValidMetadataAppliesWithoutChangingPlayerDifficulty) {
    task.GetPlayerManager()->SetDifficulty(0);
    LoadCustom();
    EXPECT_EQ(level.GetMetadata().difficulty, 7);
    EXPECT_EQ(task.GetPlayerManager()->GetDifficulty(), 0);
    EXPECT_EQ(level.GetMetadata().author, "Designer");
    EXPECT_FLOAT_EQ(level.GetMetadata().theme.blockColor.z, .7f);
}

TEST_F(LevelMetadataTest, MissingSidecarDoesNotReusePreviousLevel) {
    LoadCustom();
    ASSERT_EQ(std::remove(metadataPath.c_str()), 0);
    ASSERT_TRUE(level.Load(mapPath.c_str()));
    ExpectDefaults();
}

TEST_F(LevelMetadataTest, OmittedFieldsInheritDefaultsInsteadOfPreviousValues) {
    LoadCustom();
    Metadata(R"({"name":"Partial","theme":{"themeId":2},"gameplay":{"baseEnemyCount":8}})");
    ASSERT_TRUE(level.Load(mapPath.c_str()));
    const auto& metadata = level.GetMetadata();
    EXPECT_EQ(metadata.name, "Partial");
    EXPECT_EQ(metadata.author, "Unknown");
    EXPECT_EQ(metadata.difficulty, 1);
    EXPECT_EQ(metadata.theme.themeId, 2);
    EXPECT_FALSE(metadata.theme.useSpecialColoring);
    EXPECT_FLOAT_EQ(metadata.theme.defaultColor.x, 1);
    EXPECT_FLOAT_EQ(metadata.gameplay.enemyMultiplier, 1);
    EXPECT_EQ(level.GetEnemyCountForLevel(50), 14);
    Metadata("{}");
    ASSERT_TRUE(level.Load(mapPath.c_str()));
    ExpectDefaults();
}

TEST_F(LevelMetadataTest, MalformedOrInvalidMetadataFallsBackAtomically) {
    const char* invalid[] = {
        "{", "", "[]", "null", "{} trailing", "{} {}",
        R"({"name":42})", R"({"author":false})",
        R"({"difficulty":1.5})", R"({"difficulty":-1})",
        R"({"theme":null})", R"({"gameplay":[]})",
        R"({"theme":{"defaultColor":[1,0]}})",
        R"({"theme":{"defaultColor":[1,0,0,1]}})",
        R"({"theme":{"blockColor":[1,"red",0]}})",
        R"({"theme":{"blockColor":[1,-0.1,0]}})",
        R"({"theme":{"defaultColor":[1,1.1,0]}})",
        R"({"theme":{"useSpecialColoring":1}})",
        R"({"theme":{"themeId":-2}})",
        R"({"theme":{"themeId":4294967296}})",
        R"({"gameplay":{"baseEnemyCount":-1}})",
        R"({"gameplay":{"baseEnemyCount":5.5}})",
        R"({"gameplay":{"baseEnemyCount":18446744073709551615}})",
        R"({"gameplay":{"enemyMultiplier":-0.1}})",
        R"({"gameplay":{"enemyMultiplier":"NaN"}})",
        R"({"gameplay":{"enemyMultiplier":1e40}})",
        R"({"gameplay":{"enemyMultiplier":1e400}})",
        R"({"name":"Partial","author":"Changed","difficulty":6,
            "theme":{"defaultColor":[0.1,0.2,0.3]},"gameplay":{"baseEnemyCount":false}})"
    };
    for (const char* json : invalid) {
        SCOPED_TRACE(json);
        LoadCustom();
        Metadata(json);
        ASSERT_TRUE(level.Load(mapPath.c_str())); // Terrain loading is independent.
        ExpectDefaults();
    }
}

TEST_F(LevelMetadataTest, EmptyAndMissingMapLoadsClearPreviousMetadata) {
    LoadCustom();
    EXPECT_FALSE(level.Load(nullptr));
    ExpectDefaults();
    LoadCustom();
    EXPECT_FALSE(level.Load((mapPath + ".missing").c_str()));
    ExpectDefaults();
}

TEST_F(LevelMetadataTest, LegacyUnusedAndUnknownKeysRemainCompatible) {
    Metadata(R"({"difficulty":0,"theme":{"themeId":-1},
        "gameplay":{"baseEnemyCount":0,"enemyMultiplier":0,
                    "itemSpawnRate":"ignored legacy field"},"futureSetting":42})");
    ASSERT_TRUE(level.Load(mapPath.c_str()));
    EXPECT_EQ(level.GetMetadata().difficulty, 0);
    EXPECT_EQ(level.GetMetadata().theme.themeId, -1);
    EXPECT_EQ(level.GetEnemyCountForLevel(75), 0);
}

TEST_F(LevelMetadataTest, EnemyCountPreservesProgressionAndBoundsConversion) {
    EXPECT_FALSE(level.Load(nullptr));
    EXPECT_EQ(level.GetEnemyCountForLevel(48), 5);
    EXPECT_EQ(level.GetEnemyCountForLevel(57), 32);
    EXPECT_EQ(level.GetEnemyCountForLevel(65), 56); // Existing ASCII 9 -> A gap.
    EXPECT_EQ(level.GetEnemyCountForLevel(std::numeric_limits<int>::min()), 5);
    EXPECT_EQ(level.GetEnemyCountForLevel(std::numeric_limits<int>::max()), std::numeric_limits<int>::max());
    Metadata(R"({"gameplay":{"baseEnemyCount":7,"enemyMultiplier":0.7}})");
    ASSERT_TRUE(level.Load(mapPath.c_str()));
    EXPECT_EQ(level.GetEnemyCountForLevel(58), 28); // Preserve legacy float rounding.
    Metadata(R"({"gameplay":{"baseEnemyCount":2147483647,"enemyMultiplier":3e38}})");
    ASSERT_TRUE(level.Load(mapPath.c_str()));
    EXPECT_EQ(level.GetEnemyCountForLevel(49), std::numeric_limits<int>::max());
    EXPECT_EQ(level.GetEnemyCountForLevel(std::numeric_limits<int>::min()), std::numeric_limits<int>::max());
}

TEST_F(LevelMetadataTest, ShippedMetadataAndLegacyMapDefaultsLoadIndependently) {
    const int expectedCounts[] = {5, 9, 16, 24, 34, 43, 54, 66};
    for (int i = 0; i < 8; ++i) {
        const auto path = "levels/level" + std::to_string(i) + "@@.txt";
        ASSERT_TRUE(level.Load(path.c_str()));
        EXPECT_EQ(level.GetMetadata().difficulty, i + 1);
        EXPECT_EQ(level.GetEnemyCountForLevel(48 + i), expectedCounts[i]);
    }
    ASSERT_TRUE(level.Load("levels/title@@.txt"));
    EXPECT_EQ(level.GetMetadata().name, "Title Screen");
    EXPECT_EQ(level.GetEnemyCountForLevel(48), 0);
    ASSERT_TRUE(level.Load("levels/level8@@.txt"));
    ExpectDefaults(); // No sidecar; neither level 7 nor title values may survive.
}
