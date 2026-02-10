import json
import argparse
from collections import defaultdict
import matplotlib.pyplot as plt
from pathlib import Path
from datetime import datetime

PLOTS_DIR = Path(__file__).resolve().parent.parent / "plots"

def extract_algo(name):
    parts = name.split("/")
    return parts[1]


def extract_graph_type(name):
    return (name.split("/")[0]).split("_")[1]


def load_results(path, time_key="real_time"): # vs cpu_time
    with open(path) as f:
        data = json.load(f)

    results = defaultdict(lambda: defaultdict(dict))

    for bench in data["benchmarks"]:
        if "nodes_count" not in bench:
            continue

        graph_type = extract_graph_type(bench["name"])
        algo = extract_algo(bench["name"])

        nodes = int(bench["nodes_count"])
        edges = int(bench["edges_count"])
        time = bench[time_key]

        results[graph_type][(nodes, edges)][algo] = time

    return results


def plot_speedups(results, baseline="BMSSP"):
    for graph_type, dataset in results.items():
        x_values = []
        speedups = defaultdict(list)

        for (nodes, edges) in sorted(dataset.keys()):
            algos = dataset[(nodes, edges)]
            if baseline not in algos:
                continue
            base_time = algos[baseline]
            x_values.append(nodes)

            for algo, time in algos.items():
                speedups[algo].append(base_time / time)

        plt.figure()

        for algo, values in speedups.items():
            if algo == baseline:
                algo += " (break even)"
            plt.plot(x_values, values, label=algo)

        plt.xlabel("Nodes")
        plt.ylabel(f"Ratio over {baseline}")
        plt.title(f"Speedup Plot for ({graph_type})")
        plt.legend()
        plt.grid(True)
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

        plt.savefig(f"{PLOTS_DIR}/speedup_{graph_type}_{timestamp}.png")
        plt.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("json_file")
    parser.add_argument("--baseline", default="BMSSP")
    parser.add_argument("--time", default="real_time", choices=["real_time", "cpu_time"])

    args = parser.parse_args()

    results = load_results(args.json_file, args.time)
    plot_speedups(results, args.baseline)


if __name__ == "__main__":
    main()
