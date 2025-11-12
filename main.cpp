#include <iostream>
#include <functional>
#include <chrono>
#include <fstream>

#include "1.hpp"
#include "2.hpp"
#include "3.hpp"

using namespace std;

Graph graph;
string src;
Dist_List_T prev_dist;
bool verbose = false;
vector<string> nodes_list;

Graph constant_degree_graph;
vector<string> constant_degree_nodes_list;
Prev_List_T constant_degree_map;

void params(int N, int M) {
    graph = random_graph(N, 10, M, 42);
    /*graph = {
        {"A", {{"B", 10}, {"C", 10}, {"D", 10}, {"E", 10}}},
        {"C", {{"B", 10}}},
        {"D", {{"B", 10}}},
        {"B", {{"F", 10}, {"G", 10}, {"H", 10}}},
        {"E", {{"A", 10}}}
    };
    N=8;*/
    nodes_list = simple_node_list(N);
    auto tup = constant_degree_transformation(graph, N);
    constant_degree_graph = get<0>(tup);
    constant_degree_nodes_list = get<1>(tup);
    constant_degree_map = get<2>(tup);
}

void printResults(Dist_List_T dist, Prev_List_T parent) {
    cout << "Shortest distances from node " << src << ":\n";
    for (auto p : dist) {
        if (p.second == INF)
            cout << "Node " << p.first << ": INF" << " m : " << parent[p.first] << "\n";
        else
            cout << "Node " << p.first << ": " << p.second << " m : " << parent[p.first] << "\n";
    }
}

void export_to_dot_(string filename, bool cd_graph=false) {
    Graph g = cd_graph ? constant_degree_graph : graph;
    ofstream file(filename);
    file << "digraph G {\n";
    for (const auto& line : g) {
        for (const auto& p : line.second) {
            file << "  " << line.first << " -> " << p.first << " [label=\"" << p.second << "\"];\n";
        }
    }
    file << "}\n";
    file.close();
}

void export_to_dot() {
    export_to_dot_("graph.dot");
    export_to_dot_("cd_graph.dot", true);
}

void graph_image() {
    int result = system("dot -Tpng graph.dot -o graph.png");
    if (result != 0) {
        cout << "Error while generating graph image." << endl;
    }
    result = system("dot -Tpng cd_graph.dot -o cd_graph.png");
    if (result != 0) {
        cout << "Error while generating cd_graph image." << endl;
    }
}

pair<double, int> test(function<pair<Dist_List_T, Prev_List_T> (Graph&, string, vector<string>)> algo, bool use_cd = false) {
    Graph g = use_cd ? constant_degree_graph : graph;
    vector<string> n_l = use_cd ? constant_degree_nodes_list : nodes_list;
    src = use_cd ? get_source_from_nodes(constant_degree_nodes_list) : get_source_from_nodes(nodes_list);

    auto t0 = chrono::high_resolution_clock::now();
    pair<Dist_List_T, Prev_List_T> results = algo(g, src, n_l);
    if (use_cd) {
        results = fix_results_after_cd(results, constant_degree_map, nodes_list);
    }
    chrono::duration<double, milli> time_span = chrono::high_resolution_clock::now() - t0;

    //printResults(results.first, results.second);
    double time_span_ms = time_span.count();
    cout << "Time elapsed: " << time_span_ms << " ms" << endl;

    int mismatch = 0;
    if (!prev_dist.empty()) {
        for (auto el: prev_dist) {
            if (el.second != results.first[el.first]) {
                if (verbose) {
                    cout << "Different at " << el.first << " " << el.second << " " << results.first[el.first] << endl;
                }
                mismatch++;
            }
        }
        if (mismatch == 0) {
            cout << "Same results" << endl;
        } else {
            cout << "Different results" << endl;
            cout << "Mismatch: " << mismatch << endl;
        }
    }
    prev_dist = results.first;
    return {time_span_ms, mismatch};
}

/**
 * Finds de next combination of parameters where we have different distances than dijkstra
 * @param N_max
 */
void write_big_file(int N_max) {
    ofstream file("big_file.txt");

    for (int i=3; i<=N_max; i+=10) {
        for (int j=i; j<=3*i; j+=7) {
            params(i, j);
            prev_dist.clear();
            string line;
            auto res = test(min_heap_dijkstra);
            line += "\n Min_heap_dijkstra::  time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
            res = test(fibo_heap_dijkstra);
            line += "\n Fibo_heap_dijkstra:: time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
            double fibo_time = res.first;
            res = test(top_level_BMSSP, true);
            line += "\n BMSSP:: time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
            line += "\n BMSSP_time/Fibo_heap_dijkstra_time = " + to_string(res.first/fibo_time) + "\n";
            file << "\nN: " << i << " M: "<< j << " Results: "<< line;
        }
    }
}

int main() {
    params(1030, 2220);
    export_to_dot();
    graph_image();

    test(dijkstra);
    test(min_heap_dijkstra);
    verbose = true;
    test(fibo_heap_dijkstra);
    //verbose = false;
    test(top_level_BMSSP, true);
    //write_big_file(20000);

    return 0;
}