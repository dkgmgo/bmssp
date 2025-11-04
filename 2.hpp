#ifndef DIJKSTRA_2_HPP
#define DIJKSTRA_2_HPP

#include "utils.hpp"

pair<Dist_List_T, Prev_List_T> min_heap_dijkstra(Graph& graph, string src, vector<string> nodes_list);

pair<Dist_List_T, Prev_List_T> fibo_heap_dijkstra(Graph& graph, string src, vector<string> nodes_list);

#endif