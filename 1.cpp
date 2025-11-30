/*
 * Dijkstra's algorithm
 * src: https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
 */

#include "1.hpp"

Node_id_T minDistance(vector<Node_id_T> queue, Dist_List_T dist) {
    Dist_T mini = INF;
    Node_id_T sortie;
    for (const Node_id_T& node: queue) {
        if (dist[node] <= mini) {
            mini = dist[node];
            sortie = node;
        }
    }
    return sortie;
}

pair<Dist_List_T, Prev_List_T> dijkstra(Graph& graph, Node_id_T src, int N) {
    Dist_List_T dist;
    Prev_List_T parent;
    vector<Node_id_T> queue;
    for (int i=0; i < N; i++) {
        dist[i] = INF;
        parent[i] = -1;
        queue.push_back(i);
    }

    dist[src] = 0;

    while (!queue.empty()) {

        Node_id_T cur = minDistance(queue, dist);
        queue.erase(find(queue.begin(), queue.end(), cur));

        vector<Node_id_T> neis = neighbours(graph, cur);
        for (Node_id_T nei: neis) {
            Dist_T temp = dist[cur] + graph[cur][nei];
            bool not_visited = find(queue.begin(), queue.end(), nei) != queue.end();
            if (not_visited && temp < dist[nei]) {
                dist[nei] = temp;
                parent[nei] = cur;
            }
        }
    }

    return {dist, parent};
}
