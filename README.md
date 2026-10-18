# Howl Chess Engine

Howl is a UCI chess engine written in C++.

I started writing the first version around 17 years ago in C#. At one point I lost a large amount of progress after a hard drive failure and started again. Later I rewrote the engine in C++, followed by many abandoned versions, experiments, bugs, rewrites, and long gaps between periods of work.

After all that time, Howl finally works as a real chess engine.

For me, that history matters. Howl was not created as a short programming exercise or as a wrapper around another engine. It is a project I have kept returning to for nearly two decades.

## Design

Howl implements its own core chess engine infrastructure, including:

1. Board representation
2. Move representation and move generation
3. Make and undo move logic
4. Static exchange evaluation
5. Transposition table
6. Search stack and history structures
7. Quiescence search
8. Evaluation
9. UCI interface
10. Syzygy tablebase integration

The implementation has changed many times over the years, but understanding and controlling the complete engine has always been an important part of the project.

## Search

Howl uses a modern alpha beta search with techniques including:

1. Principal variation search
2. Iterative deepening
3. Aspiration windows
4. Transposition tables
5. Late move reductions
6. Null move pruning
7. ProbCut
8. Futility pruning
9. Razoring
10. History based move ordering
11. Continuation history
12. Killer moves and countermoves
13. Singular extensions
14. Quiescence search
15. Syzygy tablebases

A major focus of the project has always been search efficiency.

I care more about reaching useful depth by searching fewer useful nodes than simply maximizing raw nodes per second. A change that makes Howl process more nodes but requires many more nodes to understand the same position is usually not an improvement.

## Stockfish

Stockfish has been an important reference for the modern search work in Howl.

I studied Stockfish while rebuilding Howl's search into a more coherent modern architecture. It was particularly useful for understanding how techniques such as late move reductions, null move pruning, ProbCut, history heuristics, move ordering, quiescence search, transposition tables, and selective pruning interact.

Some formulas, constants, and algorithmic relationships in Howl were inspired by or based on ideas used by Stockfish.

The implementation itself was written for Howl. Howl has its own board representation, move representation, move generator, evaluator, transposition table, search structures, UCI implementation, memory management, and supporting infrastructure.

Studying an open source engine of Stockfish's quality was extremely valuable, and I am grateful to the Stockfish contributors for making that possible.

Stockfish:

https://stockfishchess.org/

Source:

https://github.com/official-stockfish/Stockfish

## NNUE

For most of its history, Howl used a handcrafted classical evaluator.

That evaluator accumulated explicit chess knowledge written into code, including material, mobility, pawn structure, king safety, piece activity, passed pawns, attacks, and many other terms.

The final production evaluator is a small NNUE trained specifically for Howl.

It uses king conditioned sparse chess features and incrementally maintained accumulators so evaluation can be updated efficiently as moves are made and undone.

The production network is stored as:

```text
nnue.weights
```

Howl loads this network automatically. No evaluator selection or weight configuration is required through UCI.

The production weight file is located in the parent directory of the executable. With the normal repository layout:

```text
Howl/
├── nnue.weights
└── build/
    └── howl
```

the executable automatically resolves and loads:

```text
Howl/nnue.weights
```

This allows Howl to work correctly when launched by chess GUIs such as Arena regardless of their current working directory.

## NNUE Training

The retained compatible training and export pipeline is in:

```text
nnue-training/
```

It contains the tools required to prepare data, train a compatible network, and export it into Howl's production weight format.

The production NNUE was trained using the Lichess Chess Position Evaluations dataset:

https://huggingface.co/datasets/Lichess/chess-position-evaluations

The training data came from deeply analysed chess positions produced by large scale distributed engine analysis.

The effect on Howl was much larger than I originally expected.

After training, the engine began finding positional and tactical moves that the old handcrafted evaluator simply did not see. In some games against strong engines, Howl produced moves at relatively modest search depths that were difficult for me to understand even after looking at the position myself.

Some of them genuinely felt magical to watch.

The interesting part for me is what this says about big data and machine learning.

The classical evaluator learns chess through human concepts that I explicitly decide to encode. The NNUE learns from a huge body of computer generated chess knowledge instead.

It does not literally search the future when evaluating a position. But after learning from enough deeply analysed positions, some of that future appears to become compressed into the evaluation itself.

That has been one of the most surprising things I have seen while developing this engine.

I am extremely grateful to Lichess and to everyone whose distributed analysis contributed to making this dataset available.

## Building

Requirements include a C++17 compiler and CMake.

Build the Release version with:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run:

```bash
./build/howl
```

Howl speaks UCI and can be used directly from the command line or through compatible chess GUIs such as Arena.

A minimal session looks like:

```text
uci
isready
position startpos
go depth 12
```

## UCI Options

The final engine exposes four UCI options.

### MultiPV

Number of principal variations to calculate.

```text
default: 1
minimum: 1
maximum: 99
```

### Hash

Transposition table memory size in megabytes.

```text
default: 40
minimum: 8
maximum: 1024
```

### SyzygyPath

Path to a directory containing Syzygy tablebase files.

Example:

```text
setoption name SyzygyPath value /path/to/syzygy
```

### SyzygyProbeLimit

Maximum number of pieces for which Howl will probe Syzygy tablebases.

```text
default: 5
minimum: 0
maximum: 7
```

## Syzygy Tablebases

Howl uses the Fathom library for Syzygy tablebase probing.

Fathom is vendored under:

```text
third_party/fathom/
```

The original Fathom licensing and attribution information is preserved there.

## Development Philosophy

There is a very large gap between Howl and the strongest modern chess engines.

Closing that gap at any cost was never really the point of the project.

I wanted to understand why the engine became stronger.

That meant being able to reason about the search, evaluator, representation, pruning behaviour, training data, and failures rather than simply accumulating techniques until the Elo number increased.

The same principle applied to NNUE.

The fact that a learned evaluator can produce chess ideas that I would never have known how to encode manually makes it more interesting, not less important to understand.

Howl went through many periods where the engine was broken, slow, weak, overcomplicated, or simply abandoned for years. A surprising amount of the project consisted of discovering that apparently clever ideas were wrong and then removing them again.

That process became as much a part of the project as the final engine.

## Project Status

Howl is now finished and archived.

The repository has been reduced to the production engine, its NNUE network, the compatible NNUE training pipeline, the Fathom tablebase dependency, and the minimal build and documentation files required to use them.

The development tools, tuning infrastructure, historical networks, test harnesses, experimental implementations, and other temporary project machinery have been removed from the final repository.

After roughly 17 years of disappearing, reappearing, breaking, and being rewritten, this is the final version of Howl.

It plays chess.

That is a good place to stop.

## Acknowledgements

Stockfish was an important reference for learning modern chess search design:

https://github.com/official-stockfish/Stockfish

Lichess provided the position evaluation dataset used to train Howl's NNUE:

https://huggingface.co/datasets/Lichess/chess-position-evaluations

Fathom provides the Syzygy tablebase probing implementation included with Howl:

https://github.com/jdart1/Fathom

These projects made knowledge and infrastructure openly available that would otherwise have been extremely difficult to reproduce independently.

I am grateful to everyone who contributed to them.

## Repository History

The original public repository was later made private because it contained some personal information.

This repository continues the same project.

Howl has been evolving, disappearing, reappearing, breaking, and being rewritten for roughly 17 years.

It is still the same engine I started all those years ago.