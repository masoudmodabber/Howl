# Howl process-memory results

These measurements report process resident-set size (RSS) and its kernel-reported high-water mark. Cache entry counts are reported separately from total process memory.

## Production diagnostic-copy impact

Both modes use five independent Release processes with one warmup and one measured corpus per process.

| Diagnostic board copies | Selected process | Aggregate elapsed ms | Search::moveCount | NPS | Peak RSS MiB |
|---|---:|---:|---:|---:|---:|
| Disabled (`UCI::IsRelease=true`) | 5 | 506.003 | 477406 | 943484 | 14.621 |
| Enabled (`UCI::IsRelease=false`) | 4 | 744.981 | 477406 | 640829 | 14.695 |

Disabled aggregate elapsed/HWM samples: P1=500.828 ms/14.660 MiB, P2=520.394 ms/14.621 MiB, P3=508.517 ms/14.656 MiB, P4=501.739 ms/14.652 MiB, P5=506.003 ms/14.621 MiB
Enabled aggregate elapsed/HWM samples: P1=731.245 ms/14.707 MiB, P2=729.271 ms/14.703 MiB, P3=745.257 ms/14.656 MiB, P4=744.981 ms/14.695 MiB, P5=745.001 ms/14.703 MiB

## Diagnostic recursive search stress (preliminary main-thread run)

- Build: Release (`-O3 -DNDEBUG -std=gnu++17`)
- Diagnostic copies: enabled (`UCI::IsRelease=false`, matching the current production default)
- Protocol: three fresh processes per case; search ran on the main thread; no cache clears or resizing
- Known fixed cache allocation: 8.312 MiB (8 MiB evaluation + 256 KiB Exchange + 64 KiB ExchangeWithoutBeginPiece)

| Case | Depth | MultiPV | Repeat | Elapsed ms | Search::moveCount | Best move | Score | Final HWM MiB | HWM minus fixed caches MiB |
|---|---:|---:|---:|---:|---:|---|---:|---:|---:|
| Kiwipete branching | 7 | 1 | 1 | 14005.761 | 8948260 | e2a6 | 77 | 13.023 | 4.711 |
| Kiwipete branching | 7 | 1 | 2 | 13875.612 | 8948260 | e2a6 | 77 | 13.020 | 4.707 |
| Kiwipete branching | 7 | 1 | 3 | 13985.844 | 8948260 | e2a6 | 77 | 13.020 | 4.707 |
| King safety tactics | 7 | 1 | 1 | 7670.573 | 4648065 | c3d5 | 37 | 12.969 | 4.656 |
| King safety tactics | 7 | 1 | 2 | 7667.888 | 4648065 | c3d5 | 37 | 13.031 | 4.719 |
| King safety tactics | 7 | 1 | 3 | 7667.282 | 4648065 | c3d5 | 37 | 12.992 | 4.680 |
| Checking QSearch line | 6 | 1 | 1 | 631.583 | 564091 | d2d8 | 159998 | 12.938 | 4.625 |
| Checking QSearch line | 6 | 1 | 2 | 637.716 | 564091 | d2d8 | 159998 | 12.961 | 4.648 |
| Checking QSearch line | 6 | 1 | 3 | 616.319 | 564091 | d2d8 | 159998 | 12.965 | 4.652 |
| Promotion tactics | 7 | 1 | 1 | 3040.789 | 1963768 | d7c8q | 721 | 12.984 | 4.672 |
| Promotion tactics | 7 | 1 | 2 | 3054.421 | 1963768 | d7c8q | 721 | 12.977 | 4.664 |
| Promotion tactics | 7 | 1 | 3 | 3054.305 | 1963768 | d7c8q | 721 | 12.977 | 4.664 |
| Advanced pawns and checks | 8 | 1 | 1 | 2050.486 | 1274744 | c4c5 | -547 | 13.020 | 4.707 |
| Advanced pawns and checks | 8 | 1 | 2 | 2087.963 | 1274744 | c4c5 | -547 | 12.965 | 4.652 |
| Advanced pawns and checks | 8 | 1 | 3 | 2065.857 | 1274744 | c4c5 | -547 | 13.023 | 4.711 |
| Kiwipete MultiPV 4 | 6 | 4 | 1 | 9563.585 | 6531078 | e2a6 | 69 | 12.965 | 4.652 |
| Kiwipete MultiPV 4 | 6 | 4 | 2 | 9614.431 | 6531078 | e2a6 | 69 | 12.980 | 4.668 |
| Kiwipete MultiPV 4 | 6 | 4 | 3 | 9578.643 | 6531078 | e2a6 | 69 | 13.023 | 4.711 |

