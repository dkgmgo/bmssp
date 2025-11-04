/*
 *Min heap and Fibonacci heap Dijkstra
 *src: https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
 */

#include "2.hpp"

struct Node {
    string name{};
    double distance{};

    friend bool operator<(const Node& lhs, const Node& rhs) {
        return lhs.distance >= rhs.distance;
    }
};

pair<Dist_List_T, Prev_List_T> min_heap_dijkstra(Graph& graph, string src, vector<string> nodes_list) {
    Dist_List_T dist;
    Prev_List_T parent;
    unordered_map<string, bool> visited;

    priority_queue<Node> min_heap;
    for (auto s_i: nodes_list) {
        dist[s_i] = INF;
        parent[s_i] = "None";
    }
    dist[src] = 0;
    min_heap.push(Node{src, dist[src]});

    while (!min_heap.empty()) {
        Node cur = min_heap.top();
        min_heap.pop();
        if (visited[cur.name]) {
            continue;
        }
        visited[cur.name] = true;

        vector<string> neis = neighbours(graph, cur.name);
        for (auto nei : neis) {
            Dist_T temp = cur.distance + graph[cur.name][nei];

            if (!visited[nei] && temp < dist[nei]) {
                parent[nei] = cur.name;
                dist[nei] = temp;
                min_heap.push(Node{nei, dist[nei]});
            }
        }
    }

    return {dist, parent};
}


#include <boost/heap/fibonacci_heap.hpp>

pair<Dist_List_T, Prev_List_T> fibo_heap_dijkstra(Graph& graph, string src, vector<string> nodes_list) {

    using Heap = boost::heap::fibonacci_heap<Node>;
    using Handle = Heap::handle_type;

    Dist_List_T dist;
    Prev_List_T parent;
    unordered_map<string, bool> visited;
    for (auto s_i: nodes_list) {
        dist[s_i] = INF;
        parent[s_i] = "None";
        visited[s_i] = false;
    }
    dist[src] = 0;

    Heap H;
    unordered_map<string, Handle> handles(static_cast<int>(nodes_list.size()));
    handles[src] = H.push({src, dist[src]});

    while (!H.empty()) {
        Node cur = H.top();
        H.pop();
        if (visited[cur.name]) {
            continue;
        }
        visited[cur.name] = true;

        vector<string> neis = neighbours(graph, cur.name);
        for (auto nei : neis) {
            Dist_T temp = cur.distance + graph[cur.name][nei];

            if (!visited[nei] && temp < dist[nei]) {
                parent[nei] = cur.name;
                dist[nei] = temp;
                if (handles[nei].node_) {
                    H.update(handles[nei], {nei, dist[nei]});
                }else {
                    handles[nei] = H.push({nei, dist[nei]});
                }
            }
        }
    }

    return {dist, parent};
}
