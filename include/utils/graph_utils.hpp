#ifndef GRAPH_UTILS_HPP
#define GRAPH_UTILS_HPP

#include <random>
#include <regex>
#include <unordered_set>
#include <unordered_map>

#include "../common.hpp"
#include "file_utils.hpp"

using namespace std;

inline uint64_t encode_edge(int u, int v) {
    return uint64_t(u) << 32 | uint64_t(v);
}

Graph random_graph(int64_t N, int max_weight, int64_t edges_count, int seed) {
    int64_t MAX_RETRIES = max(500l, edges_count+1);

    mt19937 rng(seed);
    uniform_real_distribution<> weight_dist(1, max_weight);
    uniform_int_distribution<> node_dist(0, N-1);
    edges_count = min(edges_count, N*(N-1));

    if (edges_count < 0) {
        throw runtime_error("edges must be greater than 0");
    }

    vector<Dist_T> weights(edges_count, 0);
    vector<pair<Node_id_T, Node_id_T>> edges(edges_count, {-1, -1});
    unordered_set<uint64_t> seen; seen.reserve(edges_count*2);

    //connect the source 0 with something
    int zero_outs = max(1l, edges_count/N);
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

Graph random_graph_with_unit_weights(int64_t N, int64_t edges_count, int seed) {
    return random_graph(N, 1, edges_count, seed);
}

Graph grid_graph(int w, int h) {
    return random_graph(10, 10, w*h, 42); // FIXME
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

Graph cylinder_knn_graph(const uint64_t N, double radius, double height, int k, int seed) {
    Graph G(N);
    using Point = tuple<double, double, double>;
    auto euclidian_dist = [](const Point& p1, const Point& p2) {
        return sqrt(pow(get<0>(p1)-get<0>(p2), 2) + pow(get<1>(p1)-get<1>(p2), 2) + pow(get<2>(p1)-get<2>(p2), 2));
    };

    vector<Point> points; points.reserve(N);
    mt19937 rng(seed);
    uniform_real_distribution<> angle_dist(0.0, 2*M_PI);
    uniform_real_distribution<> z_dist(-1*height/2, height/2);

    for (uint64_t i = 0; i < N; i++) {
        double phi = angle_dist(rng);
        double z = z_dist(rng);
        double x = radius*cos(phi);
        double y = radius*sin(phi);
        points.emplace_back(x, y, z);
    }

    for (uint64_t i = 0; i < N; i++) {
        vector<pair<double, uint64_t>> dists; dists.reserve(N);
        for (uint64_t j = 0; j < N; j++) {
            dists.emplace_back(euclidian_dist(points[i], points[j]), j);
        }
        nth_element(dists.begin(), dists.begin()+k+1, dists.end());
        for (uint64_t j = 0; j < k+1; j++) {
            if (i == dists[j].second) {
                continue;
            }
            boost::add_edge(i, dists[j].second, dists[j].first, G);
        }
    }

    return G;
}

inline unordered_map<string, int64_t> extract_params(const string& input) {
    unordered_map<string, int64_t> params;
    regex pattern(R"((\w+)=(\d+))");
    auto begin = sregex_iterator(input.begin(), input.end(), pattern);
    auto end = sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        const smatch& match = *it;
        params[match[1].str()] = stoi(match[2].str());
    }

    return params;
}

/**
 * This function either generates a graph or loads one located in file base the specifications
 * @param specifications a graphml filepath or instructions to generate a graph
 * @return a Graph
 */
Graph go_get_that_graph(const string& specifications) {
    unordered_map<string, int64_t> params;

    try {
        if (specifications.find(".graphml") != string::npos) {
            return FileUtils::read_graphml<BGP_Info>(specifications, false).first;
        }
        params = extract_params(specifications);
        if (specifications.find("random unweighted") != string::npos) {
            return random_graph_with_unit_weights(params["nodes_count"], params["edges_count"], static_cast<int>(params["seed"]));
        }
        if (specifications.find("random") != string::npos) {
            return random_graph(params["nodes_count"], static_cast<int>(params["max_weight"]), params["edges_count"], static_cast<int>(params["seed"]));
        }
        if (specifications.find("grid") != string::npos) {
            return grid_graph(static_cast<int>(params["w"]), static_cast<int>(params["h"]));
        }
        if (specifications.find("metric cylinder") != string::npos) {
            return cylinder_knn_graph(params["nodes_count"], static_cast<double>(params["r"]), static_cast<double>(params["h"]), static_cast<int>(params["k"]), static_cast<int>(params["seed"]));
        }
    } catch (exception& e) {
        throw invalid_argument("Couldn't find a graph based on your specifications check the API of go_get_that_graph");
    }
}
#endif //GRAPH_UTILS_HPP