### Per-stage samples

#### Kiwipete branching (repeat 1)

high legal-move branching and complex ordering.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.922 | 11.922 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.973 | 11.973 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.070 | 12.070 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.086 | 12.086 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.801 | 12.805 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.820 | 12.824 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.824 | 12.824 | 0 | 0 | 0 | 0 |
| immediately before search | 12.824 | 12.824 | 0 | 0 | 0 | 0 |
| immediately after search | 13.023 | 13.023 | 524288 | 0 | 9971 | 1536 |
| final process state | 13.023 | 13.023 | 524288 | 0 | 9971 | 1536 |

PV: `e2a6 b4c3 b2c3 e6d5 d2f4 h3g2 e1c1 `

#### King safety tactics (repeat 1)

king attacks, captures, and QSearch continuations.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.852 | 11.852 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.902 | 11.902 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.000 | 12.000 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.016 | 12.016 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.738 | 12.742 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.758 | 12.762 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.762 | 12.762 | 0 | 0 | 0 | 0 |
| immediately before search | 12.762 | 12.762 | 0 | 0 | 0 | 0 |
| immediately after search | 12.969 | 12.969 | 524247 | 0 | 5089 | 902 |
| final process state | 12.969 | 12.969 | 524247 | 0 | 5089 | 902 |

PV: `c3d5 e7d7 g5f6 c6d4 d5e7 d7e7 f3d4 e7f6 e2g4 c5d4 c2c3 `

#### Checking QSearch line (repeat 1)

forced checking continuations beyond the nominal PVS horizon.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.902 | 11.902 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.953 | 11.953 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.051 | 12.051 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.062 | 12.062 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.723 | 12.727 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.742 | 12.746 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.746 | 12.746 | 0 | 0 | 0 | 0 |
| immediately before search | 12.746 | 12.746 | 0 | 0 | 0 | 0 |
| immediately after search | 12.938 | 12.938 | 99081 | 0 | 776 | 174 |
| final process state | 12.938 | 12.938 | 99081 | 0 | 776 | 174 |

PV: `d2d8 b8d8 d1d8 `

#### Promotion tactics (repeat 1)

promotion generation, captures, and tactical QSearch.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.910 | 11.910 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.961 | 11.961 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.059 | 12.059 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.074 | 12.074 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.797 | 12.801 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.816 | 12.820 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.820 | 12.820 | 0 | 0 | 0 | 0 |
| immediately before search | 12.820 | 12.820 | 0 | 0 | 0 | 0 |
| immediately after search | 12.984 | 12.984 | 441866 | 0 | 4564 | 766 |
| final process state | 12.984 | 12.984 | 441866 | 0 | 4564 | 766 |

PV: `d7c8q d8c8 e1f2 b8d7 e2d4 b7b5 c4d3 `

#### Advanced pawns and checks (repeat 1)

check evasions, advanced pawns, promotions, and deep recursion.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.926 | 11.926 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.977 | 11.977 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.074 | 12.074 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.090 | 12.090 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.750 | 12.754 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.770 | 12.773 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.773 | 12.773 | 0 | 0 | 0 | 0 |
| immediately before search | 12.773 | 12.773 | 0 | 0 | 0 | 0 |
| immediately after search | 13.020 | 13.020 | 331010 | 0 | 5684 | 897 |
| final process state | 13.020 | 13.020 | 331010 | 0 | 5684 | 897 |

PV: `c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4 `

#### Kiwipete MultiPV 4 (repeat 1)

