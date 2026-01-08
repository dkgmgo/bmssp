/*
 * BMSSP from paper https://arxiv.org/pdf/2504.17033
 */

#include "3.hpp"

#include <cassert>
#include <unordered_set>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>

#include "BBL_DS.hpp"

struct Path_T {
    Dist_T length;
    int alpha; // number of nodes in the path
    Node_id_T node;
    Node_id_T parent;

    Path_T(): length(INF), alpha(0), node(INF), parent(-1) {}
    Path_T(Dist_T ub): length(ub), alpha(0), node(INF), parent(-1) {}
    Path_T(const Dist_T ub, const Node_id_T n): length(ub), alpha(0), node(n), parent(-1) {}
    Path_T(Dist_T l, int a, Node_id_T n, Node_id_T p): length(l), alpha(a), node(n), parent(p) {}

    constexpr bool operator==(const Path_T& other) const noexcept {
        return length == other.length && alpha == other.alpha && node == other.node;
    }
    constexpr bool operator<(const Path_T& other) const noexcept {
        if (length != other.length) {
            return length < other.length;
        }
        if (alpha != other.alpha) {
            return alpha < other.alpha;
        }
        return node < other.node;
    }
    constexpr bool operator!=(const Path_T& other) const noexcept { return !(*this == other); }
    constexpr bool operator>(const Path_T& other)  const noexcept { return other < *this; }
    constexpr bool operator<=(const Path_T& other) const noexcept { return !(other < *this); }
    constexpr bool operator>=(const Path_T& other) const noexcept { return !(*this < other); }

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
    decltype(boost::get(boost::edge_weight, *graph_ptr)) weight_map;
    vector<Path_T> paths;
    vector<vector<Node_id_T>> forest;
    vector<int> in_degree;
    int cd_N;
    vector<Node_id_T> subtree_func_visited_set;
    int subtree_func_visited_token = 1;
    vector<uint8_t> completed_stamp;
    boost::dynamic_bitset<> W, Wi_1, Wi;

    explicit BMSSP_State(Graph &g, Node_id_T src) {
        graph_ptr = &g;
        weight_map = boost::get(boost::edge_weight, *graph_ptr);
        cd_N = boost::num_vertices(*graph_ptr);
        in_degree.assign(cd_N, 0);
        forest.assign(cd_N, vector<Node_id_T>());
        subtree_func_visited_set.assign(cd_N, 0);
        completed_stamp.assign(cd_N, -1);
        W = boost::dynamic_bitset<>(cd_N);
        Wi_1 = boost::dynamic_bitset<>(cd_N);
        Wi = boost::dynamic_bitset<>(cd_N);

        paths.clear();
        paths.reserve(cd_N);
        for (int i = 0; i < cd_N; i++) {
            paths.emplace_back(INF, i);
        }
        paths[src].length = 0;
    }
};

inline Path_T temp_Path(const BMSSP_State &state, Node_id_T u, Node_id_T v, Dist_T w) noexcept {
    return {state.paths[u].length + w, state.paths[u].alpha + 1, v, u};
}

inline bool subtree_size_at_least_k(BMSSP_State &state, Node_id_T node, int k)
{
    if (k >= 2 && state.forest[node].size() < 1) {
        return false;
    }

    auto &visited = state.subtree_func_visited_set;
    const int token = ++state.subtree_func_visited_token;

    vector<Node_id_T> stack;
    stack.reserve(32);

    int count = 0;
    stack.push_back(node);

    while (!stack.empty()) {
        Node_id_T u = stack.back();
        stack.pop_back();

        if (visited[u] == token) {
            continue;
        }

        visited[u] = token;

        if (++count >= k) {
            return true;
        }

        const auto &children = state.forest[u];
        for (Node_id_T v : children) {
            if (visited[v] != token) {
                stack.push_back(v);
            }
        }
    }
    return false;
}

pair<Path_T, vector<Node_id_T>> base_case_of_BMSSP(BMSSP_State &state, int k, const Path_T &B, vector<Node_id_T> &S) {
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
            auto edge = *ei;
            Node_id_T v = edge.m_target;
            Dist_T w = state.weight_map[edge];
            Path_T temp = temp_Path(state, u, v, w);
            if (temp <= state.paths[v] && temp < B) {
                state.paths[v] = temp;
                min_heap.push(temp);
            }
        }
    }

    if (static_cast<int>(U0.size()) <= k) {
        return {B, U0};
    } else {
        Path_T B_prime = state.paths[U0.back()]; //the max is the last one
        U0.pop_back(); // we have k+1 and we want k elements
        if (U0.size() > k) {
            throw runtime_error("Check your base case logic it's not good");
        }
        return {B_prime, U0};
    }
}

