#ifndef GRAPH_UTILS_HPP
#define GRAPH_UTILS_HPP

#include <random>
#include <unordered_set>
#include "../common.hpp"

using namespace std;

inline uint64_t encode_edge(int u, int v) {
    return uint64_t(u) << 32 | uint64_t(v);
}

Graph random_graph(long long N, int max_weight, long long edges_count, int seed) {
    long long MAX_RETRIES = max(500ll, edges_count+1);

    mt19937 rng(seed);
    uniform_int_distribution<> weight_dist(1, max_weight);
    uniform_int_distribution<> node_dist(0, N-1);
    edges_count = min(edges_count, N*(N-1));

    if (edges_count < 0) {
        throw runtime_error("edges must be greater than 0");
    }

    vector<Dist_T> weights(edges_count, 0);
    vector<pair<Node_id_T, Node_id_T>> edges(edges_count, {-1, -1});
    unordered_set<uint64_t> seen; seen.reserve(edges_count*2);

    //connect the source 0 with something
    int zero_outs = max(1ll, edges_count/N);
    for (int i = 0; i < zero_outs; i++) {
        Node_id_T v = node_dist(rng);
        while (v == 0) {
            v = node_dist(rng);
        }
        edges[i] = {0, v};
        weights[i] = weight_dist(rng);
        seen.insert(encode_edge(0, v));
    }

    for (int i = zero_outs; i < edges_count; i++) {
        int u = node_dist(rng);
        int v = node_dist(rng);
        int tries = 0;
        while (!seen.insert(encode_edge(u, v)).second || u == v) {
            u = node_dist(rng);
            v = node_dist(rng);
            tries++;
            if (tries >= MAX_RETRIES) {
                throw runtime_error("Too many collisions");
            }
        }
        edges[i] = {u, v};
        weights[i] = weight_dist(rng);
    }

    return Graph(edges.begin(), edges.end(), weights.begin(), N);
}

Graph random_graph_with_unit_weights(long long N, long long edges_count, int seed) {
    return random_graph(N, 1, edges_count, seed);
}

pair<Graph, int> constant_degree_transformation(Graph G, int N) {
    Graph G_prime;
    unordered_map<Node_id_T, string> id_to_name;
    unordered_map<string, Node_id_T> name_to_id;
    int nextId = N-1;

    unordered_map<Node_id_T, vector<Node_id_T>> outgoings;
    unordered_map<Node_id_T, vector<Node_id_T>> incomings;
    for (int i = 0; i < N; i++) {
        auto oe = boost::out_edges(i, G);
        auto ei = oe.first; auto ei_end = oe.second;
        for (; ei != ei_end; ++ei) {
            outgoings[i].push_back(boost::target(*ei, G));
        }
        for (Node_id_T nei: outgoings[i]) {
            incomings[nei].push_back(i);
        }
    }

    for (int i = 0; i < N; i++) {
        int out_size = static_cast<int>(outgoings[i].size());
        int in_size = static_cast<int>(incomings[i].size());

        vector<Node_id_T> cycle_nodes;
        if (out_size + in_size > 3 || in_size > 2 || out_size > 2) {

            bool is_first_of_cycle = true;

            for (Node_id_T desti : outgoings[i]) {
                string new_node = "x_"+to_string(i)+"_"+to_string(desti);
                if (is_first_of_cycle) {
                    is_first_of_cycle = false;
                    name_to_id[new_node] = i;
                    id_to_name[i] = new_node;
                    cycle_nodes.push_back(i);
                }else {
                    nextId++;
                    name_to_id[new_node] = nextId;
                    id_to_name[nextId] = new_node;
                    cycle_nodes.push_back(nextId);
                }

            }
            for (Node_id_T source : incomings[i]) {
                string new_node = "y_"+to_string(source)+"_"+to_string(i);
                if (is_first_of_cycle) {
                    is_first_of_cycle = false;
                    name_to_id[new_node] = i;
                    id_to_name[i] = new_node;
                    cycle_nodes.push_back(i);
                }else {
                    nextId++;
                    name_to_id[new_node] = nextId;
                    id_to_name[nextId] = new_node;
                    cycle_nodes.push_back(nextId);
                }
            }

            int cycle_size = static_cast<int>(cycle_nodes.size());
            for (int j = 0; j < cycle_size; j++) {
                boost::add_edge(cycle_nodes[j], cycle_nodes[(j+1)%cycle_size], 0, G_prime);
            }
        }
    }

    for (int i = 0; i <= nextId; i++) {
        if (i >= N || id_to_name.find(i) != id_to_name.end()) {
            string u = id_to_name[i];

            if (u.find("y_") != string::npos) {
                continue;
            }
            string A_to_B = u.substr(u.find("_")+1);
            Node_id_T A = stoi(A_to_B.substr(0, A_to_B.find("_")));
            Node_id_T B = stoi(A_to_B.substr(A_to_B.find("_")+1));
            string dest = "y_"+A_to_B;

            auto old_edge = boost::edge(A, B, G);
            if (!old_edge.second) {
                continue;
            }
            Dist_T weight = boost::get(boost::edge_weight, G, old_edge.first);

            if (name_to_id.find(dest) != name_to_id.end()) {
                boost::add_edge(i, name_to_id[dest], weight, G_prime);
            }else {
                boost::add_edge(i, B, weight, G_prime);
            }
        }else {
            auto oe = boost::out_edges(i, G);
            auto ei = oe.first; auto ei_end = oe.second;
            for (; ei != ei_end; ++ei) {
                Node_id_T j = boost::target(*ei, G);
                Dist_T weight = boost::get(boost::edge_weight, G, *ei);
                if (id_to_name.find(j) != id_to_name.end()) {
                    boost::add_edge(i, name_to_id["y_"+to_string(i)+"_"+to_string(j)], weight, G_prime);
                }else {
                    boost::add_edge(i, j, weight, G_prime);
                }
            }
        }
    }

    return {G_prime, nextId+1};
}

#endif //GRAPH_UTILS_HPP
