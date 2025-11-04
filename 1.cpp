/*
 * Dijkstra's algorithm
 * src: https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
 */

#include "1.hpp"

string minDistance(vector<string> queue, Dist_List_T dist) {
    Dist_T mini = INF;
    string sortie;
    for (const string& node: queue) {
        if (dist[node] <= mini) {
            mini = dist[node];
            sortie = node;
        }
    }
    return sortie;
}

pair<Dist_List_T, Prev_List_T> dijkstra(Graph& graph, string src, vector<string> nodes_list) {
    Dist_List_T dist;
    Prev_List_T parent;
    vector<string> queue;
    for (auto s: nodes_list) {
        dist[s] = INF;
        parent[s] = "None";
        queue.push_back(s);
    }

    dist[src] = 0;

    while (!queue.empty()) {

        string cur = minDistance(queue, dist);
        queue.erase(find(queue.begin(), queue.end(), cur));

        vector<string> neis = neighbours(graph, cur);
        for (string nei: neis) {
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
