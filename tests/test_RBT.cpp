#include "RBT.hpp"
#include <gtest/gtest.h>

class RBT_Test : public ::testing::Test {
protected:
    RBT<int> tree{};

    void SetUp() override {
        tree.insert(20);
    }
};

TEST_F(RBT_Test, insert_and_lower_bound) {
    tree.insert(10);
    tree.insert(30);
    tree.insert(45);

    ASSERT_NE(tree.lower_bound(17), nullptr);
    EXPECT_EQ(tree.lower_bound(17)->data, 20);
}

TEST_F(RBT_Test, insert_and_lower_bound_2) {
    for (int i = 1; i <= 50; i+=5) {
        tree.insert(i);
    }
    auto lb1 = tree.lower_bound(0);
    auto lb2 = tree.lower_bound(60);

    ASSERT_NE(lb1, nullptr);
    EXPECT_EQ(lb1->data, 1);
    EXPECT_EQ(lb2, nullptr);
}

TEST_F(RBT_Test, insert_remove_and_lower_bound) {
    tree.insert(10);
    tree.insert(30);
    tree.remove(20);
    tree.insert(45);
    tree.remove(10);

    ASSERT_NE(tree.lower_bound(17), nullptr);
    EXPECT_EQ(tree.lower_bound(17)->data, 30);
}

TEST_F(RBT_Test, remove_inexistent) {
    tree.insert(10);
    testing::internal::CaptureStdout();
    tree.remove(45);
    tree.remove(50);

    string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("not found"), std::string::npos);
}


