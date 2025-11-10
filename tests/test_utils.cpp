#include <gtest/gtest.h>
#include "utils.hpp"

using namespace std;

class Test_Utils : public ::testing::Test {
protected:
    void SetUp() override {
    }
};

TEST_F(Test_Utils, test_neighbours) {
    Graph G = {
        {"A", {{"B", 10}, {"C", 10}, {"D", 10}, {"E", 10}}}
    };

    auto res = neighbours(G, "A");
    EXPECT_EQ(res.size(), 4);
    EXPECT_TRUE(count(res.begin(), res.end(), "B") &&
        count(res.begin(), res.end(), "C") &&
        count(res.begin(), res.end(), "D") &&
        count(res.begin(), res.end(), "E"));
}

TEST_F(Test_Utils, test_subtree_size) {
    unordered_map<string, unordered_set<string>> F;
    F["A"] = {"B", "C", "D", "E"};
    F["B"] = {"F"};
    F["C"] = {};

    EXPECT_EQ(subtree_size("A", F), 6);
    EXPECT_EQ(subtree_size("B", F), 2);
    EXPECT_EQ(subtree_size("C", F), 1);
}

TEST_F(Test_Utils, test_constant_degree_transformation_out) {
    Graph G = {
        {"A", {{"B", 10}, {"C", 10}, {"D", 10}, {"E", 10}}}
    };
    auto res = constant_degree_transformation(G, 5);
    Graph G_prime = get<0>(res);
    vector<string> node_list = get<1>(res);

    EXPECT_GE(node_list.size(), 5);
    EXPECT_EQ(node_list.size(), 8);
    EXPECT_TRUE(count(node_list.begin(), node_list.end(), "x_A_B") &&
        count(node_list.begin(), node_list.end(), "x_A_D") &&
        count(node_list.begin(), node_list.end(), "x_A_E") &&
        count(node_list.begin(), node_list.end(), "x_A_C"));
    EXPECT_EQ(G_prime["x_A_B"]["x_A_E"], FAKE_ZERO);
    EXPECT_EQ(G_prime["x_A_B"]["B"], 10);
    EXPECT_FALSE(G_prime.count("A"));
}

TEST_F(Test_Utils, test_constant_degree_transformation_in) {
    Graph G = {
        {"B", {{"A", 10}}},
        {"C", {{"A", 10}}},
        {"D", {{"A", 10}}},
        {"E", {{"A", 10}}}
    };
    auto res = constant_degree_transformation(G, 5);
    Graph G_prime = get<0>(res);
    vector<string> node_list = get<1>(res);

    EXPECT_GE(node_list.size(), 5);
    EXPECT_EQ(node_list.size(), 8);
    EXPECT_TRUE(count(node_list.begin(), node_list.end(), "y_B_A") &&
        count(node_list.begin(), node_list.end(), "y_C_A") &&
        count(node_list.begin(), node_list.end(), "y_D_A") &&
        count(node_list.begin(), node_list.end(), "y_E_A"));
    EXPECT_EQ(G_prime["y_B_A"]["y_C_A"], FAKE_ZERO);
    EXPECT_EQ(G_prime["B"]["y_B_A"], 10);
    EXPECT_FALSE(G_prime.count("A"));
}

TEST_F(Test_Utils, test_constant_degree_transformation_both) {
    Graph G = {
        {"A", {{"B", 10}, {"C", 10}, {"D", 10}, {"E", 10}}},
        {"C", {{"B", 10}}},
        {"D", {{"B", 10}}},
        {"B", {{"F", 10}, {"G", 10}, {"H", 10}}},
        {"E", {{"A", 10}}}
    };
    auto res = constant_degree_transformation(G, 8);
    Graph G_prime = get<0>(res);
    vector<string> node_list = get<1>(res);

    EXPECT_GE(node_list.size(), 8);
    EXPECT_EQ(node_list.size(), 17);
    for (auto el: G_prime) {
        EXPECT_NE(el.first, "B");
        EXPECT_FALSE(el.second.count("B"));
    }
    EXPECT_TRUE(G_prime.count("C") && G_prime.count("D") && G_prime.count("E"));
    EXPECT_EQ(G_prime["x_A_B"]["A"], FAKE_ZERO);
    EXPECT_EQ(G_prime["A"]["x_A_E"], FAKE_ZERO);
    EXPECT_EQ(G_prime["x_A_B"]["y_A_B"], 10);
    EXPECT_EQ(G_prime["x_A_C"]["C"], 10);
    EXPECT_EQ(G_prime["x_B_F"]["F"], 10);
    EXPECT_EQ(G_prime["x_B_F"]["y_A_B"], FAKE_ZERO);
}

TEST_F(Test_Utils, test_constant_degree_transformation_already_cd) {
    Graph G = {
        {"A", {{"B", 10}, {"C", 10}}},
        {"C", {{"B", 10}}},
        {"D", {{"A", 10}}},
        {"B", {{"D", 10}, {"C", 10}}},
        {"E", {{"A", 10}}}
    };
    auto res = constant_degree_transformation(G, 5);
    Graph G_prime = get<0>(res);
    vector<string> node_list = get<1>(res);

    EXPECT_EQ(node_list.size(), 5);
    EXPECT_EQ(node_list, simple_node_list(5));
    EXPECT_TRUE(G_prime.count("C") && G_prime.count("D") && G_prime.count("E") && G_prime.count("A") && G_prime.count("B"));
    EXPECT_EQ(G_prime, G);
}

TEST_F(Test_Utils, test_constant_degree_transformation_2ins_3_outs) {
    Graph G = {
        {"A", {{"B", 10}, {"C", 10}, {"D", 10}}},
        {"C", {{"A", 10}}},
        {"D", {{"A", 10}}}
    };
    auto res = constant_degree_transformation(G, 4);
    Graph G_prime = get<0>(res);
    vector<string> node_list = get<1>(res);

    EXPECT_GE(node_list.size(), 4);
    EXPECT_EQ(node_list.size(), 8);
    for (auto el: G_prime) {
        EXPECT_NE(el.first, "A");
        EXPECT_FALSE(el.second.count("A"));
    }
    EXPECT_TRUE(G_prime.count("C") && G_prime.count("D"));
    EXPECT_TRUE(count(node_list.begin(), node_list.end(), "B"));
    EXPECT_EQ(G_prime["x_A_B"]["B"], 10);
    EXPECT_EQ(G_prime["x_A_C"]["C"], 10);
    EXPECT_EQ(G_prime["C"]["y_C_A"], 10);
    EXPECT_EQ(G_prime["D"]["y_D_A"], 10);
    EXPECT_EQ(G_prime["y_D_A"]["x_A_D"], FAKE_ZERO);
}