production-style four-line root breadth with complex move ordering.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.852 | 11.852 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.902 | 11.902 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.000 | 12.000 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.012 | 12.012 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.734 | 12.738 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.754 | 12.758 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.758 | 12.758 | 0 | 0 | 0 | 0 |
| immediately before search | 12.758 | 12.758 | 0 | 0 | 0 | 0 |
| immediately after search | 12.965 | 12.965 | 524273 | 0 | 9001 | 1480 |
| final process state | 12.965 | 12.965 | 524273 | 0 | 9001 | 1480 |

PV: `e2a6 b4c3 b2c3 e6d5 d2f4 b6c4 `

## Repeated FEN replacement lifecycle

- Protocol: 20,000 position replacements without search in a fresh Release process.
- Identical: the starting-position FEN repeated every iteration.
- Rotating: starting position, Kiwipete, and Position 3 rotated every iteration.

| Workload | Implementation | Iteration 0 RSS MiB | Iteration 1,000 RSS MiB | Iteration 5,000 RSS MiB | Iteration 10,000 RSS MiB | Iteration 20,000 RSS MiB |
|---|---|---:|---:|---:|---:|---:|
| Identical FEN | Before lifecycle fix | 12.633 | 17.133 | 34.957 | 57.234 | 101.789 |
| Rotating FENs | Before lifecycle fix | 12.707 | 17.164 | 34.984 | 57.262 | 101.816 |
| Identical FEN | After lifecycle fix | 12.629 | 12.727 | 12.730 | 12.730 | 12.730 |
| Rotating FENs | After lifecycle fix | 12.617 | 12.672 | 12.676 | 12.676 | 12.676 |

The post-fix plateau includes allocator warm-up. It does not grow with the number of position commands.

## Diagnostic recursive search stress

- Build: Release (`-O3 -DNDEBUG -std=gnu++17`)
- Diagnostic copies: enabled (`UCI::IsRelease=false`, matching the current production default)
- Protocol: three fresh processes per case; one joined production-style search thread; no cache clears or resizing
- Known fixed cache allocation: 8.312 MiB (8 MiB evaluation + 256 KiB Exchange + 64 KiB ExchangeWithoutBeginPiece)

| Case | Depth | MultiPV | Repeat | Elapsed ms | Search::moveCount | Best move | Score | Final HWM MiB | HWM minus fixed caches MiB |
|---|---:|---:|---:|---:|---:|---|---:|---:|---:|
| Kiwipete branching | 7 | 1 | 1 | 15245.846 | 8948260 | e2a6 | 77 | 13.238 | 4.926 |
| Kiwipete branching | 7 | 1 | 2 | 15215.030 | 8948260 | e2a6 | 77 | 13.297 | 4.984 |
| Kiwipete branching | 7 | 1 | 3 | 15131.748 | 8948260 | e2a6 | 77 | 13.258 | 4.945 |
| King safety tactics | 7 | 1 | 1 | 8413.106 | 4648065 | c3d5 | 37 | 13.293 | 4.980 |
| King safety tactics | 7 | 1 | 2 | 8377.132 | 4648065 | c3d5 | 37 | 13.297 | 4.984 |
| King safety tactics | 7 | 1 | 3 | 8392.238 | 4648065 | c3d5 | 37 | 13.281 | 4.969 |
| Checking QSearch line | 6 | 1 | 1 | 682.085 | 564091 | d2d8 | 159998 | 13.219 | 4.906 |
| Checking QSearch line | 6 | 1 | 2 | 680.493 | 564091 | d2d8 | 159998 | 13.207 | 4.895 |
| Checking QSearch line | 6 | 1 | 3 | 675.575 | 564091 | d2d8 | 159998 | 13.242 | 4.930 |
| Promotion tactics | 7 | 1 | 1 | 3365.245 | 1963768 | d7c8q | 721 | 13.250 | 4.938 |
| Promotion tactics | 7 | 1 | 2 | 3378.984 | 1963768 | d7c8q | 721 | 13.230 | 4.918 |
| Promotion tactics | 7 | 1 | 3 | 3369.540 | 1963768 | d7c8q | 721 | 13.258 | 4.945 |
| Advanced pawns and checks | 8 | 1 | 1 | 2269.182 | 1274744 | c4c5 | -547 | 13.250 | 4.938 |
| Advanced pawns and checks | 8 | 1 | 2 | 2294.507 | 1274744 | c4c5 | -547 | 13.250 | 4.938 |
| Advanced pawns and checks | 8 | 1 | 3 | 2269.513 | 1274744 | c4c5 | -547 | 13.246 | 4.934 |
| Kiwipete MultiPV 4 | 6 | 4 | 1 | 10363.567 | 6531078 | e2a6 | 69 | 13.277 | 4.965 |
| Kiwipete MultiPV 4 | 6 | 4 | 2 | 10380.261 | 6531078 | e2a6 | 69 | 13.258 | 4.945 |
| Kiwipete MultiPV 4 | 6 | 4 | 3 | 10320.930 | 6531078 | e2a6 | 69 | 13.297 | 4.984 |

