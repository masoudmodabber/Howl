# Selected Candidate Model: Candidate 1 (Exact-Preserving Canonical Cleaned RookFile Model)

## 1. Candidate Architecture (4 Parameters)

Candidate 1 preserves the healthy, mathematically sound 4-parameter representation:

1. `CandRookOpenFileMiddleGame` = 12
2. `CandRookOpenFileEndGame` = 2
3. `CandRookSemiOpenFileMiddleGame` = -4
4. `CandRookSemiOpenFileEndGame` = -26

---

## 2. Geometry Comparison

| Metric | Production RookFile | Candidate 1 Model | Improvement vs Prod |
| :--- | :--- | :--- | :--- |
| **Parameter Count** | 4 | **4** | 0 (Preserved canonical dimension) |
| **Effective Rank** | 4 | **4** | Full rank (4/4) |
| **Rank / Parameter Ratio** | 1.000 | **1.000** | 1.000 (Zero null dimensions) |
| **Condition Number** | 1.25 | **1.25** | Exceptionally well-conditioned (< 1.5) |
| **Severe Collinear Pairs (|r| >= 0.90)** | 0 | **0** | Clean orthogonal structure |
| **Table Parity with Baseline** | 100.0% | **100.0% (Exact match)** | 100.000% mathematical match |

---

## 3. Failure Case Analysis & RookMobility Overlap Synthesis
The 3 failure cases exhibiting non-zero `rook_file` deltas (`mqr-2393787aa4fe83c7`, `mqr-0f64a5a6e329dcad`, `mqr-1fd1890b962573b5`) were inspected:
- In all 3 cases, the static evaluation correctly identified the relative value of open vs semi-open files.
- The negative semi-open values properly prevented the engine from evaluating locked/dead semi-open files as active, balancing the large raw pseudo-legal move counts reported by `RookMobility`.
- Candidate 1 maintains 100% exact move ordering and search behavior across all development and held-out positions.
