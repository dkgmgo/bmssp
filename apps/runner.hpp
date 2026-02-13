#ifndef RUNNER_HPP
#define RUNNER_HPP

#include <functional>
#include <chrono>
#include <fstream>
#include "../include/utils/graph_utils.hpp"
#include "../include/utils/file_utils.hpp"
#include "../include/dijkstras.hpp"
#include "../include/bmssp.hpp"

using namespace std;

struct Runner {
    Graph graph;
    int N;
    Graph constant_degree_graph;
    Node_id_T src = 0;
    Dist_List_T prev_dist;
    bool verbose = true;

private:

    void initialize(const string& specifications) {
        prev_dist.clear();
        graph = go_get_that_graph(specifications);
        N = boost::num_vertices(graph);
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

    void quicktest_helper() {
        cout << "Min Heap Dijkstra" << endl; run_test(min_heap_dijkstra);
        verbose = true;
        cout << "Fibo heap Dijkstra" << endl; run_test(fibo_heap_dijkstra);
        cout << "Boost Dijkstra" << endl; run_test(boost_dijkstra);
        //verbose = false;
        cout << "BMSSP" << endl; run_test(top_level_BMSSP);
        cout << "BMSSP CD" << endl; run_test(top_level_BMSSP, true);
    }

    void avg_time_of_x_vertices_as_src_helper(int x, const string &title, const string &output){
        ofstream file(output);
        file << title;

        double bmssp_cd_time = 0;
        double boost_time = 0;
        double min_heap_time = 0;
        double fibo_heap_time = 0;
        double bmssp_time = 0;
        double last_boost_time = 0;

        for (int i=0; i<x; i++) {
            src = i;
            prev_dist.clear();
            string line = "\n Source: " + to_string(src) + " Out-degree: " + to_string(boost::out_degree(src, graph));
            auto res = run_test(boost_dijkstra);
            last_boost_time = res.first;
            line += "\n Boost_dijkstra time: " + to_string(last_boost_time);
            boost_time += last_boost_time;
            res = run_test(min_heap_dijkstra);
            line += "\n Min_heap_dijkstra time: " + to_string(res.first);
            min_heap_time += res.first;
            res = run_test(fibo_heap_dijkstra);
            line += "\n Fibo_heap_dijkstra time: " + to_string(res.first);
            fibo_heap_time += res.first;
            res = run_test(top_level_BMSSP);
            line += "\n BMSSP time: " + to_string(res.first) + " Ratio to boost: " + to_string(res.first/last_boost_time);
            bmssp_time += res.first;
            res = run_test(top_level_BMSSP, true);
            line += "\n BMSSP_CD time: " + to_string(res.first)+ " Ratio to boost: " + to_string(res.first/last_boost_time) + "\n";
            bmssp_cd_time += res.first;
            file << line;
        }

        file << "\n Average Boost Time: " << boost_time/x;
        file << "\n Average Min heap Time: " << min_heap_time/x;
        file << "\n Average Fibo heap Time: " << fibo_heap_time/x;
        file << "\n Average BMSSP Time: " << bmssp_time/x << " Ratio to boost: " << bmssp_time/boost_time;
        file << "\n Average BMSSP CD Time: " << bmssp_cd_time/x << " Ratio to boost: " << bmssp_cd_time/boost_time;
        file << "\n";
    }

public:
    /**
     * Run a quicktest on a randomly generated graph
     * @param specifications graph's specs or graphml filepath
     */
    void quicktest(const string& specifications) {
        string msg = "=========== Quicktest with graph specs: " + specifications + "===========>";
        cout << msg << endl;
        initialize(specifications);
        //export_to_dot();
        //graph_image();
        quicktest_helper();
    }

    /**
     * Run the different algorithms x times on the same graph and each time with a different node as the source
     * @param specifications graph's specs or graphml filepath
     * @param x number of sources for the average
     * @param output output filename
     */
    void avg_time_of_x_vertices_as_src(const string& specifications, int x, const string &output="avg_time_of_n_src.txt") {
        string title =  "avg_time_of_x_vertices_as_src: graph specs = " + specifications + "; with x = "+to_string(x);
        initialize(specifications);
        avg_time_of_x_vertices_as_src_helper(x, title, output);
    }

    /**
     * Comparing on a lot of random graphs
     * @param N_max
     */
    void write_big_file(long long N_max) {
        ofstream file("big_file.txt");

        for (int i=3; i<=N_max; i+=521) {
            for (int j=i; j<=3*i; j+=i/3) {
                initialize("random nodes_count=" + to_string(i) + " edges_count=" + to_string(j) + " max_weight=" + to_string(10) + " seed=" + to_string(42));
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
                line += "\n BMSSP:: time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                line += "\n BMSSP_time/Boost_dijkstra_time = " + to_string(res.first/boost_time);
                //res = run_test(top_level_BMSSP, true);
                //line += "\n BMSSP_CD:: time: " + to_string(res.first) + "ms " + "mismatch: " + to_string(res.second);
                //line += "\n BMSSP_CD_time/Boost_dijkstra_time = " + to_string(res.first/boost_time) + "\n";
                file << "\nN: " << i << " M: "<< j << " Results: "<< line;
            }
        }
    }
};

#endif //RUNNER_HPP
