Howl Chess Engine

Howl is a chess engine written in C++.

This project has a much longer history than the current codebase suggests. I started writing the engine around 17 years ago in C#. At some point I lost a large amount of progress when my hard drive was corrupted, so I started again and rewrote the engine in C#. Later, I decided to rewrite it once more in C++.

After many abandoned versions, rewrites, experiments, bugs, and long gaps between periods of work, I finally reached the point where Howl works as a real chess engine.

For me, that is the most important thing about this project. It was not created as a short exercise or as a wrapper around another engine. It is a project I kept coming back to for nearly two decades.

Design

Howl has its own implementation of the core engine components, including:

Board representation

Move representation and move generation

Make and undo move logic

Static exchange evaluation

Evaluation

Transposition table

Search stack and history structures

Quiescence search

UCI interface

Tablebase integration

Testing and engine diagnostics

The current engine uses a modern alpha beta search with techniques such as principal variation search, transposition tables, iterative deepening, aspiration windows, late move reductions, null move pruning, ProbCut, futility pruning, razoring, history based move ordering, continuation history, killer moves, countermoves, singular extensions, quiescence search, and Syzygy tablebases.

A major focus of the project is search efficiency. Reaching greater depth by searching fewer useful nodes matters more to me than simply making the engine process more nodes per second.

Stockfish Attribution

Stockfish has been an important reference for the modern search work in Howl.

I studied Stockfish 11 to understand how a strong classical chess engine combines techniques such as late move reductions, null move pruning, ProbCut, history heuristics, move ordering, quiescence search, transposition table usage, and selective pruning.

Some of the search ideas, formulas, constants, and algorithmic relationships currently used by Howl are based on or inspired by Stockfish 11.

The implementation in Howl was written for Howl's own architecture. I did not copy Stockfish source code into the engine. Howl has its own board representation, move representation, move generator, evaluator, search structures, transposition table implementation, UCI code, memory management, and supporting infrastructure.

Stockfish is free and open source software developed by the Stockfish contributors:

https://stockfishchess.org/

Source:

https://github.com/official-stockfish/Stockfish

I am grateful to the Stockfish project and its contributors for making such a strong engine openly available to study.

Search Development

The search has gone through several generations.

The early versions were my own experimental search implementations developed over many years. More recently, I used Stockfish 11 as a reference point to rebuild the search around a coherent, proven classical search architecture.

The objective was not to turn Howl into Stockfish. The objective was to understand why these techniques work together, reproduce their search behaviour within Howl's architecture, and then use that as a strong baseline for further experimentation.

The Stockfish derived numerical search parameters are being separated into named parameters so they can eventually be tuned for Howl rather than treated as permanent values. Howl has a different evaluator, representation, move generation system, and implementation characteristics, so the best parameters for Howl may not be the same as those used by Stockfish.

Testing and Tuning

Howl includes tooling for investigating both correctness and playing strength, including:

Deterministic search diagnostics

Node count and principal variation comparisons

Search profiling

Move ordering diagnostics

LMR diagnostics

Null move diagnostics

Transposition table telemetry

Evaluation tuning

SPSA based parameter tuning

Self play and version against version matches

Cloud based parallel self matches

Syzygy tablebase testing

Performance work is generally validated by separating two questions:

Does the change reduce the amount of search work without damaging search semantics?

Does the resulting engine actually play stronger chess?

A faster benchmark by itself is not considered sufficient evidence of an improvement.

Building

A typical build is:

cmake -S . -B build
cmake --build build -j

The resulting engine can then be run through its UCI interface.

For example:

./build/howl

Project Status

Howl is still under active development.

The current work is focused on making the search both stronger and more efficient, then tuning the resulting search specifically for Howl rather than blindly retaining parameters from another engine.

There is still a large gap between Howl and the strongest modern chess engines. Closing that gap is not really the point of the project. The point is to keep understanding the engine deeply enough that every major part of it remains something I can reason about, change, test, and call my own.

After 17 years, it finally plays chess.

That is a good place to start.

P.S:I had to make the previous repo private since it contained some personal information. This one continues a project that’s been evolving for 17 years!

## Azure Cloud Self-Match Runner

A disposable parallel cloud runner for evaluating Howl engine versions using Azure Container Instances (ACI).

### Prerequisites
- Python 3.10+ with `python-chess` installed (`pip install chess`)
- Azure CLI (`az`) installed and authenticated
- Docker installed locally (for image building when running non-dry matches)

### Azure Login
```bash
az login
```

### Running Cloud Self Matches
Run 20 games (10 openings played twice with colours swapped) in parallel on ACI at 5m + 1.5s time control:
```bash
python tools/cloud_self_match.py \
  ./match-engines/howl-old \
  ./build/howl \
  --time 300 \
  --inc 1.5 \
  --games 20 \
  --pgn cloud-match.pgn
```

Preview resource provisioning and opening assignments without deploying:
```bash
python tools/cloud_self_match.py ./match-engines/howl-old ./build/howl --time 300 --inc 1.5 --games 20 --dry-run
```

### Expected Output
- Live single-line game progress for each finished cloud worker instance
- Aggregated win/draw/loss scores and per-opening breakdown matching `tools/self_match.py`
- Combined PGN saved locally to `--pgn` destination

### Disposable Cleanup
All cloud resources are created inside a dedicated temporary resource group (`howl-match-rg-<id>`). Upon match completion, failure, or cancellation (`Ctrl+C`), the temporary resource group is asynchronously deleted via `az group delete`.
