#include "apps/runner.hpp"

using namespace std;

Runner runner;

string write_graph_spec(const string &graph_type, uint64_t N, uint64_t M, int max_weight, int seed) {
    return graph_type+" nodes_count=" + to_string(N) + " edges_count=" + to_string(M) + " max_weight=" + to_string(max_weight) + " seed=" + to_string(seed);
}

int main() {
    const int RANDOM_SEED = 1234;

    runner.quicktest(write_graph_spec("random", 10000, 30000, 10, RANDOM_SEED));
    runner.quicktest(write_graph_spec("random unweighted", 10000, 30000, 10, RANDOM_SEED));
    runner.quicktest("../data/1199167200.1199170800.graphml");
    //runner.avg_time_of_x_vertices_as_src(write_graph_spec("random", 10000, 10000, 10, RANDOM_SEED), 600);
    //runner.avg_time_of_x_vertices_as_src("../data/1199167200.1199170800.graphml", 600);
    //runner.write_big_file(20000000);

    return 0;
}