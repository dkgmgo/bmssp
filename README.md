# A C++ implementation of the BMSSP algorithm 

## Overview
This repository provides a header-only C++ implementation of the BMSSP algorithm introduced by Duan et al. in [Breaking the Sorting Barrier for Directed Single-Source Shortest Paths](https://arxiv.org/pdf/2504.17033).

The goal of this project is to:
- implement the BMSSP algorithm in C++
- provide a reusable Block-Based Linked List data structure from Lemma 3.3 of the paper
- provide reference implementations of Dijkstra's algorithm variants
- benchmark those implementations against each other

## Performance

Despite the fact that BMSSP and Dijkstra's algorithms with Fibonacci heap have better complexity in theory, in practice they are slower than a Dijkstra based on Binary Heap or a d-ary heap.
This is shown by the following benchmark results on randomly generated graphs (the times are in **milliseconds** and the ratios are over the BMSSP time):

| Nodes count | Edges count | BMSSP time | Fibonacci Heap Dijkstra time (ratio) | Binary Heap Dijkstra time (ratio) | D-ary Heap Dijkstra time: BGL implementation (ratio) |
|:------------|:------------|:-----------|:-------------------------------------|:----------------------------------|:-----------------------------------------------------|
| 5k          | 15k         | 2.756      | 1.261 (2.186)                        | 0.708 (3.890)                     | 0.599 (4.598)                                        |
| 10k         | 30k         | 5.231      | 3.023 (1.731)                        | 1.773 (2.950)                     | 1.518 (3.445)                                        |
| 50k         | 150k        | 30.921     | 20.230 (1.529)                       | 10.821 (2.858)                    | 10.167 (3.041)                                       |
| 100k        | 300k        | 62.873     | 46.907 (1.340)                       | 23.560 (2.669)                    | 24.263 (2.591)                                       |
| 500k        | 1.5M        | 440.811    | 364.175 (1.210)                      | 215.802 (2.043)                   | 219.309 (2.010)                                      |
| 1M          | 3M          | 1014.896   | 847.899 (1.197)                      | 454.837 (2.231)                   | 477.213 (2.127)                                      |
| 5M          | 15M         | 8249.652   | 5415.365 (1.523)                     | 3060.319 (2.696)                  | 2797.544 (2.949)                                     |

## Usage

### Prerequisites

Check the `CMakeLists.txt` file to see the required dependencies:

- C++14 or later
- Boost Graph Library
- Google Benchmark
- Google Test

### Installation
If you are only interested in testing the BMSSP, copy and include `include/bmssp.hpp`, `include/common.hpp` and `data_structures/BBL_DS.hpp` in your project, otherwise, clone the repository and build the project using CMake.
```sh
mkdir build && cd build
cmake ..
cmake --build .
```
### Unit Tests
The project includes some unit tests that can be run using the targets `test_BBL_DS` and `test_utils`.

### Quickstart and Benchmark
- As you can see in the `main.cpp` file, the `apps/runner.hpp` file provides some simple examples you might want to try. Run the `main` target to see the output.
- Uncomment the necessary lines in `apps/runner.hpp` if you want to export a graph_viz image or see the distance outputs.
- The benchmark implementation is located in the `apps/benchmark.cpp` file. There is target `bench_it` that you can use to run the benchmark, the results will be stored in `analysis/results` directory.
- The script `analysis/scripts/run_bench_and_plot_speedup.sh` builds the project, runs the benchmark and plots the time ratio against a chosen baseline algorithm (BMSSP by default). The plots are stored in `analysis/plots` directory.