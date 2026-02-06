#include "apps/runner.hpp"

using namespace std;

Runner runner;

int main() {
    runner.quicktest(5000, 15000);
    runner.quicktest(5000, 15000, true);
    runner.quicktest("../data/1199167200.1199170800.graphml");
    //runner.avg_time_of_x_vertex_as_src(5000, 15000, true, 600);
    //runner.avg_time_of_x_vertex_as_src("../data/1199167200.1199170800.graphml", 600);
    //runner.write_big_file(20000000);

    return 0;
}