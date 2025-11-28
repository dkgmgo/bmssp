#ifndef DIJKSTRA_RUNNER_HPP
#define DIJKSTRA_RUNNER_HPP

#include <functional>
#include <chrono>
#include <fstream>

#include "utils.hpp"
#include "3.hpp"
#include "2.hpp"
#include "1.hpp"

struct Runner {
    Graph graph;
    vector<string> nodes_list;
    Graph constant_degree_graph;
    vector<string> constant_degree_nodes_list;
    Prev_List_T constant_degree_map;
    string src;
    Dist_List_T prev_dist;
    bool verbose = true;

    void initialize(int N, int M) {
        graph = random_graph(N, 10, M, 42);
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

    void export_to_dot_(const string &filename, bool cd_graph=false) {
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

    pair<double, int> run_test(function<pair<Dist_List_T, Prev_List_T> (Graph&, string, vector<string>)> algo, bool use_cd = false) {
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
                cout << "Mismatch: " << mismatch << endl;
                throw runtime_error("Different results");
            }
        }
        prev_dist = results.first;
        return {time_span_ms, mismatch};
    }

    void quicktest(int N, int M) {
        initialize(N, M);
        //export_to_dot();
        //graph_image();

        //run_test(dijkstra);
        run_test(min_heap_dijkstra);
        verbose = true;
        run_test(fibo_heap_dijkstra);
        //verbose = false;
        run_test(top_level_BMSSP, false);
        run_test(top_level_BMSSP, true);
    }

    /**
     * Finds de next combination of parameters where we have different distances than dijkstra
     * @param N_max
     */
    void write_big_file(int N_max) {
        ofstream file("big_file.txt");

        for (int i=2; i<=N_max; i+=52) {
            for (int j=i; j<=3*i; j+=86) {
                initialize(i, j);
                cout << "N: " << i << " M: "<< j << endl;
                prev_dist.clear();
                string line;
                auto res = run_test(min_heap_dijkstra);
                line += "\n Min_heap_dijkstra::  time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                res = run_test(fibo_heap_dijkstra);
                line += "\n Fibo_heap_dijkstra:: time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                double fibo_time = res.first;
                res = run_test(top_level_BMSSP, true);
                line += "\n BMSSP:: time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                line += "\n BMSSP_time/Fibo_heap_dijkstra_time = " + to_string(res.first/fibo_time) + "\n";
                file << "\nN: " << i << " M: "<< j << " Results: "<< line;
            }
        }
    }
};

#endif