### Per-stage samples

#### Kiwipete branching (repeat 1)

high legal-move branching and complex ordering.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.871 | 11.871 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.922 | 11.922 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.020 | 12.020 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.035 | 12.035 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.695 | 12.699 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.715 | 12.719 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.719 | 12.719 | 0 | 0 | 0 | 0 |
| immediately before search | 12.719 | 12.719 | 0 | 0 | 0 | 0 |
| immediately after search | 13.234 | 13.238 | 524288 | 0 | 9971 | 1536 |
| final process state | 13.238 | 13.238 | 524288 | 0 | 9971 | 1536 |

PV: `e2a6 b4c3 b2c3 e6d5 d2f4 h3g2 e1c1 `

#### King safety tactics (repeat 1)

king attacks, captures, and QSearch continuations.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.906 | 11.906 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.957 | 11.957 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.055 | 12.055 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.070 | 12.070 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.793 | 12.797 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.812 | 12.816 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.816 | 12.816 | 0 | 0 | 0 | 0 |
| immediately before search | 12.816 | 12.816 | 0 | 0 | 0 | 0 |
| immediately after search | 13.289 | 13.293 | 524251 | 0 | 5089 | 902 |
| final process state | 13.293 | 13.293 | 524251 | 0 | 5089 | 902 |

PV: `c3d5 e7d7 g5f6 c6d4 d5e7 d7e7 f3d4 e7f6 e2g4 c5d4 c2c3 `

#### Checking QSearch line (repeat 1)

forced checking continuations beyond the nominal PVS horizon.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.871 | 11.871 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.969 | 11.969 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.066 | 12.066 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.078 | 12.078 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.738 | 12.742 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.758 | 12.762 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.762 | 12.762 | 0 | 0 | 0 | 0 |
| immediately before search | 12.762 | 12.762 | 0 | 0 | 0 | 0 |
| immediately after search | 13.215 | 13.219 | 99086 | 0 | 776 | 174 |
| final process state | 13.219 | 13.219 | 99086 | 0 | 776 | 174 |

PV: `d2d8 b8d8 d1d8 `

#### Promotion tactics (repeat 1)

promotion generation, captures, and tactical QSearch.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.887 | 11.887 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.984 | 11.984 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.082 | 12.082 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.094 | 12.094 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.754 | 12.758 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.773 | 12.777 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.777 | 12.777 | 0 | 0 | 0 | 0 |
| immediately before search | 12.777 | 12.777 | 0 | 0 | 0 | 0 |
| immediately after search | 13.246 | 13.250 | 440663 | 0 | 4564 | 766 |
| final process state | 13.250 | 13.250 | 440663 | 0 | 4564 | 766 |

PV: `d7c8q d8c8 e1f2 b8d7 e2d4 b7b5 c4d3 `

#### Advanced pawns and checks (repeat 1)

