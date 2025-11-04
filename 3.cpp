/*
 * BMSSP from paper https://arxiv.org/pdf/2504.17033
 */

#include "3.hpp"

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <boost/heap/fibonacci_heap.hpp>

#include "BBL_DS.hpp"

struct comp {
    bool operator()(const pair<string, Dist_T> &lhs, const pair<string, Dist_T> &rhs) const{
        return lhs.second >= rhs.second;
    }
};

pair<Dist_T, unordered_set<string>> base_case_of_BMSSP(Graph &graph, Dist_List_T &dist, Prev_List_T &pred, int k, double B, vector<string> S) {
    assert(S.size() == 1);

    string s0 = S[0];
    using Heap = boost::heap::fibonacci_heap<pair<string, Dist_T>, boost::heap::compare<comp>>;
    using Handle = Heap::handle_type;

    vector<string> U0;
    Heap H;
    unordered_map<string, Handle> handles;
    handles[s0] = H.push({s0, dist[s0]});
    unordered_map<string, bool> visited;

    while (!H.empty() && static_cast<int>(U0.size()) < k+1) {
        auto cur = H.top();
        H.pop();
        string u = cur.first;

        if (visited[u]) {
            continue;
        }

        visited[u] = true;
        U0.push_back(u);

        vector<string> neis = neighbours(graph, u);
        for (auto v : neis) {
            Dist_T temp = dist[u] + graph[u][v];
            if (temp <= dist[v] && temp < B) {
                dist[v] = temp;
                pred[v] = u;
                if (handles[v].node_) {
                    H.update(handles[v], {v, dist[v]});
                }else {
                    handles[v] = H.push({v, dist[v]});
                }
            }
        }
    }

    if (static_cast<int>(U0.size()) <= k) {
        return {B, unordered_set<string>(U0.begin(), U0.end())};
    } else {
        double B_prime = dist[U0.back()]; // due to vector and priority queue the max is the last one
        U0.pop_back(); // we have k+1 and we want k elements
        return {B_prime, unordered_set<string>(U0.begin(), U0.end())};
    }
}

pair<unordered_set<string>, unordered_set<string>> find_pivots(Graph &graph, Dist_List_T &dist, Prev_List_T &pred, int k, double B, vector<string> S) {
    unordered_set<string> W(S.begin(), S.end());
    unordered_set<string> Wi_1(S.begin(), S.end()); // W_0
    unordered_set<string> P;

    for (int i = 1; i<= k; i++) {
        unordered_set<string> Wi;
        for (string u : Wi_1) {
            vector<string> neis = neighbours(graph, u); // TODO: optimize this
            for (string v : neis) {
                Dist_T temp = dist[u] + graph[u][v];
                if (temp <= dist[v]) {
                    dist[v] = temp;
                    pred[v] = u;
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
        vector<string> neis = neighbours(graph, u);
        for (string v : neis) {
            if (inW[v] && dist[v] == dist[u] + graph[u][v]) {
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

pair<Dist_T, unordered_set<string>> BMSSP(Graph &graph, Dist_List_T &dist, Prev_List_T &pred, int t, int k, int l, double B, vector<string> S) {
    assert(static_cast<int>(S.size()) <= static_cast<int>(pow(2, l*t)));

    if (l <= 0) {
        return base_case_of_BMSSP(graph, dist, pred, k, B, S);
    }

    auto piv = find_pivots(graph, dist, pred, k, B, S);
    unordered_set<string> P = piv.first;
    unordered_set<string> W = piv.second;

    int M = static_cast<int>(pow(2, (l - 1) * t));
    BBL_DS<string, Dist_T> D;
    D.initialize(M, B);
    for (string x : P) {
        D.insert_pair({x, dist[x]});
    }
    int i = 0;
    //TODO: using vector for debugging, remember to clean this after that
    vector<Dist_T> Bs; Bs.push_back(B);
    vector<Dist_T> Bs_prime; Bs_prime.push_back(B);
    vector<vector<string>> Ss; Ss.push_back(S);
    vector<unordered_set<string>> Us; Us.push_back(unordered_set<string>());
    unordered_set<string> U;
    if (P.empty()) {
        Bs_prime[0] = B;
    }else {
        Dist_T mini = INF;
        for (string x : P) {
            mini = min(mini, dist[x]);
        }
        Bs_prime[0] = mini;
    }
    auto max_u_size = k * pow(2, l * t);

    while (static_cast<int>(U.size()) < max_u_size && !D.empty()) {
        i++;
        Dist_T bi;
        auto Si = D.pull(bi);
        Bs.push_back(bi);
        Ss.push_back(Si);

        auto bmssp = BMSSP(graph, dist, pred, t, k, l-1, Bs[i], Ss[i]);
        Bs_prime.push_back(bmssp.first);
        Us.push_back(bmssp.second);
        U.insert(Us[i].begin(), Us[i].end());

        vector<pair<string, Dist_T>> K;
        for (string u : Us[i]) {
            vector<string> neis = neighbours(graph, u);
            for (string v : neis) {
                Dist_T temp = dist[u] + graph[u][v];
                if (temp <= dist[v]) {
                    dist[v] = temp;
                    pred[v] = u;
                    if (temp >= Bs[i] && temp < B) {
                        D.insert_pair({v, temp});
                    } else if (temp >= Bs_prime[i] && temp < Bs[i]) {
                        K.push_back({v, temp});
                    }
                }
            }
        }
        for (string x : Ss[i]) {
            if (dist[x] >= Bs_prime[i] && dist[x] < Bs[i]) {
                K.push_back({x, dist[x]});
            }
        }
        D.batch_prepend(K);
    }
    Dist_T B_prime = min(Bs_prime[i], B);
    for (string x : W) {
        if (dist[x] < B_prime) {
            U.insert(x);
        }
    }

    return {B_prime, U};
}

pair<Dist_List_T, Prev_List_T> top_level_BMSSP(Graph& graph, string src, vector<string> nodes_list) {
    Dist_List_T dist;
    Prev_List_T parent;
    for (auto s: nodes_list) {
        dist[s] = INF;
        parent[s] = "None";
    }
    dist[src] = 0;

    double B = INF;
    double log_n = log2(static_cast<int>(nodes_list.size()));
    int k = static_cast<int>(floor(pow(log_n, 1.0/3.0))); // work per iteration
    int t = static_cast<int>(floor(pow(log_n, 2.0/3.0)));

    int l = static_cast<int>(ceil(log_n / static_cast<double>(t))); //number of recursions
    vector<string> S = {src};

    auto res = BMSSP(graph, dist, parent, t, k, l, B, S);
    return {dist, parent};
}