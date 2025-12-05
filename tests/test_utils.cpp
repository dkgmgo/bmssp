#include <gtest/gtest.h>
#include "utils.hpp"

using namespace std;

class Test_Utils : public ::testing::Test {
protected:
    void SetUp() override {
    }

    using Map_Graph = unordered_map<Node_id_T, unordered_map<Node_id_T, Dist_T>>;

    Map_Graph boost_to_map(const Graph& G) {
        Map_Graph sortie;
        auto weights = boost::get(boost::edge_weight, G);
        auto edges = boost::edges(G);
        auto ei = edges.first; auto ei_end = edges.second;
        for (; ei != ei_end; ++ei) {
            Node_id_T u = boost::source(*ei, G);
            Node_id_T v = boost::target(*ei, G);
            sortie[u][v] = weights[*ei];
        }

        return sortie;
    }

    Graph map_to_boost(const Map_Graph& G) {
        Graph sortie;
        for (const auto& [u, edges]: G) {
            for (const auto& [v, w]: edges) {
                boost::add_edge(u, v, w, sortie);
            }
        }
        return sortie;
    }
};

TEST_F(Test_Utils, test_subtree_size) {
    unordered_map<Node_id_T, unordered_set<Node_id_T>> F;
    F[0] = {1, 2, 3, 4};
    F[1] = {5};
    F[2] = {};

    EXPECT_EQ(subtree_size(0, F), 6);
    EXPECT_EQ(subtree_size(1, F), 2);
    EXPECT_EQ(subtree_size(2, F), 1);
}

TEST_F(Test_Utils, test_constant_degree_transformation_out) {
    Map_Graph G = {
        {0, {{1, 10}, {2, 10}, {3, 10}, {4, 10}}}
    };
    auto res = constant_degree_transformation(map_to_boost(G), 5);
    Map_Graph G_prime = boost_to_map(res.first);

    EXPECT_EQ(G_prime.size(), 4);
    EXPECT_EQ(res.second, 8);
    EXPECT_TRUE(G_prime.count(5) && G_prime.count(6) && G_prime.count(7) && G_prime.count(0));
    EXPECT_EQ(G_prime[0][5], 0);
    EXPECT_EQ(G_prime[7][0], 0);
    EXPECT_EQ(G_prime[7][1], 10);
    EXPECT_EQ(G_prime[6][2], 10);
    EXPECT_EQ(G_prime[5][3], 10);
    EXPECT_EQ(G_prime[0][4], 10);
}

TEST_F(Test_Utils, test_constant_degree_transformation_in) {
    Map_Graph G = {
        {1, {{0, 10}}},
        {2, {{0, 10}}},
        {3, {{0, 10}}},
        {4, {{0, 10}}}
    };
    auto res = constant_degree_transformation(map_to_boost(G), 5);
    Map_Graph G_prime = boost_to_map(res.first);

    EXPECT_EQ(G_prime.size(), 8);
    EXPECT_EQ(res.second, 8);
    EXPECT_TRUE(G_prime.count(1) && G_prime.count(2) && G_prime.count(3) && G_prime.count(4));
    EXPECT_TRUE(G_prime.count(5) && G_prime.count(6) && G_prime.count(7) && G_prime.count(0));
    EXPECT_EQ(G_prime[0][5], 0);
    EXPECT_EQ(G_prime[7][0], 0);
    EXPECT_EQ(G_prime[1][0], 10);
    EXPECT_EQ(G_prime[2][5], 10);
    EXPECT_EQ(G_prime[3][6], 10);
    EXPECT_EQ(G_prime[4][7], 10);
}

TEST_F(Test_Utils, test_constant_degree_transformation_both) {
    Map_Graph G = {
        {0, {{1, 10}, {2, 10}, {3, 10}, {4, 10}}},
        {2, {{1, 10}}},
        {3, {{1, 10}}},
        {1, {{5, 10}, {6, 10}, {7, 10}}},
        {4, {{0, 10}}}
    };
    auto res = constant_degree_transformation(map_to_boost(G), 8);
    Map_Graph G_prime = boost_to_map(res.first);

    EXPECT_EQ(res.second, 17);
    EXPECT_EQ(G_prime.size(), 14);
    EXPECT_EQ(G_prime[8][9], 0);
    EXPECT_EQ(G_prime[0][8], 0);
    EXPECT_EQ(G_prime[0][4], 10);
    EXPECT_EQ(G_prime[9][2], 10);
    EXPECT_EQ(G_prime[3][16], 10);
    EXPECT_EQ(G_prime[16][1], 0);
}

TEST_F(Test_Utils, test_constant_degree_transformation_already_cd) {
    Map_Graph G = {
        {0, {{1, 10}, {2, 10}}},
        {3, {{0, 10}}},
        {1, {{3, 10}, {2, 10}}}
    };
    auto res = constant_degree_transformation(map_to_boost(G), 4);
    Map_Graph G_prime = boost_to_map(res.first);

    EXPECT_EQ(res.second, 4);
    EXPECT_EQ(G_prime.size(), 3);
    EXPECT_TRUE(G_prime.count(3) && G_prime.count(0) && G_prime.count(1));
    EXPECT_EQ(G_prime, G);
}

TEST_F(Test_Utils, test_constant_degree_transformation_2ins_3_outs) {
    Map_Graph G = {
        {0, {{1, 10}, {2, 10}, {3, 10}}},
        {2, {{0, 10}}},
        {3, {{0, 10}}}
    };
    auto res = constant_degree_transformation(map_to_boost(G), 4);
    Map_Graph G_prime = boost_to_map(res.first);

    EXPECT_EQ(G_prime.size(), 7);
    EXPECT_EQ(res.second, 8);
    EXPECT_TRUE(G_prime.count(2) && G_prime.count(3) && G_prime.count(0));
    EXPECT_EQ(G_prime[0][3], 10);
    EXPECT_EQ(G_prime[5][1], 10);
    EXPECT_EQ(G_prime[2][6], 10);
    EXPECT_EQ(G_prime[3][7], 10);
    EXPECT_EQ(G_prime[7][0], 0);
}