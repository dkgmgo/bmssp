#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

BUILD_DIR=""
CANDIDATES=("build" "cmake-build-debug" "cmake-build-release")

for dir in "${CANDIDATES[@]}"; do
    if [ -d "$PROJECT_ROOT/$dir" ]; then
        BUILD_DIR="$PROJECT_ROOT/$dir"
        break
    fi
done
if [ -z "$BUILD_DIR" ]; then
    echo "ERROR: Could not find a build directory."
    exit 1
fi

BENCH_EXEC="$BUILD_DIR/bench_it"

RESULTS_DIR="$PROJECT_ROOT/analysis/results"
PLOTS_DIR="$PROJECT_ROOT/analysis/plots"
PY_SCRIPT="$PROJECT_ROOT/analysis/scripts/plot_speedup.py"

BASELINE="BMSSP"

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
OUTPUT_JSON="$RESULTS_DIR/bench_$TIMESTAMP.json"

mkdir -p "$RESULTS_DIR"
mkdir -p "$PLOTS_DIR"

echo "========== Building Project =========="
cmake --build "$BUILD_DIR"

echo "========== Running Benchmarks =========="
"$BENCH_EXEC" --benchmark_out="$OUTPUT_JSON" --benchmark_out_format=json
echo "Benchmark results saved to:"
echo "$OUTPUT_JSON"

echo "========== Generating Speedup Plots =========="
python3 "$PY_SCRIPT" "$OUTPUT_JSON" --baseline "$BASELINE"
echo "Plots generated in:"
echo "$PLOTS_DIR"
echo "========== DONE =========="