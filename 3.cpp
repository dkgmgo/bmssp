/*
 * BMSSP from paper https://arxiv.org/pdf/2504.17033
 */

#include "3.hpp"

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <boost/heap/fibonacci_heap.hpp>

#include "BBL_DS.hpp"

struct Path_T {
    Dist_T length;
    Node_id_T node;
    Node_id_T parent;
    int alpha; // number of nodes in the path

    Path_T() {
        length = INF;
        parent = "None";
        alpha = 0;
        node = "ZZ";
    }

    Path_T(Dist_T ub) {
        length = ub;
        alpha = 0;
        parent = "None";
        node = "ZZ";
    }

    Path_T(Dist_T l, int a, Node_id_T n, Node_id_T p) {
        length = l;
        alpha = a;
        parent = p;
        node = n;
    }

    bool operator==(const Path_T& other) const {
        return tie(length, alpha, node)
             == tie(other.length, other.alpha, other.node);
    }
    bool operator<(const Path_T& other) const {
        return tie(length, alpha, node)
             < tie(other.length, other.alpha, other.node);
    }
    bool operator!=(const Path_T& other) const { return !(*this == other); }
    bool operator>(const Path_T& other)  const { return other < *this; }
    bool operator<=(const Path_T& other) const { return !(other < *this); }
    bool operator>=(const Path_T& other) const { return !(*this < other); }

    string operator+(const string& other) const {
        return other + "Length: " + to_string(length) + " Alpha: " + to_string(alpha) + " Parent: " + parent + " Node: " + node;
    }
};

Graph graph_;
unordered_map<Node_id_T, Path_T> paths;
std::unordered_map<std::string, Node_id_T> nodeToId;

void relax_an_edge(Node_id_T u, Node_id_T v) {
    paths[v].length = paths[u].length + graph_[u][v];
    paths[v].alpha = paths[u].alpha + 1;
    paths[v].parent = paths[u].node;
}

Path_T temp_Path(Node_id_T u, Node_id_T v) {
    return Path_T(paths[u].length + graph_[u][v], paths[u].alpha + 1, v, u);
}

pair<Path_T, unordered_set<string>> base_case_of_BMSSP(int k, Path_T B, vector<string> S) {
    assert(S.size() == 1);

    string s0 = S[0];
    using Heap = boost::heap::fibonacci_heap<Path_T, boost::heap::compare<greater<>>>;
    using Handle = Heap::handle_type;

    vector<string> U0;
    Heap H;
    unordered_map<string, Handle> handles;
    handles[s0] = H.push(paths[s0]);
    unordered_map<string, bool> visited;

    while (!H.empty() && static_cast<int>(U0.size()) < k+1) {
        auto cur = H.top();
        H.pop();
        string u = cur.node;

        if (visited[u]) {
            continue;
        }
        // TODO add break condition on k

        visited[u] = true;
        U0.push_back(u);

        vector<string> neis = neighbours(graph_, u);
        for (auto v : neis) {
            Path_T temp = temp_Path(u, v);
            if (temp <= paths[v] && temp < B) {
                relax_an_edge(u, v);
                if (handles[v].node_) {
                    H.update(handles[v], paths[v]);
                }else {
                    handles[v] = H.push(paths[v]);
                }
            }
        }
    }

    if (static_cast<int>(U0.size()) <= k) {
        return {B, unordered_set<string>(U0.begin(), U0.end())};
    } else {
        Path_T B_prime = paths[U0.back()]; // due to vector and priority queue the max is the last one
        U0.pop_back(); // we have k+1 and we want k elements
        if (U0.size() > k) {
            throw runtime_error("Check your base case logic it's not good");
        }
        return {B_prime, unordered_set<string>(U0.begin(), U0.end())};
    }
}

