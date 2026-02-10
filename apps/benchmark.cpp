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

const vector<vector<int64_t>> ARGS = {
    {10000, 30000, 10},
    {50000, 150000, 10},
    {100000, 300000, 10},
    {500000, 1500000, 10},
    {1000000, 3000000, 10},
    {5000000, 15000000, 10},
    {10000000, 30000000, 10}
};

string data_path = string(PROJECT_ROOT) + "/data";
const vector<string> FILES = {
    data_path + "/1199167200.1199170800.graphml",
};

struct GraphDataset {
    Graph graph;
    int src;
    int64_t nodes_count;
    int64_t edges_count;
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
};

class RandomGraphFixture : public BenchFixture {
public:
    void SetUp(const benchmark::State& state) override {
        int64_t N = state.range(0);
        int64_t M = state.range(1);
        int max_weight = state.range(2);

        string key = "random_" + to_string(N) + "_" + to_string(M) + "_" + to_string(max_weight);
        auto& dataset = GraphRepository::get(key, [&]() {
            GraphDataset d;
            string specs = "random nodes_count=" + to_string(N) + " edges_count=" + to_string(M) + " max_weight=" + to_string(max_weight) + " seed=" + to_string(RANDOM_SEED);
            d.graph = go_get_that_graph(specs);
            d.nodes_count = N;
            d.edges_count = M;
            d.src = get_a_source(d.graph);
            return d;
        });

        graph = &dataset.graph;
        src = dataset.src;
        nodes_count = dataset.nodes_count;
        edges_count = dataset.edges_count;
    }
};

class RandomUnweightedGraphFixture : public BenchFixture {
public:
    void SetUp(const benchmark::State& state) override {
        int64_t N = state.range(0);
        int64_t M = state.range(1);

        string key = "random_unweighted_" + to_string(N) + "_" + to_string(M);
        auto& dataset = GraphRepository::get(key, [&]() {
            GraphDataset d;
            string specs = "random unweighted nodes_count=" + to_string(N) + " edges_count=" + to_string(M) + " seed=" + to_string(RANDOM_SEED);
            d.graph = go_get_that_graph(specs);
            d.nodes_count = N;
            d.edges_count = M;
            d.src = get_a_source(d.graph);
            return d;
        });

        graph = &dataset.graph;
        src = dataset.src;
        nodes_count = dataset.nodes_count;
        edges_count = dataset.edges_count;
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
            return d;
        });

        graph = &dataset.graph;
        src = dataset.src;
        nodes_count = dataset.nodes_count;
        edges_count = dataset.edges_count;
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

// BGP graphs
using StdPQ_BGP = SSSPBench<BGPGraphFixture, StdPQDijkstraAlgo>;
using Fibo_BGP = SSSPBench<BGPGraphFixture, FiboDijkstraAlgo>;
using Boost_BGP = SSSPBench<BGPGraphFixture, BoostDijkstraAlgo>;
using BMSSP_BGP = SSSPBench<BGPGraphFixture, BMSSPAlgo>;


#define DEFINE_BENCHMARK(Cls, Name) \
BENCHMARK_DEFINE_F(Cls, Name)(benchmark::State& st) { \
    this->RunBenchmark(st); \
}

#define REGISTER_BENCH_WITH_ARGS(Fixture, Name) \
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

// BGP graphs
DEFINE_BENCHMARK(StdPQ_BGP, STDPriorityQueue)
DEFINE_BENCHMARK(Fibo_BGP, BOOSTFibonacciHeap)
DEFINE_BENCHMARK(Boost_BGP, BOOSTDijkstra)
DEFINE_BENCHMARK(BMSSP_BGP, BMSSP)

int main(int argc, char** argv) {
    // Random weighted
    REGISTER_BENCH_WITH_ARGS(StdPQ_RandomGraph, STDPriorityQueue)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Fibo_RandomGraph, BOOSTFibonacciHeap)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Boost_RandomGraph, BOOSTDijkstra)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(BMSSP_RandomGraph, BMSSP)->Complexity()->Unit(benchmark::kMillisecond);

    // Random unweighted
    REGISTER_BENCH_WITH_ARGS(StdPQ_RandomUnweighted, STDPriorityQueue)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Fibo_RandomUnweighted, BOOSTFibonacciHeap)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(Boost_RandomUnweighted, BOOSTDijkstra)->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_ARGS(BMSSP_RandomUnweighted, BMSSP)->Complexity()->Unit(benchmark::kMillisecond);

    // BGP graphs
    REGISTER_BENCH_WITH_RANGE(StdPQ_BGP, STDPriorityQueue, FILES.size())->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_RANGE(Fibo_BGP, BOOSTFibonacciHeap, FILES.size())->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_RANGE(Boost_BGP, BOOSTDijkstra, FILES.size())->Complexity()->Unit(benchmark::kMillisecond);
    REGISTER_BENCH_WITH_RANGE(BMSSP_BGP, BMSSP, FILES.size())->Complexity()->Unit(benchmark::kMillisecond);

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
