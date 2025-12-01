#include "runner.hpp"

using namespace std;

Runner runner;

int main() {
    runner.quicktest(51061, 153181);
    runner.quicktest("../data/1199167200.1199170800.graphml");
    //runner.write_big_file(20000000);

    return 0;
}