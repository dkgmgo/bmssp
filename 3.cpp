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
        parent = -1;
        alpha = 0;
        node = INF;
    }

    Path_T(Dist_T ub) {
        length = ub;
        alpha = 0;
        parent = -1;
        node = INF;
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

    friend ostream& operator<<(ostream& os, const Path_T& p) {
        os << "{length=" << p.length
           << ", node=" << p.node
           << ", parent=" << p.parent
           << ", alpha=" << p.alpha
           << "}";
        return os;
    }
};

Graph graph_;
unordered_map<Node_id_T, Path_T> paths;

Path_T temp_Path(Node_id_T u, Node_id_T v, Dist_T w) {
    return {paths[u].length + w, paths[u].alpha + 1, v, u};
}

pair<Path_T, unordered_set<Node_id_T>> base_case_of_BMSSP(int k, Path_T B, vector<Node_id_T> S) {
    assert(S.size() == 1);

    Node_id_T s0 = S[0];
    using Heap = boost::heap::fibonacci_heap<Path_T, boost::heap::compare<greater<>>>;
    using Handle = Heap::handle_type;

    vector<Node_id_T> U0;
    Heap H;
    unordered_map<Node_id_T, Handle> handles;
    handles[s0] = H.push(paths[s0]);
    unordered_map<Node_id_T, bool> visited;

    while (!H.empty() && static_cast<int>(U0.size()) < k+1) {
        auto cur = H.top();
        H.pop();
        Node_id_T u = cur.node;

        if (visited[u]) {
            continue;
        }
        // TODO add break condition on k ?

        visited[u] = true;
        U0.push_back(u);

        auto outs = boost::out_edges(u, graph_);
        auto ei = outs.first; auto ei_end = outs.second;
        for (; ei != ei_end; ++ei) {
            Node_id_T v = boost::target(*ei, graph_);
            Dist_T w = boost::get(boost::edge_weight, graph_, *ei);
            Path_T temp = temp_Path(u, v, w);
            if (temp <= paths[v] && temp < B) {
                paths[v] = temp;
                if (handles[v].node_) {
                    H.update(handles[v], paths[v]);
                }else {
                    handles[v] = H.push(paths[v]);
                }
            }
        }
    }

    if (static_cast<int>(U0.size()) <= k) {
        return {B, unordered_set<Node_id_T>(U0.begin(), U0.end())};
    } else {
        Path_T B_prime = paths[U0.back()]; // due to vector and priority queue the max is the last one
        U0.pop_back(); // we have k+1 and we want k elements
        if (U0.size() > k) {
            throw runtime_error("Check your base case logic it's not good");
        }
        return {B_prime, unordered_set<Node_id_T>(U0.begin(), U0.end())};
    }
}