pair<vector<Node_id_T>, boost::dynamic_bitset<>> find_pivots(BMSSP_State &state, int k, const Path_T &B, vector<Node_id_T> &S) {
    state.W.reset();
    state.Wi_1.reset();
    vector<Node_id_T> P; P.reserve(S.size());

    for (const Node_id_T &u : S) {
        state.W.set(u);
        state.Wi_1.set(u);
        state.in_degree[u] = 0;
        state.forest[u].clear();
    }

    for (int i = 1; i<= k; i++) {
        state.Wi.reset();
        for (auto u = state.Wi_1.find_first(); u != boost::dynamic_bitset<>::npos; u = state.Wi_1.find_next(u)) {
            auto outs = boost::out_edges(u, *state.graph_ptr);
            auto ei = outs.first; auto ei_end = outs.second;
            for (; ei != ei_end; ++ei) {
                auto edge = *ei;
                Node_id_T v = edge.m_target;
                Dist_T w = state.weight_map[edge];
                Path_T temp = temp_Path(state, u, v, w);
                if (temp <= state.paths[v]) {
                    state.paths[v] = temp;
                    if (temp < B) {
                        state.Wi.set(v);
                    }
                }
            }
        }
        state.W |= state.Wi;
        if (static_cast<int>(state.W.count()) > k * static_cast<int>(S.size())) {
            return {S, state.W};
        }
        state.Wi_1.swap(state.Wi);
    }

    for (auto u = state.W.find_first(); u != boost::dynamic_bitset<>::npos; u = state.W.find_next(u)) {
        auto outs = boost::out_edges(u, *state.graph_ptr);
        auto ei = outs.first; auto ei_end = outs.second;
        for (; ei != ei_end; ++ei) {
            auto edge = *ei;
            Node_id_T v = edge.m_target;
            Dist_T w = state.weight_map[edge];
            Path_T temp = temp_Path(state, u, v, w);
            if (state.W.test(v) && state.paths[v] == temp) {
                state.forest[u].push_back(v);
                state.in_degree[v]++;
            }
        }
    }

    for (const Node_id_T &u : S) {
        if (state.in_degree[u] == 0 && subtree_size_at_least_k(state, u, k)) {
            P.emplace_back(u);
        }
    }

    return {P, state.W};
}

pair<Path_T, vector<Node_id_T>> BMSSP(BMSSP_State &state, int t, int k, int l, const Path_T &B, vector<Node_id_T> &S) {
    assert(static_cast<int>(S.size()) <= static_cast<int>(pow(2, l*t)));

    if (l == 0) {
        return base_case_of_BMSSP(state, k, B, S);
    }

    auto piv = find_pivots(state, k, B, S);

    int M = static_cast<int>(pow(2, (l - 1) * t));
    BBL_DS<Node_id_T, Path_T> D;
    D.initialize(M, B);
    Path_T B_prime = B;
    for (const auto &x : piv.first) {
        D.insert_pair({x, state.paths[x]});
        B_prime = min(B_prime, state.paths[x]);
    }

    int max_u_size = static_cast<int>(k * pow(2, l * t));
    vector<Node_id_T> U; U.reserve(max_u_size);

    while (static_cast<int>(U.size()) < max_u_size && !D.empty()) {
        Path_T Bi;
        auto Si = D.pull(Bi);

        Path_T prev_B_prime = B_prime;
        auto bmssp = BMSSP(state, t, k, l-1, Bi, Si);
        B_prime = bmssp.first;
        assert(prev_B_prime <= B_prime);

        vector<pair<Node_id_T, Path_T>> K;
        for (const Node_id_T &u : bmssp.second) {
            U.push_back(u);
            state.completed_stamp[u] = l;
            D.delete_pair({u, state.paths[u]});

            auto outs = boost::out_edges(u, *state.graph_ptr);
            auto ei = outs.first; auto ei_end = outs.second;
            for (; ei != ei_end; ++ei) {
                auto edge = *ei;
                Node_id_T v = edge.m_target;
                Dist_T w = state.weight_map[edge];
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
    for (auto x = piv.second.find_first(); x != boost::dynamic_bitset<>::npos; x = piv.second.find_next(x)) {
        if (state.paths[x] < B_prime && state.completed_stamp[x] != l) {
            U.push_back(x);
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