check evasions, advanced pawns, promotions, and deep recursion.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.887 | 11.887 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.938 | 11.938 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.035 | 12.035 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.047 | 12.047 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.707 | 12.711 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.727 | 12.730 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.730 | 12.730 | 0 | 0 | 0 | 0 |
| immediately before search | 12.730 | 12.730 | 0 | 0 | 0 | 0 |
| immediately after search | 13.246 | 13.250 | 330936 | 0 | 5684 | 897 |
| final process state | 13.250 | 13.250 | 330936 | 0 | 5684 | 897 |

PV: `c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4 `

#### Kiwipete MultiPV 4 (repeat 1)

production-style four-line root breadth with complex move ordering.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.875 | 11.875 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.988 | 11.988 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.086 | 12.086 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.102 | 12.102 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.762 | 12.766 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.781 | 12.785 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.785 | 12.785 | 0 | 0 | 0 | 0 |
| immediately before search | 12.785 | 12.785 | 0 | 0 | 0 | 0 |
| immediately after search | 13.273 | 13.277 | 524277 | 0 | 9001 | 1480 |
| final process state | 13.277 | 13.277 | 524277 | 0 | 9001 | 1480 |

PV: `e2a6 b4c3 b2c3 e6d5 d2f4 b6c4 `


## Production diagnostic-copy impact

Both modes use five independent Release processes with one warmup and one measured corpus per process.

| Diagnostic board copies | Selected process | Aggregate elapsed ms | Search::moveCount | NPS | Peak RSS MiB |
|---|---:|---:|---:|---:|---:|
| Disabled (`UCI::IsRelease=true`) | 2 | 211.454 | 313954 | 1484738 | 12.887 |
| Enabled (`UCI::IsRelease=false`) | 2 | 375.717 | 313954 | 835612 | 12.945 |

Disabled aggregate elapsed/HWM samples: P1=211.425 ms/12.902 MiB, P2=211.454 ms/12.887 MiB, P3=213.703 ms/12.902 MiB, P4=213.756 ms/12.891 MiB, P5=208.679 ms/12.879 MiB
Enabled aggregate elapsed/HWM samples: P1=376.643 ms/12.961 MiB, P2=375.717 ms/12.945 MiB, P3=375.346 ms/12.898 MiB, P4=376.789 ms/12.957 MiB, P5=372.315 ms/12.926 MiB

## Diagnostic recursive search stress after FEN lifecycle fix

- Build: Release (`-O3 -DNDEBUG -std=gnu++17`)
- Diagnostic copies: enabled (`UCI::IsRelease=false`, matching the current production default)
- Protocol: three fresh processes per case; one joined production-style search thread; no cache clears or resizing
- Known fixed cache allocation: 8.312 MiB (8 MiB evaluation + 256 KiB Exchange + 64 KiB ExchangeWithoutBeginPiece)

