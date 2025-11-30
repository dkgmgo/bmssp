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
#define INF 10000000

using namespace std;
using Dist_T = double;
using Node_id_T = int;
using Dist_List_T = unordered_map<Node_id_T, Dist_T>;
using Prev_List_T = unordered_map<Node_id_T, Node_id_T>;
using Graph = unordered_map<Node_id_T, Dist_List_T>;

vector<Node_id_T> neighbours(Graph& graph, Node_id_T cur);

int subtree_size(Node_id_T node, unordered_map<Node_id_T, unordered_set<Node_id_T>> forest);

Graph random_graph(int N, int max_weight, int edges, int seed);

pair<Graph, int> constant_degree_transformation(Graph G, int N);

#endif