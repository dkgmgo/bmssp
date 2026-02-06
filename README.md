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
If you are only interested in testing the BMSSP, copy and include `bmssp.hpp`, `common.hpp` and `data_structures/BBL_DS.hpp` in your project, otherwise, clone the repository.

### Quickstart and Benchmark
- As you can see in the `main.cpp` file, the `apps/runner.hpp` file provides some simple examples you might want to try. The benchmark implementation is located in the `apps/benchmark.hpp` file.
- Uncomment the necessary line in `apps/runner.hpp` if you want to export a graph_viz image or see the distance outputs.