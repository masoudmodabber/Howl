Howl Chess Engine
Howl is a chess engine written in C++.
I started writing the first version around 17 years ago in C#.
At one point I lost a large amount of progress after a hard drive failure and started again. Later I rewrote the engine in C++, followed by many abandoned versions, experiments, bugs, rewrites, and long gaps between periods of work.
After all that time, Howl finally works as a real chess engine.
For me, that history matters. Howl was not created as a short programming exercise or as a wrapper around another engine. It is a project I have kept returning to for nearly two decades.
Design
Howl implements its own core engine infrastructure, including:
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
11. Testing and diagnostic infrastructure
    The implementation has changed many times over the years, but understanding and controlling the complete engine has always been an important part of the project.
Search
Howl uses a modern alpha beta search with:
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
    A major focus of the project is search efficiency.
I care more about reaching useful depth by searching fewer useful nodes than simply maximizing raw nodes per second.
A change that makes Howl process more nodes but requires many more nodes to understand the same position is usually not an improvement.
Stockfish
Stockfish has been an important reference for the modern search work in Howl.
I studied Stockfish 11 while rebuilding Howl's search into a more coherent modern architecture. I used it to understand how techniques such as late move reductions, null move pruning, ProbCut, history heuristics, move ordering, quiescence search, transposition tables, and selective pruning interact.
Some formulas, constants, and algorithmic relationships in Howl were inspired by or based on Stockfish 11.
The implementation itself was written for Howl. Howl has its own board representation, move representation, move generator, evaluator, transposition table, search structures, UCI implementation, memory management, and supporting infrastructure.
Studying an open source engine of Stockfish's quality was extremely valuable, and I am grateful to the Stockfish contributors for making that possible.
Stockfish:
https://stockfishchess.org/
Source:
https://github.com/official-stockfish/Stockfish
NNUE
For most of its history, Howl used a handcrafted classical evaluator.
That evaluator accumulated explicit chess knowledge written into code, including material, mobility, pawn structure, king safety, piece activity, passed pawns, attacks, and many other terms.
The current production evaluator is a small NNUE trained specifically for Howl.
It uses king conditioned sparse chess features and incrementally maintained accumulators so evaluation can be updated efficiently as moves are made and undone.
The current production network is Structured NNUE V3.
Historical weights are kept separately so trained generations can be tested against each other:
weights/structured-v3-epoch1.weights
weights/structured-v3-epoch5.weights
The current default production weights are:
weights/structured-v3-epoch5.weights
A normal UCI session does not require selecting an evaluator or providing a weights path. Howl loads the production NNUE automatically.
StructuredNNUEWeights can still be used to load another saved network for experiments or matches.
Training Data
Howl's current NNUE was trained using the Lichess Chess Position Evaluations dataset:
https://huggingface.co/datasets/Lichess/chess-position-evaluations
For the current experiment I used the first 15 dataset shards and retained positions with Stockfish evaluations based on at least 3 million searched nodes.
That produced hundreds of millions of high quality training positions.
The effect on Howl was much larger than I expected.
After training, the engine began finding positional and tactical moves that the old handcrafted evaluator simply did not see. In some games against engines around the 2500 level, Howl produced moves at roughly depth 12 that were difficult for me to understand even after looking at the position myself.
Some of them genuinely felt magical to watch.
The interesting part for me is what this says about big data and machine learning.
The classical evaluator learns chess through human concepts that I explicitly decide to encode. The NNUE learns from a huge body of computer generated chess knowledge instead.
With enough examples, a function that is technically still only evaluating the current position can begin to behave as though it understands much larger structures in the game and where those structures may lead.
It does not literally search the future when evaluating a position. But after learning from enough deeply analysed positions, some of that future appears to become compressed into the evaluation itself.
That has been one of the most surprising things I have seen while developing this engine.
I am extremely grateful to Lichess and to everyone whose distributed analysis contributed to making this dataset available.
Without data of this scale and quality, this experiment would not have been possible.
NNUE Validation
The NNUE work went through several generations and exposed problems that training loss alone did not reveal, including activation collapse, incorrect resume behaviour, and a runtime bug where incremental accumulators were updated with different weights from the evaluator.
Howl therefore validates:
1. Python and C++ output parity
2. Full accumulator rebuilds
3. Incremental updates
4. Quiet moves and captures
5. Promotions and promotion captures
6. En passant
7. King moves and king captures
8. Castling
9. Make and undo sequences
10. Weight version consistency
    Incremental evaluation is checked against a fresh rebuild. This has become one of the most important correctness tests in the engine.
