#include "../BBL_DS.hpp"
#include <gtest/gtest.h>

using namespace std;

int M = 3;
int B = 50000;

class BBL_DS_Test : public ::testing::Test {
protected:
    BBL_DS<int, int> ds{};

    void SetUp() override {
        ds.initialize(M, B);
    }
};

TEST_F(BBL_DS_Test, D1_insertion_and_pull_and_split) {
    for (int i = 0; i < 6; i++) {
        ds.insert_pair({i, i*5 + 2});
    }
    int x = 0;
    auto keys = ds.pull(x);
    auto D0_D1 = ds.get_sequences();

    EXPECT_FALSE(ds.empty());
    EXPECT_EQ(keys.size(), M);
    EXPECT_TRUE(x != B);
    EXPECT_EQ(x, 17); // smallest pair remaining {3, 17}
    EXPECT_EQ(D0_D1.first.size(), 0);
    EXPECT_GE(D0_D1.second.size(), 1);
    EXPECT_EQ(ds.total_pairs(), 6 - M);
}

TEST_F(BBL_DS_Test, D1_insert_duplicates_keep_lower_and_pull) {
    for (int i = 0; i < 6; i++) {
        ds.insert_pair({i, i*5 + 2});
    }
    ds.insert_pair({0, 0});
    ds.insert_pair({1, 50});
    ds.insert_pair({3, 6});
    int x = 0;
    EXPECT_FALSE(ds.empty());
    EXPECT_EQ(ds.total_pairs(), 6);
    auto keys = ds.pull(x);
    EXPECT_EQ(keys.size(), M);
    EXPECT_EQ(x, 12);
    EXPECT_TRUE(count(keys.begin(), keys.end(),0)
        && count(keys.begin(), keys.end(),1)
        && count(keys.begin(), keys.end(),3));
}

TEST_F(BBL_DS_Test, insert_and_delete_and_pull) {
    for (int i = 0; i < 6; i++) {
        ds.insert_pair({i, i*5 + 2});
    }
    for (int i = 0; i < 4; i++) {
        ds.delete_pair({i, i*5 + 2});
    }
    int x = 0;
    EXPECT_EQ(ds.total_pairs(), 2);
    auto keys = ds.pull(x);
    EXPECT_LE(keys.size(), M);
    EXPECT_TRUE(count(keys.begin(), keys.end(),4) && count(keys.begin(), keys.end(),5));
    EXPECT_TRUE(ds.empty());
    EXPECT_EQ(x, B); // D0 and D1 are empty
}

TEST_F(BBL_DS_Test, batch_prepend) {
    vector<pair<int, int>> L1 = {{1, 1}, {2, 2}, {3, 3}, {6, 10}, {7, 7}};
    vector<pair<int, int>> L2 = {{4, 4}, {5, 5}, {3, 10}, {5, 6}, {3, 2}};

    ds.batch_prepend(L1);
    ds.batch_prepend(L2);
    auto D0_D1 = ds.get_sequences();

    EXPECT_TRUE(ds.total_pairs() == 7);
    EXPECT_EQ(D0_D1.second.size(), 1); // has one item from the initialization
    EXPECT_GE(D0_D1.first.size(), 1);
    EXPECT_EQ(D0_D1.first.size(), 3);
}