| Case | Depth | MultiPV | Repeat | Elapsed ms | Search::moveCount | Best move | Score | Final HWM MiB | HWM minus fixed caches MiB |
|---|---:|---:|---:|---:|---:|---|---:|---:|---:|
| Kiwipete branching | 7 | 1 | 1 | 15378.718 | 8948260 | e2a6 | 77 | 13.293 | 4.980 |
| Kiwipete branching | 7 | 1 | 2 | 15284.892 | 8948260 | e2a6 | 77 | 13.238 | 4.926 |
| Kiwipete branching | 7 | 1 | 3 | 15181.082 | 8948260 | e2a6 | 77 | 13.230 | 4.918 |
| King safety tactics | 7 | 1 | 1 | 8385.608 | 4648065 | c3d5 | 37 | 13.250 | 4.938 |
| King safety tactics | 7 | 1 | 2 | 8399.230 | 4648065 | c3d5 | 37 | 13.285 | 4.973 |
| King safety tactics | 7 | 1 | 3 | 8404.526 | 4648065 | c3d5 | 37 | 13.297 | 4.984 |
| Checking QSearch line | 6 | 1 | 1 | 686.831 | 564091 | d2d8 | 159998 | 13.250 | 4.938 |
| Checking QSearch line | 6 | 1 | 2 | 686.018 | 564091 | d2d8 | 159998 | 13.223 | 4.910 |
| Checking QSearch line | 6 | 1 | 3 | 678.273 | 564091 | d2d8 | 159998 | 13.250 | 4.938 |
| Promotion tactics | 7 | 1 | 1 | 3391.728 | 1963768 | d7c8q | 721 | 13.262 | 4.949 |
| Promotion tactics | 7 | 1 | 2 | 3389.277 | 1963768 | d7c8q | 721 | 13.223 | 4.910 |
| Promotion tactics | 7 | 1 | 3 | 3404.731 | 1963768 | d7c8q | 721 | 13.258 | 4.945 |
| Advanced pawns and checks | 8 | 1 | 1 | 2280.104 | 1274744 | c4c5 | -547 | 13.262 | 4.949 |
| Advanced pawns and checks | 8 | 1 | 2 | 2287.791 | 1274744 | c4c5 | -547 | 13.289 | 4.977 |
| Advanced pawns and checks | 8 | 1 | 3 | 2300.235 | 1274744 | c4c5 | -547 | 13.297 | 4.984 |
| Kiwipete MultiPV 4 | 6 | 4 | 1 | 10462.574 | 6531078 | e2a6 | 69 | 13.301 | 4.988 |
| Kiwipete MultiPV 4 | 6 | 4 | 2 | 10438.022 | 6531078 | e2a6 | 69 | 13.289 | 4.977 |
| Kiwipete MultiPV 4 | 6 | 4 | 3 | 10403.961 | 6531078 | e2a6 | 69 | 13.289 | 4.977 |

### Per-stage samples

#### Kiwipete branching (repeat 1)

high legal-move branching and complex ordering.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.934 | 11.934 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.984 | 11.984 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.082 | 12.082 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.094 | 12.094 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.754 | 12.758 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.773 | 12.777 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.820 | 12.820 | 0 | 0 | 0 | 0 |
| immediately before search | 12.820 | 12.820 | 0 | 0 | 0 | 0 |
| immediately after search | 13.289 | 13.293 | 524288 | 0 | 9971 | 1536 |
| final process state | 13.293 | 13.293 | 524288 | 0 | 9971 | 1536 |

PV: `e2a6 b4c3 b2c3 e6d5 d2f4 h3g2 e1c1 `

#### King safety tactics (repeat 1)

king attacks, captures, and QSearch continuations.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.855 | 11.855 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.961 | 11.961 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.059 | 12.059 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.070 | 12.070 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.730 | 12.734 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.750 | 12.754 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.773 | 12.773 | 0 | 0 | 0 | 0 |
| immediately before search | 12.773 | 12.773 | 0 | 0 | 0 | 0 |
| immediately after search | 13.246 | 13.250 | 524249 | 0 | 5089 | 902 |
| final process state | 13.250 | 13.250 | 524249 | 0 | 5089 | 902 |

PV: `c3d5 e7d7 g5f6 c6d4 d5e7 d7e7 f3d4 e7f6 e2g4 c5d4 c2c3 `

#### Checking QSearch line (repeat 1)

forced checking continuations beyond the nominal PVS horizon.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.902 | 11.902 | 0 | 0 | 0 | 0 |
| after Option initialization | 12.004 | 12.004 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.102 | 12.102 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.117 | 12.117 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.777 | 12.781 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.797 | 12.801 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.824 | 12.824 | 0 | 0 | 0 | 0 |
| immediately before search | 12.824 | 12.824 | 0 | 0 | 0 | 0 |
| immediately after search | 13.246 | 13.250 | 99089 | 0 | 776 | 174 |
| final process state | 13.250 | 13.250 | 99089 | 0 | 776 | 174 |

PV: `d2d8 b8d8 d1d8 `

#### Promotion tactics (repeat 1)

promotion generation, captures, and tactical QSearch.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.883 | 11.883 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.988 | 11.988 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.086 | 12.086 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.102 | 12.102 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.762 | 12.766 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.781 | 12.785 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.820 | 12.820 | 0 | 0 | 0 | 0 |
| immediately before search | 12.820 | 12.820 | 0 | 0 | 0 | 0 |
| immediately after search | 13.258 | 13.262 | 441821 | 0 | 4564 | 766 |
| final process state | 13.262 | 13.262 | 441821 | 0 | 4564 | 766 |

