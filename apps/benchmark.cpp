#include <benchmark/benchmark.h>
#include <random>
#include <unordered_map>
#include <mutex>
#include <functional>

#include "include/utils/graph_utils.hpp"
#include "include/bmssp.hpp"
#include "include/dijkstras.hpp"

using namespace std;

constexpr int RANDOM_SEED = 1234;

const vector<vector<int64_t>> random_ARGS = {
    {5000, 10},
    {10000, 10},
    {50000, 10},
    {100000, 10},
    {500000, 10},
    {1000000, 10},
    {5000000, 10},
    {10000000, 10}
};

const vector<vector<int64_t>> grid_ARGS = {
    {500, 10},
    {1000, 10},
    {500, 100},
    {1000, 100},
    {5000, 100},
    {10000, 100},
    {5000, 1000},
    {10000, 1000}
};

string data_path = string(PROJECT_ROOT) + "/data";
const vector<string> FILES = {
    data_path + "/1199167200.1199170800.graphml",
    data_path + "/1702188000.1702191600.graphml"
};

struct GraphDataset {
    Graph graph;
    int src;
    int64_t nodes_count;
    int64_t edges_count;
    vector<Dist_T> ref_dist;
};
class GraphRepository {
public:
    static GraphDataset& get(const string& key, function<GraphDataset()> builder)
    {
        static unordered_map<string, GraphDataset> cache;
        static mutex mtx;

        lock_guard<mutex> lock(mtx);

        auto it = cache.find(key);
        if (it == cache.end()) {
            it = cache.emplace(key, builder()).first;
        }

        return it->second;
    }
};

inline int get_a_source(Graph& graph) {
    int src = rand() % boost::num_vertices(graph);
    if (boost::out_degree(src, graph) > 0) {
        return src;
    }
    return 0;
}

class BenchFixture : public benchmark::Fixture {
public:
    Graph *graph;
    int src;
    int64_t nodes_count = 0;
    int64_t edges_count = 0;
    vector<Dist_T> *ref_dist;
};

class RandomGraphFixture : public BenchFixture {
public:
    void SetUp(const benchmark::State& state) override {
        int64_t N = state.range(0);
        int max_weight = state.range(1);

        string key = "random_" + to_string(N) + "_" + to_string(max_weight);
        auto& dataset = GraphRepository::get(key, [&]() {
            GraphDataset d;
            string specs = "random nodes_count=" + to_string(N) + " max_weight=" + to_string(max_weight) + " seed=" + to_string(RANDOM_SEED);
            d.graph = go_get_that_graph(specs);
            d.nodes_count = N;
            d.edges_count = boost::num_edges(d.graph);
            d.src = get_a_source(d.graph);
            d.ref_dist = boost_dijkstra(d.graph, d.src, d.nodes_count).first;
            return d;
        });

        graph = &dataset.graph;
        src = dataset.src;
        nodes_count = dataset.nodes_count;
        edges_count = dataset.edges_count;
        ref_dist = &dataset.ref_dist;
    }
};

class RandomUnweightedGraphFixture : public BenchFixture {
public:
    void SetUp(const benchmark::State& state) override {
        int64_t N = state.range(0);

        string key = "random_unweighted_" + to_string(N);
        auto& dataset = GraphRepository::get(key, [&]() {
            GraphDataset d;
            string specs = "random unweighted nodes_count=" + to_string(N) + " seed=" + to_string(RANDOM_SEED);
            d.graph = go_get_that_graph(specs);
            d.nodes_count = N;
            d.edges_count = boost::num_edges(d.graph);
            d.src = get_a_source(d.graph);
            d.ref_dist = boost_dijkstra(d.graph, d.src, d.nodes_count).first;
            return d;
        });

        graph = &dataset.graph;
        src = dataset.src;
        nodes_count = dataset.nodes_count;
        edges_count = dataset.edges_count;
        ref_dist = &dataset.ref_dist;
    }
};

class GridGraphFixture : public BenchFixture {
public:
    void SetUp(const benchmark::State& state) override {
        int w = state.range(0);
        int h = state.range(1);

        string key = "grid_" + to_string(w) + "_" + to_string(h);
        auto& dataset = GraphRepository::get(key, [&]() {
            GraphDataset d;
            string specs = "grid w=" + to_string(w) + " h=" + to_string(h);
            d.graph = go_get_that_graph(specs);
            d.nodes_count = boost::num_vertices(d.graph);
            d.edges_count = boost::num_edges(d.graph);
            d.src = get_a_source(d.graph);
            d.ref_dist = boost_dijkstra(d.graph, d.src, d.nodes_count).first;
            return d;
        });

        graph = &dataset.graph;
        src = dataset.src;
        nodes_count = dataset.nodes_count;
        edges_count = dataset.edges_count;
        ref_dist = &dataset.ref_dist;
    }
};

