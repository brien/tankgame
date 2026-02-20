#include <gtest/gtest.h>

// Simple test to verify Google Test is working
TEST(SampleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
}

// Test fixture example
class MathTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code before each test
    }

    void TearDown() override {
        // Cleanup code after each test
    }
};

TEST_F(MathTest, Addition) {
    EXPECT_EQ(2 + 2, 4);
}

TEST_F(MathTest, Subtraction) {
    EXPECT_EQ(5 - 3, 2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
