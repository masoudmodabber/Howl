# Howl

Howl is a UCI chess engine written in C++ featuring NNUE evaluation and Syzygy endgame tablebase support.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Run

```bash
./build/howl
```

## NNUE

Howl loads `nnue.weights` located in the parent directory of the executable; the retained `nnue.weights` is the production network.

## UCI Options

- `MultiPV`: Number of principal variations to calculate (default: 1, min: 1, max: 99).
- `Hash`: Transposition table memory size in megabytes (default: 40, min: 8, max: 1024).
- `SyzygyPath`: Path to the Syzygy tablebase directory.
- `SyzygyProbeLimit`: Maximum piece count for Syzygy tablebase probing (default: 5, min: 0, max: 7).

## Syzygy

The `SyzygyPath` option points to the directory containing Syzygy tablebase files (e.g. `setoption name SyzygyPath value /path/to/syzygy`).

## NNUE Training

The `nnue-training/` directory contains the retained compatible training and export pipeline.

## Attribution

Tablebase probing is powered by [Fathom](https://github.com/jdart1/Fathom) (vendored in `third_party/fathom`).