class BGPGraphFixture : public BenchFixture {
public:
    void SetUp(const benchmark::State& state) override {
        int idx = state.range(0);

        string key = "bgp_" + to_string(idx);
        auto& dataset = GraphRepository::get(key, [&]() {
            GraphDataset d;
            d.graph = go_get_that_graph(FILES[idx]);
            d.nodes_count = boost::num_vertices(d.graph);
            d.edges_count = boost::num_edges(d.graph);
            d.src = get_a_source(d.graph);
            d.ref_dist = boost_dijkstra(d.graph, d.src, d.nodes_count).first;
            return d;
        });

        graph = &dataset.graph;
        src = dataset.src;
        nodes_count = dataset.nodes_count;
        edges_count = dataset.edges_count;
        ref_dist = &dataset.ref_dist;
    }
};


struct StdPQDijkstraAlgo {
    auto operator()(Graph& g, int s, int64_t n) const {
        return min_heap_dijkstra(g, s, n);
    }
};

struct FiboDijkstraAlgo {
    auto operator()(Graph& g, int s, int64_t n) const {
        return fibo_heap_dijkstra(g, s, n);
    }
};

struct BoostDijkstraAlgo {
    auto operator()(Graph& g, int s, int64_t n) const {
        return boost_dijkstra(g, s, n);
    }
};

struct BMSSPAlgo {
    auto operator()(Graph& g, int s, int64_t n) const {
        return top_level_BMSSP(g, s, n);
    }
};


template<typename GraphFixtureT, typename AlgoFunc>
class SSSPBench : public GraphFixtureT {
public:
    AlgoFunc algo;
    void RunBenchmark(benchmark::State& st) {
        for (auto _ : st) {
            auto res = algo(*this->graph, this->src, this->nodes_count);

            st.PauseTiming();
            if (res.first != *this->ref_dist) {
                for (int i=0; i<res.first.size(); i++) {
                    if (res.first[i] != (*this->ref_dist)[i]) {
                        cout << "Different at " << i << " " << (*this->ref_dist)[i] << " " << res.first[i] << endl;
                    }
                }
                st.SkipWithError("Correctness check vs Boost Dijkstra failed!");
            }
            st.ResumeTiming();

            benchmark::DoNotOptimize(res);
            benchmark::ClobberMemory();
        }

        st.SetComplexityN(this->nodes_count);
        st.counters["nodes_count"] = this->nodes_count;
        st.counters["edges_count"] = this->edges_count;
    }
};


// Random weighted
using StdPQ_RandomGraph = SSSPBench<RandomGraphFixture, StdPQDijkstraAlgo>;
using Fibo_RandomGraph = SSSPBench<RandomGraphFixture, FiboDijkstraAlgo>;
using Boost_RandomGraph = SSSPBench<RandomGraphFixture, BoostDijkstraAlgo>;
using BMSSP_RandomGraph = SSSPBench<RandomGraphFixture, BMSSPAlgo>;

// Random unweighted
using StdPQ_RandomUnweighted = SSSPBench<RandomUnweightedGraphFixture, StdPQDijkstraAlgo>;
using Fibo_RandomUnweighted = SSSPBench<RandomUnweightedGraphFixture, FiboDijkstraAlgo>;
using Boost_RandomUnweighted = SSSPBench<RandomUnweightedGraphFixture, BoostDijkstraAlgo>;
using BMSSP_RandomUnweighted = SSSPBench<RandomUnweightedGraphFixture, BMSSPAlgo>;

// Grid
using StdPQ_Grid = SSSPBench<GridGraphFixture, StdPQDijkstraAlgo>;
using Fibo_Grid = SSSPBench<GridGraphFixture, FiboDijkstraAlgo>;
using Boost_Grid = SSSPBench<GridGraphFixture, BoostDijkstraAlgo>;
using BMSSP_Grid = SSSPBench<GridGraphFixture, BMSSPAlgo>;

