# Howl Chess Engine

Howl is a chess engine written in C++. It features an alpha-beta search (PVS, iterative deepening, aspiration windows, late move reductions, null move pruning, history heuristics) combined with a learned NNUE evaluator.

## Requirements

- C++17 compatible compiler (e.g., GCC 9+, Clang 10+)
- CMake 3.10 or newer

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The resulting executable is `build/howl`.

## Running

Howl communicates via the Universal Chess Interface (UCI) protocol:

```bash
./build/howl
```

Example UCI session:

```text
uci
isready
position startpos
go depth 12
quit
```

### NNUE Evaluation

Howl loads `./nnue.weights` from the repository root by default. No manual configuration is required for normal operation.

An alternate weights file can be loaded via the `StructuredNNUEWeights` UCI option:

```text
setoption name StructuredNNUEWeights value /path/to/custom.weights
```

### Syzygy Tablebases

Howl supports 3-4-5 piece Syzygy tablebases via Fathom:

```text
setoption name SyzygyPath value /path/to/syzygy
setoption name SyzygyProbeLimit value 5
```

## NNUE Training

The tools required to prepare datasets, train a compatible NNUE network, and export weights are located in `nnue-training/`. See [`nnue-training/README.md`](file:///home/masoud/Code/Howl/nnue-training/README.md) for instructions.

## Acknowledgements

- [Lichess](https://huggingface.co/datasets/Lichess/chess-position-evaluations): Position evaluation dataset used to train Howl's NNUE network.
- [Fathom](https://github.com/jdart1/Fathom): Syzygy tablebase probing library (vendored under `third_party/fathom`).
