#ifndef DIJKSTRA_UTILS_HPP
#define DIJKSTRA_UTILS_HPP

#include <limits>
#include <string>
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
using Node_id_T = string;
using Dist_List_T = unordered_map<Node_id_T, Dist_T>;
using Prev_List_T = unordered_map<Node_id_T, Node_id_T>;
using Graph = unordered_map<Node_id_T, Dist_List_T>;

vector<string> neighbours(Graph& graph, string cur);

int subtree_size(string node, unordered_map<string, unordered_set<string>> forest);

string int_to_label(int i);

Graph random_graph(int N, int max_weight, int edges, int seed);

vector<int> random_array(int N, int max_val, int seed);

tuple<Graph, vector<string>, Prev_List_T> constant_degree_transformation(Graph G, int N);

string get_source_from_nodes(vector<string> nodes_list);

vector<string> simple_node_list(int N);

pair<Dist_List_T, Prev_List_T> fix_results_after_cd(pair<Dist_List_T, Prev_List_T> results, Prev_List_T cd_map, vector<string> node_list);

#endif