Testing and Tuning
Howl includes tooling for:
1. Deterministic search regression
2. Node count and principal variation comparison
3. Search profiling
4. Move ordering diagnostics
5. LMR and null move diagnostics
6. Transposition table telemetry
7. Evaluation validation and response analysis
8. SPSA tuning
9. Fixed node self play
10. Version against version matches
11. Weight against weight NNUE matches
12. Cloud parallel matches
13. Syzygy testing
    Performance work is usually separated into two questions.
First, does the change reduce search work without damaging search behaviour?
Second, does the resulting engine actually play stronger chess?
A faster benchmark by itself is not considered enough evidence of an improvement.
Fixed Node Matches
For local strength testing I generally prefer fixed node matches. Using nodes rather than wall clock time makes comparisons more reproducible and avoids parallel games influencing each other through CPU contention.
The local runner supports paired openings with colours reversed.
NNUE against Classical:
python tools/matches/fixed_node_self_match.py \
  --candidate build/howl \
  --production build/howl \
  --candidate-weights weights/structured-v3-epoch5.weights \
  --pairs 4 \
  --nodes 500000 \
  --concurrency 8 \
  --out-dir /tmp/howl-match
Weight against weight:
python tools/matches/fixed_node_self_match.py \
  --candidate build/howl \
  --production build/howl \
  --candidate-weights weights/structured-v3-epoch5.weights \
  --production-weights weights/structured-v3-epoch1.weights \
  --pairs 4 \
  --nodes 500000 \
  --concurrency 8 \
  --out-dir /tmp/howl-weight-match
Classical evaluation is still retained internally as a useful benchmark even though NNUE is now the production evaluator.
Cloud Matches
Howl also contains an Azure based parallel match runner:
tools/matches/cloud_self_match.py
It can distribute games across temporary Azure resources and collect the results into a combined match report and PGN.
I mainly use cloud execution when a larger number of games would take too long locally. For most development work, deterministic diagnostics and local fixed node matches come first.
Building
cmake -S . -B build
cmake --build build -j
./build/howl
Howl speaks UCI and can be used with GUIs such as Arena or directly from the command line.
A minimal session looks like:
uci
isready
position startpos
go depth 12
No evaluator configuration is required for normal use. The production NNUE is loaded automatically.
Development Philosophy
There is still a very large gap between Howl and the strongest modern chess engines.
Closing that gap at any cost is not really the point of this project.
I want to understand why the engine becomes stronger.
That means being able to reason about the search, evaluator, representation, pruning behaviour, training data, diagnostics, and failures rather than simply accumulating techniques until the Elo number increases.
The same principle applies to NNUE.
The fact that the learned evaluator can produce chess ideas that I would never have known how to encode manually makes it more interesting, not less important to understand.
The current challenge is not to retreat from the NNUE because it is slower. It is to make this evaluator fast enough that Howl can keep the intelligence it gained from learning while recovering the search depth needed for practical play.
Project Status
Howl is under active development.
The current engine combines a modern selective search with a learned NNUE evaluator trained from a very large corpus of deeply analysed chess positions.
There are still bugs to find, performance problems to solve, search ideas to test, and a large amount I do not understand yet.
That is part of why I still enjoy working on it.
After 17 years, it finally plays chess.
That is a good place to start.
Acknowledgements
Stockfish was an important reference for learning modern chess search design:
https://github.com/official-stockfish/Stockfish
Lichess provided the position evaluation dataset used to train Howl's NNUE:
https://huggingface.co/datasets/Lichess/chess-position-evaluations
Both projects made knowledge available openly that would otherwise have been extremely difficult to reproduce independently.
I am grateful to everyone who contributed to them.
Repository History
The original public repository was later made private because it contained some personal information.
This repository continues the same project.
Howl has been evolving, disappearing, reappearing, breaking, and being rewritten for roughly 17 years.
It is still the same engine I started all those years ago.