PV: `d7c8q d8c8 e1f2 b8d7 e2d4 b7b5 c4d3 `

#### Advanced pawns and checks (repeat 1)

check evasions, advanced pawns, promotions, and deep recursion.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.883 | 11.883 | 0 | 0 | 0 | 0 |
| after Option initialization | 11.934 | 11.934 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.031 | 12.031 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.043 | 12.043 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.766 | 12.770 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.785 | 12.789 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.789 | 12.789 | 0 | 0 | 0 | 0 |
| immediately before search | 12.789 | 12.789 | 0 | 0 | 0 | 0 |
| immediately after search | 13.258 | 13.262 | 331310 | 0 | 5684 | 897 |
| final process state | 13.262 | 13.262 | 331310 | 0 | 5684 | 897 |

PV: `c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4 `

#### Kiwipete MultiPV 4 (repeat 1)

production-style four-line root breadth with complex move ordering.

| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| process entry after fixed cache allocation | 11.902 | 11.902 | 0 | 0 | 0 | 0 |
| after Option initialization | 12.008 | 12.008 | 0 | 0 | 0 | 0 |
| after attack-table initialization | 12.105 | 12.105 | 0 | 0 | 0 | 0 |
| after board/Zobrist initialization | 12.121 | 12.121 | 0 | 0 | 0 | 0 |
| after move-table initialization | 12.781 | 12.785 | 0 | 0 | 0 | 0 |
| after complete engine initialization | 12.801 | 12.805 | 0 | 0 | 0 | 0 |
| after loading stress position | 12.824 | 12.824 | 0 | 0 | 0 | 0 |
| immediately before search | 12.824 | 12.824 | 0 | 0 | 0 | 0 |
| immediately after search | 13.297 | 13.301 | 524283 | 0 | 9001 | 1480 |
| final process state | 13.301 | 13.301 | 524283 | 0 | 9001 | 1480 |

PV: `e2a6 b4c3 b2c3 e6d5 d2f4 b6c4 `

## UCI Hash total-process validation

Fresh Release process for each Hash value; Kiwipete fixed-depth 4 search.

| Hash MiB | Entry RSS MiB | After configuration RSS MiB | After search RSS MiB | Final RSS MiB | Final HWM MiB | Under limit |
|---:|---:|---:|---:|---:|---:|---|
| 8 | 3.613 | 4.441 | 6.062 | 6.062 | 6.062 | yes |
| 16 | 3.598 | 4.480 | 13.066 | 13.066 | 13.066 | yes |
| 40 | 3.594 | 4.484 | 13.070 | 13.070 | 13.070 | yes |
| 1024 | 3.562 | 4.453 | 13.066 | 13.066 | 13.066 | yes |

## Production diagnostic-copy impact

Both modes use five independent Release processes with one warmup and one measured corpus per process.

| Diagnostic board copies | Selected process | Aggregate elapsed ms | Search::moveCount | NPS | Peak RSS MiB |
|---|---:|---:|---:|---:|---:|
| Disabled (`UCI::IsRelease=true`) | 1 | 210.750 | 313954 | 1489698 | 12.973 |
| Enabled (`UCI::IsRelease=false`) | 3 | 375.291 | 313954 | 836561 | 12.934 |

Disabled aggregate elapsed/HWM samples: P1=210.750 ms/12.973 MiB, P2=212.076 ms/12.945 MiB, P3=212.477 ms/12.965 MiB, P4=209.087 ms/12.973 MiB, P5=209.353 ms/12.973 MiB
Enabled aggregate elapsed/HWM samples: P1=385.319 ms/13.012 MiB, P2=377.878 ms/12.996 MiB, P3=375.291 ms/12.934 MiB, P4=374.844 ms/12.996 MiB, P5=374.715 ms/13.031 MiB
