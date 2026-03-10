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

Graph random_barabasi_albert(int m0, int m, int64_t t, int max_weight, int seed) {
    if (m > m0) {
        throw invalid_argument("Number of edges per new node cannot be greater than initial number of nodes.");
    }

    if (t < 0) {
        throw invalid_argument("Number of steps can't be negative");
    }

    int64_t N = m0+t;
    int64_t M = 2*m*t + m0*(m0-1);
    Graph G(N);
    vector<Node_id_T> repeated_nodes; repeated_nodes.reserve(M);
    mt19937_64 rng(seed);
    uniform_real_distribution<> weight_dist(1, max_weight);

    for (int i = 0; i < m0; i++) {
        for (int j = i+1; j < m0; j++) {
            Dist_T w = weight_dist(rng);
            boost::add_edge(i, j, w, G);
            boost::add_edge(j, i, w, G);

            repeated_nodes.push_back(i);
            repeated_nodes.push_back(j);
        }
    }

    if (repeated_nodes.empty()) {
        repeated_nodes.push_back(0);
    }

    for (Node_id_T u = m0; u < N; u++) {
        unordered_set<int64_t> chosen;

        while (chosen.size() < m) {
            uniform_int_distribution<size_t> dist(0, repeated_nodes.size()-1);
            Node_id_T v = repeated_nodes[dist(rng)];

            if (v != u && chosen.insert(v).second) {
                Dist_T w = weight_dist(rng);
                boost::add_edge(u, v, w, G);
                boost::add_edge(v, u, w, G);

                repeated_nodes.push_back(u);
                repeated_nodes.push_back(v);
            }
        }
    }

    return G;
}

Graph random_graph(int64_t N, int max_weight, int seed) {
    return random_barabasi_albert(1, 1, N-1, max_weight, seed);
}

Graph random_graph_with_unit_weights(int64_t N, int seed) {
    return random_barabasi_albert(1, 1, N-1, 1, seed);
}

Graph grid_graph(int w, int h) {
    if (w <= 1 || h <= 1) {
        throw invalid_argument("Minimum grid is a 2x2");
    }

    Graph G(w*h);
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            Node_id_T id = i*h+j;
            if (j > 0) {
                boost::add_edge(id-1, id, 1, G);
                boost::add_edge(id, id-1, 1, G);
            }
            if (i > 0) {
                boost::add_edge(id-h, id, 1, G);
                boost::add_edge(id, id-h, 1, G);
            }
        }
    }
    return G;
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
            return random_graph_with_unit_weights(params["nodes_count"], static_cast<int>(params["seed"]));
        }
        if (specifications.find("random") != string::npos) {
            return random_graph(params["nodes_count"], static_cast<int>(params["max_weight"]), static_cast<int>(params["seed"]));
        }
        if (specifications.find("grid") != string::npos) {
            return grid_graph(static_cast<int>(params["w"]), static_cast<int>(params["h"]));
        }
        if (specifications.find("metric cylinder") != string::npos) {
            return cylinder_knn_graph(params["nodes_count"], static_cast<double>(params["r"]), static_cast<double>(params["h"]), static_cast<int>(params["k"]), static_cast<int>(params["seed"]));
        }
    } catch (exception& e) {
        throw invalid_argument("Couldn't find a graph based on your specifications check the API of go_get_that_graph. This error happened:\n" + string(e.what()));
    }
}
#endif //GRAPH_UTILS_HPP