// BGP graphs
using StdPQ_BGP = SSSPBench<BGPGraphFixture, StdPQDijkstraAlgo>;
using Fibo_BGP = SSSPBench<BGPGraphFixture, FiboDijkstraAlgo>;
using Boost_BGP = SSSPBench<BGPGraphFixture, BoostDijkstraAlgo>;
using BMSSP_BGP = SSSPBench<BGPGraphFixture, BMSSPAlgo>;


#define DEFINE_BENCHMARK(Cls, Name) \
BENCHMARK_DEFINE_F(Cls, Name)(benchmark::State& st) { \
    this->RunBenchmark(st); \
}

#define REGISTER_BENCH_WITH_ARGS(Fixture, Name, ARGS) \
BENCHMARK_REGISTER_F(Fixture, Name)->MinWarmUpTime(0.5)->Apply([](::benchmark::Benchmark* b) { \
for (const auto& arg : ARGS){ \
    b->Args(arg); \
} \
})

#define REGISTER_BENCH_WITH_RANGE(Fixture, Name, N) \
BENCHMARK_REGISTER_F(Fixture, Name)->MinWarmUpTime(0.5)->Apply([](::benchmark::Benchmark* b) { \
for (int arg = 0; arg<N; arg++){ \
    b->Arg(arg); \
} \
})

// Random weighted
DEFINE_BENCHMARK(StdPQ_RandomGraph, STDPriorityQueue)
DEFINE_BENCHMARK(Fibo_RandomGraph, BOOSTFibonacciHeap)
DEFINE_BENCHMARK(Boost_RandomGraph, BOOSTDijkstra)
DEFINE_BENCHMARK(BMSSP_RandomGraph, BMSSP)

// Random unweighted
DEFINE_BENCHMARK(StdPQ_RandomUnweighted, STDPriorityQueue)
DEFINE_BENCHMARK(Fibo_RandomUnweighted, BOOSTFibonacciHeap)
DEFINE_BENCHMARK(Boost_RandomUnweighted, BOOSTDijkstra)
DEFINE_BENCHMARK(BMSSP_RandomUnweighted, BMSSP)

// Grid
DEFINE_BENCHMARK(StdPQ_Grid, STDPriorityQueue)
DEFINE_BENCHMARK(Fibo_Grid, BOOSTFibonacciHeap)
DEFINE_BENCHMARK(Boost_Grid, BOOSTDijkstra)
DEFINE_BENCHMARK(BMSSP_Grid, BMSSP)

// BGP graphs
DEFINE_BENCHMARK(StdPQ_BGP, STDPriorityQueue)
DEFINE_BENCHMARK(Fibo_BGP, BOOSTFibonacciHeap)
DEFINE_BENCHMARK(Boost_BGP, BOOSTDijkstra)
DEFINE_BENCHMARK(BMSSP_BGP, BMSSP)

int main(int argc, char** argv) {
    // Random weighted
    REGISTER_BENCH_WITH_ARGS(StdPQ_RandomGraph, STDPriorityQueue, random_ARGS)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Fibo_RandomGraph, BOOSTFibonacciHeap, random_ARGS)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Boost_RandomGraph, BOOSTDijkstra, random_ARGS)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(BMSSP_RandomGraph, BMSSP, random_ARGS)->Complexity()->Unit(benchmark::kMillisecond);

    // Random unweighted
    REGISTER_BENCH_WITH_ARGS(StdPQ_RandomUnweighted, STDPriorityQueue, random_ARGS)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Fibo_RandomUnweighted, BOOSTFibonacciHeap, random_ARGS)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Boost_RandomUnweighted, BOOSTDijkstra, random_ARGS)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(BMSSP_RandomUnweighted, BMSSP, random_ARGS)->Complexity()->Unit(benchmark::kMillisecond);

    // Grid
    REGISTER_BENCH_WITH_ARGS(StdPQ_Grid, STDPriorityQueue, grid_ARGS)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Fibo_Grid, BOOSTFibonacciHeap, grid_ARGS)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Boost_Grid, BOOSTDijkstra, grid_ARGS)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(BMSSP_Grid, BMSSP, grid_ARGS)->Complexity()->Unit(benchmark::kMillisecond);

    // BGP graphs
    REGISTER_BENCH_WITH_RANGE(StdPQ_BGP, STDPriorityQueue, FILES.size())->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_RANGE(Fibo_BGP, BOOSTFibonacciHeap, FILES.size())->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_RANGE(Boost_BGP, BOOSTDijkstra, FILES.size())->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_RANGE(BMSSP_BGP, BMSSP, FILES.size())->Complexity()->Unit(benchmark::kMillisecond);

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
