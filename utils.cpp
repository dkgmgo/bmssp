#include "utils.hpp"

//TODO cache this
vector<string> neighbours(Graph& graph, string cur) {
    vector<string> sortie;
    for (auto p: graph[cur]) {
        sortie.push_back(p.first);
    }
    return sortie;
}


//TODO cache this
int subtree_size_helper(string node, unordered_map<string, unordered_set<string>> &forest, unordered_set<string> &visited) {
    if (visited.count(node)) {
        return 0;
    }
    visited.insert(node);
    int size = 1;
    for (string v: forest[node]) {
        size += subtree_size_helper(v, forest, visited);
    }
    return size;
}

int subtree_size(string node, unordered_map<string, unordered_set<string>> forest) {
    unordered_set<string> visited;
    return subtree_size_helper(node, forest, visited);
}

string int_to_label(int i) {
    char l = 'A' + i%26;
    int suf = i/26;
    if (suf == 0) {
        return string(1, l);
    } else {
        return string(1, l) + to_string(suf);
    }
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
        string s_u = int_to_label(u);
        string s_v = int_to_label(v);
        if (u != v && !graph[s_u][s_v]) {
            graph[s_u][s_v] = w;
            edges--;
        }
    }

    return graph;
}

vector<int> random_array(int N, int max_val, int seed) {
    mt19937 rng(seed);
    uniform_int_distribution<int> distr(0, max_val);
    vector<int> A;
    A.reserve(N);

    for (int i = 0; i < N; i++) {
        A.push_back(distr(rng));
    }

    return A;
}

tuple<Graph, vector<string>, Prev_List_T> constant_degree_transformation(Graph G, int N) {
    Graph G_prime;
    vector<string> nodes_list;
    Prev_List_T cd_map;

    unordered_map<string, vector<string>> outgoings;
    unordered_map<string, vector<string>> incomings;
    for (int i = 0; i < N; i++) {
        string s = int_to_label(i);
        nodes_list.push_back(s);
        outgoings[s] = neighbours(G, s);
        for (string nei: outgoings[s]) {
            incomings[nei].push_back(s);
        }
    }

    for (int i = 0; i < N; i++) {
        string s = int_to_label(i);
        int out_size = static_cast<int>(outgoings[s].size());
        int in_size = static_cast<int>(incomings[s].size());

        vector<string> cycle_nodes;
        if ((out_size + in_size > 3) || (in_size > 2) || (out_size > 2)) {

            nodes_list.erase(find(nodes_list.begin(), nodes_list.end(), s));

            for (string desti : outgoings[s]) {
                string new_node = "x_"+s+"_"+desti;
                nodes_list.push_back(new_node);
                cycle_nodes.push_back(new_node);
                cd_map[new_node] = s;
            }
            for (string source : incomings[s]) {
                string new_node = "y_"+source+"_"+s;
                nodes_list.push_back(new_node);
                cycle_nodes.push_back(new_node);
                cd_map[new_node] = s;
            }

            int cycle_size = static_cast<int>(cycle_nodes.size());
            for (int j = 0; j < cycle_size; j++) {
                G_prime[cycle_nodes[j]][cycle_nodes[(j+1)%cycle_size]] = 0;
            }
        }
    }

    unordered_map<string, bool> dest_in_list;
    for (auto u: nodes_list) {
        dest_in_list[u] = true;
    }
    for (auto u: nodes_list) {
        if (u.find("y_") != string::npos) {
            continue;
        }

        if (u.find("x_") != string::npos) {
            string A_to_B = u.substr(u.find("_")+1);
            string A = A_to_B.substr(0, A_to_B.find("_"));
            string B = A_to_B.substr(A_to_B.find("_")+1);
            string dest = "y_"+A_to_B;
            if (dest_in_list[dest]) {
                G_prime[u][dest] = G[A][B];
            }else {
                G_prime[u][B] = G[A][B];
            }
        }else {
            vector<string> nei = neighbours(G, u);
            for (auto v: nei) {
                if (find(nodes_list.begin(), nodes_list.end(), v) != nodes_list.end()) {
                    G_prime[u][v] = G[u][v];
                }else {
                    G_prime[u]["y_"+u+"_"+v] = G[u][v];
                }
            }
        }
    }

    return {G_prime, nodes_list, cd_map};
}

string get_source_from_nodes(vector<string> nodes_list) {
    for (string s: nodes_list) {
        if (s == "A") {
            return s;
        }
        if (s.find("x_A_") != string::npos) {
            return s;
        }
    }
    return nodes_list[0];
}

vector<string> simple_node_list(int N) {
    vector<string> sortie;
    for (int i=0; i<N; i++) {
        sortie.push_back(int_to_label(i));
    }
    return sortie;
}

pair<Dist_List_T, Prev_List_T> fix_results_after_cd(pair<Dist_List_T, Prev_List_T> results, Prev_List_T cd_map, vector<string> nodes_list) {
    Dist_List_T new_dist;
    Prev_List_T new_prev;

    for (auto s: nodes_list) {
        new_dist[s] = INF;
        new_prev[s] = "None";
    }

    for (auto p: results.first) {
        Dist_T d = p.second;
        string prev = results.second[p.first];
        string node = cd_map[p.first].empty() ? p.first : cd_map[p.first];
        while (cd_map[prev] == node) {
            prev = results.second[prev];
        }
        if (!cd_map[prev].empty()) {
            prev = cd_map[prev];
        }

        if (find(nodes_list.begin(), nodes_list.end(), p.first) != nodes_list.end()) {
            new_dist[p.first] = d;
            new_prev[p.first] = prev;
        } else {
            if (p.second < new_dist[cd_map[p.first]]) {
                new_dist[cd_map[p.first]] = d;
                new_prev[cd_map[p.first]] = prev;
            }
        }
    }

    return {new_dist, new_prev};
}