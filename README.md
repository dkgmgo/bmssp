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
This is shown by the following benchmark results on graphs randomly generated with the Barabási–Albert model (the times are in **milliseconds** and the ratios are over the BMSSP time):

| Nodes count | Edges count | BMSSP time | Fibonacci Heap Dijkstra time (ratio) | D-ary Heap Dijkstra time: BGL implementation (ratio) | Binary Heap Dijkstra time (ratio) |
|:------------|:------------|:-----------|:-------------------------------------|:-----------------------------------------------------|:----------------------------------|
| 5k          | 10k         | 1.170      | 0.566 (2.067)                        | 0.212 (5.518)                                        | 0.125 (9.355)                     |
| 10k         | 20k         | 1.950      | 1.251 (1.559)                        | 0.522 (3.739)                                        | 0.322 (6.062)                     |
| 50k         | 100k        | 9.324      | 7.886 (1.182)                        | 3.782 (2.465)                                        | 2.345 (3.976)                     |
| 100k        | 200k        | 19.180     | 18.929 (1.013)                       | 9.189 (2.087)                                        | 5.352 (3.584)                     |
| 500k        | 1M          | 169.767    | 195.544 (0.868)                      | 113.587 (1.495)                                      | 80.295 (2.114)                    |
| 1M          | 2M          | 471.732    | 450.538 (1.047)                      | 276.522 (1.706)                                      | 191.068 (2.469)                   |
| 5M          | 10M         | 4070.192   | 3073.702 (1.324)                     | 2709.849 (1.502)                                     | 1595.685 (2.551)                  |
| 10M         | 20M         | 7608.208   | 7793.388 (0.976)                     | 7324.826 (1.039)                                     | 3879.649 (1.961)                  |

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