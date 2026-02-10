# A C++ implementation of the BMSSP algorithm 

## Overview
This repository provides a header-only C++ implementation of the BMSSP algorithm introduced by Duan et al. in [Breaking the Sorting Barrier for Directed Single-Source Shortest Paths](https://arxiv.org/pdf/2504.17033).

The goal of this project is to:
- implement the BMSSP algorithm in C++
- provide a reusable Block-Based Linked List data structure from Lemma 3.3 of the paper
- provide reference implementations of Dijkstra's algorithm variants
- benchmark those implementations against each other

## Performance

## Usage

### Prerequisites
- C++14 or later
- Boost Graph Library 1.82 or later

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