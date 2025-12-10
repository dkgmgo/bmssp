/*
 * BMSSP from paper https://arxiv.org/pdf/2504.17033
 */

#include "3.hpp"

#include <cassert>
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

    Path_T(const Dist_T ub, const Node_id_T n) {
        length = ub;
        alpha = 0;
        parent = -1;
        node = n;
    }

    Path_T(Dist_T l, int a, Node_id_T n, Node_id_T p) {
        length = l;
        alpha = a;
        parent = p;
        node = n;
    }

    bool operator==(const Path_T& other) const {
        return length == other.length && alpha == other.alpha && node == other.node;
    }
    bool operator<(const Path_T& other) const {
        if (length != other.length) {
            return length < other.length;
        }
        if (alpha != other.alpha) {
            return alpha < other.alpha;
        }
        return node < other.node;
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

struct BMSSP_State {
    Graph *graph_ptr;
    vector<Path_T> paths;
    vector<unordered_set<Node_id_T>> forest;
    vector<int> in_degree;
    int cd_N;

    explicit BMSSP_State(Graph &g, Node_id_T src) {
        graph_ptr = &g;
        cd_N = boost::num_vertices(*graph_ptr);
        in_degree.assign(cd_N, 0);
        forest.assign(cd_N, unordered_set<Node_id_T>());

        paths.clear();
        paths.reserve(cd_N);
        for (int i = 0; i < cd_N; i++) {
            paths.emplace_back(INF, i);
        }
        paths[src].length = 0;
    }
};

Path_T temp_Path(const BMSSP_State &state, Node_id_T u, Node_id_T v, Dist_T w) {
    return {state.paths[u].length + w, state.paths[u].alpha + 1, v, u};
}

//TODO cache this
int subtree_size_helper(const BMSSP_State &state, Node_id_T node, unordered_set<Node_id_T> &visited) {
    if (visited.count(node)) {
        return 0;
    }
    visited.insert(node);
    int size = 1;
    for (const Node_id_T &v: state.forest[node]) {
        size += subtree_size_helper(state, v, visited);
    }
    return size;
}

int subtree_size(const BMSSP_State &state, Node_id_T node) {
    unordered_set<Node_id_T> visited;
    return subtree_size_helper(state, node, visited);
}

pair<Path_T, unordered_set<Node_id_T>> base_case_of_BMSSP(BMSSP_State &state, int k, const Path_T &B, vector<Node_id_T> &S) {
    assert(S.size() == 1);

    priority_queue<Path_T, vector<Path_T>, greater<>> min_heap;
    min_heap.push(state.paths[S[0]]);

    vector<Node_id_T> U0;

    while (!min_heap.empty() && static_cast<int>(U0.size()) < k+1) {
        auto cur = min_heap.top();
        min_heap.pop();
        Node_id_T u = cur.node;

        if (state.paths[u] > cur) {
            continue;
        }
        // TODO add break condition on k ?
        U0.push_back(u);

        auto outs = boost::out_edges(u, *state.graph_ptr);
        auto ei = outs.first; auto ei_end = outs.second;
        for (; ei != ei_end; ++ei) {
            Node_id_T v = boost::target(*ei, *state.graph_ptr);
            Dist_T w = boost::get(boost::edge_weight, *state.graph_ptr, *ei);
            Path_T temp = temp_Path(state, u, v, w);
            if (temp <= state.paths[v] && temp < B) {
                state.paths[v] = temp;
                min_heap.push(temp);
            }
        }
    }

    if (static_cast<int>(U0.size()) <= k) {
        return {B, unordered_set<Node_id_T>(U0.begin(), U0.end())};
    } else {
        Path_T B_prime = state.paths[U0.back()]; // due to vector and priority queue the max is the last one
        U0.pop_back(); // we have k+1 and we want k elements
        if (U0.size() > k) {
            throw runtime_error("Check your base case logic it's not good");
        }
        return {B_prime, unordered_set<Node_id_T>(U0.begin(), U0.end())};
    }
}

pair<unordered_set<Node_id_T>, unordered_set<Node_id_T>> find_pivots(BMSSP_State &state, int k, const Path_T &B, vector<Node_id_T> &S) {
    unordered_set<Node_id_T> W(S.begin(), S.end());
    unordered_set<Node_id_T> Wi_1(S.begin(), S.end()); // W_0
    unordered_set<Node_id_T> P;

    for (const Node_id_T &u : S) {
        state.in_degree[u] = 0;
        state.forest[u] = unordered_set<Node_id_T>();
    }

    for (int i = 1; i<= k; i++) {
        unordered_set<Node_id_T> Wi;
        for (const Node_id_T &u : Wi_1) {
            auto outs = boost::out_edges(u, *state.graph_ptr);
            auto ei = outs.first; auto ei_end = outs.second;
            for (; ei != ei_end; ++ei) {
                Node_id_T v = boost::target(*ei, *state.graph_ptr);
                Dist_T w = boost::get(boost::edge_weight, *state.graph_ptr, *ei);
                Path_T temp = temp_Path(state, u, v, w);
                if (temp <= state.paths[v]) {
                    state.paths[v] = temp;
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

    for (const Node_id_T &u : W) {
        auto outs = boost::out_edges(u, *state.graph_ptr);
        auto ei = outs.first; auto ei_end = outs.second;
        for (; ei != ei_end; ++ei) {
            Node_id_T v = boost::target(*ei, *state.graph_ptr);
            Dist_T w = boost::get(boost::edge_weight, *state.graph_ptr, *ei);
            Path_T temp = temp_Path(state, u, v, w);
            if (W.count(v) && state.paths[v] == temp) {
                state.forest[u].insert(v);
                state.in_degree[v]++;
            }
        }
    }

    for (const Node_id_T &u : S) {
        if (state.in_degree[u] == 0 && subtree_size(state, u) >= k) {
            P.insert(u);
        }
    }

    return {P, W};
}

pair<Path_T, unordered_set<Node_id_T>> BMSSP(BMSSP_State &state, int t, int k, int l, const Path_T &B, vector<Node_id_T> &S) {
    assert(static_cast<int>(S.size()) <= static_cast<int>(pow(2, l*t)));

    if (l == 0) {
        return base_case_of_BMSSP(state, k, B, S);
    }

    auto piv = find_pivots(state, k, B, S);
    unordered_set<Node_id_T> P = piv.first;
    unordered_set<Node_id_T> W = piv.second;

    int M = static_cast<int>(pow(2, (l - 1) * t));
    BBL_DS<Node_id_T, Path_T> D;
    D.initialize(M, B);
    Path_T B_prime = B;
    for (const Node_id_T &x : P) {
        D.insert_pair({x, state.paths[x]});
        B_prime = min(B_prime, state.paths[x]);
    }
    unordered_set<Node_id_T> U;

    auto max_u_size = k * pow(2, l * t);

    while (static_cast<int>(U.size()) < max_u_size && !D.empty()) {
        Path_T Bi;
        auto Si = D.pull(Bi);

        Path_T prev_B_prime = B_prime;
        auto bmssp = BMSSP(state, t, k, l-1, Bi, Si);
        U.insert(bmssp.second.begin(), bmssp.second.end());
        B_prime = bmssp.first;
        assert(prev_B_prime <= B_prime);

        vector<pair<Node_id_T, Path_T>> K;
        for (const Node_id_T &u : bmssp.second) {
            if (D.contains(u)) {
                D.delete_pair({u, state.paths[u]});
            }
            auto outs = boost::out_edges(u, *state.graph_ptr);
            auto ei = outs.first; auto ei_end = outs.second;
            for (; ei != ei_end; ++ei) {
                Node_id_T v = boost::target(*ei, *state.graph_ptr);
                Dist_T w = boost::get(boost::edge_weight, *state.graph_ptr, *ei);
                Path_T temp = temp_Path(state, u, v, w);
                if (temp <= state.paths[v]) {
                    state.paths[v] = temp;
                    if (Bi <= temp && temp < B) {
                        D.insert_pair({v, temp});
                    } else if (B_prime <= temp && temp < Bi) {
                        K.push_back({v, temp});
                    }
                }
            }
        }
        for (const Node_id_T &x : Si) {
            if (B_prime <= state.paths[x] && state.paths[x] < Bi) {
                K.push_back({x, state.paths[x]});
            }
        }
        D.batch_prepend(K);
    }

    B_prime = min(B_prime, B);
    for (const Node_id_T &x : W) {
        if (state.paths[x] < B_prime) {
            U.insert(x);
        }
    }

    return {B_prime, U};
}

pair<Dist_List_T, Prev_List_T> top_level_BMSSP(Graph& g, Node_id_T src, int N) {
    BMSSP_State state(g, src);
    Path_T B{};
    double log_n = log2(state.cd_N);
    int k = static_cast<int>(floor(pow(log_n, 1.0/3.0))); // work per iteration
    int t = static_cast<int>(floor(pow(log_n, 2.0/3.0)));

    int l = static_cast<int>(ceil(log_n / static_cast<double>(t))); //number of recursions
    vector<Node_id_T> S = {src};

    auto res = BMSSP(state, t, k, l, B, S);

    Dist_List_T dist; dist.reserve(N);
    Prev_List_T parent; parent.reserve(N);
    for (int x = 0; x < N; x++) {
        dist.emplace_back(state.paths[x].length);
        parent.emplace_back(state.paths[x].parent);
    }

    return {dist, parent};
}