#ifndef DIJKSTRA_UTILS_HPP
#define DIJKSTRA_UTILS_HPP

#include <limits>
#include <vector>
#include <numeric>
#include <iostream>
#include <ostream>
#include <algorithm>
#include <queue>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <boost/graph/adjacency_list.hpp>
#define INF 10000000

using namespace std;
using Dist_T = double;
using Node_id_T = int;
using VertexProp = boost::property<boost::vertex_name_t, Node_id_T>;
using EdgeProp = boost::property<boost::edge_weight_t, Dist_T>;
using Dist_List_T = vector<Dist_T>;
using Prev_List_T = vector<Node_id_T>;
using Graph = boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS, VertexProp, EdgeProp>; //TODO try CSR graphs

struct Edge {
    Node_id_T to;
    Dist_T w;

    explicit Edge(Node_id_T to, Dist_T w) : to(to), w(w) {}
};

Graph random_graph(long long N, int max_weight, long long edges_count, int seed);

Graph random_graph_with_unit_weights(long long N, long long edges_count, int seed);

pair<Graph, int> constant_degree_transformation(Graph G, int N);

#endif