#include "apps/runner.hpp"

using namespace std;

Runner runner;

int main() {
    const int RANDOM_SEED = 1234;

    runner.quicktest("random nodes_count=10000 max_weight=10 seed=" + to_string(RANDOM_SEED));
    runner.quicktest("random unweighted nodes_count=10000 max_weight=10 seed=" + to_string(RANDOM_SEED));
    runner.quicktest("grid w=100 h=100");
    runner.quicktest(string(PROJECT_ROOT) + "/data/1199167200.1199170800.graphml");
    //runner.avg_time_of_x_vertices_as_src("random nodes_count=10000 edges_count=30000 max_weight=10 seed=" + to_string(RANDOM_SEED), 6);
    //runner.avg_time_of_x_vertices_as_src(string(PROJECT_ROOT) + "/data/1199167200.1199170800.graphml", 6);
    //runner.write_big_file(20000000, RANDOM_SEED);

    return 0;
}