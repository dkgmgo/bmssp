#ifndef DIJKSTRA_RUNNER_HPP
#define DIJKSTRA_RUNNER_HPP

#include <functional>
#include <chrono>
#include <fstream>

#include "utils.hpp"
#include "file_utils.hpp"
#include "3.hpp"
#include "2.hpp"
#include "1.hpp"

struct Runner {
    Graph graph;
    int N;
    Graph constant_degree_graph;
    Node_id_T src = 0;
    Dist_List_T prev_dist;
    bool verbose = true;

    void initialize(int N_, int M) {
        prev_dist.clear();
        N=N_;
        graph = random_graph(N, 10, M, 42);
        auto cd = constant_degree_transformation(graph, N);
        constant_degree_graph = cd.first;
    }

    void initialize(const string &filename) {
        prev_dist.clear();
        auto in = FileUtils::read_bgp_graphml(filename);
        graph = in.first;
        N = in.second;
        auto cd = constant_degree_transformation(graph, N);
        constant_degree_graph = cd.first;
    }

    void printResults(Dist_List_T dist, Prev_List_T parent) {
        cout << "Shortest distances from node " << src << ":\n";
        for (int i=0; i<N; i++) {
            if (dist[i] == INF)
                cout << "Node " << i << ": INF" << " m : " << parent[i] << "\n";
            else
                cout << "Node " << i << ": " << dist[i] << " m : " << parent[i] << "\n";
        }
    }

    void export_to_dot() {
        FileUtils::export_to_dot("graph.dot", graph);
        FileUtils::export_to_dot("cd_graph.dot", constant_degree_graph);
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

    pair<double, int> run_test(function<pair<Dist_List_T, Prev_List_T> (Graph&, Node_id_T, int)> algo, bool use_cd = false) {
        Graph &g = use_cd ? constant_degree_graph : graph;

        auto t0 = chrono::high_resolution_clock::now();
        pair<Dist_List_T, Prev_List_T> results = algo(g, src, N);
        chrono::duration<double, milli> time_span = chrono::high_resolution_clock::now() - t0;

        //printResults(results.first, results.second);
        double time_span_ms = time_span.count();
        cout << "Time elapsed: " << time_span_ms << " ms" << endl;

        int mismatch = 0;
        if (!prev_dist.empty()) {
            for (int i=0; i<N; i++) {
                if (prev_dist[i] != results.first[i] && prev_dist[i] < INF) {
                    if (verbose) {
                        cout << "Different at " << i << " " << prev_dist[i] << " " << results.first[i] << endl;
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
        cout << "=========== Quicktest on a random graph N: " << N << ", M: " << M  << " ===========>" << endl;
        initialize(N, M);
        //export_to_dot();
        //graph_image();

        //cout << "Vanilla Dijkstra" << endl; run_test(dijkstra);
        cout << "Min Heap Dijkstra" << endl; run_test(min_heap_dijkstra);
        verbose = true;
        cout << "Fibo heap Dijkstra" << endl; run_test(fibo_heap_dijkstra);
        cout << "Boost Dijkstra" << endl; run_test(boost_dijkstra);
        //verbose = false;
        cout << "BMSSP no cd" << endl; run_test(top_level_BMSSP);
        cout << "BMSSP" << endl; run_test(top_level_BMSSP, true);
    }

    void quicktest(const string &filename) {
        cout << "=========== Quicktest on a graphml from file: " << filename << " ===========>" << endl;
        initialize(filename);

        //cout << "Vanilla Dijkstra" << endl; run_test(dijkstra);
        cout << "Min Heap Dijkstra" << endl; run_test(min_heap_dijkstra);
        verbose = true;
        cout << "Fibo heap Dijkstra" << endl; run_test(fibo_heap_dijkstra);
        cout << "Boost Dijkstra" << endl; run_test(boost_dijkstra);
        //verbose = false;
        cout << "BMSSP no cd" << endl; run_test(top_level_BMSSP);
        cout << "BMSSP" << endl; run_test(top_level_BMSSP, true);
    }

    /**
     * Comparing on many random graphs
     * @param N_max
     */
    void write_big_file(long long N_max) {
        ofstream file("big_file.txt");

        for (int i=3; i<=N_max; i+=521) {
            for (int j=i; j<=3*i; j+=i/3) {
                initialize(i, j);
                cout << "N: " << i << " M: "<< j << endl;
                prev_dist.clear();
                string line;
                auto res = run_test(boost_dijkstra);
                line += "\n Boost_dijkstra::  time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                double boost_time = res.first;
                res = run_test(min_heap_dijkstra);
                line += "\n Min_heap_dijkstra::  time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                res = run_test(fibo_heap_dijkstra);
                line += "\n Fibo_heap_dijkstra:: time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                res = run_test(top_level_BMSSP);
                line += "\n BMSSP_no_cd:: time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                line += "\n BMSSP_no_cd_time/Boost_dijkstra_time = " + to_string(res.first/boost_time);
                res = run_test(top_level_BMSSP, true);
                line += "\n BMSSP:: time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                line += "\n BMSSP_time/Boost_dijkstra_time = " + to_string(res.first/boost_time) + "\n";
                file << "\nN: " << i << " M: "<< j << " Results: "<< line;
            }
        }
    }
};

#endif