pair<unordered_set<Node_id_T>, unordered_set<Node_id_T>> find_pivots(int k, Path_T B, vector<Node_id_T> S) {
    unordered_set<Node_id_T> W(S.begin(), S.end());
    unordered_set<Node_id_T> Wi_1(S.begin(), S.end()); // W_0
    unordered_set<Node_id_T> P;

    for (int i = 1; i<= k; i++) {
        unordered_set<Node_id_T> Wi;
        for (Node_id_T u : Wi_1) {
            auto outs = boost::out_edges(u, graph_);
            auto ei = outs.first; auto ei_end = outs.second;
            for (; ei != ei_end; ++ei) {
                Node_id_T v = boost::target(*ei, graph_);
                Dist_T w = boost::get(boost::edge_weight, graph_, *ei);
                Path_T temp = temp_Path(u, v, w);
                if (temp <= paths[v]) {
                    paths[v] = temp;
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

    // TODO check boost unordered_map wich is faster an other alternatives
    unordered_map<Node_id_T, unordered_set<Node_id_T>> F;
    unordered_map<Node_id_T, int> in_degree;
    unordered_map<Node_id_T, bool> inW;

    for (Node_id_T u : W) {
        in_degree[u] = 0;
        inW[u] = true;
        F[u] = unordered_set<Node_id_T>();
    }

    for (Node_id_T u : W) {
        auto outs = boost::out_edges(u, graph_);
        auto ei = outs.first; auto ei_end = outs.second;
        for (; ei != ei_end; ++ei) {
            Node_id_T v = boost::target(*ei, graph_);
            Dist_T w = boost::get(boost::edge_weight, graph_, *ei);
            Path_T temp = temp_Path(u, v, w);
            if (inW[v] && paths[v] == temp) {
                F[u].insert(v);
                in_degree[v]++;
            }
        }
    }

    for (Node_id_T u : S) {
        if (in_degree[u] == 0 && subtree_size(u, F) >= k) {
            P.insert(u);
        }
    }

    return {P, W};
}

pair<Path_T, unordered_set<Node_id_T>> BMSSP(int t, int k, int l, Path_T B, vector<Node_id_T> S) {
    assert(static_cast<int>(S.size()) <= static_cast<int>(pow(2, l*t)));

    if (l == 0) {
        return base_case_of_BMSSP(k, B, S);
    }

    auto piv = find_pivots(k, B, S);
    unordered_set<Node_id_T> P = piv.first;
    unordered_set<Node_id_T> W = piv.second;

    int M = static_cast<int>(pow(2, (l - 1) * t));
    BBL_DS<Node_id_T, Path_T> D;
    D.initialize(M, B);
    Path_T B_prime = B;
    for (Node_id_T x : P) {
        D.insert_pair({x, paths[x]});
        B_prime = min(B_prime, paths[x]);
    }
    unordered_set<Node_id_T> U;

    auto max_u_size = k * pow(2, l * t);

    while (static_cast<int>(U.size()) < max_u_size && !D.empty()) {
        Path_T Bi;
        auto Si = D.pull(Bi);

        Path_T prev_B_prime = B_prime;
        auto bmssp = BMSSP(t, k, l-1, Bi, Si);
        U.insert(bmssp.second.begin(), bmssp.second.end());
        B_prime = bmssp.first;
        assert(prev_B_prime <= B_prime);

        vector<pair<Node_id_T, Path_T>> K;
        for (Node_id_T u : bmssp.second) {
            if (D.contains(u)) {
                D.delete_pair({u, paths[u]});
            }
            auto outs = boost::out_edges(u, graph_);
            auto ei = outs.first; auto ei_end = outs.second;
            for (; ei != ei_end; ++ei) {
                Node_id_T v = boost::target(*ei, graph_);
                Dist_T w = boost::get(boost::edge_weight, graph_, *ei);
                Path_T temp = temp_Path(u, v, w);
                if (temp <= paths[v]) {
                    paths[v] = temp;
                    if (Bi <= temp && temp < B) {
                        D.insert_pair({v, temp});
                    } else if (B_prime <= temp && temp < Bi) {
                        K.push_back({v, temp});
                    }
                }
            }
        }
        for (Node_id_T x : Si) {
            if (B_prime <= paths[x] && paths[x] < Bi) {
                K.push_back({x, paths[x]});
            }
        }
        D.batch_prepend(K);
    }

    B_prime = min(B_prime, B);
    for (Node_id_T x : W) {
        if (paths[x] < B_prime) {
            U.insert(x);
        }
    }

    return {B_prime, U};
}

pair<Dist_List_T, Prev_List_T> top_level_BMSSP(Graph& g, Node_id_T src, int N) {
    graph_ = g;
    int cd_N = boost::num_vertices(graph_);
    for (int i = 0; i < cd_N; i++) {
        paths[i] = Path_T();
        paths[i].node = i;
    }
    paths[src].length = 0;

    Path_T B(INF);
    double log_n = log2(cd_N);
    int k = static_cast<int>(floor(pow(log_n, 1.0/3.0))); // work per iteration
    int t = static_cast<int>(floor(pow(log_n, 2.0/3.0)));

    int l = static_cast<int>(ceil(log_n / static_cast<double>(t))); //number of recursions
    vector<Node_id_T> S = {src};

    auto res = BMSSP(t, k, l, B, S);

    Dist_List_T dist(N, INF);
    Prev_List_T parent(N, -1);
    for (int x = 0; x < N; x++) {
        dist[x] = paths[x].length;
        parent[x] = paths[x].parent;
    }

    return {dist, parent};
}