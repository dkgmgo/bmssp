#include "utils.hpp"

//TODO cache this
vector<Node_id_T> neighbours(Graph& graph, Node_id_T cur) {
    vector<Node_id_T> sortie;
    for (auto p: graph[cur]) {
        sortie.push_back(p.first);
    }
    return sortie;
}

//TODO cache this
int subtree_size_helper(Node_id_T node, unordered_map<Node_id_T, unordered_set<Node_id_T>> &forest, unordered_set<Node_id_T> &visited) {
    if (visited.count(node)) {
        return 0;
    }
    visited.insert(node);
    int size = 1;
    for (Node_id_T v: forest[node]) {
        size += subtree_size_helper(v, forest, visited);
    }
    return size;
}

int subtree_size(Node_id_T node, unordered_map<Node_id_T, unordered_set<Node_id_T>> forest) {
    unordered_set<Node_id_T> visited;
    return subtree_size_helper(node, forest, visited);
}

Graph random_graph(int N, int max_weight, int edges, int seed) {
    Graph graph;

    mt19937 rng(seed);
    uniform_int_distribution<> distr1(1, max_weight);
    uniform_int_distribution<> distr2(0, N-1);
    edges = min(edges, N*(N-1));

    while (edges > 0) {
        int u = distr2(rng);
        int v = distr2(rng);
        int w = distr1(rng);
        if (u != v && !graph[u][v]) {
            graph[u][v] = w;
            edges--;
        }
    }

    return graph;
}

pair<Graph, int> constant_degree_transformation(Graph G, int N) {
    Graph G_prime;
    unordered_map<Node_id_T, string> id_to_name;
    unordered_map<string, Node_id_T> name_to_id;
    int nextId = N-1;

    unordered_map<Node_id_T, vector<Node_id_T>> outgoings;
    unordered_map<Node_id_T, vector<Node_id_T>> incomings;
    for (int i = 0; i < N; i++) {
        outgoings[i] = neighbours(G, i);
        for (Node_id_T nei: outgoings[i]) {
            incomings[nei].push_back(i);
        }
    }

    for (int i = 0; i < N; i++) {
        int out_size = static_cast<int>(outgoings[i].size());
        int in_size = static_cast<int>(incomings[i].size());

        vector<Node_id_T> cycle_nodes;
        if ((out_size + in_size > 3) || (in_size > 2) || (out_size > 2)) {

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
                G_prime[cycle_nodes[j]][cycle_nodes[(j+1)%cycle_size]] = 0;
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
            string A = A_to_B.substr(0, A_to_B.find("_"));
            string B = A_to_B.substr(A_to_B.find("_")+1);
            string dest = "y_"+A_to_B;
            if (name_to_id.find(dest) != name_to_id.end()) {
                G_prime[i][name_to_id[dest]] = G[stoi(A)][stoi(B)];
            }else {
                G_prime[i][stoi(B)] = G[stoi(A)][stoi(B)];
            }
        }else {
            vector<Node_id_T> neis = neighbours(G, i);
            for (Node_id_T j: neis) {
                if (id_to_name.find(j) != id_to_name.end()) {
                    G_prime[i][name_to_id["y_"+to_string(i)+"_"+to_string(j)]] = G[i][j];
                }else {
                    G_prime[i][j] = G[i][j];
                }
            }
        }
    }

    return {G_prime, nextId+1};
}