import json
import argparse
from collections import defaultdict
import matplotlib.pyplot as plt
from pathlib import Path
from datetime import datetime
import csv

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
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_path = PLOTS_DIR.parent / f"results/speedup_{timestamp}.csv"
    for graph_type, dataset in results.items():
        with open(csv_path, "a") as csvfile:
            writer = csv.writer(csvfile)
            x_values = []
            speedups = defaultdict(list)
            csv_header = ["nodes", "edges", f"{baseline}_time"]
            fill_header = True
            csv_rows = []

            for (nodes, edges) in sorted(dataset.keys()):
                algos = dataset[(nodes, edges)]
                if baseline not in algos:
                    continue
                base_time = algos[baseline]
                x_values.append(nodes)
                if fill_header:
                    fill_header = False
                    for algo in algos:
                        if algo == baseline:
                            continue
                        csv_header.append(f"{algo}_time (ratio)")

                next_row = [nodes, edges, f"{base_time:.3f}"]
                for algo, time in algos.items():
                    speedups[algo].append(base_time / time)
                    if algo == baseline:
                        continue
                    next_row.append(f"{time:.3f} ({base_time / time:.3f})")
                csv_rows.append(next_row)

            writer.writerow(["Graph Type: " + graph_type])
            writer.writerow(csv_header)
            for row in csv_rows:
                writer.writerow(row)

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