pair<unordered_set<string>, unordered_set<string>> find_pivots(int k, Path_T B, vector<string> S) {
    unordered_set<string> W(S.begin(), S.end());
    unordered_set<string> Wi_1(S.begin(), S.end()); // W_0
    unordered_set<string> P;

    for (int i = 1; i<= k; i++) {
        unordered_set<string> Wi;
        for (string u : Wi_1) {
            vector<string> neis = neighbours(graph_, u); // TODO: optimize this
            for (string v : neis) {
                Path_T temp = temp_Path(u, v);
                if (temp <= paths[v]) {
                    relax_an_edge(u, v);
                    if (temp < B) {
                        Wi.insert(v);
                    }
                }
            }
        }
        W.insert(Wi.begin(), Wi.end());
        if (static_cast<int>(W.size()) > k * static_cast<int>(S.size())) {
            P.insert(S.begin(), S.end());
            return {P, W};
        }
        Wi_1.swap(Wi);
    }

    unordered_map<string, unordered_set<string>> F;
    Dist_List_T in_degree;
    unordered_map<string, bool> inW;

    for (string u : W) {
        in_degree[u] = 0;
        inW[u] = true;
        F[u] = unordered_set<string>();
    }

    for (string u : W) {
        vector<string> neis = neighbours(graph_, u);
        for (string v : neis) {
            Path_T temp = temp_Path(u, v);
            if (inW[v] && paths[v] == temp) {
                F[u].insert(v);
                in_degree[v]++;
            }
        }
    }

    for (string u : S) {
        if (in_degree[u] == 0 && subtree_size(u, F) >= k) {
            P.insert(u);
        }
    }

    return {P, W};
}

pair<Path_T, unordered_set<string>> BMSSP(int t, int k, int l, Path_T B, vector<string> S) {
    assert(static_cast<int>(S.size()) <= static_cast<int>(pow(2, l*t)));

    if (l == 0) {
        return base_case_of_BMSSP(k, B, S);
    }

    auto piv = find_pivots(k, B, S);
    unordered_set<string> P = piv.first;
    unordered_set<string> W = piv.second;

    int M = static_cast<int>(pow(2, (l - 1) * t));
    BBL_DS<string, Path_T> D;
    D.initialize(M, B);
    Path_T B_prime = B;
    for (string x : P) {
        D.insert_pair({x, paths[x]});
        B_prime = min(B_prime, paths[x]);
    }
    unordered_set<string> U;

    auto max_u_size = k * pow(2, l * t);

    while (static_cast<int>(U.size()) < max_u_size && !D.empty()) {
        Path_T Bi;
        auto Si = D.pull(Bi);

        Path_T prev_B_prime = B_prime;
        auto bmssp = BMSSP(t, k, l-1, Bi, Si);
        U.insert(bmssp.second.begin(), bmssp.second.end());
        B_prime = bmssp.first;
        assert(prev_B_prime <= B_prime);

        vector<pair<string, Path_T>> K;
        for (string u : bmssp.second) {
            if (D.contains(u)) {
                D.delete_pair({u, paths[u]});
            }
            vector<string> neis = neighbours(graph_, u);
            for (string v : neis) {
                Path_T temp = temp_Path(u, v);
                if (temp <= paths[v]) {
                    relax_an_edge(u, v);
                    if (Bi <= temp && temp < B) {
                        D.insert_pair({v, temp});
                    } else if (B_prime <= temp && temp < Bi) {
                        K.push_back({v, temp});
                    }
                }
            }
        }
        for (string x : Si) {
            if (B_prime <= paths[x] && paths[x] < Bi) {
                K.push_back({x, paths[x]});
            }
        }
        D.batch_prepend(K);
    }

    B_prime = min(B_prime, B);
    for (string x : W) {
        if (paths[x] < B_prime) {
            U.insert(x);
        }
    }

    return {B_prime, U};
}

pair<Dist_List_T, Prev_List_T> top_level_BMSSP(Graph& g, string src, vector<string> nodes_list) {
    graph_ = g;
    for (auto s: nodes_list) {
        paths[s] = Path_T();
        paths[s].node = s;
    }
    paths[src].length = 0;

    Path_T B(INF);
    double log_n = log2(static_cast<int>(nodes_list.size()));
    int k = static_cast<int>(floor(pow(log_n, 1.0/3.0))); // work per iteration
    int t = static_cast<int>(floor(pow(log_n, 2.0/3.0)));

    int l = static_cast<int>(ceil(log_n / static_cast<double>(t))); //number of recursions
    vector<string> S = {src};

    auto res = BMSSP(t, k, l, B, S);

    Dist_List_T dist;
    Prev_List_T pred;
    for (string x: nodes_list) {
        dist[x] = paths[x].length;
        pred[x] = paths[x].parent;
    }

    return {dist, pred};
}