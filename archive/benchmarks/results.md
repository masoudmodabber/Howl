# Howl benchmark results

Benchmark measurements are diagnostic only. Elapsed time, NPS, `Search::moveCount`, depth, score, and PV do not independently establish that one engine version is better or stronger than another.

## Baseline before optimization

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 43.519 | 61159 | 1405338 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 154.916 | 127276 | 821582 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 120.388 | 103064 | 856098 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 7.709 | 14873 | 1929379 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 37.882 | 27767 | 732989 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 155.680 | 143267 | 920264 | c4c5 | -515 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 g6e4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 520.094 | 477406 | 917923 |

Aggregate elapsed times for all five processes: P1=526.561 ms, P2=520.094 ms, P3=516.122 ms, P4=526.217 ms, P5=507.307 ms

## Memory instrumentation baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 41.807 | 61159 | 1462886 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 149.153 | 127276 | 853325 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 114.344 | 103064 | 901350 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 7.128 | 14873 | 2086420 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 36.999 | 27767 | 750477 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 149.839 | 143267 | 956139 | c4c5 | -515 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 g6e4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 499.271 | 477406 | 956206 |

Aggregate elapsed times for all five processes: P1=515.941 ms, P2=498.261 ms, P3=498.876 ms, P4=499.271 ms, P5=504.123 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 4.418 | 4.426 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 4.430 | 4.430 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 5.812 | 5.812 | 19889 | 0 | 1049 | 227 |
| warmup loaded: Kiwipete | 5.812 | 5.812 | 19889 | 0 | 1049 | 227 |
| warmup after search: Kiwipete | 9.074 | 9.074 | 70845 | 0 | 3093 | 513 |
| warmup loaded: King safety | 9.074 | 9.074 | 70845 | 0 | 3093 | 513 |
| warmup after search: King safety | 12.551 | 12.551 | 127086 | 0 | 3530 | 573 |
| warmup loaded: Endgame | 12.551 | 12.551 | 127086 | 0 | 3530 | 573 |
| warmup after search: Endgame | 12.777 | 12.777 | 131106 | 0 | 3578 | 584 |
| warmup loaded: Promotion tactic | 12.777 | 12.777 | 131106 | 0 | 3578 | 584 |
| warmup after search: Promotion tactic | 13.391 | 13.395 | 140680 | 0 | 3859 | 628 |
| warmup loaded: Advanced pawns/check evasion | 13.395 | 13.395 | 140680 | 0 | 3859 | 628 |
| warmup after search: Advanced pawns/check evasion | 14.625 | 14.625 | 159978 | 0 | 4514 | 679 |
| after complete warmup | 14.625 | 14.625 | 159978 | 0 | 4514 | 679 |
| measured loaded: Quiet middlegame | 14.625 | 14.625 | 159978 | 0 | 4514 | 679 |
| measured after search: Quiet middlegame | 14.625 | 14.625 | 159978 | 0 | 4514 | 679 |
| measured loaded: Kiwipete | 14.625 | 14.625 | 159978 | 0 | 4514 | 679 |
| measured after search: Kiwipete | 14.637 | 14.641 | 159978 | 0 | 4514 | 679 |
| measured loaded: King safety | 14.641 | 14.641 | 159978 | 0 | 4514 | 679 |
| measured after search: King safety | 14.641 | 14.641 | 159978 | 0 | 4514 | 679 |
| measured loaded: Endgame | 14.641 | 14.641 | 159978 | 0 | 4514 | 679 |
| measured after search: Endgame | 14.641 | 14.641 | 159978 | 0 | 4514 | 679 |
| measured loaded: Promotion tactic | 14.641 | 14.641 | 159978 | 0 | 4514 | 679 |
| measured after search: Promotion tactic | 14.641 | 14.641 | 159978 | 0 | 4514 | 679 |
| measured loaded: Advanced pawns/check evasion | 14.641 | 14.641 | 159978 | 0 | 4514 | 679 |
| measured after search: Advanced pawns/check evasion | 14.641 | 14.641 | 159978 | 0 | 4514 | 679 |
| final process state | 14.641 | 14.641 | 159978 | 0 | 4514 | 679 |

## Fixed ExchangeWithoutBeginPiece cache lookup

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 42.027 | 61159 | 1455234 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 151.301 | 127276 | 841208 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 113.590 | 103064 | 907331 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 7.134 | 14873 | 2084917 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 37.432 | 27767 | 741789 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 150.334 | 143267 | 952990 | c4c5 | -515 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 g6e4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 501.819 | 477406 | 951351 |

Aggregate elapsed times for all five processes: P1=514.068 ms, P2=501.819 ms, P3=500.738 ms, P4=501.294 ms, P5=510.835 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 4.434 | 4.441 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 4.445 | 4.445 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 5.836 | 5.836 | 19889 | 0 | 1049 | 256 |
| warmup loaded: Kiwipete | 5.836 | 5.836 | 19889 | 0 | 1049 | 256 |
| warmup after search: Kiwipete | 9.094 | 9.094 | 70845 | 0 | 3093 | 583 |
| warmup loaded: King safety | 9.094 | 9.094 | 70845 | 0 | 3093 | 583 |
| warmup after search: King safety | 12.570 | 12.570 | 127086 | 0 | 3530 | 653 |
| warmup loaded: Endgame | 12.570 | 12.570 | 127086 | 0 | 3530 | 653 |
| warmup after search: Endgame | 12.797 | 12.797 | 131106 | 0 | 3578 | 667 |
| warmup loaded: Promotion tactic | 12.797 | 12.797 | 131106 | 0 | 3578 | 667 |
| warmup after search: Promotion tactic | 13.414 | 13.414 | 140680 | 0 | 3859 | 716 |
| warmup loaded: Advanced pawns/check evasion | 13.414 | 13.414 | 140680 | 0 | 3859 | 716 |
| warmup after search: Advanced pawns/check evasion | 14.645 | 14.648 | 159978 | 0 | 4514 | 773 |
| after complete warmup | 14.648 | 14.648 | 159978 | 0 | 4514 | 773 |
| measured loaded: Quiet middlegame | 14.648 | 14.648 | 159978 | 0 | 4514 | 773 |
| measured after search: Quiet middlegame | 14.648 | 14.648 | 159978 | 0 | 4514 | 773 |
| measured loaded: Kiwipete | 14.648 | 14.648 | 159978 | 0 | 4514 | 773 |
| measured after search: Kiwipete | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |
| measured loaded: King safety | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |
| measured after search: King safety | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |
| measured loaded: Endgame | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |
| measured after search: Endgame | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |
| measured loaded: Promotion tactic | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |
| measured after search: Promotion tactic | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |
| measured loaded: Advanced pawns/check evasion | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |
| measured after search: Advanced pawns/check evasion | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |
| final process state | 14.660 | 14.660 | 159978 | 0 | 4514 | 773 |

## Packed exchange representation and collision safe key

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 39.409 | 62609 | 1588707 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 132.278 | 109744 | 829645 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 112.430 | 103261 | 918449 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 5.035 | 8568 | 1701770 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 33.947 | 16663 | 490858 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 16.752 | 13109 | 782523 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 339.850 | 313954 | 923800 |

Aggregate elapsed times for all five processes: P1=339.850 ms, P2=333.901 ms, P3=337.965 ms, P4=342.981 ms, P5=344.152 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 4.375 | 4.383 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 4.426 | 4.426 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 5.824 | 5.828 | 19666 | 0 | 1130 | 258 |
| warmup loaded: Kiwipete | 5.828 | 5.828 | 19666 | 0 | 1130 | 258 |
| warmup after search: Kiwipete | 9.098 | 9.102 | 70515 | 0 | 3465 | 647 |
| warmup loaded: King safety | 9.102 | 9.102 | 70515 | 0 | 3465 | 647 |
| warmup after search: King safety | 12.715 | 12.715 | 129037 | 0 | 3954 | 717 |
| warmup loaded: Endgame | 12.715 | 12.715 | 129037 | 0 | 3954 | 717 |
| warmup after search: Endgame | 12.891 | 12.891 | 132132 | 0 | 4000 | 730 |
| warmup loaded: Promotion tactic | 12.891 | 12.891 | 132132 | 0 | 4000 | 730 |
| warmup after search: Promotion tactic | 13.219 | 13.223 | 136997 | 0 | 4346 | 783 |
| warmup loaded: Advanced pawns/check evasion | 13.223 | 13.223 | 136997 | 0 | 4346 | 783 |
| warmup after search: Advanced pawns/check evasion | 13.496 | 13.500 | 141053 | 0 | 4646 | 827 |
| after complete warmup | 13.500 | 13.500 | 141053 | 0 | 4646 | 827 |
| measured loaded: Quiet middlegame | 13.500 | 13.500 | 141053 | 0 | 4646 | 827 |
| measured after search: Quiet middlegame | 13.504 | 13.504 | 141053 | 0 | 4646 | 827 |
| measured loaded: Kiwipete | 13.504 | 13.504 | 141053 | 0 | 4646 | 827 |
| measured after search: Kiwipete | 13.508 | 13.508 | 141053 | 0 | 4646 | 827 |
| measured loaded: King safety | 13.508 | 13.508 | 141053 | 0 | 4646 | 827 |
| measured after search: King safety | 13.512 | 13.512 | 141053 | 0 | 4646 | 827 |
| measured loaded: Endgame | 13.512 | 13.512 | 141053 | 0 | 4646 | 827 |
| measured after search: Endgame | 13.512 | 13.512 | 141053 | 0 | 4646 | 827 |
| measured loaded: Promotion tactic | 13.512 | 13.512 | 141053 | 0 | 4646 | 827 |
| measured after search: Promotion tactic | 13.516 | 13.516 | 141053 | 0 | 4646 | 827 |
| measured loaded: Advanced pawns/check evasion | 13.516 | 13.516 | 141053 | 0 | 4646 | 827 |
| measured after search: Advanced pawns/check evasion | 13.516 | 13.516 | 141053 | 0 | 4646 | 827 |
| final process state | 13.516 | 13.516 | 141053 | 0 | 4646 | 827 |

## Fixed exchange tables

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 31.225 | 62609 | 2005106 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 93.425 | 109744 | 1174671 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 83.454 | 103261 | 1237344 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 3.886 | 8568 | 2204967 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 23.124 | 16663 | 720580 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 11.544 | 13109 | 1135547 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 246.658 | 313954 | 1272830 |

Aggregate elapsed times for all five processes: P1=246.075 ms, P2=247.907 ms, P3=252.721 ms, P4=246.658 ms, P5=243.655 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 4.688 | 4.695 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 4.699 | 4.699 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 6.047 | 6.047 | 19666 | 0 | 1129 | 257 |
| warmup loaded: Kiwipete | 6.047 | 6.047 | 19666 | 0 | 1129 | 257 |
| warmup after search: Kiwipete | 9.152 | 9.152 | 70515 | 0 | 3442 | 645 |
| warmup loaded: King safety | 9.152 | 9.152 | 70515 | 0 | 3442 | 645 |
| warmup after search: King safety | 12.734 | 12.734 | 129037 | 0 | 3922 | 715 |
| warmup loaded: Endgame | 12.734 | 12.734 | 129037 | 0 | 3922 | 715 |
| warmup after search: Endgame | 12.906 | 12.906 | 132132 | 0 | 3967 | 728 |
| warmup loaded: Promotion tactic | 12.906 | 12.906 | 132132 | 0 | 3967 | 728 |
| warmup after search: Promotion tactic | 13.211 | 13.215 | 136997 | 0 | 4307 | 780 |
| warmup loaded: Advanced pawns/check evasion | 13.215 | 13.215 | 136997 | 0 | 4307 | 780 |
| warmup after search: Advanced pawns/check evasion | 13.469 | 13.469 | 141053 | 0 | 4598 | 824 |
| after complete warmup | 13.469 | 13.469 | 141053 | 0 | 4598 | 824 |
| measured loaded: Quiet middlegame | 13.469 | 13.469 | 141053 | 0 | 4598 | 824 |
| measured after search: Quiet middlegame | 13.473 | 13.473 | 141053 | 0 | 4598 | 824 |
| measured loaded: Kiwipete | 13.473 | 13.473 | 141053 | 0 | 4598 | 824 |
| measured after search: Kiwipete | 13.477 | 13.477 | 141053 | 0 | 4598 | 824 |
| measured loaded: King safety | 13.477 | 13.477 | 141053 | 0 | 4598 | 824 |
| measured after search: King safety | 13.480 | 13.484 | 141053 | 0 | 4598 | 824 |
| measured loaded: Endgame | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |
| measured after search: Endgame | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |
| measured loaded: Promotion tactic | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |
| measured after search: Promotion tactic | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |
| measured loaded: Advanced pawns/check evasion | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |
| measured after search: Advanced pawns/check evasion | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |
| final process state | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |

## Deterministic rook connection evaluation

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 31.602 | 62609 | 1981179 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 93.921 | 109744 | 1168471 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 81.314 | 103261 | 1269910 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 3.954 | 8568 | 2167185 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 23.361 | 16663 | 713284 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 11.651 | 13109 | 1125175 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 245.802 | 313954 | 1277265 |

Aggregate elapsed times for all five processes: P1=244.149 ms, P2=246.942 ms, P3=245.420 ms, P4=246.260 ms, P5=245.802 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 4.754 | 4.762 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 4.766 | 4.766 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 6.059 | 6.059 | 19666 | 0 | 1129 | 257 |
| warmup loaded: Kiwipete | 6.059 | 6.059 | 19666 | 0 | 1129 | 257 |
| warmup after search: Kiwipete | 9.164 | 9.164 | 70515 | 0 | 3442 | 645 |
| warmup loaded: King safety | 9.164 | 9.164 | 70515 | 0 | 3442 | 645 |
| warmup after search: King safety | 12.750 | 12.750 | 129037 | 0 | 3922 | 715 |
| warmup loaded: Endgame | 12.750 | 12.750 | 129037 | 0 | 3922 | 715 |
| warmup after search: Endgame | 12.922 | 12.922 | 132132 | 0 | 3967 | 728 |
| warmup loaded: Promotion tactic | 12.922 | 12.922 | 132132 | 0 | 3967 | 728 |
| warmup after search: Promotion tactic | 13.227 | 13.230 | 136997 | 0 | 4307 | 780 |
| warmup loaded: Advanced pawns/check evasion | 13.230 | 13.230 | 136997 | 0 | 4307 | 780 |
| warmup after search: Advanced pawns/check evasion | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |
| after complete warmup | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |
| measured loaded: Quiet middlegame | 13.484 | 13.484 | 141053 | 0 | 4598 | 824 |
| measured after search: Quiet middlegame | 13.488 | 13.488 | 141053 | 0 | 4598 | 824 |
| measured loaded: Kiwipete | 13.488 | 13.488 | 141053 | 0 | 4598 | 824 |
| measured after search: Kiwipete | 13.492 | 13.492 | 141053 | 0 | 4598 | 824 |
| measured loaded: King safety | 13.492 | 13.492 | 141053 | 0 | 4598 | 824 |
| measured after search: King safety | 13.496 | 13.500 | 141053 | 0 | 4598 | 824 |
| measured loaded: Endgame | 13.500 | 13.500 | 141053 | 0 | 4598 | 824 |
| measured after search: Endgame | 13.500 | 13.500 | 141053 | 0 | 4598 | 824 |
| measured loaded: Promotion tactic | 13.500 | 13.500 | 141053 | 0 | 4598 | 824 |
| measured after search: Promotion tactic | 13.500 | 13.500 | 141053 | 0 | 4598 | 824 |
| measured loaded: Advanced pawns/check evasion | 13.500 | 13.500 | 141053 | 0 | 4598 | 824 |
| measured after search: Advanced pawns/check evasion | 13.500 | 13.500 | 141053 | 0 | 4598 | 824 |
| final process state | 13.500 | 13.500 | 141053 | 0 | 4598 | 824 |

## Fixed evaluation table

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 24.620 | 62609 | 2542963 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 81.927 | 109744 | 1339536 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 70.137 | 103261 | 1472269 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 3.170 | 8568 | 2703085 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 21.551 | 16663 | 773173 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 10.192 | 13109 | 1286243 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 211.597 | 313954 | 1483732 |

Aggregate elapsed times for all five processes: P1=211.032 ms, P2=214.516 ms, P3=211.597 ms, P4=209.457 ms, P5=212.146 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.750 | 12.758 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.762 | 12.762 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.883 | 12.883 | 19666 | 0 | 1129 | 257 |
| warmup loaded: Kiwipete | 12.883 | 12.883 | 19666 | 0 | 1129 | 257 |
| warmup after search: Kiwipete | 12.887 | 12.887 | 70484 | 0 | 3442 | 645 |
| warmup loaded: King safety | 12.887 | 12.887 | 70484 | 0 | 3442 | 645 |
| warmup after search: King safety | 12.898 | 12.898 | 128499 | 0 | 3922 | 715 |
| warmup loaded: Endgame | 12.898 | 12.898 | 128499 | 0 | 3922 | 715 |
| warmup after search: Endgame | 12.902 | 12.902 | 131543 | 0 | 3967 | 728 |
| warmup loaded: Promotion tactic | 12.902 | 12.902 | 131543 | 0 | 3967 | 728 |
| warmup after search: Promotion tactic | 12.902 | 12.902 | 136320 | 0 | 4307 | 780 |
| warmup loaded: Advanced pawns/check evasion | 12.902 | 12.902 | 136320 | 0 | 4307 | 780 |
| warmup after search: Advanced pawns/check evasion | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| after complete warmup | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured loaded: Quiet middlegame | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured after search: Quiet middlegame | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured loaded: Kiwipete | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured after search: Kiwipete | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured loaded: King safety | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured after search: King safety | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured loaded: Endgame | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured after search: Endgame | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured loaded: Promotion tactic | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured after search: Promotion tactic | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured loaded: Advanced pawns/check evasion | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| measured after search: Advanced pawns/check evasion | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |
| final process state | 12.902 | 12.902 | 140290 | 0 | 4598 | 824 |

## IGG presearch disabled

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 10.301 | 27027 | 2623838 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 12.904 | 12421 | 962604 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 10.184 | 10293 | 1010715 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.654 | 1725 | 2638587 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 5.850 | 3852 | 658413 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 3.728 | 7550 | 2025439 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 43.620 | 62868 | 1441274 |

Aggregate elapsed times for all five processes: P1=43.965 ms, P2=43.528 ms, P3=43.604 ms, P4=44.527 ms, P5=43.620 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.738 | 12.746 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.809 | 12.809 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.914 | 12.914 | 15523 | 0 | 1045 | 250 |
| warmup loaded: Kiwipete | 12.914 | 12.914 | 15523 | 0 | 1045 | 250 |
| warmup after search: Kiwipete | 12.922 | 12.922 | 21396 | 0 | 2343 | 455 |
| warmup loaded: King safety | 12.922 | 12.922 | 21396 | 0 | 2343 | 455 |
| warmup after search: King safety | 12.922 | 12.922 | 27549 | 0 | 2679 | 500 |
| warmup loaded: Endgame | 12.922 | 12.922 | 27549 | 0 | 2679 | 500 |
| warmup after search: Endgame | 12.922 | 12.922 | 28208 | 0 | 2721 | 513 |
| warmup loaded: Promotion tactic | 12.922 | 12.922 | 28208 | 0 | 2721 | 513 |
| warmup after search: Promotion tactic | 12.922 | 12.922 | 29264 | 0 | 2919 | 547 |
| warmup loaded: Advanced pawns/check evasion | 12.922 | 12.922 | 29264 | 0 | 2919 | 547 |
| warmup after search: Advanced pawns/check evasion | 12.922 | 12.922 | 32528 | 0 | 3226 | 594 |
| after complete warmup | 12.922 | 12.922 | 32528 | 0 | 3226 | 594 |
| measured loaded: Quiet middlegame | 12.922 | 12.922 | 32528 | 0 | 3226 | 594 |
| measured after search: Quiet middlegame | 12.922 | 12.922 | 32528 | 0 | 3226 | 594 |
| measured loaded: Kiwipete | 12.922 | 12.922 | 32528 | 0 | 3226 | 594 |
| measured after search: Kiwipete | 12.922 | 12.926 | 32528 | 0 | 3226 | 594 |
| measured loaded: King safety | 12.926 | 12.926 | 32528 | 0 | 3226 | 594 |
| measured after search: King safety | 12.926 | 12.926 | 32528 | 0 | 3226 | 594 |
| measured loaded: Endgame | 12.926 | 12.926 | 32528 | 0 | 3226 | 594 |
| measured after search: Endgame | 12.926 | 12.926 | 32528 | 0 | 3226 | 594 |
| measured loaded: Promotion tactic | 12.926 | 12.926 | 32528 | 0 | 3226 | 594 |
| measured after search: Promotion tactic | 12.926 | 12.926 | 32528 | 0 | 3226 | 594 |
| measured loaded: Advanced pawns/check evasion | 12.926 | 12.926 | 32528 | 0 | 3226 | 594 |
| measured after search: Advanced pawns/check evasion | 12.926 | 12.926 | 32528 | 0 | 3226 | 594 |
| final process state | 12.926 | 12.926 | 32528 | 0 | 3226 | 594 |

## QSearch legal check handling

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 12.569 | 50159 | 3990683 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 42.001 | 252646 | 6015259 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 17.237 | 94900 | 5505473 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.341 | 6934 | 5170556 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 11.035 | 71973 | 6522107 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 7.774 | 39097 | 5029513 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 91.957 | 515709 | 5608149 |

Aggregate elapsed times for all five processes: P1=91.957 ms, P2=92.309 ms, P3=92.633 ms, P4=91.832 ms, P5=91.885 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.781 | 12.785 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.844 | 12.848 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.953 | 12.953 | 15666 | 0 | 1047 | 251 |
| warmup loaded: Kiwipete | 12.953 | 12.953 | 15666 | 0 | 1047 | 251 |
| warmup after search: Kiwipete | 12.957 | 12.957 | 25992 | 0 | 2936 | 537 |
| warmup loaded: King safety | 12.957 | 12.957 | 25992 | 0 | 2936 | 537 |
| warmup after search: King safety | 12.961 | 12.961 | 32609 | 0 | 3282 | 586 |
| warmup loaded: Endgame | 12.961 | 12.961 | 32609 | 0 | 3282 | 586 |
| warmup after search: Endgame | 12.961 | 12.961 | 33777 | 0 | 3320 | 598 |
| warmup loaded: Promotion tactic | 12.961 | 12.961 | 33777 | 0 | 3320 | 598 |
| warmup after search: Promotion tactic | 12.961 | 12.961 | 35932 | 0 | 3576 | 634 |
| warmup loaded: Advanced pawns/check evasion | 12.961 | 12.961 | 35932 | 0 | 3576 | 634 |
| warmup after search: Advanced pawns/check evasion | 12.961 | 12.961 | 39795 | 0 | 3943 | 693 |
| after complete warmup | 12.961 | 12.961 | 39795 | 0 | 3943 | 693 |
| measured loaded: Quiet middlegame | 12.961 | 12.961 | 39795 | 0 | 3943 | 693 |
| measured after search: Quiet middlegame | 12.961 | 12.961 | 39795 | 0 | 3943 | 693 |
| measured loaded: Kiwipete | 12.961 | 12.961 | 39795 | 0 | 3943 | 693 |
| measured after search: Kiwipete | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |
| measured loaded: King safety | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |
| measured after search: King safety | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |
| measured loaded: Endgame | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |
| measured after search: Endgame | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |
| measured loaded: Promotion tactic | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |
| measured after search: Promotion tactic | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |
| measured loaded: Advanced pawns/check evasion | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |
| measured after search: Advanced pawns/check evasion | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |
| final process state | 12.965 | 12.965 | 39795 | 0 | 3943 | 693 |

## QSearch promotion and check delta safety

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.588 | 50558 | 3720715 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 60.826 | 322865 | 5307985 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 32.837 | 165001 | 5024917 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 2.775 | 14381 | 5182164 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 39.697 | 219580 | 5531426 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 13.036 | 61896 | 4748181 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 162.759 | 834281 | 5125875 |

Aggregate elapsed times for all five processes: P1=162.375 ms, P2=162.861 ms, P3=162.056 ms, P4=165.177 ms, P5=162.759 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.812 | 12.816 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.844 | 12.848 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.953 | 12.953 | 15711 | 0 | 1066 | 257 |
| warmup loaded: Kiwipete | 12.953 | 12.953 | 15711 | 0 | 1066 | 257 |
| warmup after search: Kiwipete | 12.988 | 12.988 | 29573 | 0 | 3611 | 621 |
| warmup loaded: King safety | 12.988 | 12.988 | 29573 | 0 | 3611 | 621 |
| warmup after search: King safety | 12.988 | 12.988 | 39283 | 0 | 4065 | 687 |
| warmup loaded: Endgame | 12.988 | 12.988 | 39283 | 0 | 4065 | 687 |
| warmup after search: Endgame | 12.988 | 12.988 | 41151 | 0 | 4094 | 698 |
| warmup loaded: Promotion tactic | 12.988 | 12.988 | 41151 | 0 | 4094 | 698 |
| warmup after search: Promotion tactic | 12.988 | 12.988 | 48048 | 0 | 4531 | 761 |
| warmup loaded: Advanced pawns/check evasion | 12.988 | 12.988 | 48048 | 0 | 4531 | 761 |
| warmup after search: Advanced pawns/check evasion | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| after complete warmup | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured loaded: Quiet middlegame | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured after search: Quiet middlegame | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured loaded: Kiwipete | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured after search: Kiwipete | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured loaded: King safety | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured after search: King safety | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured loaded: Endgame | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured after search: Endgame | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured loaded: Promotion tactic | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured after search: Promotion tactic | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured loaded: Advanced pawns/check evasion | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| measured after search: Advanced pawns/check evasion | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |
| final process state | 12.988 | 12.988 | 52672 | 0 | 4884 | 818 |

## QSearch pawn capture continuation

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.180 | 52877 | 3729003 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 66.730 | 351177 | 5262623 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 32.975 | 165083 | 5006263 | c3d5 | 75 | c3d5 e7d7 g5f6 g7f6 d3d4  |
| Endgame | 5 | 3.238 | 17099 | 5281033 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 42.864 | 233311 | 5443009 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 15.235 | 72183 | 4737867 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 175.223 | 891730 | 5089111 |

Aggregate elapsed times for all five processes: P1=177.145 ms, P2=172.921 ms, P3=175.223 ms, P4=174.646 ms, P5=177.848 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.805 | 12.809 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.844 | 12.848 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 13.000 | 13.004 | 32287 | 0 | 3784 | 643 |
| warmup loaded: King safety | 13.004 | 13.004 | 32287 | 0 | 3784 | 643 |
| warmup after search: King safety | 13.004 | 13.004 | 42023 | 0 | 4231 | 708 |
| warmup loaded: Endgame | 13.004 | 13.004 | 42023 | 0 | 4231 | 708 |
| warmup after search: Endgame | 13.004 | 13.004 | 44105 | 0 | 4257 | 719 |
| warmup loaded: Promotion tactic | 13.004 | 13.004 | 44105 | 0 | 4257 | 719 |
| warmup after search: Promotion tactic | 13.004 | 13.004 | 51455 | 0 | 4681 | 784 |
| warmup loaded: Advanced pawns/check evasion | 13.004 | 13.004 | 51455 | 0 | 4681 | 784 |
| warmup after search: Advanced pawns/check evasion | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| after complete warmup | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured loaded: Quiet middlegame | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured after search: Quiet middlegame | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured loaded: Kiwipete | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured after search: Kiwipete | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured loaded: King safety | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured after search: King safety | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured loaded: Endgame | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured after search: Endgame | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured loaded: Promotion tactic | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured after search: Promotion tactic | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured loaded: Advanced pawns/check evasion | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| measured after search: Advanced pawns/check evasion | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |
| final process state | 13.008 | 13.008 | 56505 | 0 | 5037 | 842 |

## QSearch PV check extension alignment

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.262 | 52877 | 3707513 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 43.910 | 238081 | 5422065 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.403 | 167509 | 5014734 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.857 | 9173 | 4938977 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.785 | 88704 | 5619544 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 10.459 | 48964 | 4681491 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 119.676 | 605308 | 5057874 |

Aggregate elapsed times for all five processes: P1=118.900 ms, P2=118.316 ms, P3=121.580 ms, P4=119.676 ms, P5=120.536 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.789 | 12.793 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.797 | 12.801 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.906 | 12.906 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.906 | 12.906 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.934 | 12.934 | 27617 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.934 | 12.934 | 27617 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.934 | 12.934 | 37108 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.934 | 12.934 | 37108 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.934 | 12.934 | 38496 | 0 | 3592 | 627 |
| warmup loaded: Promotion tactic | 12.934 | 12.934 | 38496 | 0 | 3592 | 627 |
| warmup after search: Promotion tactic | 12.934 | 12.934 | 41363 | 0 | 3862 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.934 | 12.934 | 41363 | 0 | 3862 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.941 | 12.941 | 45861 | 0 | 4248 | 732 |
| after complete warmup | 12.941 | 12.941 | 45861 | 0 | 4248 | 732 |
| measured loaded: Quiet middlegame | 12.941 | 12.941 | 45861 | 0 | 4248 | 732 |
| measured after search: Quiet middlegame | 12.941 | 12.941 | 45861 | 0 | 4248 | 732 |
| measured loaded: Kiwipete | 12.941 | 12.941 | 45861 | 0 | 4248 | 732 |
| measured after search: Kiwipete | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |
| measured loaded: King safety | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |
| measured after search: King safety | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |
| measured loaded: Endgame | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |
| measured after search: Endgame | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |
| measured loaded: Promotion tactic | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |
| measured after search: Promotion tactic | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |
| measured loaded: Advanced pawns/check evasion | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |
| measured after search: Advanced pawns/check evasion | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |
| final process state | 12.945 | 12.945 | 45861 | 0 | 4248 | 732 |

## LMR legal move accounting

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.043 | 52877 | 3765243 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 43.810 | 238081 | 5434340 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.721 | 167509 | 4967475 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.778 | 8704 | 4895211 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.832 | 88704 | 5602893 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 10.437 | 48964 | 4691210 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 119.622 | 604839 | 5056237 |

Aggregate elapsed times for all five processes: P1=120.946 ms, P2=118.417 ms, P3=119.940 ms, P4=119.622 ms, P5=119.028 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.816 | 12.820 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.840 | 12.844 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.977 | 12.977 | 27617 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.977 | 12.977 | 27617 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.977 | 12.977 | 37108 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.977 | 12.977 | 37108 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.977 | 12.977 | 38447 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.977 | 12.977 | 38447 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.977 | 12.977 | 41316 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.977 | 12.977 | 41316 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.980 | 12.980 | 45817 | 0 | 4248 | 732 |
| after complete warmup | 12.980 | 12.980 | 45817 | 0 | 4248 | 732 |
| measured loaded: Quiet middlegame | 12.980 | 12.980 | 45817 | 0 | 4248 | 732 |
| measured after search: Quiet middlegame | 12.980 | 12.980 | 45817 | 0 | 4248 | 732 |
| measured loaded: Kiwipete | 12.980 | 12.980 | 45817 | 0 | 4248 | 732 |
| measured after search: Kiwipete | 12.984 | 12.984 | 45817 | 0 | 4248 | 732 |
| measured loaded: King safety | 12.984 | 12.984 | 45817 | 0 | 4248 | 732 |
| measured after search: King safety | 12.984 | 12.984 | 45817 | 0 | 4248 | 732 |
| measured loaded: Endgame | 12.984 | 12.984 | 45817 | 0 | 4248 | 732 |
| measured after search: Endgame | 12.984 | 12.984 | 45817 | 0 | 4248 | 732 |
| measured loaded: Promotion tactic | 12.984 | 12.984 | 45817 | 0 | 4248 | 732 |
| measured after search: Promotion tactic | 12.984 | 12.984 | 45817 | 0 | 4248 | 732 |
| final process state | 12.988 | 12.988 | 45817 | 0 | 4248 | 732 |


## A (Current)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.749 | 52877 | 3845824 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 45.152 | 238081 | 5272820 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.279 | 167509 | 5033470 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.749 | 8704 | 4976680 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 16.082 | 88704 | 5515808 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 10.429 | 48964 | 4694837 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 120.441 | 604839 | 5021879 |

Aggregate elapsed times for all five processes: P1=118.568 ms, P2=120.441 ms, P3=119.377 ms, P4=122.850 ms, P5=124.019 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.785 | 12.789 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.793 | 12.797 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.902 | 12.902 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.902 | 12.902 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.930 | 12.930 | 27618 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.930 | 12.930 | 27618 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.930 | 12.930 | 37109 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.930 | 12.930 | 37109 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.930 | 12.930 | 38448 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.930 | 12.930 | 38448 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.930 | 12.930 | 41317 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.930 | 12.930 | 41317 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.934 | 12.934 | 45812 | 0 | 4248 | 732 |
| after complete warmup | 12.934 | 12.934 | 45812 | 0 | 4248 | 732 |
| measured loaded: Quiet middlegame | 12.934 | 12.934 | 45812 | 0 | 4248 | 732 |
| measured after search: Quiet middlegame | 12.934 | 12.934 | 45812 | 0 | 4248 | 732 |
| measured loaded: Kiwipete | 12.934 | 12.934 | 45812 | 0 | 4248 | 732 |
| measured after search: Kiwipete | 12.938 | 12.938 | 45812 | 0 | 4248 | 732 |
| measured loaded: King safety | 12.938 | 12.938 | 45812 | 0 | 4248 | 732 |
| measured after search: King safety | 12.938 | 12.938 | 45812 | 0 | 4248 | 732 |
| measured loaded: Endgame | 12.938 | 12.938 | 45812 | 0 | 4248 | 732 |
| measured after search: Endgame | 12.938 | 12.938 | 45812 | 0 | 4248 | 732 |
| measured loaded: Promotion tactic | 12.938 | 12.938 | 45812 | 0 | 4248 | 732 |
| measured after search: Promotion tactic | 12.938 | 12.938 | 45812 | 0 | 4248 | 732 |
| measured loaded: Advanced pawns/check evasion | 12.938 | 12.938 | 45812 | 0 | 4248 | 732 |
| measured after search: Advanced pawns/check evasion | 12.941 | 12.941 | 45812 | 0 | 4248 | 732 |
| final process state | 12.941 | 12.941 | 45812 | 0 | 4248 | 732 |

## B (Exempt in-check)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 15.227 | 58794 | 3861040 | g7h6 | 24 | g7h6 d2b3 h6c1 a1c1 c8e6  |
| Kiwipete | 4 | 45.080 | 248507 | 5512524 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 51.252 | 265026 | 5171009 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 3.292 | 17312 | 5258266 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 16.957 | 96215 | 5673973 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 61.474 | 311435 | 5066139 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 193.284 | 997289 | 5159717 |

Aggregate elapsed times for all five processes: P1=193.051 ms, P2=194.404 ms, P3=193.741 ms, P4=193.284 ms, P5=193.283 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.770 | 12.773 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.840 | 12.844 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.949 | 12.953 | 15953 | 0 | 1151 | 264 |
| warmup loaded: Kiwipete | 12.953 | 12.953 | 15953 | 0 | 1151 | 264 |
| warmup after search: Kiwipete | 12.977 | 12.977 | 28253 | 0 | 3180 | 564 |
| warmup loaded: King safety | 12.977 | 12.977 | 28253 | 0 | 3180 | 564 |
| warmup after search: King safety | 12.977 | 12.977 | 42081 | 0 | 3624 | 622 |
| warmup loaded: Endgame | 12.977 | 12.977 | 42081 | 0 | 3624 | 622 |
| warmup after search: Endgame | 12.977 | 12.977 | 44436 | 0 | 3652 | 635 |
| warmup loaded: Promotion tactic | 12.977 | 12.977 | 44436 | 0 | 3652 | 635 |
| warmup after search: Promotion tactic | 12.977 | 12.977 | 47396 | 0 | 3914 | 679 |
| warmup loaded: Advanced pawns/check evasion | 12.977 | 12.977 | 47396 | 0 | 3914 | 679 |
| warmup after search: Advanced pawns/check evasion | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| after complete warmup | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured loaded: Quiet middlegame | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured after search: Quiet middlegame | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured loaded: Kiwipete | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured after search: Kiwipete | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured loaded: King safety | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured after search: King safety | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured loaded: Endgame | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured after search: Endgame | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured loaded: Promotion tactic | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured after search: Promotion tactic | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured loaded: Advanced pawns/check evasion | 12.984 | 12.984 | 58572 | 0 | 4648 | 807 |
| measured after search: Advanced pawns/check evasion | 12.988 | 12.988 | 58572 | 0 | 4648 | 807 |
| final process state | 12.988 | 12.988 | 58572 | 0 | 4648 | 807 |

## C (Exempt promotions)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.387 | 52877 | 3950004 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 42.388 | 238081 | 5616658 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.364 | 167509 | 5020627 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.780 | 8704 | 4888813 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.679 | 88704 | 5657501 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 16.922 | 81774 | 4832487 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 123.520 | 637649 | 5162305 |

Aggregate elapsed times for all five processes: P1=123.746 ms, P2=121.421 ms, P3=123.520 ms, P4=125.739 ms, P5=123.333 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.723 | 12.727 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.793 | 12.797 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.906 | 12.906 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.906 | 12.906 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.930 | 12.930 | 27617 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.930 | 12.930 | 27617 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.930 | 12.930 | 37107 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.930 | 12.930 | 37107 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.930 | 12.930 | 38446 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.930 | 12.930 | 38446 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.930 | 12.930 | 41314 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.930 | 12.930 | 41314 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| after complete warmup | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured loaded: Quiet middlegame | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured after search: Quiet middlegame | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured loaded: Kiwipete | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured after search: Kiwipete | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured loaded: King safety | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured after search: King safety | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured loaded: Endgame | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured after search: Endgame | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured loaded: Promotion tactic | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured after search: Promotion tactic | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured loaded: Advanced pawns/check evasion | 12.938 | 12.938 | 46339 | 0 | 4299 | 739 |
| measured after search: Advanced pawns/check evasion | 12.941 | 12.941 | 46339 | 0 | 4299 | 739 |
| final process state | 12.941 | 12.941 | 46339 | 0 | 4299 | 739 |

## D (Exempt captures)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.606 | 52877 | 3886284 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 44.055 | 246317 | 5591126 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 43.916 | 229857 | 5234027 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.805 | 9132 | 5059232 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 16.647 | 95700 | 5748679 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 25.411 | 126613 | 4982596 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 145.440 | 760496 | 5228922 |

Aggregate elapsed times for all five processes: P1=147.038 ms, P2=145.440 ms, P3=145.766 ms, P4=145.369 ms, P5=144.777 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.801 | 12.805 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.844 | 12.848 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.957 | 12.957 | 16712 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.957 | 12.957 | 16712 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.980 | 12.980 | 27764 | 0 | 3167 | 563 |
| warmup loaded: King safety | 12.980 | 12.980 | 27764 | 0 | 3167 | 563 |
| warmup after search: King safety | 12.980 | 12.980 | 36669 | 0 | 3586 | 615 |
| warmup loaded: Endgame | 12.980 | 12.980 | 36669 | 0 | 3586 | 615 |
| warmup after search: Endgame | 12.980 | 12.980 | 38133 | 0 | 3612 | 626 |
| warmup loaded: Promotion tactic | 12.980 | 12.980 | 38133 | 0 | 3612 | 626 |
| warmup after search: Promotion tactic | 12.980 | 12.980 | 40945 | 0 | 3883 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.980 | 12.980 | 40945 | 0 | 3883 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| after complete warmup | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured loaded: Quiet middlegame | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured after search: Quiet middlegame | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured loaded: Kiwipete | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured after search: Kiwipete | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured loaded: King safety | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured after search: King safety | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured loaded: Endgame | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured after search: Endgame | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured loaded: Promotion tactic | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured after search: Promotion tactic | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured loaded: Advanced pawns/check evasion | 12.988 | 12.988 | 47427 | 0 | 4490 | 762 |
| measured after search: Advanced pawns/check evasion | 12.992 | 12.992 | 47427 | 0 | 4490 | 762 |
| final process state | 12.992 | 12.992 | 47427 | 0 | 4490 | 762 |

## E (Exempt in-check + promotions)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 15.410 | 58794 | 3815264 | g7h6 | 24 | g7h6 d2b3 h6c1 a1c1 c8e6  |
| Kiwipete | 4 | 46.413 | 248507 | 5354290 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 52.444 | 265026 | 5053484 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 3.332 | 17312 | 5195208 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 17.322 | 96215 | 5554380 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 62.755 | 311435 | 4962684 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 197.677 | 997289 | 5045040 |

Aggregate elapsed times for all five processes: P1=194.605 ms, P2=200.142 ms, P3=204.189 ms, P4=197.677 ms, P5=195.162 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.816 | 12.820 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.840 | 12.844 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.953 | 12.957 | 15953 | 0 | 1151 | 264 |
| warmup loaded: Kiwipete | 12.957 | 12.957 | 15953 | 0 | 1151 | 264 |
| warmup after search: Kiwipete | 12.977 | 12.977 | 28253 | 0 | 3180 | 564 |
| warmup loaded: King safety | 12.977 | 12.977 | 28253 | 0 | 3180 | 564 |
| warmup after search: King safety | 12.977 | 12.977 | 42082 | 0 | 3624 | 622 |
| warmup loaded: Endgame | 12.977 | 12.977 | 42082 | 0 | 3624 | 622 |
| warmup after search: Endgame | 12.977 | 12.977 | 44439 | 0 | 3652 | 635 |
| warmup loaded: Promotion tactic | 12.977 | 12.977 | 44439 | 0 | 3652 | 635 |
| warmup after search: Promotion tactic | 12.977 | 12.977 | 47398 | 0 | 3914 | 679 |
| warmup loaded: Advanced pawns/check evasion | 12.977 | 12.977 | 47398 | 0 | 3914 | 679 |
| warmup after search: Advanced pawns/check evasion | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| after complete warmup | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured loaded: Quiet middlegame | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured after search: Quiet middlegame | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured loaded: Kiwipete | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured after search: Kiwipete | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured loaded: King safety | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured after search: King safety | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured loaded: Endgame | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured after search: Endgame | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured loaded: Promotion tactic | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured after search: Promotion tactic | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured loaded: Advanced pawns/check evasion | 12.984 | 12.984 | 58575 | 0 | 4648 | 807 |
| measured after search: Advanced pawns/check evasion | 12.988 | 12.988 | 58575 | 0 | 4648 | 807 |
| final process state | 12.988 | 12.988 | 58575 | 0 | 4648 | 807 |

## F (Exempt in-check + captures)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 15.241 | 58794 | 3857503 | g7h6 | 24 | g7h6 d2b3 h6c1 a1c1 c8e6  |
| Kiwipete | 4 | 45.466 | 248507 | 5465783 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 51.783 | 265026 | 5118027 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 3.146 | 15792 | 5019402 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 17.125 | 96215 | 5618349 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 62.103 | 311435 | 5014796 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 194.865 | 995769 | 5110050 |

Aggregate elapsed times for all five processes: P1=203.044 ms, P2=194.865 ms, P3=194.422 ms, P4=194.715 ms, P5=195.223 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.766 | 12.770 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.836 | 12.840 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.945 | 12.949 | 15953 | 0 | 1151 | 264 |
| warmup loaded: Kiwipete | 12.949 | 12.949 | 15953 | 0 | 1151 | 264 |
| warmup after search: Kiwipete | 12.969 | 12.969 | 28253 | 0 | 3180 | 564 |
| warmup loaded: King safety | 12.969 | 12.969 | 28253 | 0 | 3180 | 564 |
| warmup after search: King safety | 12.969 | 12.969 | 42083 | 0 | 3624 | 622 |
| warmup loaded: Endgame | 12.969 | 12.969 | 42083 | 0 | 3624 | 622 |
| warmup after search: Endgame | 12.969 | 12.969 | 44543 | 0 | 3653 | 635 |
| warmup loaded: Promotion tactic | 12.969 | 12.969 | 44543 | 0 | 3653 | 635 |
| warmup after search: Promotion tactic | 12.969 | 12.969 | 47501 | 0 | 3915 | 679 |
| warmup loaded: Advanced pawns/check evasion | 12.969 | 12.969 | 47501 | 0 | 3915 | 679 |
| warmup after search: Advanced pawns/check evasion | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| after complete warmup | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured loaded: Quiet middlegame | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured after search: Quiet middlegame | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured loaded: Kiwipete | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured after search: Kiwipete | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured loaded: King safety | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured after search: King safety | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured loaded: Endgame | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured after search: Endgame | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured loaded: Promotion tactic | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured after search: Promotion tactic | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured loaded: Advanced pawns/check evasion | 12.980 | 12.980 | 58676 | 0 | 4648 | 807 |
| measured after search: Advanced pawns/check evasion | 12.984 | 12.984 | 58676 | 0 | 4648 | 807 |
| final process state | 12.984 | 12.984 | 58676 | 0 | 4648 | 807 |

## LMR promotion exemption

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.742 | 52877 | 3847960 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 43.338 | 238081 | 5493610 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.325 | 167509 | 5026578 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.762 | 8704 | 4940898 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.664 | 88704 | 5662762 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 16.967 | 81774 | 4819685 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 124.797 | 637649 | 5109499 |

Aggregate elapsed times for all five processes: P1=127.052 ms, P2=124.698 ms, P3=125.621 ms, P4=124.797 ms, P5=124.472 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.781 | 12.785 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.840 | 12.844 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.977 | 12.977 | 27618 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.977 | 12.977 | 27618 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.977 | 12.977 | 37108 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.977 | 12.977 | 37108 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.977 | 12.977 | 38447 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.977 | 12.977 | 38447 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.977 | 12.977 | 41316 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.977 | 12.977 | 41316 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| after complete warmup | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured loaded: Quiet middlegame | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured after search: Quiet middlegame | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured loaded: Kiwipete | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured after search: Kiwipete | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured loaded: King safety | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured after search: King safety | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured loaded: Endgame | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured after search: Endgame | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured loaded: Promotion tactic | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured after search: Promotion tactic | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured loaded: Advanced pawns/check evasion | 12.984 | 12.984 | 46338 | 0 | 4299 | 739 |
| measured after search: Advanced pawns/check evasion | 12.988 | 12.988 | 46338 | 0 | 4299 | 739 |
| final process state | 12.988 | 12.988 | 46338 | 0 | 4299 | 739 |

## 1 (Exempt captures where captured >= moving)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.844 | 52877 | 3819506 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 44.181 | 242063 | 5478951 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 35.327 | 167509 | 4741615 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.820 | 8704 | 4783545 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.887 | 88704 | 5583291 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 18.898 | 88056 | 4659619 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 129.957 | 647913 | 4985612 |

Aggregate elapsed times for all five processes: P1=128.286 ms, P2=129.957 ms, P3=130.134 ms, P4=129.035 ms, P5=131.271 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.824 | 12.828 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.832 | 12.836 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.941 | 12.941 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.941 | 12.941 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.961 | 12.961 | 27593 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.961 | 12.961 | 27593 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.961 | 12.961 | 37082 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.961 | 12.961 | 37082 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.961 | 12.961 | 38421 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.961 | 12.961 | 38421 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.961 | 12.961 | 41287 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.961 | 12.961 | 41287 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| after complete warmup | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured loaded: Quiet middlegame | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured after search: Quiet middlegame | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured loaded: Kiwipete | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured after search: Kiwipete | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured loaded: King safety | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured after search: King safety | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured loaded: Endgame | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured after search: Endgame | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured loaded: Promotion tactic | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured after search: Promotion tactic | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured loaded: Advanced pawns/check evasion | 12.977 | 12.977 | 46612 | 0 | 4304 | 740 |
| measured after search: Advanced pawns/check evasion | 12.980 | 12.980 | 46612 | 0 | 4304 | 740 |
| final process state | 12.980 | 12.980 | 46612 | 0 | 4304 | 740 |

## 2 (Exempt captures giving check)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.822 | 52877 | 3825499 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 42.981 | 238081 | 5539240 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 32.983 | 167509 | 5078664 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.841 | 9132 | 4959429 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.513 | 88704 | 5718222 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 17.869 | 86594 | 4846053 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 125.009 | 642897 | 5142815 |

Aggregate elapsed times for all five processes: P1=126.397 ms, P2=125.009 ms, P3=125.923 ms, P4=124.599 ms, P5=124.176 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.785 | 12.789 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.848 | 12.852 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.957 | 12.957 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.957 | 12.957 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.980 | 12.980 | 27617 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.980 | 12.980 | 27617 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.980 | 12.980 | 37183 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.980 | 12.980 | 37183 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.980 | 12.980 | 38637 | 0 | 3592 | 627 |
| warmup loaded: Promotion tactic | 12.980 | 12.980 | 38637 | 0 | 3592 | 627 |
| warmup after search: Promotion tactic | 12.980 | 12.980 | 41506 | 0 | 3862 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.980 | 12.980 | 41506 | 0 | 3862 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| after complete warmup | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured loaded: Quiet middlegame | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured after search: Quiet middlegame | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured loaded: Kiwipete | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured after search: Kiwipete | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured loaded: King safety | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured after search: King safety | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured loaded: Endgame | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured after search: Endgame | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured loaded: Promotion tactic | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured after search: Promotion tactic | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured loaded: Advanced pawns/check evasion | 12.992 | 12.992 | 46577 | 0 | 4333 | 739 |
| measured after search: Advanced pawns/check evasion | 12.996 | 12.996 | 46577 | 0 | 4333 | 739 |
| final process state | 12.996 | 12.996 | 46577 | 0 | 4333 | 739 |

## Rule 1 (Exempt captures where captured >= moving)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.145 | 52877 | 3738155 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 44.729 | 242063 | 5411802 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 34.457 | 167509 | 4861343 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.787 | 8704 | 4870152 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.724 | 88704 | 5641474 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 18.625 | 88056 | 4727788 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 129.467 | 647913 | 5004455 |

Aggregate elapsed times for all five processes: P1=129.599 ms, P2=128.366 ms, P3=129.467 ms, P4=128.525 ms, P5=129.476 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.824 | 12.828 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.832 | 12.836 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.941 | 12.941 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.941 | 12.941 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.965 | 12.965 | 27593 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.965 | 12.965 | 27593 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.965 | 12.965 | 37084 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.965 | 12.965 | 37084 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.965 | 12.965 | 38423 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.965 | 12.965 | 38423 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.965 | 12.965 | 41290 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.965 | 12.965 | 41290 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| after complete warmup | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured loaded: Quiet middlegame | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured after search: Quiet middlegame | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured loaded: Kiwipete | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured after search: Kiwipete | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured loaded: King safety | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured after search: King safety | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured loaded: Endgame | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured after search: Endgame | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured loaded: Promotion tactic | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured after search: Promotion tactic | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured loaded: Advanced pawns/check evasion | 12.977 | 12.977 | 46614 | 0 | 4304 | 740 |
| measured after search: Advanced pawns/check evasion | 12.980 | 12.980 | 46614 | 0 | 4304 | 740 |
| final process state | 12.980 | 12.980 | 46614 | 0 | 4304 | 740 |

## Rule 2 (Exempt captures giving check)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.035 | 52877 | 3767383 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 42.873 | 238081 | 5553202 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.118 | 167509 | 5057875 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.835 | 9132 | 4976775 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.653 | 88704 | 5666903 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 18.123 | 86594 | 4778078 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 125.638 | 642897 | 5117068 |

Aggregate elapsed times for all five processes: P1=125.128 ms, P2=125.786 ms, P3=126.072 ms, P4=125.314 ms, P5=125.638 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.793 | 12.797 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.844 | 12.848 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.977 | 12.977 | 27618 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.977 | 12.977 | 27618 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.977 | 12.977 | 37184 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.977 | 12.977 | 37184 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.977 | 12.977 | 38638 | 0 | 3592 | 627 |
| warmup loaded: Promotion tactic | 12.977 | 12.977 | 38638 | 0 | 3592 | 627 |
| warmup after search: Promotion tactic | 12.977 | 12.977 | 41506 | 0 | 3862 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.977 | 12.977 | 41506 | 0 | 3862 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| after complete warmup | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured loaded: Quiet middlegame | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured after search: Quiet middlegame | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured loaded: Kiwipete | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured after search: Kiwipete | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured loaded: King safety | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured after search: King safety | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured loaded: Endgame | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured after search: Endgame | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured loaded: Promotion tactic | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured after search: Promotion tactic | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured loaded: Advanced pawns/check evasion | 12.988 | 12.988 | 46576 | 0 | 4333 | 739 |
| measured after search: Advanced pawns/check evasion | 12.992 | 12.992 | 46576 | 0 | 4333 | 739 |
| final process state | 12.992 | 12.992 | 46576 | 0 | 4333 | 739 |

## Rule 3 (Exempt in-check with 1 evasion)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.969 | 52877 | 3785220 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 43.391 | 238081 | 5486842 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.362 | 167509 | 5020981 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.801 | 8937 | 4963373 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.745 | 88704 | 5633820 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 17.067 | 81774 | 4791307 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 125.335 | 637882 | 5089414 |

Aggregate elapsed times for all five processes: P1=123.498 ms, P2=125.102 ms, P3=125.954 ms, P4=125.335 ms, P5=125.379 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.812 | 12.816 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.844 | 12.848 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.957 | 12.957 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.957 | 12.957 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.980 | 12.980 | 27618 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.980 | 12.980 | 27618 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.980 | 12.980 | 37107 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.980 | 12.980 | 37107 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.980 | 12.980 | 38443 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.980 | 12.980 | 38443 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.980 | 12.980 | 41311 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.980 | 12.980 | 41311 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| after complete warmup | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured loaded: Quiet middlegame | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured after search: Quiet middlegame | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured loaded: Kiwipete | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured after search: Kiwipete | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured loaded: King safety | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured after search: King safety | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured loaded: Endgame | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured after search: Endgame | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured loaded: Promotion tactic | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured after search: Promotion tactic | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured loaded: Advanced pawns/check evasion | 12.988 | 12.988 | 46333 | 0 | 4299 | 739 |
| measured after search: Advanced pawns/check evasion | 12.992 | 12.992 | 46333 | 0 | 4299 | 739 |
| final process state | 12.992 | 12.992 | 46333 | 0 | 4299 | 739 |

## Rule 4 (Exempt in-check with <=2 evasions)

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.780 | 58555 | 3961712 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 42.428 | 240337 | 5664578 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 34.567 | 175326 | 5072055 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.782 | 9118 | 5117282 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 16.490 | 95700 | 5803545 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 16.584 | 82341 | 4965132 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 126.631 | 661377 | 5222872 |

Aggregate elapsed times for all five processes: P1=125.913 ms, P2=126.769 ms, P3=125.913 ms, P4=127.306 ms, P5=126.631 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.820 | 12.824 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.828 | 12.832 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.941 | 12.945 | 17464 | 0 | 1170 | 264 |
| warmup loaded: Kiwipete | 12.945 | 12.945 | 17464 | 0 | 1170 | 264 |
| warmup after search: Kiwipete | 12.965 | 12.965 | 28474 | 0 | 3178 | 566 |
| warmup loaded: King safety | 12.965 | 12.965 | 28474 | 0 | 3178 | 566 |
| warmup after search: King safety | 12.965 | 12.965 | 38855 | 0 | 3596 | 617 |
| warmup loaded: Endgame | 12.965 | 12.965 | 38855 | 0 | 3596 | 617 |
| warmup after search: Endgame | 12.965 | 12.965 | 40271 | 0 | 3621 | 628 |
| warmup loaded: Promotion tactic | 12.965 | 12.965 | 40271 | 0 | 3621 | 628 |
| warmup after search: Promotion tactic | 12.965 | 12.965 | 43086 | 0 | 3891 | 673 |
| warmup loaded: Advanced pawns/check evasion | 12.965 | 12.965 | 43086 | 0 | 3891 | 673 |
| warmup after search: Advanced pawns/check evasion | 12.969 | 12.969 | 48253 | 0 | 4327 | 740 |
| after complete warmup | 12.969 | 12.969 | 48253 | 0 | 4327 | 740 |
| measured loaded: Quiet middlegame | 12.969 | 12.969 | 48253 | 0 | 4327 | 740 |
| measured after search: Quiet middlegame | 12.969 | 12.969 | 48253 | 0 | 4327 | 740 |
| measured loaded: Kiwipete | 12.969 | 12.969 | 48253 | 0 | 4327 | 740 |
| measured after search: Kiwipete | 12.973 | 12.973 | 48253 | 0 | 4327 | 740 |
| measured loaded: King safety | 12.973 | 12.973 | 48253 | 0 | 4327 | 740 |
| measured after search: King safety | 12.973 | 12.973 | 48253 | 0 | 4327 | 740 |
| measured loaded: Endgame | 12.973 | 12.973 | 48253 | 0 | 4327 | 740 |
| measured after search: Endgame | 12.973 | 12.973 | 48253 | 0 | 4327 | 740 |
| measured loaded: Promotion tactic | 12.973 | 12.973 | 48253 | 0 | 4327 | 740 |
| measured after search: Promotion tactic | 12.973 | 12.973 | 48253 | 0 | 4327 | 740 |
| measured loaded: Advanced pawns/check evasion | 12.973 | 12.973 | 48253 | 0 | 4327 | 740 |
| measured after search: Advanced pawns/check evasion | 12.977 | 12.977 | 48253 | 0 | 4327 | 740 |
| final process state | 12.977 | 12.977 | 48253 | 0 | 4327 | 740 |

## LMR equal winning capture exemption

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.761 | 52877 | 3842420 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 44.937 | 242063 | 5386698 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.846 | 167509 | 4949081 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.807 | 8704 | 4816514 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.879 | 88704 | 5586308 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 18.205 | 88056 | 4836876 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 128.436 | 647913 | 5044632 |

Aggregate elapsed times for all five processes: P1=129.247 ms, P2=129.416 ms, P3=127.790 ms, P4=128.436 ms, P5=127.965 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.781 | 12.785 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.844 | 12.848 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.977 | 12.977 | 27593 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.977 | 12.977 | 27593 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.977 | 12.977 | 37084 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.977 | 12.977 | 37084 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.977 | 12.977 | 38423 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.977 | 12.977 | 38423 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.977 | 12.977 | 41288 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.977 | 12.977 | 41288 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| after complete warmup | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured loaded: Quiet middlegame | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured after search: Quiet middlegame | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured loaded: Kiwipete | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured after search: Kiwipete | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured loaded: King safety | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured after search: King safety | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured loaded: Endgame | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured after search: Endgame | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured loaded: Promotion tactic | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured after search: Promotion tactic | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured loaded: Advanced pawns/check evasion | 12.988 | 12.988 | 46610 | 0 | 4304 | 740 |
| measured after search: Advanced pawns/check evasion | 12.992 | 12.992 | 46610 | 0 | 4304 | 740 |
| final process state | 12.992 | 12.992 | 46610 | 0 | 4304 | 740 |

## Zobrist repetition detection

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.660 | 52877 | 3606858 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 46.287 | 242063 | 5229636 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 34.683 | 167509 | 4829710 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.799 | 8704 | 4837818 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 16.056 | 88704 | 5524789 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 18.517 | 88056 | 4755502 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 132.001 | 647913 | 4908380 |

Aggregate elapsed times for all five processes: P1=139.062 ms, P2=130.228 ms, P3=132.001 ms, P4=132.780 ms, P5=129.456 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.762 | 12.766 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.832 | 12.836 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.941 | 12.945 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.945 | 12.945 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.961 | 12.961 | 27593 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.961 | 12.961 | 27593 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.965 | 12.965 | 37084 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.965 | 12.965 | 37084 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.969 | 12.969 | 38423 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.969 | 12.969 | 38423 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.969 | 12.969 | 41287 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.969 | 12.969 | 41287 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| after complete warmup | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured loaded: Quiet middlegame | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured after search: Quiet middlegame | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured loaded: Kiwipete | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured after search: Kiwipete | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured loaded: King safety | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured after search: King safety | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured loaded: Endgame | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured after search: Endgame | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured loaded: Promotion tactic | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured after search: Promotion tactic | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured loaded: Advanced pawns/check evasion | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| measured after search: Advanced pawns/check evasion | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |
| final process state | 12.980 | 12.980 | 46607 | 0 | 4304 | 740 |

## Mate illegal sentinel protection

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.940 | 52877 | 3793233 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 43.945 | 242063 | 5508260 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.505 | 167509 | 4999546 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.772 | 8704 | 4912851 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 16.052 | 88704 | 5525916 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 18.170 | 88056 | 4846356 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 127.384 | 647913 | 5086311 |

Aggregate elapsed times for all five processes: P1=127.289 ms, P2=127.384 ms, P3=130.721 ms, P4=127.162 ms, P5=127.947 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.781 | 12.785 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.836 | 12.840 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.949 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.953 | 12.953 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.969 | 12.969 | 27592 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.969 | 12.969 | 27592 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.973 | 12.973 | 37080 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.973 | 12.973 | 37080 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.977 | 12.977 | 38418 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.977 | 12.977 | 38418 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.977 | 12.977 | 41285 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.977 | 12.977 | 41285 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| after complete warmup | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured loaded: Quiet middlegame | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured after search: Quiet middlegame | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured loaded: Kiwipete | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured after search: Kiwipete | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured loaded: King safety | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured after search: King safety | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured loaded: Endgame | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured after search: Endgame | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured loaded: Promotion tactic | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured after search: Promotion tactic | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured loaded: Advanced pawns/check evasion | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| measured after search: Advanced pawns/check evasion | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |
| final process state | 12.984 | 12.984 | 46611 | 0 | 4304 | 740 |

## Symmetric mate distance scoring

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.823 | 52877 | 3825257 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 44.623 | 242063 | 5424612 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.363 | 167509 | 5020731 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.761 | 8704 | 4942323 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.782 | 88704 | 5620644 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 18.571 | 88056 | 4741711 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 127.923 | 647913 | 5064862 |

Aggregate elapsed times for all five processes: P1=129.307 ms, P2=127.478 ms, P3=127.923 ms, P4=131.639 ms, P5=127.626 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.785 | 12.789 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.824 | 12.828 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.934 | 12.938 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.938 | 12.938 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.953 | 12.953 | 27592 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.953 | 12.953 | 27592 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.957 | 12.957 | 37083 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.957 | 12.957 | 37083 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.961 | 12.961 | 38422 | 0 | 3591 | 627 |
| warmup loaded: Promotion tactic | 12.961 | 12.961 | 38422 | 0 | 3591 | 627 |
| warmup after search: Promotion tactic | 12.961 | 12.961 | 41289 | 0 | 3861 | 671 |
| warmup loaded: Advanced pawns/check evasion | 12.961 | 12.961 | 41289 | 0 | 3861 | 671 |
| warmup after search: Advanced pawns/check evasion | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| after complete warmup | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured loaded: Quiet middlegame | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured after search: Quiet middlegame | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured loaded: Kiwipete | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured after search: Kiwipete | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured loaded: King safety | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured after search: King safety | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured loaded: Endgame | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured after search: Endgame | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured loaded: Promotion tactic | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured after search: Promotion tactic | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured loaded: Advanced pawns/check evasion | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| measured after search: Advanced pawns/check evasion | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |
| final process state | 12.973 | 12.973 | 46613 | 0 | 4304 | 740 |

## Depth based null move pruning

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.969 | 52877 | 3785213 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 44.247 | 242063 | 5470776 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 33.014 | 167509 | 5073896 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.680 | 3146 | 4625069 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 15.663 | 88704 | 5663128 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 18.064 | 88056 | 4874545 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 125.638 | 642355 | 5112749 |

Aggregate elapsed times for all five processes: P1=125.330 ms, P2=129.059 ms, P3=125.311 ms, P4=125.638 ms, P5=126.642 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.797 | 12.801 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.852 | 12.855 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.961 | 12.965 | 16527 | 0 | 1147 | 263 |
| warmup loaded: Kiwipete | 12.965 | 12.965 | 16527 | 0 | 1147 | 263 |
| warmup after search: Kiwipete | 12.980 | 12.980 | 27593 | 0 | 3143 | 564 |
| warmup loaded: King safety | 12.980 | 12.980 | 27593 | 0 | 3143 | 564 |
| warmup after search: King safety | 12.984 | 12.984 | 37081 | 0 | 3566 | 616 |
| warmup loaded: Endgame | 12.984 | 12.984 | 37081 | 0 | 3566 | 616 |
| warmup after search: Endgame | 12.988 | 12.988 | 37742 | 0 | 3585 | 621 |
| warmup loaded: Promotion tactic | 12.988 | 12.988 | 37742 | 0 | 3585 | 621 |
| warmup after search: Promotion tactic | 12.988 | 12.988 | 40609 | 0 | 3855 | 665 |
| warmup loaded: Advanced pawns/check evasion | 12.988 | 12.988 | 40609 | 0 | 3855 | 665 |
| warmup after search: Advanced pawns/check evasion | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| after complete warmup | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured loaded: Quiet middlegame | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured after search: Quiet middlegame | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured loaded: Kiwipete | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured after search: Kiwipete | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured loaded: King safety | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured after search: King safety | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured loaded: Endgame | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured after search: Endgame | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured loaded: Promotion tactic | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured after search: Promotion tactic | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured loaded: Advanced pawns/check evasion | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| measured after search: Advanced pawns/check evasion | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |
| final process state | 13.000 | 13.000 | 45939 | 0 | 4298 | 734 |

## Shallow quiet futility pruning

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.370 | 51165 | 3826862 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 17.170 | 89506 | 5212904 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 15.999 | 80055 | 5003774 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 1.051 | 3146 | 2994134 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 4.276 | 20650 | 4829246 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 11.475 | 54159 | 4719877 | c4c5 | -547 | c4c5 b2a1q b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 63.340 | 298681 | 4715491 |

Aggregate elapsed times for all five processes: P1=63.538 ms, P2=63.340 ms, P3=62.919 ms, P4=66.628 ms, P5=63.264 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 12.785 | 12.789 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 12.848 | 12.852 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 12.961 | 12.965 | 16946 | 0 | 1122 | 260 |
| warmup loaded: Kiwipete | 12.965 | 12.965 | 16946 | 0 | 1122 | 260 |
| warmup after search: Kiwipete | 12.980 | 12.980 | 23365 | 0 | 2732 | 511 |
| warmup loaded: King safety | 12.980 | 12.980 | 23365 | 0 | 2732 | 511 |
| warmup after search: King safety | 12.984 | 12.984 | 28109 | 0 | 3121 | 568 |
| warmup loaded: Endgame | 12.984 | 12.984 | 28109 | 0 | 3121 | 568 |
| warmup after search: Endgame | 12.984 | 12.984 | 28772 | 0 | 3141 | 573 |
| warmup loaded: Promotion tactic | 12.984 | 12.984 | 28772 | 0 | 3141 | 573 |
| warmup after search: Promotion tactic | 12.984 | 12.984 | 29814 | 0 | 3318 | 598 |
| warmup loaded: Advanced pawns/check evasion | 12.984 | 12.984 | 29814 | 0 | 3318 | 598 |
| warmup after search: Advanced pawns/check evasion | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| after complete warmup | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured loaded: Quiet middlegame | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured after search: Quiet middlegame | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured loaded: Kiwipete | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured after search: Kiwipete | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured loaded: King safety | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured after search: King safety | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured loaded: Endgame | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured after search: Endgame | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured loaded: Promotion tactic | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured after search: Promotion tactic | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured loaded: Advanced pawns/check evasion | 12.988 | 12.988 | 33354 | 0 | 3781 | 669 |
| measured after search: Advanced pawns/check evasion | 12.996 | 12.996 | 33354 | 0 | 3781 | 669 |
| final process state | 12.996 | 12.996 | 33354 | 0 | 3781 | 669 |

## TT move ordering

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 10.443 | 37855 | 3624980 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 17.453 | 88781 | 5086766 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 18.286 | 87298 | 4774014 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.750 | 3316 | 4421810 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 4.073 | 19881 | 4880798 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 16.229 | 75118 | 4628765 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 67.234 | 312249 | 4644215 |

Aggregate elapsed times for all five processes: P1=66.947 ms, P2=67.571 ms, P3=67.636 ms, P4=66.627 ms, P5=67.234 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.840 | 28.844 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.848 | 28.852 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 28.949 | 28.953 | 14881 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 28.953 | 28.953 | 14881 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 28.984 | 28.984 | 21128 | 0 | 2682 | 509 |
| warmup loaded: King safety | 28.984 | 28.984 | 21128 | 0 | 2682 | 509 |
| warmup after search: King safety | 28.984 | 28.984 | 26425 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 28.984 | 28.984 | 26425 | 0 | 3118 | 571 |
| warmup after search: Endgame | 28.984 | 28.984 | 27168 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 28.984 | 28.984 | 27168 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 28.984 | 28.984 | 28105 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 28.984 | 28.984 | 28105 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| after complete warmup | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured loaded: King safety | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured after search: King safety | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured loaded: Endgame | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured after search: Endgame | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |
| final process state | 28.984 | 28.984 | 33496 | 0 | 3872 | 673 |

## TT rigorous score cutoffs

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 10.558 | 37855 | 3585310 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 17.546 | 88781 | 5059887 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 17.842 | 87298 | 4892769 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.777 | 3316 | 4269685 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 3.959 | 19881 | 5021857 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 15.833 | 75118 | 4744296 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 66.516 | 312249 | 4694378 |

Aggregate elapsed times for all five processes: P1=66.566 ms, P2=66.155 ms, P3=66.516 ms, P4=66.823 ms, P5=65.975 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.805 | 28.809 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.812 | 28.816 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 28.914 | 28.918 | 14881 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 28.918 | 28.918 | 14881 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 28.949 | 28.949 | 21129 | 0 | 2682 | 509 |
| warmup loaded: King safety | 28.949 | 28.949 | 21129 | 0 | 2682 | 509 |
| warmup after search: King safety | 28.949 | 28.949 | 26426 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 28.949 | 28.949 | 26426 | 0 | 3118 | 571 |
| warmup after search: Endgame | 28.949 | 28.949 | 27169 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 28.949 | 28.949 | 27169 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 28.949 | 28.949 | 28106 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 28.949 | 28.949 | 28106 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| after complete warmup | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured loaded: King safety | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured after search: King safety | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured loaded: Endgame | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured after search: Endgame | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |
| final process state | 28.949 | 28.949 | 33496 | 0 | 3872 | 673 |

## TT rigorous score cutoffs

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 10.004 | 35112 | 3509788 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 16.466 | 80234 | 4872811 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 17.156 | 83298 | 4855351 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.670 | 2995 | 4468375 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 3.964 | 19528 | 4926109 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 13.820 | 62527 | 4524264 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 62.080 | 283694 | 4569783 |

Aggregate elapsed times for all five processes: P1=61.497 ms, P2=63.058 ms, P3=62.285 ms, P4=62.080 ms, P5=61.987 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.832 | 28.836 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.840 | 28.844 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.336 | 29.336 | 14889 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 29.336 | 29.336 | 14889 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 29.461 | 29.461 | 21133 | 0 | 2682 | 509 |
| warmup loaded: King safety | 29.461 | 29.461 | 21133 | 0 | 2682 | 509 |
| warmup after search: King safety | 29.590 | 29.590 | 26429 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 29.590 | 29.590 | 26429 | 0 | 3118 | 571 |
| warmup after search: Endgame | 29.594 | 29.594 | 27172 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 29.594 | 29.594 | 27172 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 29.621 | 29.625 | 28103 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 29.625 | 29.625 | 28103 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 30.113 | 30.113 | 33486 | 0 | 3872 | 673 |
| after complete warmup | 30.113 | 30.113 | 33486 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 30.113 | 30.113 | 33486 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 30.508 | 30.508 | 33486 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 30.508 | 30.508 | 33486 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 30.625 | 30.625 | 33486 | 0 | 3872 | 673 |
| measured loaded: King safety | 30.625 | 30.625 | 33486 | 0 | 3872 | 673 |
| measured after search: King safety | 30.754 | 30.754 | 33486 | 0 | 3872 | 673 |
| measured loaded: Endgame | 30.754 | 30.754 | 33486 | 0 | 3872 | 673 |
| measured after search: Endgame | 30.758 | 30.758 | 33486 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 30.758 | 30.758 | 33486 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 30.785 | 30.785 | 33486 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 30.785 | 30.789 | 33486 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 31.277 | 31.277 | 33486 | 0 | 3872 | 673 |
| final process state | 31.277 | 31.277 | 33486 | 0 | 3872 | 673 |

## Production A baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 11.362 | 35112 | 3090180 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 18.549 | 80234 | 4325474 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 19.808 | 83298 | 4205344 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.771 | 2995 | 3882505 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 4.482 | 19528 | 4357315 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 15.698 | 62527 | 3983088 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 70.670 | 283694 | 4014322 |

Aggregate elapsed times for all five processes: P1=70.670 ms, P2=70.174 ms, P3=69.758 ms, P4=70.955 ms, P5=72.413 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.875 | 28.879 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.883 | 28.887 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.375 | 29.375 | 14889 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 29.375 | 29.375 | 14889 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 29.504 | 29.504 | 21133 | 0 | 2682 | 509 |
| warmup loaded: King safety | 29.504 | 29.504 | 21133 | 0 | 2682 | 509 |
| warmup after search: King safety | 29.633 | 29.633 | 26429 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 29.633 | 29.633 | 26429 | 0 | 3118 | 571 |
| warmup after search: Endgame | 29.637 | 29.637 | 27172 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 29.637 | 29.637 | 27172 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 29.664 | 29.668 | 28103 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 29.668 | 29.668 | 28103 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 30.156 | 30.156 | 33488 | 0 | 3872 | 673 |
| after complete warmup | 30.156 | 30.156 | 33488 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 30.156 | 30.156 | 33488 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 30.551 | 30.551 | 33488 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 30.551 | 30.551 | 33488 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 30.668 | 30.668 | 33488 | 0 | 3872 | 673 |
| measured loaded: King safety | 30.668 | 30.668 | 33488 | 0 | 3872 | 673 |
| measured after search: King safety | 30.797 | 30.797 | 33488 | 0 | 3872 | 673 |
| measured loaded: Endgame | 30.797 | 30.797 | 33488 | 0 | 3872 | 673 |
| measured after search: Endgame | 30.801 | 30.801 | 33488 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 30.801 | 30.801 | 33488 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 30.828 | 30.828 | 33488 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 30.828 | 30.832 | 33488 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 31.320 | 31.320 | 33488 | 0 | 3872 | 673 |
| final process state | 31.320 | 31.320 | 33488 | 0 | 3872 | 673 |

## --profile-kiwipete-qsearch

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 12.543 | 54959 | 4381595 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 94.183 | 480576 | 5102558 | d5d6 | -135 | d5d6 b4c3 d6e7 c3d2 e1f1 d2d1q a1d1 a6e2 f1g1  |
| King safety | 4 | 90.728 | 441351 | 4864539 | d3d4 | -16 | d3d4 c5d4 c4d5 h7h6  |
| Endgame | 5 | 0.891 | 4542 | 5099548 | b4f4 | 58 | b4f4 h4g3 f4f7 h5h2 e2e4  |
| Promotion tactic | 4 | 3.801 | 19214 | 5055324 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 9.352 | 41117 | 4396665 | d2d4 | -792 | d2d4 b2a1q d1a1 a3b4 h6f5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 211.498 | 1041759 | 4925621 |

Aggregate elapsed times for all five processes: P1=213.075 ms, P2=210.990 ms, P3=211.482 ms, P4=212.298 ms, P5=211.498 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.871 | 28.875 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.500 | 29.504 | 7655 | 0 | 1338 | 269 |
| warmup loaded: Kiwipete | 29.504 | 29.504 | 7655 | 0 | 1338 | 269 |
| warmup after search: Kiwipete | 30.867 | 30.867 | 31810 | 0 | 4357 | 748 |
| warmup loaded: King safety | 30.867 | 30.867 | 31810 | 0 | 4357 | 748 |
| warmup after search: King safety | 32.121 | 32.121 | 61660 | 0 | 5019 | 855 |
| warmup loaded: Endgame | 32.121 | 32.121 | 61660 | 0 | 5019 | 855 |
| warmup after search: Endgame | 32.145 | 32.145 | 62276 | 0 | 5031 | 860 |
| warmup loaded: Promotion tactic | 32.145 | 32.145 | 62276 | 0 | 5031 | 860 |
| warmup after search: Promotion tactic | 32.176 | 32.176 | 63022 | 0 | 5146 | 874 |
| warmup loaded: Advanced pawns/check evasion | 32.176 | 32.176 | 63022 | 0 | 5146 | 874 |
| warmup after search: Advanced pawns/check evasion | 32.504 | 32.508 | 66047 | 0 | 5473 | 929 |
| after complete warmup | 32.508 | 32.508 | 66047 | 0 | 5473 | 929 |
| measured loaded: Quiet middlegame | 32.508 | 32.508 | 66047 | 0 | 5473 | 929 |
| measured after search: Quiet middlegame | 33.023 | 33.027 | 66047 | 0 | 5473 | 929 |
| measured loaded: Kiwipete | 33.027 | 33.027 | 66047 | 0 | 5473 | 929 |
| measured after search: Kiwipete | 34.367 | 34.371 | 66047 | 0 | 5473 | 929 |
| measured loaded: King safety | 34.371 | 34.371 | 66047 | 0 | 5473 | 929 |
| measured after search: King safety | 35.625 | 35.625 | 66047 | 0 | 5473 | 929 |
| measured loaded: Endgame | 35.625 | 35.625 | 66047 | 0 | 5473 | 929 |
| measured after search: Endgame | 35.648 | 35.652 | 66047 | 0 | 5473 | 929 |
| measured loaded: Promotion tactic | 35.652 | 35.652 | 66047 | 0 | 5473 | 929 |
| measured after search: Promotion tactic | 35.680 | 35.680 | 66047 | 0 | 5473 | 929 |
| measured loaded: Advanced pawns/check evasion | 35.684 | 35.684 | 66047 | 0 | 5473 | 929 |
| measured after search: Advanced pawns/check evasion | 36.012 | 36.012 | 66047 | 0 | 5473 | 929 |
| final process state | 36.012 | 36.012 | 66047 | 0 | 5473 | 929 |

## Variant B

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 12.594 | 54959 | 4364007 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 4 | 94.553 | 480576 | 5082611 | d5d6 | -135 | d5d6 b4c3 d6e7 c3d2 e1f1 d2d1q a1d1 a6e2 f1g1  |
| King safety | 4 | 91.143 | 441351 | 4842377 | d3d4 | -16 | d3d4 c5d4 c4d5 h7h6  |
| Endgame | 5 | 0.909 | 4542 | 4994002 | b4f4 | 58 | b4f4 h4g3 f4f7 h5h2 e2e4  |
| Promotion tactic | 4 | 3.851 | 19214 | 4989063 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 9.509 | 41117 | 4323873 | d2d4 | -792 | d2d4 b2a1q d1a1 a3b4 h6f5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 212.560 | 1041759 | 4901008 |

Aggregate elapsed times for all five processes: P1=212.665 ms, P2=213.062 ms, P3=212.560 ms, P4=211.415 ms, P5=211.463 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.871 | 28.875 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.500 | 29.504 | 7655 | 0 | 1338 | 269 |
| warmup loaded: Kiwipete | 29.504 | 29.504 | 7655 | 0 | 1338 | 269 |
| warmup after search: Kiwipete | 30.867 | 30.867 | 31812 | 0 | 4357 | 748 |
| warmup loaded: King safety | 30.867 | 30.867 | 31812 | 0 | 4357 | 748 |
| warmup after search: King safety | 32.121 | 32.121 | 61651 | 0 | 5019 | 855 |
| warmup loaded: Endgame | 32.121 | 32.121 | 61651 | 0 | 5019 | 855 |
| warmup after search: Endgame | 32.145 | 32.145 | 62266 | 0 | 5031 | 860 |
| warmup loaded: Promotion tactic | 32.145 | 32.145 | 62266 | 0 | 5031 | 860 |
| warmup after search: Promotion tactic | 32.176 | 32.176 | 63014 | 0 | 5146 | 874 |
| warmup loaded: Advanced pawns/check evasion | 32.176 | 32.176 | 63014 | 0 | 5146 | 874 |
| warmup after search: Advanced pawns/check evasion | 32.504 | 32.508 | 66035 | 0 | 5473 | 929 |
| after complete warmup | 32.508 | 32.508 | 66035 | 0 | 5473 | 929 |
| measured loaded: Quiet middlegame | 32.508 | 32.508 | 66035 | 0 | 5473 | 929 |
| measured after search: Quiet middlegame | 33.023 | 33.027 | 66035 | 0 | 5473 | 929 |
| measured loaded: Kiwipete | 33.027 | 33.027 | 66035 | 0 | 5473 | 929 |
| measured after search: Kiwipete | 34.367 | 34.371 | 66035 | 0 | 5473 | 929 |
| measured loaded: King safety | 34.371 | 34.371 | 66035 | 0 | 5473 | 929 |
| measured after search: King safety | 35.625 | 35.625 | 66035 | 0 | 5473 | 929 |
| measured loaded: Endgame | 35.625 | 35.625 | 66035 | 0 | 5473 | 929 |
| measured after search: Endgame | 35.648 | 35.652 | 66035 | 0 | 5473 | 929 |
| measured loaded: Promotion tactic | 35.652 | 35.652 | 66035 | 0 | 5473 | 929 |
| measured after search: Promotion tactic | 35.680 | 35.680 | 66035 | 0 | 5473 | 929 |
| measured loaded: Advanced pawns/check evasion | 35.684 | 35.684 | 66035 | 0 | 5473 | 929 |
| measured after search: Advanced pawns/check evasion | 36.012 | 36.012 | 66035 | 0 | 5473 | 929 |
| final process state | 36.012 | 36.012 | 66035 | 0 | 5473 | 929 |

## Variant B Kiwipete 6

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 12.616 | 54965 | 4356925 | d6d5 | 39 | d6d5 f3e5 g7h6 g2h3  |
| Kiwipete | 6 | 504.873 | 2401555 | 4756748 | c3b5 | 35 | c3b5 h3g2 f3g2 a6b5 e2b5 b6c4  |
| King safety | 4 | 93.008 | 441417 | 4745995 | d3d4 | -16 | d3d4 c5d4 c4d5 h7h6  |
| Endgame | 5 | 0.919 | 4542 | 4941118 | b4f4 | 58 | b4f4 h4g3 f4f7 h5h2 e2e4  |
| Promotion tactic | 4 | 3.913 | 19214 | 4910108 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 9.431 | 41117 | 4359895 | d2d4 | -792 | d2d4 b2a1q d1a1 a3b4 h6f5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 624.760 | 2962810 | 4742315 |

Aggregate elapsed times for all five processes: P1=627.194 ms, P2=624.760 ms, P3=622.962 ms, P4=621.683 ms, P5=633.244 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.863 | 28.863 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.871 | 28.871 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.539 | 29.539 | 7657 | 0 | 1338 | 269 |
| warmup loaded: Kiwipete | 29.539 | 29.539 | 7657 | 0 | 1338 | 269 |
| warmup after search: Kiwipete | 35.848 | 35.848 | 118527 | 0 | 8095 | 1336 |
| warmup loaded: King safety | 35.848 | 35.848 | 118527 | 0 | 8095 | 1336 |
| warmup after search: King safety | 37.105 | 37.105 | 147785 | 0 | 8401 | 1378 |
| warmup loaded: Endgame | 37.105 | 37.105 | 147785 | 0 | 8401 | 1378 |
| warmup after search: Endgame | 37.129 | 37.129 | 148384 | 0 | 8408 | 1381 |
| warmup loaded: Promotion tactic | 37.129 | 37.129 | 148384 | 0 | 8408 | 1381 |
| warmup after search: Promotion tactic | 37.156 | 37.156 | 149100 | 0 | 8453 | 1384 |
| warmup loaded: Advanced pawns/check evasion | 37.156 | 37.156 | 149100 | 0 | 8453 | 1384 |
| warmup after search: Advanced pawns/check evasion | 37.488 | 37.488 | 152047 | 0 | 8626 | 1420 |
| after complete warmup | 37.488 | 37.488 | 152047 | 0 | 8626 | 1420 |
| measured loaded: Quiet middlegame | 37.488 | 37.488 | 152047 | 0 | 8626 | 1420 |
| measured after search: Quiet middlegame | 38.000 | 38.004 | 152047 | 0 | 8626 | 1420 |
| measured loaded: Kiwipete | 38.004 | 38.004 | 152047 | 0 | 8626 | 1420 |
| measured after search: Kiwipete | 44.242 | 44.242 | 152047 | 0 | 8626 | 1420 |
| measured loaded: King safety | 44.242 | 44.242 | 152047 | 0 | 8626 | 1420 |
| measured after search: King safety | 45.500 | 45.500 | 152047 | 0 | 8626 | 1420 |
| measured loaded: Endgame | 45.500 | 45.500 | 152047 | 0 | 8626 | 1420 |
| measured after search: Endgame | 45.523 | 45.523 | 152047 | 0 | 8626 | 1420 |
| measured loaded: Promotion tactic | 45.523 | 45.523 | 152047 | 0 | 8626 | 1420 |
| measured after search: Promotion tactic | 45.551 | 45.555 | 152047 | 0 | 8626 | 1420 |
| measured loaded: Advanced pawns/check evasion | 45.555 | 45.555 | 152047 | 0 | 8626 | 1420 |
| measured after search: Advanced pawns/check evasion | 45.887 | 45.887 | 152047 | 0 | 8626 | 1420 |
| final process state | 45.887 | 45.887 | 152047 | 0 | 8626 | 1420 |

## Variant B generation optimization

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 10.085 | 35112 | 3481736 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 15.917 | 80234 | 5040773 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 16.967 | 83298 | 4909381 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.646 | 2995 | 4633597 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 3.846 | 19528 | 5077583 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 13.632 | 62581 | 4590701 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 61.093 | 283748 | 4644514 |

Aggregate elapsed times for all five processes: P1=61.647 ms, P2=61.078 ms, P3=61.251 ms, P4=60.519 ms, P5=61.093 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.785 | 28.789 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.793 | 28.797 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.285 | 29.285 | 14889 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 29.285 | 29.285 | 14889 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 29.414 | 29.414 | 21132 | 0 | 2682 | 509 |
| warmup loaded: King safety | 29.414 | 29.414 | 21132 | 0 | 2682 | 509 |
| warmup after search: King safety | 29.543 | 29.543 | 26428 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 29.543 | 29.543 | 26428 | 0 | 3118 | 571 |
| warmup after search: Endgame | 29.547 | 29.547 | 27171 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 29.547 | 29.547 | 27171 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 29.574 | 29.574 | 28102 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 29.574 | 29.574 | 28102 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 30.062 | 30.062 | 33487 | 0 | 3872 | 673 |
| after complete warmup | 30.062 | 30.062 | 33487 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 30.062 | 30.062 | 33487 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 30.457 | 30.457 | 33487 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 30.457 | 30.457 | 33487 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 30.574 | 30.574 | 33487 | 0 | 3872 | 673 |
| measured loaded: King safety | 30.574 | 30.574 | 33487 | 0 | 3872 | 673 |
| measured after search: King safety | 30.703 | 30.703 | 33487 | 0 | 3872 | 673 |
| measured loaded: Endgame | 30.703 | 30.703 | 33487 | 0 | 3872 | 673 |
| measured after search: Endgame | 30.707 | 30.707 | 33487 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 30.707 | 30.707 | 33487 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 30.734 | 30.734 | 33487 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 30.734 | 30.738 | 33487 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 31.223 | 31.227 | 33487 | 0 | 3872 | 673 |
| final process state | 31.227 | 31.227 | 33487 | 0 | 3872 | 673 |

## Run 1

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 10.257 | 35112 | 3423163 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 16.065 | 80234 | 4994255 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 17.118 | 83298 | 4866068 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.669 | 2995 | 4477815 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 3.935 | 19528 | 4962373 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 14.160 | 62573 | 4419023 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 62.205 | 283740 | 4561402 |

Aggregate elapsed times for all five processes: P1=61.928 ms, P2=62.641 ms, P3=61.865 ms, P4=62.416 ms, P5=62.205 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.871 | 28.875 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.363 | 29.363 | 14889 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 29.363 | 29.363 | 14889 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 29.492 | 29.492 | 21133 | 0 | 2682 | 509 |
| warmup loaded: King safety | 29.492 | 29.492 | 21133 | 0 | 2682 | 509 |
| warmup after search: King safety | 29.621 | 29.621 | 26429 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 29.621 | 29.621 | 26429 | 0 | 3118 | 571 |
| warmup after search: Endgame | 29.625 | 29.625 | 27172 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 29.625 | 29.625 | 27172 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 29.652 | 29.652 | 28103 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 29.652 | 29.652 | 28103 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 30.141 | 30.145 | 33488 | 0 | 3872 | 673 |
| after complete warmup | 30.145 | 30.145 | 33488 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 30.145 | 30.145 | 33488 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 30.535 | 30.535 | 33488 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 30.535 | 30.535 | 33488 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 30.652 | 30.652 | 33488 | 0 | 3872 | 673 |
| measured loaded: King safety | 30.652 | 30.652 | 33488 | 0 | 3872 | 673 |
| measured after search: King safety | 30.781 | 30.781 | 33488 | 0 | 3872 | 673 |
| measured loaded: Endgame | 30.781 | 30.781 | 33488 | 0 | 3872 | 673 |
| measured after search: Endgame | 30.785 | 30.785 | 33488 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 30.785 | 30.785 | 33488 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 30.812 | 30.816 | 33488 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 30.816 | 30.816 | 33488 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 31.301 | 31.305 | 33488 | 0 | 3872 | 673 |
| final process state | 31.305 | 31.305 | 33488 | 0 | 3872 | 673 |

## Run 2

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 10.281 | 35112 | 3415187 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 16.101 | 80234 | 4983069 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 17.233 | 83298 | 4833634 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.663 | 2995 | 4516003 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 3.913 | 19528 | 4990932 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 13.974 | 62527 | 4474488 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 62.165 | 283694 | 4563531 |

Aggregate elapsed times for all five processes: P1=63.033 ms, P2=63.255 ms, P3=62.004 ms, P4=62.165 ms, P5=61.726 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.871 | 28.875 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.879 | 28.883 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.371 | 29.371 | 14889 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 29.371 | 29.371 | 14889 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 29.496 | 29.496 | 21133 | 0 | 2682 | 509 |
| warmup loaded: King safety | 29.496 | 29.496 | 21133 | 0 | 2682 | 509 |
| warmup after search: King safety | 29.625 | 29.625 | 26429 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 29.625 | 29.625 | 26429 | 0 | 3118 | 571 |
| warmup after search: Endgame | 29.629 | 29.629 | 27172 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 29.629 | 29.629 | 27172 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 29.656 | 29.656 | 28103 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 29.656 | 29.656 | 28103 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 30.148 | 30.148 | 33487 | 0 | 3872 | 673 |
| after complete warmup | 30.148 | 30.148 | 33487 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 30.148 | 30.148 | 33487 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 30.543 | 30.543 | 33487 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 30.543 | 30.543 | 33487 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 30.656 | 30.656 | 33487 | 0 | 3872 | 673 |
| measured loaded: King safety | 30.656 | 30.656 | 33487 | 0 | 3872 | 673 |
| measured after search: King safety | 30.789 | 30.789 | 33487 | 0 | 3872 | 673 |
| measured loaded: Endgame | 30.789 | 30.789 | 33487 | 0 | 3872 | 673 |
| measured after search: Endgame | 30.793 | 30.793 | 33487 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 30.793 | 30.793 | 33487 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 30.820 | 30.820 | 33487 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 30.820 | 30.820 | 33487 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 31.309 | 31.312 | 33487 | 0 | 3872 | 673 |
| final process state | 31.312 | 31.312 | 33487 | 0 | 3872 | 673 |

## Adopt fixed stack AttackerState and 256 MoveList capacity

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 9.651 | 35112 | 3638166 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 15.723 | 80234 | 5103119 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 16.794 | 83298 | 4959976 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.636 | 2995 | 4711060 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 3.841 | 19528 | 5084209 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 13.629 | 62527 | 4587858 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 60.273 | 283694 | 4706814 |

Aggregate elapsed times for all five processes: P1=60.864 ms, P2=60.513 ms, P3=60.241 ms, P4=59.887 ms, P5=60.273 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.871 | 28.875 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.395 | 29.395 | 14889 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 29.395 | 29.395 | 14889 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 29.539 | 29.539 | 21133 | 0 | 2682 | 509 |
| warmup loaded: King safety | 29.539 | 29.539 | 21133 | 0 | 2682 | 509 |
| warmup after search: King safety | 29.668 | 29.668 | 26429 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 29.668 | 29.668 | 26429 | 0 | 3118 | 571 |
| warmup after search: Endgame | 29.672 | 29.672 | 27172 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 29.672 | 29.672 | 27172 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 29.699 | 29.699 | 28103 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 29.699 | 29.699 | 28103 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 30.191 | 30.191 | 33486 | 0 | 3872 | 673 |
| after complete warmup | 30.191 | 30.191 | 33486 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 30.191 | 30.191 | 33486 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 30.586 | 30.586 | 33486 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 30.586 | 30.586 | 33486 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 30.699 | 30.699 | 33486 | 0 | 3872 | 673 |
| measured loaded: King safety | 30.699 | 30.699 | 33486 | 0 | 3872 | 673 |
| measured after search: King safety | 30.832 | 30.832 | 33486 | 0 | 3872 | 673 |
| measured loaded: Endgame | 30.832 | 30.832 | 33486 | 0 | 3872 | 673 |
| measured after search: Endgame | 30.836 | 30.836 | 33486 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 30.836 | 30.836 | 33486 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 30.863 | 30.863 | 33486 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 30.863 | 30.863 | 33486 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 31.355 | 31.355 | 33486 | 0 | 3872 | 673 |
| final process state | 31.355 | 31.355 | 33486 | 0 | 3872 | 673 |

## Baseline A

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 9.784 | 35112 | 3588800 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 15.701 | 80234 | 5110260 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 16.857 | 83298 | 4941583 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.656 | 2995 | 4562697 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 3.861 | 19528 | 5057445 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 13.609 | 62527 | 4594458 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 60.468 | 283694 | 4691658 |

Aggregate elapsed times for all five processes: P1=60.718 ms, P2=60.264 ms, P3=60.468 ms, P4=60.798 ms, P5=60.277 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.871 | 28.875 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.398 | 29.398 | 14889 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 29.398 | 29.398 | 14889 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 29.539 | 29.539 | 21133 | 0 | 2682 | 509 |
| warmup loaded: King safety | 29.539 | 29.539 | 21133 | 0 | 2682 | 509 |
| warmup after search: King safety | 29.668 | 29.668 | 26428 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 29.668 | 29.668 | 26428 | 0 | 3118 | 571 |
| warmup after search: Endgame | 29.672 | 29.672 | 27171 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 29.672 | 29.672 | 27171 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 29.699 | 29.699 | 28102 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 29.699 | 29.699 | 28102 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 30.191 | 30.191 | 33487 | 0 | 3872 | 673 |
| after complete warmup | 30.191 | 30.191 | 33487 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 30.191 | 30.191 | 33487 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 30.586 | 30.586 | 33487 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 30.586 | 30.586 | 33487 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 30.699 | 30.699 | 33487 | 0 | 3872 | 673 |
| measured loaded: King safety | 30.699 | 30.699 | 33487 | 0 | 3872 | 673 |
| measured after search: King safety | 30.832 | 30.832 | 33487 | 0 | 3872 | 673 |
| measured loaded: Endgame | 30.832 | 30.832 | 33487 | 0 | 3872 | 673 |
| measured after search: Endgame | 30.836 | 30.836 | 33487 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 30.836 | 30.836 | 33487 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 30.863 | 30.863 | 33487 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 30.863 | 30.863 | 33487 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 31.355 | 31.355 | 33487 | 0 | 3872 | 673 |
| final process state | 31.355 | 31.355 | 33487 | 0 | 3872 | 673 |

## Variant B Experimental IID

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 10.561 | 35112 | 3324730 | c8e6 | 65 | c8e6 g3g4 g7h6 g4g5  |
| Kiwipete | 4 | 16.159 | 80234 | 4965236 | e2a6 | 128 | e2a6 b4c3 d2f4 g6g5  |
| King safety | 4 | 17.219 | 83298 | 4837538 | c3d5 | 119 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.688 | 2995 | 4353298 | b4f4 | 101 | b4f4 h4g5 f4f7 h5h2 g2g4  |
| Promotion tactic | 4 | 3.943 | 19528 | 4952947 | d7c8q | 650 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 13.944 | 62527 | 4483997 | c4c5 | -547 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 62.514 | 283694 | 4538068 |

Aggregate elapsed times for all five processes: P1=62.192 ms, P2=62.885 ms, P3=62.514 ms, P4=63.593 ms, P5=62.092 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.871 | 28.875 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.395 | 29.395 | 14889 | 0 | 1016 | 246 |
| warmup loaded: Kiwipete | 29.395 | 29.395 | 14889 | 0 | 1016 | 246 |
| warmup after search: Kiwipete | 29.539 | 29.539 | 21133 | 0 | 2682 | 509 |
| warmup loaded: King safety | 29.539 | 29.539 | 21133 | 0 | 2682 | 509 |
| warmup after search: King safety | 29.668 | 29.668 | 26429 | 0 | 3118 | 571 |
| warmup loaded: Endgame | 29.668 | 29.668 | 26429 | 0 | 3118 | 571 |
| warmup after search: Endgame | 29.672 | 29.672 | 27172 | 0 | 3139 | 576 |
| warmup loaded: Promotion tactic | 29.672 | 29.672 | 27172 | 0 | 3139 | 576 |
| warmup after search: Promotion tactic | 29.699 | 29.699 | 28103 | 0 | 3312 | 599 |
| warmup loaded: Advanced pawns/check evasion | 29.699 | 29.699 | 28103 | 0 | 3312 | 599 |
| warmup after search: Advanced pawns/check evasion | 30.191 | 30.191 | 33488 | 0 | 3872 | 673 |
| after complete warmup | 30.191 | 30.191 | 33488 | 0 | 3872 | 673 |
| measured loaded: Quiet middlegame | 30.191 | 30.191 | 33488 | 0 | 3872 | 673 |
| measured after search: Quiet middlegame | 30.586 | 30.586 | 33488 | 0 | 3872 | 673 |
| measured loaded: Kiwipete | 30.586 | 30.586 | 33488 | 0 | 3872 | 673 |
| measured after search: Kiwipete | 30.699 | 30.699 | 33488 | 0 | 3872 | 673 |
| measured loaded: King safety | 30.699 | 30.699 | 33488 | 0 | 3872 | 673 |
| measured after search: King safety | 30.832 | 30.832 | 33488 | 0 | 3872 | 673 |
| measured loaded: Endgame | 30.832 | 30.832 | 33488 | 0 | 3872 | 673 |
| measured after search: Endgame | 30.836 | 30.836 | 33488 | 0 | 3872 | 673 |
| measured loaded: Promotion tactic | 30.836 | 30.836 | 33488 | 0 | 3872 | 673 |
| measured after search: Promotion tactic | 30.863 | 30.863 | 33488 | 0 | 3872 | 673 |
| measured loaded: Advanced pawns/check evasion | 30.863 | 30.863 | 33488 | 0 | 3872 | 673 |
| measured after search: Advanced pawns/check evasion | 31.352 | 31.355 | 33488 | 0 | 3872 | 673 |
| final process state | 31.355 | 31.355 | 33488 | 0 | 3872 | 673 |

## Pawn File Indexing Experiment

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 10.128 | 33757 | 3333129 | g7h6 | 24 | g7h6 d2b3 h6c1 a1c1 c8e6  |
| Kiwipete | 4 | 17.025 | 85591 | 5027435 | e2a6 | 148 | e2a6 b4c3 d2f4 c3b2 e1g1  |
| King safety | 4 | 16.329 | 79946 | 4895873 | c3d5 | 139 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.620 | 2828 | 4558334 | b4f4 | 120 | b4f4 h4g5 f4f7 h5h2 f7c7 g5f6  |
| Promotion tactic | 4 | 3.864 | 19466 | 5037169 | d7c8q | 670 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 13.640 | 62402 | 4575053 | c4c5 | -587 | c4c5 b2a1r b4a3 a1d1 a4d1 g7h6 c5b6 a5c4  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 61.606 | 283990 | 4609758 |

Aggregate elapsed times for all five processes: P1=61.606 ms, P2=61.610 ms, P3=60.263 ms, P4=60.501 ms, P5=62.048 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.855 | 28.859 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.465 | 29.465 | 13331 | 0 | 983 | 238 |
| warmup loaded: Kiwipete | 29.465 | 29.465 | 13331 | 0 | 983 | 238 |
| warmup after search: Kiwipete | 29.594 | 29.594 | 20895 | 0 | 2690 | 504 |
| warmup loaded: King safety | 29.594 | 29.594 | 20895 | 0 | 2690 | 504 |
| warmup after search: King safety | 29.723 | 29.723 | 25941 | 0 | 3118 | 567 |
| warmup loaded: Endgame | 29.723 | 29.723 | 25941 | 0 | 3118 | 567 |
| warmup after search: Endgame | 29.723 | 29.723 | 26659 | 0 | 3139 | 574 |
| warmup loaded: Promotion tactic | 29.723 | 29.723 | 26659 | 0 | 3139 | 574 |
| warmup after search: Promotion tactic | 29.754 | 29.754 | 27616 | 0 | 3309 | 598 |
| warmup loaded: Advanced pawns/check evasion | 29.754 | 29.754 | 27616 | 0 | 3309 | 598 |
| warmup after search: Advanced pawns/check evasion | 30.312 | 30.312 | 32102 | 0 | 3865 | 677 |
| after complete warmup | 30.312 | 30.312 | 32102 | 0 | 3865 | 677 |
| measured loaded: Quiet middlegame | 30.312 | 30.312 | 32102 | 0 | 3865 | 677 |
| measured after search: Quiet middlegame | 30.773 | 30.773 | 32102 | 0 | 3865 | 677 |
| measured loaded: Kiwipete | 30.773 | 30.773 | 32102 | 0 | 3865 | 677 |
| measured after search: Kiwipete | 30.887 | 30.887 | 32102 | 0 | 3865 | 677 |
| measured loaded: King safety | 30.887 | 30.887 | 32102 | 0 | 3865 | 677 |
| measured after search: King safety | 31.016 | 31.016 | 32102 | 0 | 3865 | 677 |
| measured loaded: Endgame | 31.016 | 31.016 | 32102 | 0 | 3865 | 677 |
| measured after search: Endgame | 31.016 | 31.016 | 32102 | 0 | 3865 | 677 |
| measured loaded: Promotion tactic | 31.016 | 31.016 | 32102 | 0 | 3865 | 677 |
| measured after search: Promotion tactic | 31.047 | 31.047 | 32102 | 0 | 3865 | 677 |
| measured loaded: Advanced pawns/check evasion | 31.047 | 31.047 | 32102 | 0 | 3865 | 677 |
| measured after search: Advanced pawns/check evasion | 31.590 | 31.590 | 32102 | 0 | 3865 | 677 |
| final process state | 31.590 | 31.590 | 32102 | 0 | 3865 | 677 |

## Pawn Value 100 Experiment

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.045 | 57208 | 4073062 | c8e6 | 47 | c8e6 d2c4 f6e4 g2h3  |
| Kiwipete | 4 | 15.273 | 78633 | 5148472 | e2a6 | 143 | e2a6 b4c3 b2c3 h3g2 f3g2 b6c4  |
| King safety | 4 | 14.510 | 71308 | 4914407 | c3d5 | 139 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.706 | 3424 | 4850566 | b4f4 | 212 | b4f4 h4g3 f4f7 g3g2 f7c7 h5c5  |
| Promotion tactic | 4 | 3.816 | 19498 | 5109558 | d7c8q | 580 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 10.218 | 47688 | 4666854 | d2d4 | -560 | d2d4 a3b4 a1b1 g7h6 d1d2  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 58.569 | 277759 | 4742436 |

Aggregate elapsed times for all five processes: P1=59.206 ms, P2=58.445 ms, P3=58.346 ms, P4=58.786 ms, P5=58.569 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.859 | 28.863 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.867 | 28.871 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.387 | 29.391 | 15416 | 0 | 1189 | 269 |
| warmup loaded: Kiwipete | 29.391 | 29.391 | 15416 | 0 | 1189 | 269 |
| warmup after search: Kiwipete | 29.539 | 29.539 | 21184 | 0 | 2605 | 497 |
| warmup loaded: King safety | 29.539 | 29.539 | 21184 | 0 | 2605 | 497 |
| warmup after search: King safety | 29.660 | 29.660 | 25906 | 0 | 3026 | 554 |
| warmup loaded: Endgame | 29.660 | 29.660 | 25906 | 0 | 3026 | 554 |
| warmup after search: Endgame | 29.672 | 29.672 | 26573 | 0 | 3051 | 560 |
| warmup loaded: Promotion tactic | 29.672 | 29.672 | 26573 | 0 | 3051 | 560 |
| warmup after search: Promotion tactic | 29.695 | 29.699 | 27559 | 0 | 3227 | 583 |
| warmup loaded: Advanced pawns/check evasion | 29.699 | 29.699 | 27559 | 0 | 3227 | 583 |
| warmup after search: Advanced pawns/check evasion | 30.000 | 30.000 | 30211 | 0 | 3722 | 664 |
| after complete warmup | 30.000 | 30.000 | 30211 | 0 | 3722 | 664 |
| measured loaded: Quiet middlegame | 30.000 | 30.000 | 30211 | 0 | 3722 | 664 |
| measured after search: Quiet middlegame | 30.383 | 30.387 | 30211 | 0 | 3722 | 664 |
| measured loaded: Kiwipete | 30.387 | 30.387 | 30211 | 0 | 3722 | 664 |
| measured after search: Kiwipete | 30.480 | 30.480 | 30211 | 0 | 3722 | 664 |
| measured loaded: King safety | 30.480 | 30.480 | 30211 | 0 | 3722 | 664 |
| measured after search: King safety | 30.602 | 30.602 | 30211 | 0 | 3722 | 664 |
| measured loaded: Endgame | 30.602 | 30.602 | 30211 | 0 | 3722 | 664 |
| measured after search: Endgame | 30.613 | 30.613 | 30211 | 0 | 3722 | 664 |
| measured loaded: Promotion tactic | 30.613 | 30.613 | 30211 | 0 | 3722 | 664 |
| measured after search: Promotion tactic | 30.637 | 30.641 | 30211 | 0 | 3722 | 664 |
| measured loaded: Advanced pawns/check evasion | 30.645 | 30.645 | 30211 | 0 | 3722 | 664 |
| measured after search: Advanced pawns/check evasion | 30.941 | 30.941 | 30211 | 0 | 3722 | 664 |
| final process state | 30.941 | 30.941 | 30211 | 0 | 3722 | 664 |

## Adopt Pawn File Indexing and PawnValue 100 Bug Fixes

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 13.894 | 57208 | 4117325 | c8e6 | 47 | c8e6 d2c4 f6e4 g2h3  |
| Kiwipete | 4 | 15.174 | 78633 | 5182008 | e2a6 | 143 | e2a6 b4c3 b2c3 h3g2 f3g2 b6c4  |
| King safety | 4 | 14.505 | 71308 | 4916249 | c3d5 | 139 | c3d5 e7d8 g5f6 g7f6 d3d4  |
| Endgame | 5 | 0.685 | 3424 | 5001628 | b4f4 | 212 | b4f4 h4g3 f4f7 g3g2 f7c7 h5c5  |
| Promotion tactic | 4 | 3.812 | 19498 | 5114387 | d7c8q | 580 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 10.121 | 47688 | 4711839 | d2d4 | -560 | d2d4 a3b4 a1b1 g7h6 d1d2  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 58.191 | 277759 | 4773222 |

Aggregate elapsed times for all five processes: P1=58.442 ms, P2=58.502 ms, P3=58.191 ms, P4=57.868 ms, P5=57.905 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.848 | 28.852 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.855 | 28.859 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.379 | 29.383 | 15416 | 0 | 1189 | 269 |
| warmup loaded: Kiwipete | 29.383 | 29.383 | 15416 | 0 | 1189 | 269 |
| warmup after search: Kiwipete | 29.531 | 29.531 | 21184 | 0 | 2605 | 497 |
| warmup loaded: King safety | 29.531 | 29.531 | 21184 | 0 | 2605 | 497 |
| warmup after search: King safety | 29.652 | 29.652 | 25906 | 0 | 3026 | 554 |
| warmup loaded: Endgame | 29.652 | 29.652 | 25906 | 0 | 3026 | 554 |
| warmup after search: Endgame | 29.664 | 29.664 | 26573 | 0 | 3051 | 560 |
| warmup loaded: Promotion tactic | 29.664 | 29.664 | 26573 | 0 | 3051 | 560 |
| warmup after search: Promotion tactic | 29.688 | 29.691 | 27559 | 0 | 3227 | 583 |
| warmup loaded: Advanced pawns/check evasion | 29.691 | 29.691 | 27559 | 0 | 3227 | 583 |
| warmup after search: Advanced pawns/check evasion | 29.992 | 29.992 | 30211 | 0 | 3722 | 664 |
| after complete warmup | 29.992 | 29.992 | 30211 | 0 | 3722 | 664 |
| measured loaded: Quiet middlegame | 29.992 | 29.992 | 30211 | 0 | 3722 | 664 |
| measured after search: Quiet middlegame | 30.375 | 30.379 | 30211 | 0 | 3722 | 664 |
| measured loaded: Kiwipete | 30.379 | 30.379 | 30211 | 0 | 3722 | 664 |
| measured after search: Kiwipete | 30.473 | 30.473 | 30211 | 0 | 3722 | 664 |
| measured loaded: King safety | 30.473 | 30.473 | 30211 | 0 | 3722 | 664 |
| measured after search: King safety | 30.594 | 30.594 | 30211 | 0 | 3722 | 664 |
| measured loaded: Endgame | 30.594 | 30.594 | 30211 | 0 | 3722 | 664 |
| measured after search: Endgame | 30.605 | 30.605 | 30211 | 0 | 3722 | 664 |
| measured loaded: Promotion tactic | 30.605 | 30.605 | 30211 | 0 | 3722 | 664 |
| measured after search: Promotion tactic | 30.629 | 30.633 | 30211 | 0 | 3722 | 664 |
| measured loaded: Advanced pawns/check evasion | 30.637 | 30.637 | 30211 | 0 | 3722 | 664 |
| measured after search: Advanced pawns/check evasion | 30.934 | 30.934 | 30211 | 0 | 3722 | 664 |
| final process state | 30.934 | 30.934 | 30211 | 0 | 3722 | 664 |

## Four Evaluation Correctness Fixes

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.873 | 65692 | 4416725 | c8e6 | 59 | c8e6 a2a4 g7h6 d2c4  |
| Kiwipete | 4 | 34.114 | 173292 | 5079800 | e2a6 | 194 | e2a6 e6d5 c3d5 e7e5 e1g1  |
| King safety | 4 | 28.063 | 137958 | 4916018 | c3d5 | 94 | c3d5 e7d8 d5f6 g7f6 d3d4  |
| Endgame | 5 | 1.016 | 4990 | 4909817 | b4f4 | 152 | b4f4 h4g3 f4f3 g3g2 f3h3  |
| Promotion tactic | 4 | 3.521 | 18352 | 5211869 | d7c8q | 533 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 46.354 | 247102 | 5330800 | d2d4 | -544 | d2d4 a3b4 a1b1 g7h6 f3e5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 127.942 | 647386 | 5060015 |

Aggregate elapsed times for all five processes: P1=127.942 ms, P2=127.914 ms, P3=127.547 ms, P4=128.061 ms, P5=128.992 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.855 | 28.859 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.457 | 29.457 | 12431 | 0 | 1316 | 288 |
| warmup loaded: Kiwipete | 29.457 | 29.457 | 12431 | 0 | 1316 | 288 |
| warmup after search: Kiwipete | 29.680 | 29.680 | 23913 | 0 | 3323 | 632 |
| warmup loaded: King safety | 29.680 | 29.680 | 23913 | 0 | 3323 | 632 |
| warmup after search: King safety | 29.945 | 29.945 | 36770 | 0 | 3745 | 705 |
| warmup loaded: Endgame | 29.945 | 29.945 | 36770 | 0 | 3745 | 705 |
| warmup after search: Endgame | 29.957 | 29.957 | 37833 | 0 | 3764 | 710 |
| warmup loaded: Promotion tactic | 29.957 | 29.957 | 37833 | 0 | 3764 | 710 |
| warmup after search: Promotion tactic | 29.988 | 29.988 | 38577 | 0 | 3888 | 727 |
| warmup loaded: Advanced pawns/check evasion | 29.988 | 29.988 | 38577 | 0 | 3888 | 727 |
| warmup after search: Advanced pawns/check evasion | 30.434 | 30.434 | 45771 | 0 | 4609 | 827 |
| after complete warmup | 30.434 | 30.434 | 45771 | 0 | 4609 | 827 |
| measured loaded: Quiet middlegame | 30.434 | 30.434 | 45771 | 0 | 4609 | 827 |
| measured after search: Quiet middlegame | 30.879 | 30.879 | 45771 | 0 | 4609 | 827 |
| measured loaded: Kiwipete | 30.879 | 30.879 | 45771 | 0 | 4609 | 827 |
| measured after search: Kiwipete | 31.055 | 31.055 | 45771 | 0 | 4609 | 827 |
| measured loaded: King safety | 31.055 | 31.055 | 45771 | 0 | 4609 | 827 |
| measured after search: King safety | 31.320 | 31.320 | 45771 | 0 | 4609 | 827 |
| measured loaded: Endgame | 31.320 | 31.320 | 45771 | 0 | 4609 | 827 |
| measured after search: Endgame | 31.332 | 31.336 | 45771 | 0 | 4609 | 827 |
| measured loaded: Promotion tactic | 31.336 | 31.336 | 45771 | 0 | 4609 | 827 |
| measured after search: Promotion tactic | 31.363 | 31.363 | 45771 | 0 | 4609 | 827 |
| measured loaded: Advanced pawns/check evasion | 31.363 | 31.363 | 45771 | 0 | 4609 | 827 |
| measured after search: Advanced pawns/check evasion | 31.797 | 31.797 | 45771 | 0 | 4609 | 827 |
| final process state | 31.797 | 31.797 | 45771 | 0 | 4609 | 827 |

## Evaluation Correctness and Transcription Fixes

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.762 | 65692 | 4450110 | c8e6 | 59 | c8e6 a2a4 g7h6 d2c4  |
| Kiwipete | 4 | 35.100 | 177405 | 5054282 | e2a6 | 194 | e2a6 e6d5 c3d5 e7e5 e1g1  |
| King safety | 4 | 27.971 | 137958 | 4932203 | c3d5 | 94 | c3d5 e7d8 d5f6 g7f6 d3d4  |
| Endgame | 5 | 0.976 | 4990 | 5111427 | b4f4 | 152 | b4f4 h4g3 f4f3 g3g2 f3h3  |
| Promotion tactic | 4 | 3.493 | 18352 | 5254375 | d7c8q | 533 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 46.383 | 247102 | 5327426 | d2d4 | -544 | d2d4 a3b4 a1b1 g7h6 f3e5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 128.685 | 651499 | 5062756 |

Aggregate elapsed times for all five processes: P1=129.448 ms, P2=128.685 ms, P3=127.907 ms, P4=128.615 ms, P5=129.959 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.816 | 28.820 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.824 | 28.828 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.418 | 29.418 | 12431 | 0 | 1316 | 288 |
| warmup loaded: Kiwipete | 29.418 | 29.418 | 12431 | 0 | 1316 | 288 |
| warmup after search: Kiwipete | 29.645 | 29.645 | 23962 | 0 | 3360 | 633 |
| warmup loaded: King safety | 29.645 | 29.645 | 23962 | 0 | 3360 | 633 |
| warmup after search: King safety | 29.910 | 29.910 | 36820 | 0 | 3781 | 706 |
| warmup loaded: Endgame | 29.910 | 29.910 | 36820 | 0 | 3781 | 706 |
| warmup after search: Endgame | 29.922 | 29.922 | 37883 | 0 | 3800 | 711 |
| warmup loaded: Promotion tactic | 29.922 | 29.922 | 37883 | 0 | 3800 | 711 |
| warmup after search: Promotion tactic | 29.953 | 29.953 | 38627 | 0 | 3924 | 728 |
| warmup loaded: Advanced pawns/check evasion | 29.953 | 29.953 | 38627 | 0 | 3924 | 728 |
| warmup after search: Advanced pawns/check evasion | 30.398 | 30.402 | 45822 | 0 | 4639 | 828 |
| after complete warmup | 30.402 | 30.402 | 45822 | 0 | 4639 | 828 |
| measured loaded: Quiet middlegame | 30.402 | 30.402 | 45822 | 0 | 4639 | 828 |
| measured after search: Quiet middlegame | 30.844 | 30.844 | 45822 | 0 | 4639 | 828 |
| measured loaded: Kiwipete | 30.844 | 30.844 | 45822 | 0 | 4639 | 828 |
| measured after search: Kiwipete | 31.023 | 31.027 | 45822 | 0 | 4639 | 828 |
| measured loaded: King safety | 31.027 | 31.027 | 45822 | 0 | 4639 | 828 |
| measured after search: King safety | 31.293 | 31.293 | 45822 | 0 | 4639 | 828 |
| measured loaded: Endgame | 31.293 | 31.293 | 45822 | 0 | 4639 | 828 |
| measured after search: Endgame | 31.305 | 31.305 | 45822 | 0 | 4639 | 828 |
| measured loaded: Promotion tactic | 31.305 | 31.305 | 45822 | 0 | 4639 | 828 |
| measured after search: Promotion tactic | 31.336 | 31.336 | 45822 | 0 | 4639 | 828 |
| measured loaded: Advanced pawns/check evasion | 31.340 | 31.340 | 45822 | 0 | 4639 | 828 |
| measured after search: Advanced pawns/check evasion | 31.770 | 31.770 | 45822 | 0 | 4639 | 828 |
| final process state | 31.770 | 31.770 | 45822 | 0 | 4639 | 828 |

## PawnMoveCenterValueWhite Normalization

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 15.010 | 66376 | 4422160 | c8e6 | 59 | c8e6 a2a4 g7h6 d2c4  |
| Kiwipete | 4 | 33.836 | 171997 | 5083250 | e2a6 | 197 | e2a6 e6d5 c3d5 e7e5 e1g1  |
| King safety | 4 | 26.087 | 127866 | 4901496 | c3d5 | 94 | c3d5 e7d8 d5f6 g7f6 d3d4  |
| Endgame | 5 | 0.979 | 4856 | 4960431 | b4f4 | 152 | b4f4 h4g3 f4f3 g3g2 f3h3  |
| Promotion tactic | 4 | 3.502 | 18352 | 5240387 | d7c8q | 533 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 45.220 | 240352 | 5315113 | d2d4 | -541 | d2d4 a3b4 a1b1 g7h6 f3e5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 124.634 | 629799 | 5053167 |

Aggregate elapsed times for all five processes: P1=124.478 ms, P2=126.440 ms, P3=124.634 ms, P4=125.723 ms, P5=123.176 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.859 | 28.863 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.867 | 28.871 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.461 | 29.461 | 12302 | 0 | 1320 | 287 |
| warmup loaded: Kiwipete | 29.461 | 29.461 | 12302 | 0 | 1320 | 287 |
| warmup after search: Kiwipete | 29.680 | 29.680 | 23017 | 0 | 3315 | 625 |
| warmup loaded: King safety | 29.680 | 29.680 | 23017 | 0 | 3315 | 625 |
| warmup after search: King safety | 29.945 | 29.945 | 34688 | 0 | 3752 | 696 |
| warmup loaded: Endgame | 29.945 | 29.945 | 34688 | 0 | 3752 | 696 |
| warmup after search: Endgame | 29.957 | 29.961 | 35710 | 0 | 3770 | 701 |
| warmup loaded: Promotion tactic | 29.961 | 29.961 | 35710 | 0 | 3770 | 701 |
| warmup after search: Promotion tactic | 29.988 | 29.988 | 36454 | 0 | 3895 | 718 |
| warmup loaded: Advanced pawns/check evasion | 29.988 | 29.988 | 36454 | 0 | 3895 | 718 |
| warmup after search: Advanced pawns/check evasion | 30.438 | 30.438 | 43674 | 0 | 4628 | 823 |
| after complete warmup | 30.438 | 30.438 | 43674 | 0 | 4628 | 823 |
| measured loaded: Quiet middlegame | 30.438 | 30.438 | 43674 | 0 | 4628 | 823 |
| measured after search: Quiet middlegame | 30.883 | 30.883 | 43674 | 0 | 4628 | 823 |
| measured loaded: Kiwipete | 30.883 | 30.883 | 43674 | 0 | 4628 | 823 |
| measured after search: Kiwipete | 31.059 | 31.062 | 43674 | 0 | 4628 | 823 |
| measured loaded: King safety | 31.062 | 31.062 | 43674 | 0 | 4628 | 823 |
| measured after search: King safety | 31.324 | 31.324 | 43674 | 0 | 4628 | 823 |
| measured loaded: Endgame | 31.324 | 31.324 | 43674 | 0 | 4628 | 823 |
| measured after search: Endgame | 31.340 | 31.340 | 43674 | 0 | 4628 | 823 |
| measured loaded: Promotion tactic | 31.340 | 31.340 | 43674 | 0 | 4628 | 823 |
| measured after search: Promotion tactic | 31.371 | 31.371 | 43674 | 0 | 4628 | 823 |
| measured loaded: Advanced pawns/check evasion | 31.371 | 31.371 | 43674 | 0 | 4628 | 823 |
| measured after search: Advanced pawns/check evasion | 31.805 | 31.805 | 43674 | 0 | 4628 | 823 |
| final process state | 31.805 | 31.805 | 43674 | 0 | 4628 | 823 |

## Preserve Exact Root Move Ordering

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 15.032 | 66376 | 4415537 | c8e6 | 59 | c8e6 a2a4 g7h6 d2c4  |
| Kiwipete | 4 | 33.598 | 171997 | 5119287 | e2a6 | 197 | e2a6 e6d5 c3d5 e7e5 e1g1  |
| King safety | 4 | 25.902 | 127866 | 4936605 | c3d5 | 94 | c3d5 e7d8 d5f6 g7f6 d3d4  |
| Endgame | 5 | 0.962 | 4856 | 5050305 | b4f4 | 152 | b4f4 h4g3 f4f3 g3g2 f3h3  |
| Promotion tactic | 4 | 3.468 | 18352 | 5291287 | d7c8q | 533 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 44.895 | 240352 | 5353613 | d2d4 | -541 | d2d4 a3b4 a1b1 g7h6 f3e5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 123.857 | 629799 | 5084889 |

Aggregate elapsed times for all five processes: P1=124.889 ms, P2=123.419 ms, P3=123.857 ms, P4=123.443 ms, P5=123.893 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.855 | 28.859 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.457 | 29.457 | 12302 | 0 | 1320 | 287 |
| warmup loaded: Kiwipete | 29.457 | 29.457 | 12302 | 0 | 1320 | 287 |
| warmup after search: Kiwipete | 29.680 | 29.680 | 23018 | 0 | 3315 | 625 |
| warmup loaded: King safety | 29.680 | 29.680 | 23018 | 0 | 3315 | 625 |
| warmup after search: King safety | 29.945 | 29.945 | 34684 | 0 | 3752 | 696 |
| warmup loaded: Endgame | 29.945 | 29.945 | 34684 | 0 | 3752 | 696 |
| warmup after search: Endgame | 29.957 | 29.961 | 35705 | 0 | 3770 | 701 |
| warmup loaded: Promotion tactic | 29.961 | 29.961 | 35705 | 0 | 3770 | 701 |
| warmup after search: Promotion tactic | 29.988 | 29.988 | 36449 | 0 | 3895 | 718 |
| warmup loaded: Advanced pawns/check evasion | 29.988 | 29.988 | 36449 | 0 | 3895 | 718 |
| warmup after search: Advanced pawns/check evasion | 30.438 | 30.438 | 43669 | 0 | 4628 | 823 |
| after complete warmup | 30.438 | 30.438 | 43669 | 0 | 4628 | 823 |
| measured loaded: Quiet middlegame | 30.438 | 30.438 | 43669 | 0 | 4628 | 823 |
| measured after search: Quiet middlegame | 30.883 | 30.883 | 43669 | 0 | 4628 | 823 |
| measured loaded: Kiwipete | 30.883 | 30.883 | 43669 | 0 | 4628 | 823 |
| measured after search: Kiwipete | 31.059 | 31.062 | 43669 | 0 | 4628 | 823 |
| measured loaded: King safety | 31.062 | 31.062 | 43669 | 0 | 4628 | 823 |
| measured after search: King safety | 31.324 | 31.324 | 43669 | 0 | 4628 | 823 |
| measured loaded: Endgame | 31.324 | 31.324 | 43669 | 0 | 4628 | 823 |
| measured after search: Endgame | 31.340 | 31.340 | 43669 | 0 | 4628 | 823 |
| measured loaded: Promotion tactic | 31.340 | 31.340 | 43669 | 0 | 4628 | 823 |
| measured after search: Promotion tactic | 31.371 | 31.371 | 43669 | 0 | 4628 | 823 |
| measured loaded: Advanced pawns/check evasion | 31.371 | 31.371 | 43669 | 0 | 4628 | 823 |
| measured after search: Advanced pawns/check evasion | 31.805 | 31.805 | 43669 | 0 | 4628 | 823 |
| final process state | 31.805 | 31.805 | 43669 | 0 | 4628 | 823 |

## UCI PV Reporting Cleanliness

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.954 | 66376 | 4438542 | c8e6 | 59 | c8e6 a2a4 g7h6 d2c4  |
| Kiwipete | 4 | 33.714 | 171997 | 5101718 | e2a6 | 197 | e2a6 e6d5 c3d5 e7e5 e1g1  |
| King safety | 4 | 25.935 | 127866 | 4930156 | c3d5 | 94 | c3d5 e7d8 d5f6 g7f6 d3d4  |
| Endgame | 5 | 0.966 | 4856 | 5025515 | b4f4 | 152 | b4f4 h4g3 f4f3 g3g2 f3h3  |
| Promotion tactic | 4 | 3.475 | 18352 | 5280576 | d7c8q | 533 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 45.260 | 240352 | 5310438 | d2d4 | -541 | d2d4 a3b4 a1b1 g7h6 f3e5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 124.305 | 629799 | 5066544 |

Aggregate elapsed times for all five processes: P1=124.489 ms, P2=123.823 ms, P3=124.445 ms, P4=124.305 ms, P5=123.579 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.844 | 28.848 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.852 | 28.855 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.445 | 29.445 | 12302 | 0 | 1320 | 287 |
| warmup loaded: Kiwipete | 29.445 | 29.445 | 12302 | 0 | 1320 | 287 |
| warmup after search: Kiwipete | 29.668 | 29.668 | 23018 | 0 | 3315 | 625 |
| warmup loaded: King safety | 29.668 | 29.668 | 23018 | 0 | 3315 | 625 |
| warmup after search: King safety | 29.934 | 29.934 | 34688 | 0 | 3752 | 696 |
| warmup loaded: Endgame | 29.934 | 29.934 | 34688 | 0 | 3752 | 696 |
| warmup after search: Endgame | 29.945 | 29.949 | 35710 | 0 | 3770 | 701 |
| warmup loaded: Promotion tactic | 29.949 | 29.949 | 35710 | 0 | 3770 | 701 |
| warmup after search: Promotion tactic | 29.977 | 29.977 | 36454 | 0 | 3895 | 718 |
| warmup loaded: Advanced pawns/check evasion | 29.977 | 29.977 | 36454 | 0 | 3895 | 718 |
| warmup after search: Advanced pawns/check evasion | 30.426 | 30.426 | 43674 | 0 | 4628 | 823 |
| after complete warmup | 30.426 | 30.426 | 43674 | 0 | 4628 | 823 |
| measured loaded: Quiet middlegame | 30.426 | 30.426 | 43674 | 0 | 4628 | 823 |
| measured after search: Quiet middlegame | 30.871 | 30.871 | 43674 | 0 | 4628 | 823 |
| measured loaded: Kiwipete | 30.871 | 30.871 | 43674 | 0 | 4628 | 823 |
| measured after search: Kiwipete | 31.047 | 31.051 | 43674 | 0 | 4628 | 823 |
| measured loaded: King safety | 31.051 | 31.051 | 43674 | 0 | 4628 | 823 |
| measured after search: King safety | 31.312 | 31.312 | 43674 | 0 | 4628 | 823 |
| measured loaded: Endgame | 31.312 | 31.312 | 43674 | 0 | 4628 | 823 |
| measured after search: Endgame | 31.328 | 31.328 | 43674 | 0 | 4628 | 823 |
| measured loaded: Promotion tactic | 31.328 | 31.328 | 43674 | 0 | 4628 | 823 |
| measured after search: Promotion tactic | 31.359 | 31.359 | 43674 | 0 | 4628 | 823 |
| measured loaded: Advanced pawns/check evasion | 31.359 | 31.359 | 43674 | 0 | 4628 | 823 |
| measured after search: Advanced pawns/check evasion | 31.793 | 31.793 | 43674 | 0 | 4628 | 823 |
| final process state | 31.793 | 31.793 | 43674 | 0 | 4628 | 823 |

## PV Swap Fix Test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.638 | 66376 | 4534592 | c8e6 | 59 | c8e6 a2a4 g7h6 d2c4  |
| Kiwipete | 4 | 33.739 | 171997 | 5097901 | e2a6 | 197 | e2a6 e6d5 c3d5 e7e5 e1g1  |
| King safety | 4 | 25.773 | 127866 | 4961315 | c3d5 | 94 | c3d5 e7d8 d5f6 g7f6 d3d4  |
| Endgame | 5 | 0.952 | 4856 | 5102678 | b4f4 | 152 | b4f4 h4g3 f4f3 g3g2 f3h3  |
| Promotion tactic | 4 | 3.437 | 18352 | 5340253 | d7c8q | 533 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 45.088 | 240352 | 5330769 | d2d4 | -541 | d2d4 a3b4 a1b1 g7h6 f3e5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 123.625 | 629799 | 5094432 |

Aggregate elapsed times for all five processes: P1=123.656 ms, P2=123.132 ms, P3=123.625 ms, P4=123.781 ms, P5=123.218 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.820 | 28.824 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.828 | 28.832 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.422 | 29.422 | 12302 | 0 | 1320 | 287 |
| warmup loaded: Kiwipete | 29.422 | 29.422 | 12302 | 0 | 1320 | 287 |
| warmup after search: Kiwipete | 29.645 | 29.645 | 23018 | 0 | 3315 | 625 |
| warmup loaded: King safety | 29.645 | 29.645 | 23018 | 0 | 3315 | 625 |
| warmup after search: King safety | 29.910 | 29.910 | 34687 | 0 | 3752 | 696 |
| warmup loaded: Endgame | 29.910 | 29.910 | 34687 | 0 | 3752 | 696 |
| warmup after search: Endgame | 29.922 | 29.926 | 35709 | 0 | 3770 | 701 |
| warmup loaded: Promotion tactic | 29.926 | 29.926 | 35709 | 0 | 3770 | 701 |
| warmup after search: Promotion tactic | 29.953 | 29.953 | 36453 | 0 | 3895 | 718 |
| warmup loaded: Advanced pawns/check evasion | 29.953 | 29.953 | 36453 | 0 | 3895 | 718 |
| warmup after search: Advanced pawns/check evasion | 30.402 | 30.402 | 43673 | 0 | 4628 | 823 |
| after complete warmup | 30.402 | 30.402 | 43673 | 0 | 4628 | 823 |
| measured loaded: Quiet middlegame | 30.402 | 30.402 | 43673 | 0 | 4628 | 823 |
| measured after search: Quiet middlegame | 30.848 | 30.848 | 43673 | 0 | 4628 | 823 |
| measured loaded: Kiwipete | 30.848 | 30.848 | 43673 | 0 | 4628 | 823 |
| measured after search: Kiwipete | 31.023 | 31.027 | 43673 | 0 | 4628 | 823 |
| measured loaded: King safety | 31.027 | 31.027 | 43673 | 0 | 4628 | 823 |
| measured after search: King safety | 31.289 | 31.289 | 43673 | 0 | 4628 | 823 |
| measured loaded: Endgame | 31.289 | 31.289 | 43673 | 0 | 4628 | 823 |
| measured after search: Endgame | 31.305 | 31.305 | 43673 | 0 | 4628 | 823 |
| measured loaded: Promotion tactic | 31.305 | 31.305 | 43673 | 0 | 4628 | 823 |
| measured after search: Promotion tactic | 31.336 | 31.336 | 43673 | 0 | 4628 | 823 |
| measured loaded: Advanced pawns/check evasion | 31.336 | 31.336 | 43673 | 0 | 4628 | 823 |
| measured after search: Advanced pawns/check evasion | 31.770 | 31.770 | 43673 | 0 | 4628 | 823 |
| final process state | 31.770 | 31.770 | 43673 | 0 | 4628 | 823 |

## TT LMR Reduced Depth Fix

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 15.123 | 66376 | 4389086 | c8e6 | 59 | c8e6 a2a4 g7h6 d2c4  |
| Kiwipete | 4 | 33.926 | 171997 | 5069734 | e2a6 | 197 | e2a6 e6d5 c3d5 e7e5 e1g1  |
| King safety | 4 | 26.200 | 127866 | 4880314 | c3d5 | 94 | c3d5 e7d8 d5f6 g7f6 d3d4  |
| Endgame | 5 | 0.989 | 4856 | 4909553 | b4f4 | 152 | b4f4 h4g3 f4f3 g3g2 f3h3  |
| Promotion tactic | 4 | 3.511 | 18352 | 5226256 | d7c8q | 533 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 45.538 | 240404 | 5279194 | d2d4 | -541 | d2d4 a3b4 a1b1 g7h6 f3e5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 125.288 | 629851 | 5027219 |

Aggregate elapsed times for all five processes: P1=129.863 ms, P2=125.389 ms, P3=124.236 ms, P4=124.770 ms, P5=125.288 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.855 | 28.859 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.863 | 28.867 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.457 | 29.457 | 12302 | 0 | 1320 | 287 |
| warmup loaded: Kiwipete | 29.457 | 29.457 | 12302 | 0 | 1320 | 287 |
| warmup after search: Kiwipete | 29.680 | 29.680 | 23018 | 0 | 3315 | 625 |
| warmup loaded: King safety | 29.680 | 29.680 | 23018 | 0 | 3315 | 625 |
| warmup after search: King safety | 29.945 | 29.945 | 34689 | 0 | 3752 | 696 |
| warmup loaded: Endgame | 29.945 | 29.945 | 34689 | 0 | 3752 | 696 |
| warmup after search: Endgame | 29.957 | 29.961 | 35711 | 0 | 3770 | 701 |
| warmup loaded: Promotion tactic | 29.961 | 29.961 | 35711 | 0 | 3770 | 701 |
| warmup after search: Promotion tactic | 29.988 | 29.988 | 36455 | 0 | 3895 | 718 |
| warmup loaded: Advanced pawns/check evasion | 29.988 | 29.988 | 36455 | 0 | 3895 | 718 |
| warmup after search: Advanced pawns/check evasion | 30.434 | 30.434 | 43676 | 0 | 4628 | 823 |
| after complete warmup | 30.434 | 30.434 | 43676 | 0 | 4628 | 823 |
| measured loaded: Quiet middlegame | 30.434 | 30.434 | 43676 | 0 | 4628 | 823 |
| measured after search: Quiet middlegame | 30.879 | 30.879 | 43676 | 0 | 4628 | 823 |
| measured loaded: Kiwipete | 30.879 | 30.879 | 43676 | 0 | 4628 | 823 |
| measured after search: Kiwipete | 31.059 | 31.059 | 43676 | 0 | 4628 | 823 |
| measured loaded: King safety | 31.059 | 31.059 | 43676 | 0 | 4628 | 823 |
| measured after search: King safety | 31.324 | 31.324 | 43676 | 0 | 4628 | 823 |
| measured loaded: Endgame | 31.324 | 31.324 | 43676 | 0 | 4628 | 823 |
| measured after search: Endgame | 31.336 | 31.336 | 43676 | 0 | 4628 | 823 |
| measured loaded: Promotion tactic | 31.336 | 31.336 | 43676 | 0 | 4628 | 823 |
| measured after search: Promotion tactic | 31.367 | 31.367 | 43676 | 0 | 4628 | 823 |
| measured loaded: Advanced pawns/check evasion | 31.371 | 31.371 | 43676 | 0 | 4628 | 823 |
| measured after search: Advanced pawns/check evasion | 31.797 | 31.797 | 43676 | 0 | 4628 | 823 |
| final process state | 31.797 | 31.797 | 43676 | 0 | 4628 | 823 |

## PV-First Production Adoption

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.977 | 66376 | 4431974 | c8e6 | 59 | c8e6 a2a4 g7h6 d2c4  |
| Kiwipete | 4 | 33.317 | 171997 | 5162440 | e2a6 | 197 | e2a6 e6d5 c3d5 e7e5 e1g1  |
| King safety | 4 | 25.670 | 127933 | 4983758 | c3d5 | 94 | c3d5 e7d8 d5f6 g7f6 d3d4  |
| Endgame | 5 | 0.973 | 4856 | 4993228 | b4f4 | 152 | b4f4 h4g3 f4f3 g3g2 f3h3  |
| Promotion tactic | 4 | 3.448 | 18352 | 5323144 | d7c8q | 533 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 44.875 | 240352 | 5356001 | d2d4 | -541 | d2d4 a3b4 a1b1 g7h6 f3e5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 123.259 | 629866 | 5110102 |

Aggregate elapsed times for all five processes: P1=125.912 ms, P2=123.867 ms, P3=122.691 ms, P4=123.259 ms, P5=123.057 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.840 | 28.844 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.848 | 28.852 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.441 | 29.441 | 12302 | 0 | 1320 | 287 |
| warmup loaded: Kiwipete | 29.441 | 29.441 | 12302 | 0 | 1320 | 287 |
| warmup after search: Kiwipete | 29.664 | 29.664 | 23018 | 0 | 3315 | 625 |
| warmup loaded: King safety | 29.664 | 29.664 | 23018 | 0 | 3315 | 625 |
| warmup after search: King safety | 29.930 | 29.930 | 34688 | 0 | 3752 | 696 |
| warmup loaded: Endgame | 29.930 | 29.930 | 34688 | 0 | 3752 | 696 |
| warmup after search: Endgame | 29.938 | 29.941 | 35710 | 0 | 3770 | 701 |
| warmup loaded: Promotion tactic | 29.941 | 29.941 | 35710 | 0 | 3770 | 701 |
| warmup after search: Promotion tactic | 29.969 | 29.969 | 36454 | 0 | 3895 | 718 |
| warmup loaded: Advanced pawns/check evasion | 29.969 | 29.969 | 36454 | 0 | 3895 | 718 |
| warmup after search: Advanced pawns/check evasion | 30.418 | 30.418 | 43674 | 0 | 4628 | 823 |
| after complete warmup | 30.418 | 30.418 | 43674 | 0 | 4628 | 823 |
| measured loaded: Quiet middlegame | 30.418 | 30.418 | 43674 | 0 | 4628 | 823 |
| measured after search: Quiet middlegame | 30.863 | 30.863 | 43674 | 0 | 4628 | 823 |
| measured loaded: Kiwipete | 30.863 | 30.863 | 43674 | 0 | 4628 | 823 |
| measured after search: Kiwipete | 31.043 | 31.043 | 43674 | 0 | 4628 | 823 |
| measured loaded: King safety | 31.043 | 31.043 | 43674 | 0 | 4628 | 823 |
| measured after search: King safety | 31.305 | 31.305 | 43674 | 0 | 4628 | 823 |
| measured loaded: Endgame | 31.305 | 31.305 | 43674 | 0 | 4628 | 823 |
| measured after search: Endgame | 31.316 | 31.320 | 43674 | 0 | 4628 | 823 |
| measured loaded: Promotion tactic | 31.320 | 31.320 | 43674 | 0 | 4628 | 823 |
| measured after search: Promotion tactic | 31.348 | 31.348 | 43674 | 0 | 4628 | 823 |
| measured loaded: Advanced pawns/check evasion | 31.348 | 31.348 | 43674 | 0 | 4628 | 823 |
| measured after search: Advanced pawns/check evasion | 31.781 | 31.781 | 43674 | 0 | 4628 | 823 |
| final process state | 31.781 | 31.781 | 43674 | 0 | 4628 | 823 |

## PV-First Production Adoption

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 14.977 | 66376 | 4431974 | c8e6 | 59 | c8e6 a2a4 g7h6 d2c4  |
| Kiwipete | 4 | 33.317 | 171997 | 5162440 | e2a6 | 197 | e2a6 e6d5 c3d5 e7e5 e1g1  |
| King safety | 4 | 25.670 | 127933 | 4983758 | c3d5 | 94 | c3d5 e7d8 d5f6 g7f6 d3d4  |
| Endgame | 5 | 0.973 | 4856 | 4993228 | b4f4 | 152 | b4f4 h4g3 f4f3 g3g2 f3h3  |
| Promotion tactic | 4 | 3.448 | 18352 | 5323144 | d7c8q | 533 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 44.875 | 240352 | 5356001 | d2d4 | -541 | d2d4 a3b4 a1b1 g7h6 f3e5  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 123.259 | 629866 | 5110102 |

Aggregate elapsed times for all five processes: P1=125.912 ms, P2=123.867 ms, P3=122.691 ms, P4=123.259 ms, P5=123.057 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.840 | 28.844 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.848 | 28.852 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 29.441 | 29.441 | 12302 | 0 | 1320 | 287 |
| warmup loaded: Kiwipete | 29.441 | 29.441 | 12302 | 0 | 1320 | 287 |
| warmup after search: Kiwipete | 29.664 | 29.664 | 23018 | 0 | 3315 | 625 |
| warmup loaded: King safety | 29.664 | 29.664 | 23018 | 0 | 3315 | 625 |
| warmup after search: Endgame | 29.930 | 29.930 | 34688 | 0 | 3752 | 696 |
| warmup loaded: Endgame | 29.930 | 29.930 | 34688 | 0 | 3752 | 696 |
| warmup after search: Endgame | 29.938 | 29.941 | 35710 | 0 | 3770 | 701 |
| warmup loaded: Promotion tactic | 29.941 | 29.941 | 35710 | 0 | 3770 | 701 |
| warmup after search: Promotion tactic | 29.969 | 29.969 | 36454 | 0 | 3895 | 718 |
| warmup loaded: Advanced pawns/check evasion | 29.969 | 29.969 | 36454 | 0 | 3895 | 718 |
| warmup after search: Advanced pawns/check evasion | 30.418 | 30.418 | 43674 | 0 | 4628 | 823 |
| after complete warmup | 30.418 | 30.418 | 43674 | 0 | 4628 | 823 |
| measured loaded: Quiet middlegame | 30.418 | 30.418 | 43674 | 0 | 4628 | 823 |
| measured after search: Quiet middlegame | 30.863 | 30.863 | 43674 | 0 | 4628 | 823 |
| measured loaded: Kiwipete | 30.863 | 30.863 | 43674 | 0 | 4628 | 823 |
| measured after search: Kiwipete | 31.043 | 31.043 | 43674 | 0 | 4628 | 823 |
| measured loaded: King safety | 31.043 | 31.043 | 43674 | 0 | 4628 | 823 |
| measured after search: King safety | 31.305 | 31.305 | 43674 | 0 | 4628 | 823 |
| measured loaded: Endgame | 31.305 | 31.305 | 43674 | 0 | 4628 | 823 |
| measured after search: Endgame | 31.316 | 31.320 | 43674 | 0 | 4628 | 823 |
| measured loaded: Promotion tactic | 31.320 | 31.320 | 43674 | 0 | 4628 | 823 |
| measured after search: Promotion tactic | 31.348 | 31.348 | 43674 | 0 | 4628 | 823 |
| measured loaded: Advanced pawns/check evasion | 31.348 | 31.348 | 43674 | 0 | 4628 | 823 |
| measured after search: Advanced pawns/check evasion | 31.781 | 31.781 | 43674 | 0 | 4628 | 823 |
| final process state | 31.781 | 31.781 | 43674 | 0 | 4628 | 823 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 2.300 | 3806 | 1654990 | c7c5 | 21 | c7c5 d2c4 b7b5 c4b6  |
| Kiwipete | 4 | 1.466 | 4117 | 2807992 | e2a6 | 64 | e2a6 b4c3 b2c3 e6d5 a6c8  |
| King safety | 4 | 2.246 | 5367 | 2389295 | c3d5 | 114 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.329 | 991 | 3010950 | b4f4 | 175 | b4f4 h4g3 f4c4 g3f2 c4c7 f2e2  |
| Promotion tactic | 4 | 0.366 | 967 | 2639948 | d7c8q | 663 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.642 | 3763 | 2291689 | c4c5 | -577 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 8.350 | 19011 | 2276875 |

Aggregate elapsed times for all five processes: P1=8.419 ms, P2=8.344 ms, P3=8.684 ms, P4=8.350 ms, P5=8.257 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.254 | 29.258 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.262 | 29.266 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.102 | 30.105 | 1431 | 0 | 538 | 144 |
| warmup loaded: Kiwipete | 30.105 | 30.105 | 1431 | 0 | 538 | 144 |
| warmup after search: Kiwipete | 30.223 | 30.223 | 3703 | 0 | 1670 | 323 |
| warmup loaded: King safety | 30.223 | 30.223 | 3703 | 0 | 1670 | 323 |
| warmup after search: King safety | 30.406 | 30.406 | 6144 | 0 | 1919 | 367 |
| warmup loaded: Endgame | 30.406 | 30.406 | 6144 | 0 | 1919 | 367 |
| warmup after search: Endgame | 30.430 | 30.430 | 6438 | 0 | 1954 | 377 |
| warmup loaded: Promotion tactic | 30.430 | 30.430 | 6438 | 0 | 1954 | 377 |
| warmup after search: Promotion tactic | 30.445 | 30.445 | 6796 | 0 | 2070 | 391 |
| warmup loaded: Advanced pawns/check evasion | 30.445 | 30.445 | 6796 | 0 | 2070 | 391 |
| warmup after search: Advanced pawns/check evasion | 30.480 | 30.480 | 8414 | 0 | 2379 | 439 |
| after complete warmup | 30.480 | 30.480 | 8414 | 0 | 2379 | 439 |
| measured loaded: Quiet middlegame | 30.480 | 30.480 | 8414 | 0 | 2379 | 439 |
| measured after search: Quiet middlegame | 30.488 | 30.488 | 8590 | 0 | 2383 | 439 |
| measured loaded: Kiwipete | 30.488 | 30.488 | 8590 | 0 | 2383 | 439 |
| measured after search: Kiwipete | 30.488 | 30.488 | 8590 | 0 | 2383 | 439 |
| measured loaded: King safety | 30.488 | 30.488 | 8590 | 0 | 2383 | 439 |
| measured after search: King safety | 30.496 | 30.496 | 8639 | 0 | 2383 | 439 |
| measured loaded: Endgame | 30.496 | 30.496 | 8639 | 0 | 2383 | 439 |
| measured after search: Endgame | 30.496 | 30.496 | 8641 | 0 | 2383 | 439 |
| measured loaded: Promotion tactic | 30.496 | 30.496 | 8641 | 0 | 2383 | 439 |
| measured after search: Promotion tactic | 30.496 | 30.496 | 8642 | 0 | 2383 | 439 |
| measured loaded: Advanced pawns/check evasion | 30.496 | 30.496 | 8642 | 0 | 2383 | 439 |
| measured after search: Advanced pawns/check evasion | 30.500 | 30.500 | 8643 | 0 | 2383 | 439 |
| final process state | 30.500 | 30.500 | 8643 | 0 | 2383 | 439 |

## Exact Board Restoration Fix

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.103 | 8219 | 1610615 | b8c6 | 42 | b8c6 d2b3 c8e6 c1g5  |
| Kiwipete | 4 | 4.132 | 13161 | 3185074 | e2a6 | 80 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 5.065 | 14515 | 2865499 | c3d5 | 116 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.465 | 1392 | 2991174 | b4f4 | 154 | b4f4 h4g3 f4f7 c7c6 f7f5  |
| Promotion tactic | 4 | 0.576 | 1543 | 2677299 | d7c8q | 653 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 4.164 | 13557 | 3255891 | c4c5 | -612 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 19.506 | 52387 | 2685676 |

Aggregate elapsed times for all five processes: P1=19.618 ms, P2=19.530 ms, P3=19.228 ms, P4=19.506 ms, P5=19.372 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.312 | 29.316 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.320 | 29.324 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.324 | 30.324 | 2907 | 0 | 578 | 150 |
| warmup loaded: Kiwipete | 30.324 | 30.324 | 2907 | 0 | 578 | 150 |
| warmup after search: Kiwipete | 30.434 | 30.434 | 4904 | 0 | 1458 | 288 |
| warmup loaded: King safety | 30.434 | 30.434 | 4904 | 0 | 1458 | 288 |
| warmup after search: King safety | 30.465 | 30.465 | 8014 | 0 | 1725 | 338 |
| warmup loaded: Endgame | 30.465 | 30.465 | 8014 | 0 | 1725 | 338 |
| warmup after search: Endgame | 30.465 | 30.465 | 8348 | 0 | 1770 | 349 |
| warmup loaded: Promotion tactic | 30.465 | 30.465 | 8348 | 0 | 1770 | 349 |
| warmup after search: Promotion tactic | 30.465 | 30.465 | 8693 | 0 | 1880 | 362 |
| warmup loaded: Advanced pawns/check evasion | 30.465 | 30.465 | 8693 | 0 | 1880 | 362 |
| warmup after search: Advanced pawns/check evasion | 30.516 | 30.516 | 10447 | 0 | 2181 | 422 |
| after complete warmup | 30.516 | 30.516 | 10447 | 0 | 2181 | 422 |
| measured loaded: Quiet middlegame | 30.516 | 30.516 | 10447 | 0 | 2181 | 422 |
| measured after search: Quiet middlegame | 30.516 | 30.516 | 10447 | 0 | 2181 | 422 |
| measured loaded: Kiwipete | 30.516 | 30.516 | 10447 | 0 | 2181 | 422 |
| measured after search: Kiwipete | 30.516 | 30.516 | 10447 | 0 | 2181 | 422 |
| measured loaded: King safety | 30.516 | 30.516 | 10447 | 0 | 2181 | 422 |
| measured after search: King safety | 30.516 | 30.520 | 10447 | 0 | 2181 | 422 |
| measured loaded: Endgame | 30.520 | 30.520 | 10447 | 0 | 2181 | 422 |
| measured after search: Endgame | 30.520 | 30.520 | 10447 | 0 | 2181 | 422 |
| measured loaded: Promotion tactic | 30.520 | 30.520 | 10447 | 0 | 2181 | 422 |
| measured after search: Promotion tactic | 30.520 | 30.520 | 10447 | 0 | 2181 | 422 |
| measured loaded: Advanced pawns/check evasion | 30.520 | 30.520 | 10447 | 0 | 2181 | 422 |
| measured after search: Advanced pawns/check evasion | 30.520 | 30.520 | 10447 | 0 | 2181 | 422 |
| final process state | 30.520 | 30.520 | 10447 | 0 | 2181 | 422 |

## QSearch Quiet Checks Pruned

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.324 | 8219 | 1543706 | b8c6 | 42 | b8c6 d2b3 c8e6 c1g5  |
| Kiwipete | 4 | 4.272 | 13161 | 3080811 | e2a6 | 80 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 5.236 | 14515 | 2772008 | c3d5 | 116 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.483 | 1392 | 2880675 | b4f4 | 154 | b4f4 h4g3 f4f7 c7c6 f7f5  |
| Promotion tactic | 4 | 0.573 | 1543 | 2694725 | d7c8q | 653 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 4.312 | 13557 | 3144050 | c4c5 | -612 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 20.200 | 52387 | 2593393 |

Aggregate elapsed times for all five processes: P1=20.642 ms, P2=20.137 ms, P3=20.200 ms, P4=20.249 ms, P5=20.141 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.391 | 29.395 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.398 | 29.402 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.402 | 30.402 | 2907 | 0 | 578 | 149 |
| warmup loaded: Kiwipete | 30.402 | 30.402 | 2907 | 0 | 578 | 149 |
| warmup after search: Kiwipete | 30.512 | 30.512 | 4904 | 0 | 1458 | 287 |
| warmup loaded: King safety | 30.512 | 30.512 | 4904 | 0 | 1458 | 287 |
| warmup after search: King safety | 30.539 | 30.539 | 8014 | 0 | 1725 | 336 |
| warmup loaded: Endgame | 30.539 | 30.539 | 8014 | 0 | 1725 | 336 |
| warmup after search: Endgame | 30.539 | 30.539 | 8348 | 0 | 1770 | 347 |
| warmup loaded: Promotion tactic | 30.539 | 30.539 | 8348 | 0 | 1770 | 347 |
| warmup after search: Promotion tactic | 30.539 | 30.539 | 8693 | 0 | 1880 | 361 |
| warmup loaded: Advanced pawns/check evasion | 30.539 | 30.539 | 8693 | 0 | 1880 | 361 |
| warmup after search: Advanced pawns/check evasion | 30.590 | 30.590 | 10447 | 0 | 2181 | 419 |
| after complete warmup | 30.590 | 30.590 | 10447 | 0 | 2181 | 419 |
| measured loaded: Quiet middlegame | 30.590 | 30.590 | 10447 | 0 | 2181 | 419 |
| measured after search: Quiet middlegame | 30.590 | 30.590 | 10447 | 0 | 2181 | 419 |
| measured loaded: Kiwipete | 30.590 | 30.590 | 10447 | 0 | 2181 | 419 |
| measured after search: Kiwipete | 30.590 | 30.590 | 10447 | 0 | 2181 | 419 |
| measured loaded: King safety | 30.590 | 30.590 | 10447 | 0 | 2181 | 419 |
| measured after search: King safety | 30.590 | 30.594 | 10447 | 0 | 2181 | 419 |
| measured loaded: Endgame | 30.594 | 30.594 | 10447 | 0 | 2181 | 419 |
| measured after search: Endgame | 30.594 | 30.594 | 10447 | 0 | 2181 | 419 |
| measured loaded: Promotion tactic | 30.594 | 30.594 | 10447 | 0 | 2181 | 419 |
| measured after search: Promotion tactic | 30.594 | 30.594 | 10447 | 0 | 2181 | 419 |
| measured loaded: Advanced pawns/check evasion | 30.594 | 30.594 | 10447 | 0 | 2181 | 419 |
| measured after search: Advanced pawns/check evasion | 30.594 | 30.594 | 10447 | 0 | 2181 | 419 |
| final process state | 30.594 | 30.594 | 10447 | 0 | 2181 | 419 |

## QSearch Quiet Checks Optimization

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.083 | 8219 | 1617061 | b8c6 | 42 | b8c6 d2b3 c8e6 c1g5  |
| Kiwipete | 4 | 4.005 | 13161 | 3285905 | e2a6 | 80 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 5.037 | 14515 | 2881843 | c3d5 | 116 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.436 | 1392 | 3189704 | b4f4 | 154 | b4f4 h4g3 f4f7 c7c6 f7f5  |
| Promotion tactic | 4 | 0.575 | 1543 | 2684103 | d7c8q | 653 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 4.087 | 13557 | 3317376 | c4c5 | -612 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 19.223 | 52387 | 2725281 |

Aggregate elapsed times for all five processes: P1=19.750 ms, P2=19.056 ms, P3=19.071 ms, P4=19.223 ms, P5=19.558 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.332 | 29.336 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.340 | 29.344 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.344 | 30.344 | 2907 | 0 | 578 | 149 |
| warmup loaded: Kiwipete | 30.344 | 30.344 | 2907 | 0 | 578 | 149 |
| warmup after search: Kiwipete | 30.457 | 30.457 | 4904 | 0 | 1458 | 287 |
| warmup loaded: King safety | 30.457 | 30.457 | 4904 | 0 | 1458 | 287 |
| warmup after search: King safety | 30.484 | 30.484 | 8014 | 0 | 1725 | 336 |
| warmup loaded: Endgame | 30.484 | 30.484 | 8014 | 0 | 1725 | 336 |
| warmup after search: Endgame | 30.484 | 30.484 | 8348 | 0 | 1770 | 347 |
| warmup loaded: Promotion tactic | 30.484 | 30.484 | 8348 | 0 | 1770 | 347 |
| warmup after search: Promotion tactic | 30.484 | 30.484 | 8693 | 0 | 1880 | 361 |
| warmup loaded: Advanced pawns/check evasion | 30.484 | 30.484 | 8693 | 0 | 1880 | 361 |
| warmup after search: Advanced pawns/check evasion | 30.535 | 30.535 | 10447 | 0 | 2181 | 419 |
| after complete warmup | 30.535 | 30.535 | 10447 | 0 | 2181 | 419 |
| measured loaded: Quiet middlegame | 30.535 | 30.535 | 10447 | 0 | 2181 | 419 |
| measured after search: Quiet middlegame | 30.535 | 30.535 | 10447 | 0 | 2181 | 419 |
| measured loaded: Kiwipete | 30.535 | 30.535 | 10447 | 0 | 2181 | 419 |
| measured after search: Kiwipete | 30.535 | 30.535 | 10447 | 0 | 2181 | 419 |
| measured loaded: King safety | 30.535 | 30.535 | 10447 | 0 | 2181 | 419 |
| measured after search: King safety | 30.535 | 30.539 | 10447 | 0 | 2181 | 419 |
| measured loaded: Endgame | 30.539 | 30.539 | 10447 | 0 | 2181 | 419 |
| measured after search: Endgame | 30.539 | 30.539 | 10447 | 0 | 2181 | 419 |
| measured loaded: Promotion tactic | 30.539 | 30.539 | 10447 | 0 | 2181 | 419 |
| measured after search: Promotion tactic | 30.539 | 30.539 | 10447 | 0 | 2181 | 419 |
| measured loaded: Advanced pawns/check evasion | 30.539 | 30.539 | 10447 | 0 | 2181 | 419 |
| measured after search: Advanced pawns/check evasion | 30.539 | 30.539 | 10447 | 0 | 2181 | 419 |
| final process state | 30.539 | 30.539 | 10447 | 0 | 2181 | 419 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 7.367 | 12640 | 1715685 | b8c6 | 20 | b8c6 f3g5 g7h6 g5e6  |
| Kiwipete | 4 | 1.626 | 6189 | 3805985 | e2a6 | 132 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 3.861 | 10752 | 2784835 | c3d5 | 126 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.745 | 2275 | 3054101 | b4f4 | 220 | b4f4 h4g5 f4f7 h5h2 f7g7 g5f5 g7c7 h2g2  |
| Promotion tactic | 4 | 0.382 | 997 | 2612861 | d7c8q | 588 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.536 | 4220 | 2747145 | c4c5 | -575 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 15.517 | 37073 | 2389191 |

Aggregate elapsed times for all five processes: P1=15.335 ms, P2=15.156 ms, P3=16.385 ms, P4=15.517 ms, P5=15.534 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.277 | 29.277 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.285 | 29.285 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.289 | 30.289 | 4811 | 0 | 684 | 182 |
| warmup loaded: Kiwipete | 30.289 | 30.289 | 4811 | 0 | 684 | 182 |
| warmup after search: Kiwipete | 30.324 | 30.324 | 5877 | 0 | 1177 | 258 |
| warmup loaded: King safety | 30.324 | 30.324 | 5877 | 0 | 1177 | 258 |
| warmup after search: King safety | 30.324 | 30.324 | 8374 | 0 | 1469 | 306 |
| warmup loaded: Endgame | 30.324 | 30.324 | 8374 | 0 | 1469 | 306 |
| warmup after search: Endgame | 30.324 | 30.324 | 8954 | 0 | 1522 | 316 |
| warmup loaded: Promotion tactic | 30.324 | 30.324 | 8954 | 0 | 1522 | 316 |
| warmup after search: Promotion tactic | 30.324 | 30.324 | 9215 | 0 | 1630 | 329 |
| warmup loaded: Advanced pawns/check evasion | 30.324 | 30.324 | 9215 | 0 | 1630 | 329 |
| warmup after search: Advanced pawns/check evasion | 30.344 | 30.344 | 10190 | 0 | 1793 | 352 |
| after complete warmup | 30.344 | 30.344 | 10190 | 0 | 1793 | 352 |
| measured loaded: Quiet middlegame | 30.344 | 30.344 | 10190 | 0 | 1793 | 352 |
| measured after search: Quiet middlegame | 30.344 | 30.344 | 10190 | 0 | 1793 | 352 |
| measured loaded: Kiwipete | 30.344 | 30.344 | 10190 | 0 | 1793 | 352 |
| measured after search: Kiwipete | 30.344 | 30.344 | 10190 | 0 | 1793 | 352 |
| measured loaded: King safety | 30.344 | 30.344 | 10190 | 0 | 1793 | 352 |
| measured after search: King safety | 30.348 | 30.348 | 10190 | 0 | 1793 | 352 |
| measured loaded: Endgame | 30.348 | 30.348 | 10190 | 0 | 1793 | 352 |
| measured after search: Endgame | 30.348 | 30.348 | 10190 | 0 | 1793 | 352 |
| measured loaded: Promotion tactic | 30.348 | 30.348 | 10190 | 0 | 1793 | 352 |
| measured after search: Promotion tactic | 30.348 | 30.348 | 10190 | 0 | 1793 | 352 |
| measured loaded: Advanced pawns/check evasion | 30.348 | 30.348 | 10190 | 0 | 1793 | 352 |
| measured after search: Advanced pawns/check evasion | 30.348 | 30.348 | 10190 | 0 | 1793 | 352 |
| final process state | 30.348 | 30.348 | 10190 | 0 | 1793 | 352 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.287 | 8227 | 1555952 | b8c6 | 39 | b8c6 d2c4 b7b5 c4a5  |
| Kiwipete | 4 | 1.715 | 6188 | 3607183 | e2a6 | 132 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 4.222 | 11303 | 2677184 | c3d5 | 126 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.787 | 2162 | 2747713 | b4f4 | 220 | b4f4 h4g5 f4f7 h5h2 f7g7 g5f5 g7c7 h2g2  |
| Promotion tactic | 4 | 0.387 | 997 | 2577199 | d7c8q | 588 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.834 | 4651 | 2535487 | c4c5 | -575 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 14.233 | 33528 | 2355664 |

Aggregate elapsed times for all five processes: P1=14.644 ms, P2=13.983 ms, P3=14.233 ms, P4=14.145 ms, P5=14.759 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.273 | 29.273 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.281 | 29.281 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.266 | 30.266 | 2685 | 0 | 570 | 153 |
| warmup loaded: Kiwipete | 30.266 | 30.266 | 2685 | 0 | 570 | 153 |
| warmup after search: Kiwipete | 30.320 | 30.320 | 3751 | 0 | 1092 | 236 |
| warmup loaded: King safety | 30.320 | 30.320 | 3751 | 0 | 1092 | 236 |
| warmup after search: King safety | 30.320 | 30.320 | 6022 | 0 | 1376 | 283 |
| warmup loaded: Endgame | 30.320 | 30.320 | 6022 | 0 | 1376 | 283 |
| warmup after search: Endgame | 30.320 | 30.320 | 6580 | 0 | 1429 | 293 |
| warmup loaded: Promotion tactic | 30.320 | 30.320 | 6580 | 0 | 1429 | 293 |
| warmup after search: Promotion tactic | 30.320 | 30.320 | 6841 | 0 | 1540 | 309 |
| warmup loaded: Advanced pawns/check evasion | 30.320 | 30.320 | 6841 | 0 | 1540 | 309 |
| warmup after search: Advanced pawns/check evasion | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| after complete warmup | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured loaded: Quiet middlegame | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured after search: Quiet middlegame | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured loaded: Kiwipete | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured after search: Kiwipete | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured loaded: King safety | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured after search: King safety | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured loaded: Endgame | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured after search: Endgame | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured loaded: Promotion tactic | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured after search: Promotion tactic | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured loaded: Advanced pawns/check evasion | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| measured after search: Advanced pawns/check evasion | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |
| final process state | 30.344 | 30.344 | 7883 | 0 | 1720 | 337 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.960 | 8754 | 1468752 | b8c6 | 39 | b8c6 d2c4 b7b5 c4a5  |
| Kiwipete | 4 | 1.664 | 6188 | 3717976 | e2a6 | 132 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 3.952 | 10455 | 2645589 | c3d5 | 126 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.766 | 2162 | 2822255 | b4f4 | 220 | b4f4 h4g5 f4f7 h5h2 f7g7 g5f5 g7c7 h2g2  |
| Promotion tactic | 4 | 0.378 | 997 | 2640115 | d7c8q | 588 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.812 | 4651 | 2567015 | c4c5 | -575 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 14.532 | 33207 | 2285112 |

Aggregate elapsed times for all five processes: P1=14.532 ms, P2=14.852 ms, P3=14.339 ms, P4=14.492 ms, P5=14.541 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.289 | 29.289 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.297 | 29.297 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.289 | 30.289 | 2884 | 0 | 566 | 151 |
| warmup loaded: Kiwipete | 30.289 | 30.289 | 2884 | 0 | 566 | 151 |
| warmup after search: Kiwipete | 30.344 | 30.344 | 3950 | 0 | 1088 | 233 |
| warmup loaded: King safety | 30.344 | 30.344 | 3950 | 0 | 1088 | 233 |
| warmup after search: King safety | 30.344 | 30.344 | 6442 | 0 | 1386 | 281 |
| warmup loaded: Endgame | 30.344 | 30.344 | 6442 | 0 | 1386 | 281 |
| warmup after search: Endgame | 30.344 | 30.344 | 7000 | 0 | 1439 | 291 |
| warmup loaded: Promotion tactic | 30.344 | 30.344 | 7000 | 0 | 1439 | 291 |
| warmup after search: Promotion tactic | 30.344 | 30.344 | 7261 | 0 | 1550 | 307 |
| warmup loaded: Advanced pawns/check evasion | 30.344 | 30.344 | 7261 | 0 | 1550 | 307 |
| warmup after search: Advanced pawns/check evasion | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| after complete warmup | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured loaded: Quiet middlegame | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured after search: Quiet middlegame | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured loaded: Kiwipete | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured after search: Kiwipete | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured loaded: King safety | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured after search: King safety | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured loaded: Endgame | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured after search: Endgame | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured loaded: Promotion tactic | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured after search: Promotion tactic | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured loaded: Advanced pawns/check evasion | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| measured after search: Advanced pawns/check evasion | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |
| final process state | 30.367 | 30.367 | 8303 | 0 | 1734 | 335 |

## phase1e

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.265 | 8219 | 1560969 | b8c6 | 42 | b8c6 d2b3 c8e6 c1g5  |
| Kiwipete | 4 | 4.121 | 13161 | 3193848 | e2a6 | 80 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 5.045 | 14515 | 2877216 | c3d5 | 116 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.452 | 1392 | 3080961 | b4f4 | 154 | b4f4 h4g3 f4f7 c7c6 f7f5  |
| Promotion tactic | 4 | 0.559 | 1543 | 2759634 | d7c8q | 653 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 4.151 | 13557 | 3265846 | c4c5 | -612 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 19.593 | 52387 | 2673768 |

Aggregate elapsed times for all five processes: P1=19.593 ms, P2=19.907 ms, P3=19.697 ms, P4=19.449 ms, P5=19.425 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.316 | 29.320 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.324 | 29.328 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.328 | 30.328 | 2907 | 0 | 578 | 149 |
| warmup loaded: Kiwipete | 30.328 | 30.328 | 2907 | 0 | 578 | 149 |
| warmup after search: Kiwipete | 30.438 | 30.438 | 4904 | 0 | 1458 | 287 |
| warmup loaded: King safety | 30.438 | 30.438 | 4904 | 0 | 1458 | 287 |
| warmup after search: King safety | 30.465 | 30.465 | 8014 | 0 | 1725 | 336 |
| warmup loaded: Endgame | 30.465 | 30.465 | 8014 | 0 | 1725 | 336 |
| warmup after search: Endgame | 30.465 | 30.465 | 8348 | 0 | 1770 | 347 |
| warmup loaded: Promotion tactic | 30.465 | 30.465 | 8348 | 0 | 1770 | 347 |
| warmup after search: Promotion tactic | 30.465 | 30.465 | 8693 | 0 | 1880 | 361 |
| warmup loaded: Advanced pawns/check evasion | 30.465 | 30.465 | 8693 | 0 | 1880 | 361 |
| warmup after search: Advanced pawns/check evasion | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| after complete warmup | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured loaded: Quiet middlegame | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured after search: Quiet middlegame | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured loaded: Kiwipete | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured after search: Kiwipete | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured loaded: King safety | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured after search: King safety | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured loaded: Endgame | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured after search: Endgame | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured loaded: Promotion tactic | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured after search: Promotion tactic | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured loaded: Advanced pawns/check evasion | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| measured after search: Advanced pawns/check evasion | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |
| final process state | 30.516 | 30.516 | 10447 | 0 | 2181 | 419 |

## Phase 1E readiness baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.133 | 8227 | 1602907 | b8c6 | 39 | b8c6 d2c4 b7b5 c4a5  |
| Kiwipete | 4 | 1.665 | 6188 | 3716786 | e2a6 | 132 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 4.059 | 11303 | 2784430 | c3d5 | 126 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.760 | 2162 | 2846587 | b4f4 | 220 | b4f4 h4g5 f4f7 h5h2 f7g7 g5f5 g7c7 h2g2  |
| Promotion tactic | 4 | 0.376 | 997 | 2655098 | d7c8q | 588 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.805 | 4651 | 2576373 | c4c5 | -575 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 13.797 | 33528 | 2430085 |

Aggregate elapsed times for all five processes: P1=13.782 ms, P2=13.820 ms, P3=13.797 ms, P4=13.798 ms, P5=13.681 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.285 | 29.285 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.293 | 29.293 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.273 | 30.273 | 2685 | 0 | 570 | 153 |
| warmup loaded: Kiwipete | 30.273 | 30.273 | 2685 | 0 | 570 | 153 |
| warmup after search: Kiwipete | 30.328 | 30.328 | 3751 | 0 | 1092 | 236 |
| warmup loaded: King safety | 30.328 | 30.328 | 3751 | 0 | 1092 | 236 |
| warmup after search: King safety | 30.328 | 30.328 | 6022 | 0 | 1376 | 283 |
| warmup loaded: Endgame | 30.328 | 30.328 | 6022 | 0 | 1376 | 283 |
| warmup after search: Endgame | 30.328 | 30.328 | 6580 | 0 | 1429 | 293 |
| warmup loaded: Promotion tactic | 30.328 | 30.328 | 6580 | 0 | 1429 | 293 |
| warmup after search: Promotion tactic | 30.328 | 30.328 | 6841 | 0 | 1540 | 309 |
| warmup loaded: Advanced pawns/check evasion | 30.328 | 30.328 | 6841 | 0 | 1540 | 309 |
| warmup after search: Advanced pawns/check evasion | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| after complete warmup | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured loaded: Quiet middlegame | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured after search: Quiet middlegame | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured loaded: Kiwipete | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured after search: Kiwipete | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured loaded: King safety | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured after search: King safety | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured loaded: Endgame | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured after search: Endgame | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured loaded: Promotion tactic | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured after search: Promotion tactic | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured loaded: Advanced pawns/check evasion | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| measured after search: Advanced pawns/check evasion | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |
| final process state | 30.352 | 30.352 | 7883 | 0 | 1720 | 337 |

## Central defensive readiness candidate

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.204 | 8243 | 1584039 | b8c6 | 39 | b8c6 d2c4 b7b5 c4a5  |
| Kiwipete | 4 | 1.501 | 5745 | 3827468 | e2a6 | 149 | e2a6 b4c3 d2c3 e6d5 a6c8  |
| King safety | 4 | 4.047 | 11303 | 2792659 | c3d5 | 126 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.754 | 2162 | 2866640 | b4f4 | 220 | b4f4 h4g5 f4f7 h5h2 f7g7 g5f5 g7c7 h2g2  |
| Promotion tactic | 4 | 0.326 | 843 | 2588223 | d7c8q | 588 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.748 | 4561 | 2609275 | c4c5 | -560 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 13.580 | 32857 | 2419502 |

Aggregate elapsed times for all five processes: P1=13.580 ms, P2=13.768 ms, P3=14.102 ms, P4=13.501 ms, P5=13.573 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.289 | 29.289 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.297 | 29.297 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.281 | 30.281 | 2689 | 0 | 570 | 154 |
| warmup loaded: Kiwipete | 30.281 | 30.281 | 2689 | 0 | 570 | 154 |
| warmup after search: Kiwipete | 30.336 | 30.336 | 3654 | 0 | 1047 | 231 |
| warmup loaded: King safety | 30.336 | 30.336 | 3654 | 0 | 1047 | 231 |
| warmup after search: King safety | 30.336 | 30.336 | 5925 | 0 | 1334 | 279 |
| warmup loaded: Endgame | 30.336 | 30.336 | 5925 | 0 | 1334 | 279 |
| warmup after search: Endgame | 30.336 | 30.336 | 6483 | 0 | 1387 | 289 |
| warmup loaded: Promotion tactic | 30.336 | 30.336 | 6483 | 0 | 1387 | 289 |
| warmup after search: Promotion tactic | 30.336 | 30.336 | 6722 | 0 | 1490 | 302 |
| warmup loaded: Advanced pawns/check evasion | 30.336 | 30.336 | 6722 | 0 | 1490 | 302 |
| warmup after search: Advanced pawns/check evasion | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| after complete warmup | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured loaded: Quiet middlegame | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured after search: Quiet middlegame | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured loaded: Kiwipete | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured after search: Kiwipete | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured loaded: King safety | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured after search: King safety | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured loaded: Endgame | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured after search: Endgame | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured loaded: Promotion tactic | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured after search: Promotion tactic | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured loaded: Advanced pawns/check evasion | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| measured after search: Advanced pawns/check evasion | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |
| final process state | 30.355 | 30.355 | 7748 | 0 | 1665 | 330 |

## Central king pressure candidate

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.372 | 8230 | 1531898 | b8c6 | 39 | b8c6 d2c4 b7b5 c4a5  |
| Kiwipete | 4 | 1.738 | 6201 | 3567212 | e2a6 | 132 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 4.212 | 11303 | 2683469 | c3d5 | 126 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.776 | 2162 | 2786211 | b4f4 | 220 | b4f4 h4g5 f4f7 h5h2 f7g7 g5f5 g7c7 h2g2  |
| Promotion tactic | 4 | 0.395 | 997 | 2526673 | d7c8q | 596 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.962 | 4651 | 2370975 | c4c5 | -574 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 14.455 | 33544 | 2320576 |

Aggregate elapsed times for all five processes: P1=14.494 ms, P2=14.455 ms, P3=14.597 ms, P4=14.341 ms, P5=14.051 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.277 | 29.277 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.285 | 29.285 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.270 | 30.270 | 2686 | 0 | 570 | 154 |
| warmup loaded: Kiwipete | 30.270 | 30.270 | 2686 | 0 | 570 | 154 |
| warmup after search: Kiwipete | 30.324 | 30.324 | 3777 | 0 | 1093 | 237 |
| warmup loaded: King safety | 30.324 | 30.324 | 3777 | 0 | 1093 | 237 |
| warmup after search: King safety | 30.324 | 30.324 | 6048 | 0 | 1377 | 284 |
| warmup loaded: Endgame | 30.324 | 30.324 | 6048 | 0 | 1377 | 284 |
| warmup after search: Endgame | 30.324 | 30.324 | 6606 | 0 | 1430 | 294 |
| warmup loaded: Promotion tactic | 30.324 | 30.324 | 6606 | 0 | 1430 | 294 |
| warmup after search: Promotion tactic | 30.324 | 30.324 | 6867 | 0 | 1541 | 309 |
| warmup loaded: Advanced pawns/check evasion | 30.324 | 30.324 | 6867 | 0 | 1541 | 309 |
| warmup after search: Advanced pawns/check evasion | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| after complete warmup | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured loaded: Quiet middlegame | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured after search: Quiet middlegame | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured loaded: Kiwipete | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured after search: Kiwipete | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured loaded: King safety | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured after search: King safety | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured loaded: Endgame | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured after search: Endgame | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured loaded: Promotion tactic | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured after search: Promotion tactic | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured loaded: Advanced pawns/check evasion | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| measured after search: Advanced pawns/check evasion | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |
| final process state | 30.348 | 30.348 | 7909 | 0 | 1721 | 337 |

## Central king attack pressure candidate

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.238 | 8230 | 1571259 | b8c6 | 39 | b8c6 d2c4 b7b5 c4a5  |
| Kiwipete | 4 | 1.685 | 6214 | 3688705 | e2a6 | 132 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 4.079 | 11303 | 2770980 | c3d5 | 126 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.769 | 2162 | 2813239 | b4f4 | 220 | b4f4 h4g5 f4f7 h5h2 f7g7 g5f5 g7c7 h2g2  |
| Promotion tactic | 4 | 0.379 | 1030 | 2721088 | d7c8q | 593 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.799 | 4632 | 2574244 | c4c5 | -571 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 13.948 | 33571 | 2406886 |

Aggregate elapsed times for all five processes: P1=14.204 ms, P2=14.199 ms, P3=13.948 ms, P4=13.873 ms, P5=13.786 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.262 | 29.262 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.270 | 29.270 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.254 | 30.254 | 2686 | 0 | 570 | 154 |
| warmup loaded: Kiwipete | 30.254 | 30.254 | 2686 | 0 | 570 | 154 |
| warmup after search: Kiwipete | 30.309 | 30.309 | 3777 | 0 | 1093 | 237 |
| warmup loaded: King safety | 30.309 | 30.309 | 3777 | 0 | 1093 | 237 |
| warmup after search: King safety | 30.309 | 30.309 | 6048 | 0 | 1377 | 284 |
| warmup loaded: Endgame | 30.309 | 30.309 | 6048 | 0 | 1377 | 284 |
| warmup after search: Endgame | 30.309 | 30.309 | 6606 | 0 | 1430 | 294 |
| warmup loaded: Promotion tactic | 30.309 | 30.309 | 6606 | 0 | 1430 | 294 |
| warmup after search: Promotion tactic | 30.309 | 30.309 | 6871 | 0 | 1541 | 309 |
| warmup loaded: Advanced pawns/check evasion | 30.309 | 30.309 | 6871 | 0 | 1541 | 309 |
| warmup after search: Advanced pawns/check evasion | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| after complete warmup | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured loaded: Quiet middlegame | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured after search: Quiet middlegame | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured loaded: Kiwipete | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured after search: Kiwipete | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured loaded: King safety | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured after search: King safety | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured loaded: Endgame | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured after search: Endgame | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured loaded: Promotion tactic | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured after search: Promotion tactic | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured loaded: Advanced pawns/check evasion | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| measured after search: Advanced pawns/check evasion | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |
| final process state | 30.332 | 30.332 | 7910 | 0 | 1721 | 337 |

## Central king attack geometry candidate

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.276 | 8230 | 1559764 | b8c6 | 39 | b8c6 d2c4 b7b5 c4a5  |
| Kiwipete | 4 | 1.697 | 6201 | 3653150 | e2a6 | 132 | e2a6 b4c3 d2e3 b6c4  |
| King safety | 4 | 4.156 | 11303 | 2719635 | c3d5 | 126 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.773 | 2162 | 2797220 | b4f4 | 220 | b4f4 h4g5 f4f7 h5h2 f7g7 g5f5 g7c7 h2g2  |
| Promotion tactic | 4 | 0.382 | 997 | 2613162 | d7c8q | 598 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.811 | 4641 | 2562507 | c4c5 | -572 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 14.096 | 33534 | 2379056 |

Aggregate elapsed times for all five processes: P1=14.096 ms, P2=14.193 ms, P3=14.313 ms, P4=13.970 ms, P5=13.974 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.297 | 29.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.305 | 29.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.289 | 30.289 | 2686 | 0 | 570 | 154 |
| warmup loaded: Kiwipete | 30.289 | 30.289 | 2686 | 0 | 570 | 154 |
| warmup after search: Kiwipete | 30.344 | 30.344 | 3777 | 0 | 1093 | 237 |
| warmup loaded: King safety | 30.344 | 30.344 | 3777 | 0 | 1093 | 237 |
| warmup after search: King safety | 30.344 | 30.344 | 6048 | 0 | 1377 | 284 |
| warmup loaded: Endgame | 30.344 | 30.344 | 6048 | 0 | 1377 | 284 |
| warmup after search: Endgame | 30.344 | 30.344 | 6606 | 0 | 1430 | 294 |
| warmup loaded: Promotion tactic | 30.344 | 30.344 | 6606 | 0 | 1430 | 294 |
| warmup after search: Promotion tactic | 30.344 | 30.344 | 6867 | 0 | 1541 | 309 |
| warmup loaded: Advanced pawns/check evasion | 30.344 | 30.344 | 6867 | 0 | 1541 | 309 |
| warmup after search: Advanced pawns/check evasion | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| after complete warmup | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured loaded: Quiet middlegame | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured after search: Quiet middlegame | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured loaded: Kiwipete | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured after search: Kiwipete | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured loaded: King safety | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured after search: King safety | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured loaded: Endgame | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured after search: Endgame | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured loaded: Promotion tactic | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured after search: Promotion tactic | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured loaded: Advanced pawns/check evasion | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| measured after search: Advanced pawns/check evasion | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |
| final process state | 30.367 | 30.367 | 7907 | 0 | 1721 | 337 |

## Central king attack pressure 2x

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.369 | 8230 | 1532879 | b8c6 | 39 | b8c6 d2c4 b7b5 c4a5  |
| Kiwipete | 4 | 1.745 | 6149 | 3523150 | e2a6 | 134 | e2a6 b4c3 d2c3 e6d5 a6c8  |
| King safety | 4 | 4.254 | 11303 | 2656926 | c3d5 | 126 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.787 | 2162 | 2746097 | b4f4 | 220 | b4f4 h4g5 f4f7 h5h2 f7g7 g5f5 g7c7 h2g2  |
| Promotion tactic | 4 | 0.395 | 997 | 2521172 | d7c8q | 608 | d7c8q d8c8 e1f2 c8g4  |
| Advanced pawns/check evasion | 5 | 1.832 | 4630 | 2527757 | c4c5 | -569 | c4c5 b6c5 b4c5 a3c5 d2d4 b2a1q d1a1 c5a7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 14.383 | 33471 | 2327143 |

Aggregate elapsed times for all five processes: P1=14.383 ms, P2=14.930 ms, P3=14.496 ms, P4=14.014 ms, P5=13.804 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.266 | 29.266 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.273 | 29.273 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.258 | 30.258 | 2686 | 0 | 570 | 154 |
| warmup loaded: Kiwipete | 30.258 | 30.258 | 2686 | 0 | 570 | 154 |
| warmup after search: Kiwipete | 30.312 | 30.312 | 3774 | 0 | 1093 | 237 |
| warmup loaded: King safety | 30.312 | 30.312 | 3774 | 0 | 1093 | 237 |
| warmup after search: King safety | 30.312 | 30.312 | 6045 | 0 | 1377 | 284 |
| warmup loaded: Endgame | 30.312 | 30.312 | 6045 | 0 | 1377 | 284 |
| warmup after search: Endgame | 30.312 | 30.312 | 6603 | 0 | 1430 | 294 |
| warmup loaded: Promotion tactic | 30.312 | 30.312 | 6603 | 0 | 1430 | 294 |
| warmup after search: Promotion tactic | 30.312 | 30.312 | 6864 | 0 | 1541 | 309 |
| warmup loaded: Advanced pawns/check evasion | 30.312 | 30.312 | 6864 | 0 | 1541 | 309 |
| warmup after search: Advanced pawns/check evasion | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| after complete warmup | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured loaded: Quiet middlegame | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured after search: Quiet middlegame | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured loaded: Kiwipete | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured after search: Kiwipete | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured loaded: King safety | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured after search: King safety | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured loaded: Endgame | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured after search: Endgame | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured loaded: Promotion tactic | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured after search: Promotion tactic | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured loaded: Advanced pawns/check evasion | 30.332 | 30.332 | 7902 | 0 | 1721 | 337 |
| measured after search: Advanced pawns/check evasion | 30.336 | 30.336 | 7902 | 0 | 1721 | 337 |
| final process state | 30.336 | 30.336 | 7902 | 0 | 1721 | 337 |

## Baseline benchmark

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 7.172 | 10011 | 1395784 | b8c6 | 33 | b8c6 c2c3 d6d5 d2b3  |
| Kiwipete | 4 | 8.702 | 27020 | 3105097 | e2a6 | 3 | e2a6 b4c3 d2c3 h3g2 a6c8  |
| King safety | 4 | 4.956 | 13712 | 2766583 | c3d5 | 131 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.859 | 2367 | 2755731 | b4f4 | 114 | b4f4 h4g3 f4c4 g3g2 c4c7 h5e5  |
| Promotion tactic | 4 | 0.517 | 1531 | 2961584 | d7c8q | 677 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 4.489 | 11689 | 2604155 | g1h1 | -527 | g1h1 b2a1q b4a3 a1d1 a4d1 g7h6 a3e7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 26.695 | 66330 | 2484743 |

Aggregate elapsed times for all five processes: P1=27.303 ms, P2=26.695 ms, P3=26.757 ms, P4=26.658 ms, P5=26.685 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.258 | 29.262 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.266 | 29.270 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.227 | 30.230 | 3468 | 0 | 591 | 158 |
| warmup loaded: Kiwipete | 30.230 | 30.230 | 3468 | 0 | 591 | 158 |
| warmup after search: Kiwipete | 30.418 | 30.418 | 6688 | 0 | 1680 | 339 |
| warmup loaded: King safety | 30.418 | 30.418 | 6688 | 0 | 1680 | 339 |
| warmup after search: King safety | 30.418 | 30.418 | 9354 | 0 | 1912 | 382 |
| warmup loaded: Endgame | 30.418 | 30.418 | 9354 | 0 | 1912 | 382 |
| warmup after search: Endgame | 30.418 | 30.418 | 9998 | 0 | 1957 | 393 |
| warmup loaded: Promotion tactic | 30.418 | 30.418 | 9998 | 0 | 1957 | 393 |
| warmup after search: Promotion tactic | 30.418 | 30.418 | 10320 | 0 | 2060 | 408 |
| warmup loaded: Advanced pawns/check evasion | 30.418 | 30.418 | 10320 | 0 | 2060 | 408 |
| warmup after search: Advanced pawns/check evasion | 30.422 | 30.426 | 12512 | 0 | 2349 | 451 |
| after complete warmup | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured loaded: Quiet middlegame | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured after search: Quiet middlegame | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured loaded: Kiwipete | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured after search: Kiwipete | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured loaded: King safety | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured after search: King safety | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured loaded: Endgame | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured after search: Endgame | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured loaded: Promotion tactic | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured after search: Promotion tactic | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured loaded: Advanced pawns/check evasion | 30.426 | 30.426 | 12512 | 0 | 2349 | 451 |
| measured after search: Advanced pawns/check evasion | 30.430 | 30.430 | 12512 | 0 | 2349 | 451 |
| final process state | 30.430 | 30.430 | 12512 | 0 | 2349 | 451 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 7.573 | 10011 | 1321955 | b8c6 | 33 | b8c6 c2c3 d6d5 d2b3  |
| Kiwipete | 4 | 8.948 | 27020 | 3019721 | e2a6 | 3 | e2a6 b4c3 d2c3 h3g2 a6c8  |
| King safety | 4 | 5.098 | 13712 | 2689862 | c3d5 | 131 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.879 | 2367 | 2692955 | b4f4 | 114 | b4f4 h4g3 f4c4 g3g2 c4c7 h5e5  |
| Promotion tactic | 4 | 0.535 | 1531 | 2860730 | d7c8q | 677 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 4.617 | 11689 | 2531486 | g1h1 | -527 | g1h1 b2a1q b4a3 a1d1 a4d1 g7h6 a3e7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 27.650 | 66330 | 2398918 |

Aggregate elapsed times for all five processes: P1=27.657 ms, P2=28.185 ms, P3=27.650 ms, P4=27.509 ms, P5=26.835 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.215 | 29.219 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.223 | 29.227 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.188 | 30.191 | 3468 | 0 | 591 | 158 |
| warmup loaded: Kiwipete | 30.191 | 30.191 | 3468 | 0 | 591 | 158 |
| warmup after search: Kiwipete | 30.375 | 30.375 | 6688 | 0 | 1680 | 339 |
| warmup loaded: King safety | 30.375 | 30.375 | 6688 | 0 | 1680 | 339 |
| warmup after search: King safety | 30.375 | 30.375 | 9354 | 0 | 1912 | 382 |
| warmup loaded: Endgame | 30.375 | 30.375 | 9354 | 0 | 1912 | 382 |
| warmup after search: Endgame | 30.375 | 30.375 | 9998 | 0 | 1957 | 393 |
| warmup loaded: Promotion tactic | 30.375 | 30.375 | 9998 | 0 | 1957 | 393 |
| warmup after search: Promotion tactic | 30.375 | 30.375 | 10320 | 0 | 2060 | 408 |
| warmup loaded: Advanced pawns/check evasion | 30.375 | 30.375 | 10320 | 0 | 2060 | 408 |
| warmup after search: Advanced pawns/check evasion | 30.379 | 30.383 | 12512 | 0 | 2349 | 451 |
| after complete warmup | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured loaded: Quiet middlegame | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured after search: Quiet middlegame | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured loaded: Kiwipete | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured after search: Kiwipete | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured loaded: King safety | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured after search: King safety | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured loaded: Endgame | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured after search: Endgame | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured loaded: Promotion tactic | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured after search: Promotion tactic | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured loaded: Advanced pawns/check evasion | 30.383 | 30.383 | 12512 | 0 | 2349 | 451 |
| measured after search: Advanced pawns/check evasion | 30.387 | 30.387 | 12512 | 0 | 2349 | 451 |
| final process state | 30.387 | 30.387 | 12512 | 0 | 2349 | 451 |

## stage1

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 7.107 | 10011 | 1408690 | b8c6 | 33 | b8c6 c2c3 d6d5 d2b3  |
| Kiwipete | 4 | 8.458 | 27020 | 3194457 | e2a6 | 3 | e2a6 b4c3 d2c3 h3g2 a6c8  |
| King safety | 4 | 4.822 | 13712 | 2843362 | c3d5 | 131 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.845 | 2367 | 2799944 | b4f4 | 114 | b4f4 h4g3 f4c4 g3g2 c4c7 h5e5  |
| Promotion tactic | 4 | 0.503 | 1531 | 3045850 | d7c8q | 677 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 4.488 | 11689 | 2604431 | g1h1 | -527 | g1h1 b2a1q b4a3 a1d1 a4d1 g7h6 a3e7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 26.224 | 66330 | 2529400 |

Aggregate elapsed times for all five processes: P1=26.910 ms, P2=26.224 ms, P3=26.148 ms, P4=25.871 ms, P5=26.333 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.266 | 29.270 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.273 | 29.277 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.238 | 30.238 | 3468 | 0 | 591 | 158 |
| warmup loaded: Kiwipete | 30.238 | 30.238 | 3468 | 0 | 591 | 158 |
| warmup after search: Kiwipete | 30.430 | 30.430 | 6688 | 0 | 1680 | 339 |
| warmup loaded: King safety | 30.430 | 30.430 | 6688 | 0 | 1680 | 339 |
| warmup after search: King safety | 30.430 | 30.430 | 9354 | 0 | 1912 | 382 |
| warmup loaded: Endgame | 30.430 | 30.430 | 9354 | 0 | 1912 | 382 |
| warmup after search: Endgame | 30.430 | 30.430 | 9998 | 0 | 1957 | 393 |
| warmup loaded: Promotion tactic | 30.430 | 30.430 | 9998 | 0 | 1957 | 393 |
| warmup after search: Promotion tactic | 30.430 | 30.430 | 10320 | 0 | 2060 | 408 |
| warmup loaded: Advanced pawns/check evasion | 30.430 | 30.430 | 10320 | 0 | 2060 | 408 |
| warmup after search: Advanced pawns/check evasion | 30.430 | 30.434 | 12512 | 0 | 2349 | 451 |
| after complete warmup | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured loaded: Quiet middlegame | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured after search: Quiet middlegame | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured loaded: Kiwipete | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured after search: Kiwipete | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured loaded: King safety | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured after search: King safety | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured loaded: Endgame | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured after search: Endgame | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured loaded: Promotion tactic | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured after search: Promotion tactic | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured loaded: Advanced pawns/check evasion | 30.434 | 30.434 | 12512 | 0 | 2349 | 451 |
| measured after search: Advanced pawns/check evasion | 30.438 | 30.438 | 12512 | 0 | 2349 | 451 |
| final process state | 30.438 | 30.438 | 12512 | 0 | 2349 | 451 |

## stage2

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 7.277 | 10011 | 1375671 | b8c6 | 33 | b8c6 c2c3 d6d5 d2b3  |
| Kiwipete | 4 | 8.852 | 27020 | 3052573 | e2a6 | 3 | e2a6 b4c3 d2c3 h3g2 a6c8  |
| King safety | 4 | 4.965 | 13712 | 2761548 | c3d5 | 131 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 0.852 | 2367 | 2779718 | b4f4 | 114 | b4f4 h4g3 f4c4 g3g2 c4c7 h5e5  |
| Promotion tactic | 4 | 0.511 | 1531 | 2998973 | d7c8q | 677 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 4.596 | 11689 | 2543154 | g1h1 | -527 | g1h1 b2a1q b4a3 a1d1 a4d1 g7h6 a3e7  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 27.052 | 66330 | 2451913 |

Aggregate elapsed times for all five processes: P1=27.349 ms, P2=26.638 ms, P3=27.447 ms, P4=26.712 ms, P5=27.052 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.352 | 29.355 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.359 | 29.363 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.320 | 30.320 | 3468 | 0 | 591 | 158 |
| warmup loaded: Kiwipete | 30.320 | 30.320 | 3468 | 0 | 591 | 158 |
| warmup after search: Kiwipete | 30.512 | 30.512 | 6688 | 0 | 1680 | 339 |
| warmup loaded: King safety | 30.512 | 30.512 | 6688 | 0 | 1680 | 339 |
| warmup after search: King safety | 30.512 | 30.512 | 9354 | 0 | 1912 | 382 |
| warmup loaded: Endgame | 30.512 | 30.512 | 9354 | 0 | 1912 | 382 |
| warmup after search: Endgame | 30.512 | 30.512 | 9998 | 0 | 1957 | 393 |
| warmup loaded: Promotion tactic | 30.512 | 30.512 | 9998 | 0 | 1957 | 393 |
| warmup after search: Promotion tactic | 30.512 | 30.512 | 10320 | 0 | 2060 | 408 |
| warmup loaded: Advanced pawns/check evasion | 30.512 | 30.512 | 10320 | 0 | 2060 | 408 |
| warmup after search: Advanced pawns/check evasion | 30.516 | 30.520 | 12512 | 0 | 2349 | 451 |
| after complete warmup | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured loaded: Quiet middlegame | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured after search: Quiet middlegame | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured loaded: Kiwipete | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured after search: Kiwipete | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured loaded: King safety | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured after search: King safety | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured loaded: Endgame | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured after search: Endgame | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured loaded: Promotion tactic | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured after search: Promotion tactic | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured loaded: Advanced pawns/check evasion | 30.520 | 30.520 | 12512 | 0 | 2349 | 451 |
| measured after search: Advanced pawns/check evasion | 30.523 | 30.523 | 12512 | 0 | 2349 | 451 |
| final process state | 30.523 | 30.523 | 12512 | 0 | 2349 | 451 |

## king commitment

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.660 | 8144 | 1438934 | c7c5 | 38 | c7c5 h2h3 b7b5 d2b3  |
| Kiwipete | 4 | 7.269 | 22201 | 3054239 | e2a6 | 84 | e2a6 e6d5 c3d5 e7e5  |
| King safety | 4 | 8.980 | 27678 | 3082240 | c3d5 | 38 | c3d5 e7d8 h2h3 h7h6  |
| Endgame | 5 | 0.778 | 2215 | 2848812 | b4f4 | 114 | b4f4 h4g3 f4c4 h5e5 c4c7 g3g2  |
| Promotion tactic | 4 | 0.816 | 3374 | 4134864 | d7c8q | 725 | d7c8q f2d1 c8d8 e7d8 e1d1  |
| Advanced pawns/check evasion | 5 | 2.849 | 7118 | 2498137 | g1h1 | -532 | g1h1 b2a1q b4a3 a1d1 a4d1 g7h6  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 26.351 | 70730 | 2684116 |

Aggregate elapsed times for all five processes: P1=27.341 ms, P2=26.628 ms, P3=26.351 ms, P4=26.124 ms, P5=26.045 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.375 | 29.379 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.383 | 29.387 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.348 | 30.348 | 3082 | 0 | 531 | 150 |
| warmup loaded: Kiwipete | 30.348 | 30.348 | 3082 | 0 | 531 | 150 |
| warmup after search: Kiwipete | 30.461 | 30.461 | 6233 | 0 | 1586 | 300 |
| warmup loaded: King safety | 30.461 | 30.461 | 6233 | 0 | 1586 | 300 |
| warmup after search: King safety | 30.461 | 30.461 | 10157 | 0 | 1867 | 362 |
| warmup loaded: Endgame | 30.461 | 30.461 | 10157 | 0 | 1867 | 362 |
| warmup after search: Endgame | 30.461 | 30.461 | 10715 | 0 | 1913 | 373 |
| warmup loaded: Promotion tactic | 30.461 | 30.461 | 10715 | 0 | 1913 | 373 |
| warmup after search: Promotion tactic | 30.461 | 30.461 | 11208 | 0 | 2067 | 393 |
| warmup loaded: Advanced pawns/check evasion | 30.461 | 30.461 | 11208 | 0 | 2067 | 393 |
| warmup after search: Advanced pawns/check evasion | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| after complete warmup | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured loaded: Quiet middlegame | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured after search: Quiet middlegame | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured loaded: Kiwipete | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured after search: Kiwipete | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured loaded: King safety | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured after search: King safety | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured loaded: Endgame | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured after search: Endgame | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured loaded: Promotion tactic | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured after search: Promotion tactic | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured loaded: Advanced pawns/check evasion | 30.531 | 30.531 | 12771 | 0 | 2242 | 414 |
| measured after search: Advanced pawns/check evasion | 30.539 | 30.539 | 12771 | 0 | 2242 | 414 |
| final process state | 30.539 | 30.539 | 12771 | 0 | 2242 | 414 |

## after revert

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.677 | 8145 | 1434771 | c7c5 | 38 | c7c5 h2h3 b7b5 d2b3  |
| Kiwipete | 4 | 6.929 | 22393 | 3231748 | e2a6 | 47 | e2a6 b4c3 d2c3 h3g2 f3g2 e6d5  |
| King safety | 4 | 8.845 | 27678 | 3129379 | c3d5 | 38 | c3d5 e7d8 h2h3 h7h6  |
| Endgame | 5 | 0.753 | 2215 | 2942766 | b4f4 | 114 | b4f4 h4g3 f4c4 h5e5 c4c7 g3g2  |
| Promotion tactic | 4 | 0.720 | 3195 | 4435768 | d7c8q | 677 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 2.514 | 6152 | 2447074 | g1h1 | -532 | g1h1 b2a1q b4a3 a1d1 a4d1 g7h6  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 25.437 | 69778 | 2743116 |

Aggregate elapsed times for all five processes: P1=25.437 ms, P2=24.702 ms, P3=25.257 ms, P4=25.630 ms, P5=25.512 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.367 | 29.371 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.375 | 29.379 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.344 | 30.344 | 3083 | 0 | 531 | 150 |
| warmup loaded: Kiwipete | 30.344 | 30.344 | 3083 | 0 | 531 | 150 |
| warmup after search: Kiwipete | 30.523 | 30.523 | 5795 | 0 | 1537 | 320 |
| warmup loaded: King safety | 30.523 | 30.523 | 5795 | 0 | 1537 | 320 |
| warmup after search: King safety | 30.523 | 30.523 | 9719 | 0 | 1838 | 383 |
| warmup loaded: Endgame | 30.523 | 30.523 | 9719 | 0 | 1838 | 383 |
| warmup after search: Endgame | 30.523 | 30.523 | 10277 | 0 | 1885 | 394 |
| warmup loaded: Promotion tactic | 30.523 | 30.523 | 10277 | 0 | 1885 | 394 |
| warmup after search: Promotion tactic | 30.523 | 30.523 | 10662 | 0 | 2038 | 411 |
| warmup loaded: Advanced pawns/check evasion | 30.523 | 30.523 | 10662 | 0 | 2038 | 411 |
| warmup after search: Advanced pawns/check evasion | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| after complete warmup | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured loaded: Quiet middlegame | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured after search: Quiet middlegame | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured loaded: Kiwipete | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured after search: Kiwipete | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured loaded: King safety | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured after search: King safety | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured loaded: Endgame | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured after search: Endgame | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured loaded: Promotion tactic | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured after search: Promotion tactic | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured loaded: Advanced pawns/check evasion | 30.523 | 30.523 | 12180 | 0 | 2194 | 430 |
| measured after search: Advanced pawns/check evasion | 30.531 | 30.531 | 12180 | 0 | 2194 | 430 |
| final process state | 30.531 | 30.531 | 12180 | 0 | 2194 | 430 |

## static evaluator correction

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.523 | 8174 | 1479995 | c7c5 | 38 | c7c5 h2h3 b7b5 d2b3  |
| Kiwipete | 4 | 6.265 | 20439 | 3262299 | e2a6 | 66 | e2a6 b4c3 d2c3 h3g2  |
| King safety | 4 | 8.840 | 27504 | 3111344 | c3d5 | 38 | c3d5 e7d8 h2h3 h7h6  |
| Endgame | 5 | 0.796 | 2215 | 2781594 | b4f4 | 114 | b4f4 h4g3 f4c4 h5e5 c4c7 g3g2  |
| Promotion tactic | 4 | 0.738 | 3182 | 4308973 | d7c8q | 690 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 2.476 | 6094 | 2461028 | g1h1 | -529 | g1h1 b2a1q b4a3 a1d1 a4d1 g7h6  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 24.639 | 67608 | 2743934 |

Aggregate elapsed times for all five processes: P1=24.791 ms, P2=24.639 ms, P3=24.548 ms, P4=24.551 ms, P5=24.666 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.395 | 29.398 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.402 | 29.406 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.367 | 30.367 | 2957 | 0 | 551 | 153 |
| warmup loaded: Kiwipete | 30.367 | 30.367 | 2957 | 0 | 551 | 153 |
| warmup after search: Kiwipete | 30.547 | 30.547 | 5436 | 0 | 1502 | 308 |
| warmup loaded: King safety | 30.547 | 30.547 | 5436 | 0 | 1502 | 308 |
| warmup after search: King safety | 30.547 | 30.547 | 9305 | 0 | 1803 | 371 |
| warmup loaded: Endgame | 30.547 | 30.547 | 9305 | 0 | 1803 | 371 |
| warmup after search: Endgame | 30.547 | 30.547 | 9863 | 0 | 1851 | 382 |
| warmup loaded: Promotion tactic | 30.547 | 30.547 | 9863 | 0 | 1851 | 382 |
| warmup after search: Promotion tactic | 30.547 | 30.547 | 10245 | 0 | 1999 | 399 |
| warmup loaded: Advanced pawns/check evasion | 30.547 | 30.547 | 10245 | 0 | 1999 | 399 |
| warmup after search: Advanced pawns/check evasion | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| after complete warmup | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured loaded: Quiet middlegame | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured after search: Quiet middlegame | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured loaded: Kiwipete | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured after search: Kiwipete | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured loaded: King safety | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured after search: King safety | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured loaded: Endgame | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured after search: Endgame | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured loaded: Promotion tactic | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured after search: Promotion tactic | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured loaded: Advanced pawns/check evasion | 30.547 | 30.547 | 11759 | 0 | 2155 | 418 |
| measured after search: Advanced pawns/check evasion | 30.555 | 30.555 | 11759 | 0 | 2155 | 418 |
| final process state | 30.555 | 30.555 | 11759 | 0 | 2155 | 418 |

## interaction pass

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.533 | 8136 | 1470553 | c7c5 | 38 | c7c5 h2h3 b7b5 d2b3  |
| Kiwipete | 4 | 6.300 | 20443 | 3244927 | e2a6 | 66 | e2a6 b4c3 d2c3 h3g2  |
| King safety | 4 | 8.340 | 24748 | 2967247 | c3d5 | 23 | c3d5 e7d8 h2h3 h7h6  |
| Endgame | 5 | 0.680 | 1868 | 2745443 | b4f4 | 114 | b4f4 h4g3 f4c4 h5e5 c4c7 g3g2  |
| Promotion tactic | 4 | 0.739 | 3182 | 4304682 | d7c8q | 690 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 2.503 | 6084 | 2430336 | g1h1 | -529 | g1h1 b2a1q b4a3 a1d1 a4d1 g7h6  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 24.096 | 64461 | 2675181 |

Aggregate elapsed times for all five processes: P1=24.350 ms, P2=24.096 ms, P3=23.865 ms, P4=24.357 ms, P5=23.975 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.418 | 29.422 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.426 | 29.430 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.395 | 30.395 | 2953 | 0 | 551 | 153 |
| warmup loaded: Kiwipete | 30.395 | 30.395 | 2953 | 0 | 551 | 153 |
| warmup after search: Kiwipete | 30.574 | 30.574 | 5433 | 0 | 1504 | 309 |
| warmup loaded: King safety | 30.574 | 30.574 | 5433 | 0 | 1504 | 309 |
| warmup after search: King safety | 30.574 | 30.574 | 9246 | 0 | 1817 | 374 |
| warmup loaded: Endgame | 30.574 | 30.574 | 9246 | 0 | 1817 | 374 |
| warmup after search: Endgame | 30.574 | 30.574 | 9739 | 0 | 1862 | 385 |
| warmup loaded: Promotion tactic | 30.574 | 30.574 | 9739 | 0 | 1862 | 385 |
| warmup after search: Promotion tactic | 30.574 | 30.574 | 10121 | 0 | 2011 | 402 |
| warmup loaded: Advanced pawns/check evasion | 30.574 | 30.574 | 10121 | 0 | 2011 | 402 |
| warmup after search: Advanced pawns/check evasion | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| after complete warmup | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured loaded: Quiet middlegame | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured after search: Quiet middlegame | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured loaded: Kiwipete | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured after search: Kiwipete | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured loaded: King safety | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured after search: King safety | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured loaded: Endgame | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured after search: Endgame | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured loaded: Promotion tactic | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured after search: Promotion tactic | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured loaded: Advanced pawns/check evasion | 30.574 | 30.574 | 11634 | 0 | 2165 | 421 |
| measured after search: Advanced pawns/check evasion | 30.582 | 30.582 | 11634 | 0 | 2165 | 421 |
| final process state | 30.582 | 30.582 | 11634 | 0 | 2165 | 421 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.598 | 8136 | 1453246 | c7c5 | 38 | c7c5 h2h3 b7b5 d2b3  |
| Kiwipete | 4 | 6.387 | 20443 | 3200575 | e2a6 | 66 | e2a6 b4c3 d2c3 h3g2  |
| King safety | 4 | 8.429 | 24748 | 2935902 | c3d5 | 23 | c3d5 e7d8 h2h3 h7h6  |
| Endgame | 5 | 0.681 | 1868 | 2743677 | b4f4 | 114 | b4f4 h4g3 f4c4 h5e5 c4c7 g3g2  |
| Promotion tactic | 4 | 0.760 | 3182 | 4186175 | d7c8q | 690 | d7c8q d8c8 e1f2 b7b5  |
| Advanced pawns/check evasion | 5 | 2.562 | 6084 | 2374939 | g1h1 | -529 | g1h1 b2a1q b4a3 a1d1 a4d1 g7h6  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 24.418 | 64461 | 2639904 |

Aggregate elapsed times for all five processes: P1=25.221 ms, P2=24.450 ms, P3=24.418 ms, P4=24.049 ms, P5=24.258 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.383 | 29.387 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.391 | 29.395 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.355 | 30.355 | 2953 | 0 | 551 | 153 |
| warmup loaded: Kiwipete | 30.355 | 30.355 | 2953 | 0 | 551 | 153 |
| warmup after search: Kiwipete | 30.535 | 30.535 | 5433 | 0 | 1504 | 309 |
| warmup loaded: King safety | 30.535 | 30.535 | 5433 | 0 | 1504 | 309 |
| warmup after search: King safety | 30.535 | 30.535 | 9246 | 0 | 1817 | 374 |
| warmup loaded: Endgame | 30.535 | 30.535 | 9246 | 0 | 1817 | 374 |
| warmup after search: Endgame | 30.535 | 30.535 | 9739 | 0 | 1862 | 385 |
| warmup loaded: Promotion tactic | 30.535 | 30.535 | 9739 | 0 | 1862 | 385 |
| warmup after search: Promotion tactic | 30.535 | 30.535 | 10121 | 0 | 2011 | 402 |
| warmup loaded: Advanced pawns/check evasion | 30.535 | 30.535 | 10121 | 0 | 2011 | 402 |
| warmup after search: Advanced pawns/check evasion | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| after complete warmup | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured loaded: Quiet middlegame | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured after search: Quiet middlegame | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured loaded: Kiwipete | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured after search: Kiwipete | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured loaded: King safety | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured after search: King safety | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured loaded: Endgame | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured after search: Endgame | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured loaded: Promotion tactic | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured after search: Promotion tactic | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured loaded: Advanced pawns/check evasion | 30.535 | 30.535 | 11634 | 0 | 2165 | 421 |
| measured after search: Advanced pawns/check evasion | 30.543 | 30.543 | 11634 | 0 | 2165 | 421 |
| final process state | 30.543 | 30.543 | 11634 | 0 | 2165 | 421 |

## ./tools/eval_calibration_positions.json

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.002 | 8414 | 1681974 | c8e6 | 33 | c8e6 d2b3 c7c5 c2c4  |
| Kiwipete | 4 | 7.024 | 22120 | 3149112 | e2a6 | 71 | e2a6 b4c3 d2c3 h3g2  |
| King safety | 4 | 5.966 | 18197 | 3050006 | c3d5 | 83 | c3d5 e7d8 g5f6 g7f6  |
| Endgame | 5 | 0.917 | 2325 | 2534250 | b4f4 | 112 | b4f4 h4g3 f4c4 h5c5 e2e3  |
| Promotion tactic | 4 | 0.842 | 3299 | 3917321 | d7c8q | 651 | d7c8q d8c8 e1f2 f8g8  |
| Advanced pawns/check evasion | 5 | 4.131 | 10139 | 2454636 | c4c5 | -643 | c4c5 a3b4 a1b1 b6c5 d2d4 g6e4 d4c5 e4b1  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 23.883 | 64494 | 2700413 |

Aggregate elapsed times for all five processes: P1=23.883 ms, P2=30.295 ms, P3=23.273 ms, P4=23.441 ms, P5=23.991 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.367 | 29.371 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.441 | 29.445 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.410 | 30.410 | 2615 | 0 | 585 | 162 |
| warmup loaded: Kiwipete | 30.410 | 30.410 | 2615 | 0 | 585 | 162 |
| warmup after search: Kiwipete | 30.523 | 30.523 | 5536 | 0 | 1584 | 323 |
| warmup loaded: King safety | 30.523 | 30.523 | 5536 | 0 | 1584 | 323 |
| warmup after search: King safety | 30.523 | 30.523 | 7905 | 0 | 1840 | 373 |
| warmup loaded: Endgame | 30.523 | 30.523 | 7905 | 0 | 1840 | 373 |
| warmup after search: Endgame | 30.523 | 30.523 | 8485 | 0 | 1889 | 385 |
| warmup loaded: Promotion tactic | 30.523 | 30.523 | 8485 | 0 | 1889 | 385 |
| warmup after search: Promotion tactic | 30.523 | 30.523 | 8932 | 0 | 2048 | 403 |
| warmup loaded: Advanced pawns/check evasion | 30.523 | 30.523 | 8932 | 0 | 2048 | 403 |
| warmup after search: Advanced pawns/check evasion | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| after complete warmup | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured loaded: Quiet middlegame | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured after search: Quiet middlegame | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured loaded: Kiwipete | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured after search: Kiwipete | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured loaded: King safety | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured after search: King safety | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured loaded: Endgame | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured after search: Endgame | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured loaded: Promotion tactic | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured after search: Promotion tactic | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured loaded: Advanced pawns/check evasion | 30.703 | 30.703 | 10330 | 0 | 2415 | 463 |
| measured after search: Advanced pawns/check evasion | 30.707 | 30.707 | 10330 | 0 | 2415 | 463 |
| final process state | 30.707 | 30.707 | 10330 | 0 | 2415 | 463 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 4.958 | 8414 | 1697180 | c8e6 | 33 | c8e6 d2b3 c7c5 c2c4  |
| Kiwipete | 4 | 7.081 | 22120 | 3124008 | e2a6 | 71 | e2a6 b4c3 d2c3 h3g2  |
| King safety | 4 | 5.586 | 18197 | 3257340 | c3d5 | 83 | c3d5 e7d8 g5f6 g7f6  |
| Endgame | 5 | 4.259 | 11538 | 2709350 | a5a6 | 57 | a5a6 d6d5 a6b7 h5h7 b4d4  |
| Promotion tactic | 4 | 0.855 | 3299 | 3860226 | d7c8q | 651 | d7c8q d8c8 e1f2 f8g8  |
| Advanced pawns/check evasion | 5 | 4.128 | 10156 | 2460151 | c4c5 | -643 | c4c5 a3b4 a1b1 b6c5 d2d4 g6e4 d4c5 e4b1  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 26.866 | 73724 | 2744123 |

Aggregate elapsed times for all five processes: P1=26.866 ms, P2=26.891 ms, P3=27.058 ms, P4=26.177 ms, P5=26.254 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.406 | 29.410 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.414 | 29.418 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.379 | 30.379 | 2615 | 0 | 585 | 162 |
| warmup loaded: Kiwipete | 30.379 | 30.379 | 2615 | 0 | 585 | 162 |
| warmup after search: Kiwipete | 30.496 | 30.496 | 5536 | 0 | 1584 | 323 |
| warmup loaded: King safety | 30.496 | 30.496 | 5536 | 0 | 1584 | 323 |
| warmup after search: King safety | 30.496 | 30.496 | 7905 | 0 | 1840 | 373 |
| warmup loaded: Endgame | 30.496 | 30.496 | 7905 | 0 | 1840 | 373 |
| warmup after search: Endgame | 30.496 | 30.496 | 10540 | 0 | 1918 | 395 |
| warmup loaded: Promotion tactic | 30.496 | 30.496 | 10540 | 0 | 1918 | 395 |
| warmup after search: Promotion tactic | 30.496 | 30.496 | 10987 | 0 | 2077 | 413 |
| warmup loaded: Advanced pawns/check evasion | 30.496 | 30.496 | 10987 | 0 | 2077 | 413 |
| warmup after search: Advanced pawns/check evasion | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| after complete warmup | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured loaded: Quiet middlegame | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured after search: Quiet middlegame | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured loaded: Kiwipete | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured after search: Kiwipete | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured loaded: King safety | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured after search: King safety | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured loaded: Endgame | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured after search: Endgame | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured loaded: Promotion tactic | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured after search: Promotion tactic | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured loaded: Advanced pawns/check evasion | 30.672 | 30.672 | 12388 | 0 | 2442 | 468 |
| measured after search: Advanced pawns/check evasion | 30.676 | 30.676 | 12388 | 0 | 2442 | 468 |
| final process state | 30.676 | 30.676 | 12388 | 0 | 2442 | 468 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 5.146 | 8620 | 1675181 | c8e6 | 33 | c8e6 d2b3 c7c5 c2c4  |
| Kiwipete | 4 | 7.550 | 22751 | 3013296 | e2a6 | 38 | e2a6 b4c3 d2c3 h3g2 f3g2 e6d5  |
| King safety | 4 | 4.047 | 11364 | 2808268 | c3d5 | 171 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 4.392 | 12765 | 2906671 | a5a6 | 31 | a5a6 h5g5 a6b7 g5c5 b4c4  |
| Promotion tactic | 4 | 0.470 | 1331 | 2829482 | d7c8q | 651 | d7c8q d8c8 e1f2 f8g8  |
| Advanced pawns/check evasion | 5 | 3.293 | 7611 | 2311277 | d2d4 | -560 | d2d4 a3e3 f1f2 b2a1q d1a1 e3h6 a4c2  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 24.898 | 64442 | 2588287 |

Aggregate elapsed times for all five processes: P1=25.595 ms, P2=24.898 ms, P3=24.939 ms, P4=24.694 ms, P5=24.649 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.430 | 29.434 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.438 | 29.441 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.402 | 30.402 | 2805 | 0 | 590 | 162 |
| warmup loaded: Kiwipete | 30.402 | 30.402 | 2805 | 0 | 590 | 162 |
| warmup after search: Kiwipete | 30.586 | 30.586 | 6261 | 0 | 1645 | 331 |
| warmup loaded: King safety | 30.586 | 30.586 | 6261 | 0 | 1645 | 331 |
| warmup after search: King safety | 30.586 | 30.586 | 8290 | 0 | 1863 | 369 |
| warmup loaded: Endgame | 30.586 | 30.586 | 8290 | 0 | 1863 | 369 |
| warmup after search: Endgame | 30.586 | 30.586 | 11628 | 0 | 1940 | 392 |
| warmup loaded: Promotion tactic | 30.586 | 30.586 | 11628 | 0 | 1940 | 392 |
| warmup after search: Promotion tactic | 30.586 | 30.586 | 11961 | 0 | 2025 | 404 |
| warmup loaded: Advanced pawns/check evasion | 30.586 | 30.586 | 11961 | 0 | 2025 | 404 |
| warmup after search: Advanced pawns/check evasion | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| after complete warmup | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured loaded: Quiet middlegame | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured after search: Quiet middlegame | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured loaded: Kiwipete | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured after search: Kiwipete | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured loaded: King safety | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured after search: King safety | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured loaded: Endgame | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured after search: Endgame | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured loaded: Promotion tactic | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured after search: Promotion tactic | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured loaded: Advanced pawns/check evasion | 30.598 | 30.598 | 13342 | 0 | 2288 | 451 |
| measured after search: Advanced pawns/check evasion | 30.605 | 30.605 | 13342 | 0 | 2288 | 451 |
| final process state | 30.605 | 30.605 | 13342 | 0 | 2288 | 451 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 3.689 | 7829 | 2122234 | c8e6 | 38 | c8e6 c2c3 b8c6 d2c4  |
| Kiwipete | 4 | 3.916 | 15206 | 3883078 | e2a6 | 31 | e2a6 b4c3 d2c3 h3g2 f3g2 e6d5  |
| King safety | 4 | 2.783 | 10266 | 3688286 | c3d5 | 171 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 4.257 | 14509 | 3408376 | a5a6 | 57 | a5a6 d6d5 a6b7 h5h7 b4d4  |
| Promotion tactic | 4 | 0.356 | 1058 | 2974374 | d7c8q | 638 | d7c8q d8c8 e1f2 f8g8  |
| Advanced pawns/check evasion | 5 | 2.083 | 5829 | 2798887 | c4c5 | -568 | c4c5 a3b4 a1b1 b6c5 d2d4 g6e4 b5b6  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 17.084 | 54697 | 3201727 |

Aggregate elapsed times for all five processes: P1=17.084 ms, P2=17.535 ms, P3=17.293 ms, P4=16.909 ms, P5=16.939 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.434 | 29.438 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.441 | 29.445 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.645 | 30.645 | 3009 | 0 | 505 | 101 |
| warmup loaded: Kiwipete | 30.645 | 30.645 | 3009 | 0 | 505 | 101 |
| warmup after search: Kiwipete | 30.777 | 30.777 | 5846 | 0 | 1283 | 218 |
| warmup loaded: King safety | 30.777 | 30.777 | 5846 | 0 | 1283 | 218 |
| warmup after search: King safety | 30.777 | 30.781 | 7710 | 0 | 1499 | 263 |
| warmup loaded: Endgame | 30.781 | 30.781 | 7710 | 0 | 1499 | 263 |
| warmup after search: Endgame | 30.781 | 30.781 | 11384 | 0 | 1588 | 281 |
| warmup loaded: Promotion tactic | 30.781 | 30.781 | 11384 | 0 | 1588 | 281 |
| warmup after search: Promotion tactic | 30.781 | 30.781 | 11697 | 0 | 1668 | 292 |
| warmup loaded: Advanced pawns/check evasion | 30.781 | 30.781 | 11697 | 0 | 1668 | 292 |
| warmup after search: Advanced pawns/check evasion | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| after complete warmup | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured loaded: Quiet middlegame | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured after search: Quiet middlegame | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured loaded: Kiwipete | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured after search: Kiwipete | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured loaded: King safety | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured after search: King safety | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured loaded: Endgame | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured after search: Endgame | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured loaded: Promotion tactic | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured after search: Promotion tactic | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured loaded: Advanced pawns/check evasion | 30.859 | 30.859 | 12835 | 0 | 1926 | 322 |
| measured after search: Advanced pawns/check evasion | 30.863 | 30.863 | 12835 | 0 | 1926 | 322 |
| final process state | 30.863 | 30.863 | 12835 | 0 | 1926 | 322 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 3.521 | 7428 | 2109920 | c8e6 | 38 | c8e6 c2c3 b8c6 d2c4  |
| Kiwipete | 4 | 4.027 | 15318 | 3803680 | e2a6 | 31 | e2a6 b4c3 d2c3 h3g2 f3g2 e6d5  |
| King safety | 4 | 2.639 | 9351 | 3543136 | c3d5 | 171 | c3d5 e7d8 d5f6 g7f6 g5h6  |
| Endgame | 5 | 4.274 | 14512 | 3395368 | a5a6 | 57 | a5a6 d6d5 a6b7 h5h7 b4d4  |
| Promotion tactic | 4 | 0.351 | 1022 | 2915069 | d7c8q | 638 | d7c8q d8c8 e1f2 f8g8  |
| Advanced pawns/check evasion | 5 | 2.143 | 5829 | 2720474 | c4c5 | -568 | c4c5 a3b4 a1b1 b6c5 d2d4 g6e4 b5b6  |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 16.954 | 53460 | 3153211 |

Aggregate elapsed times for all five processes: P1=16.851 ms, P2=17.220 ms, P3=17.367 ms, P4=16.954 ms, P5=16.757 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 29.406 | 29.410 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 29.414 | 29.418 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 30.609 | 30.609 | 2873 | 0 | 486 | 95 |
| warmup loaded: Kiwipete | 30.609 | 30.609 | 2873 | 0 | 486 | 95 |
| warmup after search: Kiwipete | 30.746 | 30.746 | 5710 | 0 | 1269 | 214 |
| warmup loaded: King safety | 30.746 | 30.746 | 5710 | 0 | 1269 | 214 |
| warmup after search: King safety | 30.746 | 30.750 | 7574 | 0 | 1489 | 262 |
| warmup loaded: Endgame | 30.750 | 30.750 | 7574 | 0 | 1489 | 262 |
| warmup after search: Endgame | 30.750 | 30.750 | 11247 | 0 | 1576 | 280 |
| warmup loaded: Promotion tactic | 30.750 | 30.750 | 11247 | 0 | 1576 | 280 |
| warmup after search: Promotion tactic | 30.750 | 30.750 | 11560 | 0 | 1659 | 292 |
| warmup loaded: Advanced pawns/check evasion | 30.750 | 30.750 | 11560 | 0 | 1659 | 292 |
| warmup after search: Advanced pawns/check evasion | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| after complete warmup | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured loaded: Quiet middlegame | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured after search: Quiet middlegame | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured loaded: Kiwipete | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured after search: Kiwipete | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured loaded: King safety | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured after search: King safety | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured loaded: Endgame | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured after search: Endgame | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured loaded: Promotion tactic | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured after search: Promotion tactic | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured loaded: Advanced pawns/check evasion | 30.824 | 30.824 | 12698 | 0 | 1920 | 322 |
| measured after search: Advanced pawns/check evasion | 30.828 | 30.828 | 12698 | 0 | 1920 | 322 |
| final process state | 30.828 | 30.828 | 12698 | 0 | 1920 | 322 |

## speed_check

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.991 | 451 | 455215 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.515 | 1447 | 575389 | e2a6 | 71 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 1.078 | 496 | 460002 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.392 | 343 | 875317 | b4f4 | 257 | b4f4 h4g3 f4f7 g3g2 f7g7 g2h2 |
| Promotion tactic | 4 | 0.376 | 214 | 569332 | d7c8q | 559 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.439 | 803 | 557922 | c4c5 | -664 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.791 | 3754 | 552804 |

Aggregate elapsed times for all five processes: P1=6.791 ms, P2=6.549 ms, P3=6.685 ms, P4=8.712 ms, P5=7.991 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.449 | 28.453 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.457 | 28.461 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.348 | 32.348 | 417 | 0 | 282 | 60 |
| warmup loaded: Kiwipete | 32.348 | 32.348 | 417 | 0 | 282 | 60 |
| warmup after search: Kiwipete | 32.395 | 32.395 | 1551 | 0 | 719 | 144 |
| warmup loaded: King safety | 32.395 | 32.395 | 1551 | 0 | 719 | 144 |
| warmup after search: King safety | 32.395 | 32.395 | 1856 | 0 | 859 | 174 |
| warmup loaded: Endgame | 32.395 | 32.395 | 1856 | 0 | 859 | 174 |
| warmup after search: Endgame | 32.395 | 32.395 | 2035 | 0 | 898 | 183 |
| warmup loaded: Promotion tactic | 32.395 | 32.395 | 2035 | 0 | 898 | 183 |
| warmup after search: Promotion tactic | 32.410 | 32.414 | 2147 | 0 | 988 | 193 |
| warmup loaded: Advanced pawns/check evasion | 32.414 | 32.414 | 2147 | 0 | 988 | 193 |
| warmup after search: Advanced pawns/check evasion | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| after complete warmup | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured loaded: Quiet middlegame | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured after search: Quiet middlegame | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured loaded: Kiwipete | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured after search: Kiwipete | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured loaded: King safety | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured after search: King safety | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured loaded: Endgame | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured after search: Endgame | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured loaded: Promotion tactic | 32.414 | 32.414 | 2651 | 0 | 1183 | 219 |
| measured after search: Promotion tactic | 32.418 | 32.418 | 2651 | 0 | 1183 | 219 |
| measured loaded: Advanced pawns/check evasion | 32.418 | 32.418 | 2651 | 0 | 1183 | 219 |
| measured after search: Advanced pawns/check evasion | 32.418 | 32.418 | 2651 | 0 | 1183 | 219 |
| final process state | 32.418 | 32.418 | 2651 | 0 | 1183 | 219 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.989 | 451 | 455916 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.427 | 1447 | 596314 | e2a6 | 71 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.955 | 496 | 519417 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.358 | 343 | 959335 | b4f4 | 257 | b4f4 h4g3 f4f7 g3g2 f7g7 g2h2 |
| Promotion tactic | 4 | 0.363 | 214 | 589179 | d7c8q | 559 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.403 | 803 | 572419 | c4c5 | -664 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.494 | 3754 | 578047 |

Aggregate elapsed times for all five processes: P1=6.477 ms, P2=6.494 ms, P3=6.373 ms, P4=6.530 ms, P5=9.475 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.422 | 28.426 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.430 | 28.434 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.316 | 32.316 | 417 | 0 | 282 | 60 |
| warmup loaded: Kiwipete | 32.316 | 32.316 | 417 | 0 | 282 | 60 |
| warmup after search: Kiwipete | 32.367 | 32.367 | 1551 | 0 | 719 | 144 |
| warmup loaded: King safety | 32.367 | 32.367 | 1551 | 0 | 719 | 144 |
| warmup after search: King safety | 32.367 | 32.367 | 1856 | 0 | 859 | 174 |
| warmup loaded: Endgame | 32.367 | 32.367 | 1856 | 0 | 859 | 174 |
| warmup after search: Endgame | 32.367 | 32.367 | 2035 | 0 | 898 | 183 |
| warmup loaded: Promotion tactic | 32.367 | 32.367 | 2035 | 0 | 898 | 183 |
| warmup after search: Promotion tactic | 32.383 | 32.387 | 2147 | 0 | 988 | 193 |
| warmup loaded: Advanced pawns/check evasion | 32.387 | 32.387 | 2147 | 0 | 988 | 193 |
| warmup after search: Advanced pawns/check evasion | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| after complete warmup | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured loaded: Quiet middlegame | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured after search: Quiet middlegame | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured loaded: Kiwipete | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured after search: Kiwipete | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured loaded: King safety | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured after search: King safety | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured loaded: Endgame | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured after search: Endgame | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured loaded: Promotion tactic | 32.387 | 32.387 | 2651 | 0 | 1183 | 219 |
| measured after search: Promotion tactic | 32.391 | 32.391 | 2651 | 0 | 1183 | 219 |
| measured loaded: Advanced pawns/check evasion | 32.391 | 32.391 | 2651 | 0 | 1183 | 219 |
| measured after search: Advanced pawns/check evasion | 32.391 | 32.391 | 2651 | 0 | 1183 | 219 |
| final process state | 32.391 | 32.391 | 2651 | 0 | 1183 | 219 |

## PieceSquare Candidate Regression Check

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.945 | 451 | 477407 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.456 | 1447 | 589226 | e2a6 | 71 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.975 | 496 | 508526 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.371 | 343 | 923865 | b4f4 | 257 | b4f4 h4g3 f4f7 g3g2 f7g7 g2h2 |
| Promotion tactic | 4 | 0.366 | 214 | 584884 | d7c8q | 559 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.394 | 803 | 575888 | c4c5 | -664 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.507 | 3754 | 576888 |

Aggregate elapsed times for all five processes: P1=6.507 ms, P2=6.602 ms, P3=6.307 ms, P4=6.348 ms, P5=8.172 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.430 | 28.434 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.438 | 28.441 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.324 | 32.324 | 417 | 0 | 282 | 60 |
| warmup loaded: Kiwipete | 32.324 | 32.324 | 417 | 0 | 282 | 60 |
| warmup after search: Kiwipete | 32.375 | 32.375 | 1551 | 0 | 719 | 144 |
| warmup loaded: King safety | 32.375 | 32.375 | 1551 | 0 | 719 | 144 |
| warmup after search: King safety | 32.375 | 32.375 | 1856 | 0 | 859 | 174 |
| warmup loaded: Endgame | 32.375 | 32.375 | 1856 | 0 | 859 | 174 |
| warmup after search: Endgame | 32.375 | 32.375 | 2035 | 0 | 898 | 183 |
| warmup loaded: Promotion tactic | 32.375 | 32.375 | 2035 | 0 | 898 | 183 |
| warmup after search: Promotion tactic | 32.391 | 32.395 | 2147 | 0 | 988 | 193 |
| warmup loaded: Advanced pawns/check evasion | 32.395 | 32.395 | 2147 | 0 | 988 | 193 |
| warmup after search: Advanced pawns/check evasion | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| after complete warmup | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured loaded: Quiet middlegame | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured after search: Quiet middlegame | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured loaded: Kiwipete | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured after search: Kiwipete | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured loaded: King safety | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured after search: King safety | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured loaded: Endgame | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured after search: Endgame | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured loaded: Promotion tactic | 32.395 | 32.395 | 2651 | 0 | 1183 | 219 |
| measured after search: Promotion tactic | 32.398 | 32.398 | 2651 | 0 | 1183 | 219 |
| measured loaded: Advanced pawns/check evasion | 32.398 | 32.398 | 2651 | 0 | 1183 | 219 |
| measured after search: Advanced pawns/check evasion | 32.398 | 32.398 | 2651 | 0 | 1183 | 219 |
| final process state | 32.398 | 32.398 | 2651 | 0 | 1183 | 219 |

## Candidate 2 Baseline Check

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.983 | 451 | 458790 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.738 | 1651 | 603101 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.998 | 500 | 501133 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.615 | 596 | 969744 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.367 | 213 | 580541 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.394 | 757 | 543127 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.094 | 4168 | 587576 |

Aggregate elapsed times for all five processes: P1=7.087 ms, P2=7.160 ms, P3=7.094 ms, P4=7.015 ms, P5=7.117 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.426 | 28.430 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.434 | 28.438 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.320 | 32.320 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 32.320 | 32.320 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 32.383 | 32.383 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 32.383 | 32.383 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 32.383 | 32.383 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 32.383 | 32.383 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 32.383 | 32.383 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 32.383 | 32.383 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 32.387 | 32.387 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 32.387 | 32.387 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 32.449 | 32.449 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 32.453 | 32.453 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 32.453 | 32.453 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 32.453 | 32.453 | 2896 | 0 | 1231 | 231 |
| final process state | 32.453 | 32.453 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.982 | 451 | 459424 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.685 | 1651 | 614988 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 1.001 | 500 | 499429 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.610 | 596 | 976356 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.376 | 213 | 566275 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.401 | 757 | 540314 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.055 | 4168 | 590785 |

Aggregate elapsed times for all five processes: P1=7.033 ms, P2=7.102 ms, P3=7.150 ms, P4=7.055 ms, P5=6.974 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.414 | 28.418 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.422 | 28.426 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.309 | 32.309 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 32.309 | 32.309 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 32.371 | 32.371 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 32.371 | 32.371 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 32.371 | 32.371 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 32.371 | 32.371 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 32.371 | 32.371 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 32.371 | 32.371 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 32.375 | 32.375 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 32.375 | 32.375 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 32.438 | 32.438 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 32.441 | 32.441 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 32.441 | 32.441 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 32.441 | 32.441 | 2896 | 0 | 1231 | 231 |
| final process state | 32.441 | 32.441 | 2896 | 0 | 1231 | 231 |

## Candidate QueenMobility Benchmark

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 1.011 | 451 | 446054 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.739 | 1651 | 602691 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 1.001 | 500 | 499305 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.607 | 596 | 982145 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.364 | 213 | 585686 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.407 | 757 | 538168 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.129 | 4168 | 584655 |

Aggregate elapsed times for all five processes: P1=7.012 ms, P2=7.046 ms, P3=7.814 ms, P4=8.193 ms, P5=7.129 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.438 | 28.441 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.445 | 28.449 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.332 | 32.332 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 32.332 | 32.332 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 32.398 | 32.398 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 32.398 | 32.398 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 32.398 | 32.398 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 32.398 | 32.398 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 32.398 | 32.398 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 32.398 | 32.398 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 32.402 | 32.402 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 32.402 | 32.402 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 32.461 | 32.461 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 32.465 | 32.465 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 32.465 | 32.465 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 32.465 | 32.465 | 2896 | 0 | 1231 | 231 |
| final process state | 32.465 | 32.465 | 2896 | 0 | 1231 | 231 |

## Candidate KingSafety Benchmark

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.957 | 451 | 471335 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.636 | 1651 | 626348 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.974 | 500 | 513476 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.610 | 596 | 976748 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.377 | 213 | 564896 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.408 | 757 | 537698 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.962 | 4168 | 598711 |

Aggregate elapsed times for all five processes: P1=6.962 ms, P2=6.886 ms, P3=6.725 ms, P4=8.121 ms, P5=7.004 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.457 | 28.461 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.465 | 28.469 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.352 | 32.352 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 32.352 | 32.352 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 32.418 | 32.418 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 32.418 | 32.418 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 32.418 | 32.418 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 32.418 | 32.418 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 32.418 | 32.418 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 32.418 | 32.418 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 32.422 | 32.422 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 32.422 | 32.422 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 32.484 | 32.484 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 32.484 | 32.484 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 32.484 | 32.484 | 2896 | 0 | 1231 | 231 |
| final process state | 32.484 | 32.484 | 2896 | 0 | 1231 | 231 |

## Candidate EndgameWeights Benchmark

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.973 | 451 | 463508 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.728 | 1651 | 605255 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.992 | 500 | 504041 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.603 | 596 | 988384 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.373 | 213 | 571390 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.428 | 757 | 530037 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.097 | 4168 | 587311 |

Aggregate elapsed times for all five processes: P1=7.176 ms, P2=7.147 ms, P3=7.080 ms, P4=7.006 ms, P5=7.097 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.457 | 28.461 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.465 | 28.469 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.352 | 32.352 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 32.352 | 32.352 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 32.414 | 32.414 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 32.414 | 32.414 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 32.414 | 32.414 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 32.414 | 32.414 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 32.414 | 32.414 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 32.414 | 32.414 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 32.418 | 32.418 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 32.418 | 32.418 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 32.484 | 32.484 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 32.484 | 32.484 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 32.484 | 32.484 | 2896 | 0 | 1231 | 231 |
| final process state | 32.484 | 32.484 | 2896 | 0 | 1231 | 231 |

## Candidate Inline Benchmark

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.954 | 451 | 472993 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.660 | 1651 | 620568 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.976 | 500 | 512391 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.603 | 596 | 988752 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.361 | 213 | 590292 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.376 | 757 | 550141 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.929 | 4168 | 601494 |

Aggregate elapsed times for all five processes: P1=8.796 ms, P2=7.027 ms, P3=6.926 ms, P4=6.787 ms, P5=6.929 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.457 | 28.461 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.465 | 28.469 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.348 | 32.348 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 32.348 | 32.348 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 32.414 | 32.414 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 32.414 | 32.414 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 32.414 | 32.414 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 32.414 | 32.414 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 32.414 | 32.414 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 32.414 | 32.414 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 32.418 | 32.418 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 32.418 | 32.418 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 32.477 | 32.477 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |
| final process state | 32.480 | 32.480 | 2896 | 0 | 1231 | 231 |

## Candidate RookFile Benchmark

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.978 | 451 | 461261 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.640 | 1651 | 625283 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.995 | 500 | 502761 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.594 | 596 | 1004110 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.362 | 213 | 588285 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.367 | 757 | 553925 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.935 | 4168 | 601018 |

Aggregate elapsed times for all five processes: P1=6.945 ms, P2=7.059 ms, P3=6.887 ms, P4=6.935 ms, P5=6.882 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.480 | 28.484 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.488 | 28.492 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.371 | 32.371 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 32.371 | 32.371 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 32.434 | 32.434 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 32.434 | 32.434 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 32.434 | 32.434 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 32.434 | 32.434 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 32.434 | 32.434 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 32.434 | 32.434 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 32.438 | 32.438 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 32.438 | 32.438 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 32.500 | 32.500 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 32.504 | 32.504 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 32.504 | 32.504 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 32.504 | 32.504 | 2896 | 0 | 1231 | 231 |
| final process state | 32.504 | 32.504 | 2896 | 0 | 1231 | 231 |

## Regression test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.946 | 451 | 476636 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.775 | 1651 | 595007 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.960 | 500 | 520711 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.579 | 596 | 1029455 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.356 | 213 | 599044 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.351 | 757 | 560411 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.967 | 4168 | 598291 |

Aggregate elapsed times for all five processes: P1=7.041 ms, P2=7.219 ms, P3=6.833 ms, P4=6.735 ms, P5=6.967 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.410 | 28.414 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.418 | 28.422 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 32.301 | 32.301 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 32.301 | 32.301 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 32.367 | 32.367 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 32.367 | 32.367 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 32.367 | 32.367 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 32.367 | 32.367 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 32.367 | 32.367 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 32.367 | 32.367 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 32.371 | 32.371 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 32.371 | 32.371 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 32.430 | 32.430 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 32.434 | 32.434 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 32.434 | 32.434 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 32.434 | 32.434 | 2896 | 0 | 1231 | 231 |
| final process state | 32.434 | 32.434 | 2896 | 0 | 1231 | 231 |

## --help

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.958 | 451 | 470533 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.770 | 1651 | 595984 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 1.024 | 500 | 488084 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.776 | 596 | 767869 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.378 | 213 | 563521 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.453 | 757 | 520986 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.360 | 4168 | 566283 |

Aggregate elapsed times for all five processes: P1=8.549 ms, P2=7.343 ms, P3=7.360 ms, P4=7.349 ms, P5=7.441 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.754 | 28.754 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.762 | 28.762 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.980 | 72.980 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.980 | 72.980 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.090 | 73.090 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.090 | 73.090 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.090 | 73.090 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.090 | 73.090 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.094 | 73.094 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.094 | 73.094 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.094 | 73.094 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.094 | 73.094 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| final process state | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |

## test_run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.965 | 451 | 467467 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.740 | 1651 | 602446 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 1.003 | 500 | 498545 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.774 | 596 | 769786 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.372 | 213 | 572099 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.449 | 757 | 522477 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.304 | 4168 | 570677 |

Aggregate elapsed times for all five processes: P1=7.365 ms, P2=7.259 ms, P3=7.204 ms, P4=7.304 ms, P5=7.343 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.637 | 28.637 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.707 | 28.707 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.980 | 72.980 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.980 | 72.980 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.090 | 73.090 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.090 | 73.090 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.090 | 73.090 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.090 | 73.090 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.094 | 73.094 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.094 | 73.094 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.094 | 73.094 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.094 | 73.094 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| final process state | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |

## test_run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.963 | 451 | 468517 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.778 | 1651 | 594339 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 1.009 | 500 | 495514 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.744 | 596 | 800613 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.386 | 213 | 551970 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.480 | 757 | 511630 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.359 | 4168 | 566347 |

Aggregate elapsed times for all five processes: P1=7.359 ms, P2=7.362 ms, P3=7.308 ms, P4=7.296 ms, P5=7.452 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.738 | 28.738 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.746 | 28.746 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.965 | 72.965 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.965 | 72.965 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.074 | 73.074 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.074 | 73.074 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.074 | 73.074 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.074 | 73.074 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.078 | 73.078 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.078 | 73.078 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.078 | 73.078 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.078 | 73.078 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |
| final process state | 73.168 | 73.168 | 2896 | 0 | 1231 | 231 |

## test_run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.966 | 451 | 466940 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.783 | 1651 | 593218 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 1.007 | 500 | 496579 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.753 | 596 | 791340 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.382 | 213 | 557997 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.474 | 757 | 513453 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.365 | 4168 | 565914 |

Aggregate elapsed times for all five processes: P1=7.263 ms, P2=7.392 ms, P3=7.279 ms, P4=7.412 ms, P5=7.365 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.668 | 28.668 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.738 | 28.738 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 73.012 | 73.012 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 73.012 | 73.012 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.121 | 73.121 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.121 | 73.121 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.121 | 73.121 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.121 | 73.121 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.125 | 73.125 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.125 | 73.125 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.125 | 73.125 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.125 | 73.125 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |
| final process state | 73.215 | 73.215 | 2896 | 0 | 1231 | 231 |

## test_run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.972 | 451 | 463766 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.739 | 1651 | 602674 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.995 | 500 | 502691 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.756 | 596 | 788788 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.375 | 213 | 567564 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.461 | 757 | 518291 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.298 | 4168 | 571114 |

Aggregate elapsed times for all five processes: P1=7.226 ms, P2=7.251 ms, P3=7.301 ms, P4=7.319 ms, P5=7.298 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.637 | 28.637 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.645 | 28.645 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.973 | 72.973 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.973 | 72.973 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.082 | 73.082 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.082 | 73.082 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.082 | 73.082 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.082 | 73.082 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.086 | 73.086 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.086 | 73.086 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.086 | 73.086 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.086 | 73.086 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| final process state | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |

## test_run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.973 | 451 | 463667 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.752 | 1651 | 599836 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.979 | 500 | 510498 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.739 | 596 | 806037 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.376 | 213 | 567097 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.455 | 757 | 520233 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.275 | 4168 | 572947 |

Aggregate elapsed times for all five processes: P1=7.271 ms, P2=7.308 ms, P3=7.259 ms, P4=7.318 ms, P5=7.275 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.660 | 28.660 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.668 | 28.668 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.887 | 72.887 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.887 | 72.887 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 72.996 | 72.996 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 72.996 | 72.996 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 72.996 | 72.996 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 72.996 | 72.996 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.000 | 73.000 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.000 | 73.000 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.000 | 73.000 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.000 | 73.000 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |
| final process state | 73.090 | 73.090 | 2896 | 0 | 1231 | 231 |

## test_run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.960 | 451 | 469776 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.721 | 1651 | 606838 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.996 | 500 | 502021 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.762 | 596 | 782422 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.374 | 213 | 570246 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.445 | 757 | 524012 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.257 | 4168 | 574378 |

Aggregate elapsed times for all five processes: P1=7.205 ms, P2=7.206 ms, P3=7.262 ms, P4=7.257 ms, P5=7.322 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.750 | 28.750 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.758 | 28.758 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.977 | 72.977 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.977 | 72.977 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.082 | 73.082 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.082 | 73.082 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.082 | 73.082 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.082 | 73.082 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.086 | 73.086 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.086 | 73.086 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.086 | 73.086 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.086 | 73.086 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |
| final process state | 73.180 | 73.180 | 2896 | 0 | 1231 | 231 |

## test_run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.957 | 451 | 471037 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.745 | 1651 | 601448 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.999 | 500 | 500524 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.774 | 596 | 769692 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.373 | 213 | 571335 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.461 | 757 | 518183 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.309 | 4168 | 570218 |

Aggregate elapsed times for all five processes: P1=7.309 ms, P2=7.260 ms, P3=7.294 ms, P4=7.318 ms, P5=7.347 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.695 | 28.695 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.703 | 28.703 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.922 | 72.922 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.922 | 72.922 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.031 | 73.031 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.031 | 73.031 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.031 | 73.031 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.031 | 73.031 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.035 | 73.035 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.035 | 73.035 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.035 | 73.035 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.035 | 73.035 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |
| final process state | 73.125 | 73.125 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.953 | 451 | 473331 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.826 | 1651 | 584199 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.992 | 500 | 503827 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.739 | 596 | 806596 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.370 | 213 | 574920 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.468 | 757 | 515520 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.349 | 4168 | 567142 |

Aggregate elapsed times for all five processes: P1=7.341 ms, P2=7.393 ms, P3=7.351 ms, P4=7.178 ms, P5=7.349 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.660 | 28.660 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.715 | 28.715 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.934 | 72.934 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.934 | 72.934 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.043 | 73.043 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.043 | 73.043 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.043 | 73.043 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.043 | 73.043 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.047 | 73.047 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.047 | 73.047 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.047 | 73.047 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.047 | 73.047 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| final process state | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.949 | 451 | 475349 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.756 | 1651 | 599150 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.981 | 500 | 509548 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.745 | 596 | 800008 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.367 | 213 | 580270 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.438 | 757 | 526528 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.235 | 4168 | 576057 |

Aggregate elapsed times for all five processes: P1=7.216 ms, P2=7.235 ms, P3=7.286 ms, P4=7.217 ms, P5=7.424 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.668 | 28.668 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.676 | 28.676 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.895 | 72.895 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.895 | 72.895 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.004 | 73.004 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.004 | 73.004 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.004 | 73.004 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.004 | 73.004 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.008 | 73.008 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.008 | 73.008 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.008 | 73.008 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.008 | 73.008 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |
| final process state | 73.098 | 73.098 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.950 | 451 | 474981 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.715 | 1651 | 608088 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.977 | 500 | 511631 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.735 | 596 | 810416 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.364 | 213 | 585233 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.431 | 757 | 529147 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.172 | 4168 | 581163 |

Aggregate elapsed times for all five processes: P1=7.159 ms, P2=7.287 ms, P3=7.213 ms, P4=7.172 ms, P5=7.146 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.664 | 28.664 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.672 | 28.672 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.891 | 72.891 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.891 | 72.891 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.000 | 73.000 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.000 | 73.000 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.000 | 73.000 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.000 | 73.000 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.004 | 73.004 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.004 | 73.004 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.004 | 73.004 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.004 | 73.004 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |
| final process state | 73.094 | 73.094 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.979 | 451 | 460800 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.733 | 1651 | 604052 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.986 | 500 | 506931 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.737 | 596 | 808182 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.372 | 213 | 572409 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.444 | 757 | 524369 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.251 | 4168 | 574779 |

Aggregate elapsed times for all five processes: P1=7.354 ms, P2=7.219 ms, P3=7.197 ms, P4=7.327 ms, P5=7.251 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.754 | 28.754 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.762 | 28.762 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.980 | 72.980 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.980 | 72.980 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.090 | 73.090 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.090 | 73.090 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.090 | 73.090 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.090 | 73.090 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.094 | 73.094 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.094 | 73.094 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.094 | 73.094 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.094 | 73.094 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |
| final process state | 73.184 | 73.184 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.961 | 451 | 469171 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.738 | 1651 | 603017 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.986 | 500 | 507234 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.761 | 596 | 783005 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.373 | 213 | 571661 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.477 | 757 | 512537 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.296 | 4168 | 571300 |

Aggregate elapsed times for all five processes: P1=7.185 ms, P2=7.310 ms, P3=7.282 ms, P4=7.296 ms, P5=7.396 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.730 | 28.730 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.738 | 28.738 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.957 | 72.957 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.957 | 72.957 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.066 | 73.066 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.066 | 73.066 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.066 | 73.066 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.066 | 73.066 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.070 | 73.070 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.070 | 73.070 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.070 | 73.070 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.070 | 73.070 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |
| final process state | 73.160 | 73.160 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.955 | 451 | 472256 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.774 | 1651 | 595133 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.990 | 500 | 504814 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.742 | 596 | 803442 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.368 | 213 | 578188 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.461 | 757 | 518088 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.291 | 4168 | 571666 |

Aggregate elapsed times for all five processes: P1=7.276 ms, P2=7.304 ms, P3=7.334 ms, P4=7.262 ms, P5=7.291 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.598 | 28.598 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.652 | 28.652 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.871 | 72.871 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.871 | 72.871 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 72.980 | 72.980 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 72.980 | 72.980 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 72.980 | 72.980 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 72.980 | 72.980 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 72.984 | 72.984 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 72.984 | 72.984 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 72.984 | 72.984 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 72.984 | 72.984 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |
| final process state | 73.074 | 73.074 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.963 | 451 | 468224 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.741 | 1651 | 602382 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.996 | 500 | 502024 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.749 | 596 | 795288 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.371 | 213 | 574131 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.472 | 757 | 514214 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.293 | 4168 | 571544 |

Aggregate elapsed times for all five processes: P1=7.336 ms, P2=7.291 ms, P3=7.286 ms, P4=7.293 ms, P5=7.320 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.711 | 28.711 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.719 | 28.719 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.938 | 72.938 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.938 | 72.938 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.043 | 73.043 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.043 | 73.043 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.043 | 73.043 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.043 | 73.043 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.047 | 73.047 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.047 | 73.047 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.047 | 73.047 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.047 | 73.047 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |
| final process state | 73.137 | 73.137 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.956 | 451 | 471714 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.747 | 1651 | 600994 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.996 | 500 | 502023 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.759 | 596 | 785638 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.375 | 213 | 567795 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.456 | 757 | 519806 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.289 | 4168 | 571802 |

Aggregate elapsed times for all five processes: P1=7.365 ms, P2=7.269 ms, P3=7.383 ms, P4=7.289 ms, P5=7.265 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.746 | 28.746 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.754 | 28.754 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.973 | 72.973 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.973 | 72.973 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.078 | 73.078 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.078 | 73.078 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.078 | 73.078 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.078 | 73.078 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.082 | 73.082 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.082 | 73.082 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.082 | 73.082 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.082 | 73.082 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |
| final process state | 73.176 | 73.176 | 2896 | 0 | 1231 | 231 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.961 | 495 | 515239 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.676 | 1658 | 619636 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.887 | 455 | 512987 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.414 | 340 | 821196 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.373 | 227 | 608944 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.412 | 764 | 541167 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.722 | 3939 | 585985 |

Aggregate elapsed times for all five processes: P1=6.912 ms, P2=6.640 ms, P3=6.688 ms, P4=6.722 ms, P5=6.841 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.707 | 28.707 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.715 | 28.715 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.883 | 72.883 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.883 | 72.883 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.984 | 72.984 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.984 | 72.984 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.988 | 72.988 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.988 | 72.988 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.988 | 72.988 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.988 | 72.988 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.988 | 72.988 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.988 | 72.988 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |
| final process state | 73.070 | 73.070 | 2796 | 0 | 1241 | 230 |

## Contiguous MoveList no zero-init

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.948 | 495 | 522122 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.647 | 1658 | 626387 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.901 | 455 | 505209 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.400 | 340 | 849587 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.383 | 227 | 592888 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.387 | 764 | 550691 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.666 | 3939 | 590908 |

Aggregate elapsed times for all five processes: P1=6.857 ms, P2=6.651 ms, P3=6.650 ms, P4=6.666 ms, P5=6.715 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.723 | 28.723 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.777 | 28.777 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.945 | 72.945 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.945 | 72.945 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 73.043 | 73.043 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 73.043 | 73.043 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 73.047 | 73.047 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 73.047 | 73.047 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 73.047 | 73.047 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 73.047 | 73.047 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 73.047 | 73.047 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 73.047 | 73.047 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |
| final process state | 73.133 | 73.133 | 2796 | 0 | 1241 | 230 |

## test_clean_stage1

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.954 | 451 | 472526 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.742 | 1651 | 602224 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 1.002 | 500 | 499209 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.777 | 596 | 766902 | b4f4 | 113 | b4f4 h4g3 f4f8 g3g2 f8g8 g2h2 |
| Promotion tactic | 4 | 0.379 | 213 | 561803 | d7c8q | 550 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2g3 c5d4 d1d4 f5c2 |
| Advanced pawns/check evasion | 5 | 1.452 | 757 | 521192 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 7.306 | 4168 | 570469 |

Aggregate elapsed times for all five processes: P1=7.339 ms, P2=7.319 ms, P3=7.267 ms, P4=7.306 ms, P5=7.297 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.590 | 28.590 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.660 | 28.660 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.941 | 72.941 | 417 | 0 | 281 | 59 |
| warmup loaded: Kiwipete | 72.941 | 72.941 | 417 | 0 | 281 | 59 |
| warmup after search: Kiwipete | 73.051 | 73.051 | 1617 | 0 | 764 | 157 |
| warmup loaded: King safety | 73.051 | 73.051 | 1617 | 0 | 764 | 157 |
| warmup after search: King safety | 73.051 | 73.051 | 1926 | 0 | 905 | 188 |
| warmup loaded: Endgame | 73.051 | 73.051 | 1926 | 0 | 905 | 188 |
| warmup after search: Endgame | 73.055 | 73.055 | 2278 | 0 | 958 | 198 |
| warmup loaded: Promotion tactic | 73.055 | 73.055 | 2278 | 0 | 958 | 198 |
| warmup after search: Promotion tactic | 73.055 | 73.055 | 2389 | 0 | 1048 | 208 |
| warmup loaded: Advanced pawns/check evasion | 73.055 | 73.055 | 2389 | 0 | 1048 | 208 |
| warmup after search: Advanced pawns/check evasion | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| after complete warmup | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured loaded: Quiet middlegame | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured after search: Quiet middlegame | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured loaded: Kiwipete | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured after search: Kiwipete | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured loaded: King safety | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured after search: King safety | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured loaded: Endgame | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured after search: Endgame | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured loaded: Promotion tactic | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured after search: Promotion tactic | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured loaded: Advanced pawns/check evasion | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| measured after search: Advanced pawns/check evasion | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |
| final process state | 73.145 | 73.145 | 2896 | 0 | 1231 | 231 |

## Stage 2 Refactor

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.944 | 495 | 524519 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.625 | 1658 | 631564 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.864 | 455 | 526431 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.391 | 340 | 868472 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.362 | 227 | 627803 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.353 | 764 | 564849 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.539 | 3939 | 602394 |

Aggregate elapsed times for all five processes: P1=6.854 ms, P2=6.551 ms, P3=6.499 ms, P4=6.506 ms, P5=6.539 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.738 | 28.738 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.746 | 28.746 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.859 | 72.859 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.859 | 72.859 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.949 | 72.949 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.949 | 72.949 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.953 | 72.953 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.953 | 72.953 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.953 | 72.953 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.953 | 72.953 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.953 | 72.953 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.953 | 72.953 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |
| final process state | 73.023 | 73.023 | 2796 | 0 | 1241 | 230 |

## MultiRun 1

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.933 | 495 | 530704 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.628 | 1658 | 630896 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.859 | 455 | 529424 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.396 | 340 | 858644 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.382 | 227 | 594145 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.355 | 764 | 563731 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.553 | 3939 | 601057 |

Aggregate elapsed times for all five processes: P1=6.723 ms, P2=6.553 ms, P3=6.529 ms, P4=6.632 ms, P5=6.552 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.668 | 28.668 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.676 | 28.676 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.793 | 72.793 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.793 | 72.793 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.879 | 72.879 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.879 | 72.879 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.883 | 72.883 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.883 | 72.883 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.883 | 72.883 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.883 | 72.883 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.883 | 72.883 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.883 | 72.883 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |
| final process state | 72.957 | 72.957 | 2796 | 0 | 1241 | 230 |

## MultiRun 2

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.925 | 495 | 535319 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.620 | 1658 | 632878 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.860 | 455 | 528833 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.382 | 340 | 889416 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.364 | 227 | 623047 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.339 | 764 | 570506 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.491 | 3939 | 606876 |

Aggregate elapsed times for all five processes: P1=6.574 ms, P2=6.491 ms, P3=6.449 ms, P4=6.479 ms, P5=7.479 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.691 | 28.691 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.699 | 28.699 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.816 | 72.816 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.816 | 72.816 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.902 | 72.902 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.902 | 72.902 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.906 | 72.906 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.906 | 72.906 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.906 | 72.906 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.906 | 72.906 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.906 | 72.906 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.906 | 72.906 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |
| final process state | 72.977 | 72.977 | 2796 | 0 | 1241 | 230 |

## MultiRun 3

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.937 | 495 | 528463 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.628 | 1658 | 630869 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.865 | 455 | 526023 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.395 | 340 | 860221 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.383 | 227 | 592338 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.360 | 764 | 561801 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.568 | 3939 | 599711 |

Aggregate elapsed times for all five processes: P1=6.671 ms, P2=6.572 ms, P3=6.513 ms, P4=6.510 ms, P5=6.568 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.695 | 28.695 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.750 | 28.750 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.863 | 72.863 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.863 | 72.863 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.953 | 72.953 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.953 | 72.953 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.957 | 72.957 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.957 | 72.957 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.957 | 72.957 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.957 | 72.957 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.957 | 72.957 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.957 | 72.957 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |
| final process state | 73.027 | 73.027 | 2796 | 0 | 1241 | 230 |

## MultiRun 4

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.920 | 495 | 538323 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.588 | 1658 | 640760 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.852 | 455 | 533917 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.392 | 340 | 868250 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.390 | 227 | 582181 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.341 | 764 | 569686 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.482 | 3939 | 607696 |

Aggregate elapsed times for all five processes: P1=6.527 ms, P2=6.482 ms, P3=6.549 ms, P4=6.482 ms, P5=6.455 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.703 | 28.703 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.711 | 28.711 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.828 | 72.828 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.828 | 72.828 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.914 | 72.914 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.914 | 72.914 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.918 | 72.918 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.918 | 72.918 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.918 | 72.918 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.918 | 72.918 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.918 | 72.918 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.918 | 72.918 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |
| final process state | 72.988 | 72.988 | 2796 | 0 | 1241 | 230 |

## MultiRun 5

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.926 | 495 | 534317 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.604 | 1658 | 636703 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.857 | 455 | 530640 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.389 | 340 | 874182 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.358 | 227 | 634301 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.338 | 764 | 570983 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.473 | 3939 | 608550 |

Aggregate elapsed times for all five processes: P1=6.502 ms, P2=6.473 ms, P3=6.472 ms, P4=6.454 ms, P5=6.528 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.699 | 28.699 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.707 | 28.707 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.820 | 72.820 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.820 | 72.820 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.910 | 72.910 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.910 | 72.910 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.914 | 72.914 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.914 | 72.914 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.914 | 72.914 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.914 | 72.914 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.914 | 72.914 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.914 | 72.914 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |
| final process state | 72.984 | 72.984 | 2796 | 0 | 1241 | 230 |

## perf-run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.927 | 495 | 533898 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.606 | 1658 | 636178 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.858 | 455 | 530273 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.400 | 340 | 850814 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.359 | 227 | 632287 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.381 | 764 | 553356 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.531 | 3939 | 603153 |

Aggregate elapsed times for all five processes: P1=6.531 ms, P2=6.576 ms, P3=6.761 ms, P4=6.480 ms, P5=6.456 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.664 | 28.664 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.672 | 28.672 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.789 | 72.789 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.789 | 72.789 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.875 | 72.875 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.875 | 72.875 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.879 | 72.879 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.879 | 72.879 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.879 | 72.879 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.879 | 72.879 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.879 | 72.879 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.879 | 72.879 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |
| final process state | 72.949 | 72.949 | 2796 | 0 | 1241 | 230 |

## perf-run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.926 | 495 | 534516 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.603 | 1658 | 637057 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.856 | 455 | 531578 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.411 | 340 | 827701 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.364 | 227 | 622791 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.339 | 764 | 570395 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.499 | 3939 | 606066 |

Aggregate elapsed times for all five processes: P1=6.425 ms, P2=6.499 ms, P3=6.517 ms, P4=6.626 ms, P5=6.454 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.730 | 28.730 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.785 | 28.785 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.898 | 72.898 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.898 | 72.898 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.988 | 72.988 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.988 | 72.988 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.992 | 72.992 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.992 | 72.992 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.992 | 72.992 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.992 | 72.992 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.992 | 72.992 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.992 | 72.992 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| final process state | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |

## perf-run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.933 | 495 | 530586 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.604 | 1658 | 636688 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.858 | 455 | 530424 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.392 | 340 | 866562 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.357 | 227 | 635640 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.345 | 764 | 567946 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.490 | 3939 | 606980 |

Aggregate elapsed times for all five processes: P1=6.442 ms, P2=6.502 ms, P3=6.462 ms, P4=6.519 ms, P5=6.490 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.746 | 28.746 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.754 | 28.754 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.867 | 72.867 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.867 | 72.867 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.957 | 72.957 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.957 | 72.957 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.961 | 72.961 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.961 | 72.961 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.961 | 72.961 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.961 | 72.961 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.961 | 72.961 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.961 | 72.961 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| final process state | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |

## perf-run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.930 | 495 | 532125 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.615 | 1658 | 634064 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.855 | 455 | 531939 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.391 | 340 | 868898 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.362 | 227 | 627059 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.360 | 764 | 561753 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.514 | 3939 | 604715 |

Aggregate elapsed times for all five processes: P1=6.500 ms, P2=6.474 ms, P3=6.514 ms, P4=6.516 ms, P5=6.522 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.652 | 28.652 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.723 | 28.723 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.902 | 72.902 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.902 | 72.902 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.988 | 72.988 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.988 | 72.988 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.992 | 72.992 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.992 | 72.992 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.992 | 72.992 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.992 | 72.992 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.992 | 72.992 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.992 | 72.992 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |
| final process state | 73.062 | 73.062 | 2796 | 0 | 1241 | 230 |

## perf-run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.933 | 495 | 530409 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.610 | 1658 | 635317 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.863 | 455 | 526979 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.399 | 340 | 851586 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.359 | 227 | 632814 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.344 | 764 | 568583 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.508 | 3939 | 605252 |

Aggregate elapsed times for all five processes: P1=6.557 ms, P2=6.546 ms, P3=6.457 ms, P4=6.508 ms, P5=6.485 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.746 | 28.746 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.754 | 28.754 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.867 | 72.867 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.867 | 72.867 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.957 | 72.957 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.957 | 72.957 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.961 | 72.961 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.961 | 72.961 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.961 | 72.961 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.961 | 72.961 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.961 | 72.961 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.961 | 72.961 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |
| final process state | 73.031 | 73.031 | 2796 | 0 | 1241 | 230 |

## profile

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.988 | 495 | 500989 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.682 | 1658 | 618123 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.876 | 455 | 519443 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.388 | 340 | 876080 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.371 | 227 | 612546 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.364 | 764 | 560132 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.669 | 3939 | 590649 |

Aggregate elapsed times for all five processes: P1=6.687 ms, P2=6.556 ms, P3=6.725 ms, P4=6.669 ms, P5=6.570 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.730 | 28.730 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.738 | 28.738 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.852 | 72.852 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.852 | 72.852 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.941 | 72.941 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.941 | 72.941 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.945 | 72.945 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.945 | 72.945 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.945 | 72.945 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.945 | 72.945 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.945 | 72.945 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.945 | 72.945 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |
| final process state | 73.016 | 73.016 | 2796 | 0 | 1241 | 230 |

## Stage 3 16Byte Move

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.897 | 495 | 552068 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.540 | 1658 | 652713 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.850 | 455 | 535423 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.387 | 340 | 879477 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.360 | 227 | 630697 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.284 | 764 | 595178 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.317 | 3939 | 623580 |

Aggregate elapsed times for all five processes: P1=6.472 ms, P2=6.246 ms, P3=8.227 ms, P4=6.317 ms, P5=6.177 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.316 | 28.316 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.273 | 72.273 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.273 | 72.273 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.328 | 72.328 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.328 | 72.328 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.328 | 72.328 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.328 | 72.328 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.328 | 72.328 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.328 | 72.328 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.328 | 72.328 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| final process state | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |

## Stage 3 Run 1

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.913 | 495 | 542354 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.503 | 1658 | 662279 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.813 | 455 | 559833 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.389 | 340 | 873842 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.353 | 227 | 642275 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.307 | 764 | 584366 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.279 | 3939 | 627347 |

Aggregate elapsed times for all five processes: P1=6.481 ms, P2=6.326 ms, P3=6.279 ms, P4=6.256 ms, P5=6.201 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.266 | 28.266 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.227 | 72.227 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.227 | 72.227 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.277 | 72.277 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.277 | 72.277 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.277 | 72.277 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.277 | 72.277 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.277 | 72.277 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.277 | 72.277 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.277 | 72.277 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.277 | 72.277 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.328 | 72.328 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.328 | 72.328 | 2796 | 0 | 1241 | 230 |
| final process state | 72.328 | 72.328 | 2796 | 0 | 1241 | 230 |

## Stage 3 Run 2

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.892 | 495 | 555157 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.531 | 1658 | 655159 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.852 | 455 | 533785 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.382 | 340 | 890140 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.364 | 227 | 622984 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.309 | 764 | 583503 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.330 | 3939 | 622236 |

Aggregate elapsed times for all five processes: P1=6.330 ms, P2=6.381 ms, P3=6.230 ms, P4=6.345 ms, P5=6.292 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.301 | 28.301 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.309 | 28.309 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.262 | 72.262 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.262 | 72.262 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.316 | 72.316 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.316 | 72.316 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.316 | 72.316 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.316 | 72.316 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.316 | 72.316 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.316 | 72.316 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.316 | 72.316 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.316 | 72.316 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.363 | 72.363 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.363 | 72.363 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2796 | 0 | 1241 | 230 |
| final process state | 72.363 | 72.363 | 2796 | 0 | 1241 | 230 |

## Stage 3 Run 3

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.935 | 495 | 529317 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.521 | 1658 | 657749 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.810 | 455 | 561822 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.390 | 340 | 870834 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.353 | 227 | 642719 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.280 | 764 | 596975 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.289 | 3939 | 626317 |

Aggregate elapsed times for all five processes: P1=6.363 ms, P2=6.282 ms, P3=6.289 ms, P4=6.274 ms, P5=6.376 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.266 | 28.266 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.227 | 72.227 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.227 | 72.227 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.277 | 72.277 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.277 | 72.277 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.277 | 72.277 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.277 | 72.277 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.277 | 72.277 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.277 | 72.277 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.277 | 72.277 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.277 | 72.277 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |
| final process state | 72.324 | 72.324 | 2796 | 0 | 1241 | 230 |

## Stage 3 Run 4

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.908 | 495 | 545193 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.520 | 1658 | 658016 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.825 | 455 | 551189 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.377 | 340 | 901981 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.352 | 227 | 644375 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.306 | 764 | 584907 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.289 | 3939 | 626378 |

Aggregate elapsed times for all five processes: P1=6.289 ms, P2=6.281 ms, P3=7.098 ms, P4=6.314 ms, P5=6.195 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.281 | 28.281 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.234 | 72.234 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.234 | 72.234 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.289 | 72.289 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.289 | 72.289 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.289 | 72.289 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.289 | 72.289 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.289 | 72.289 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.289 | 72.289 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.289 | 72.289 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.289 | 72.289 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.332 | 72.332 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.336 | 72.336 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.336 | 72.336 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.336 | 72.336 | 2796 | 0 | 1241 | 230 |
| final process state | 72.336 | 72.336 | 2796 | 0 | 1241 | 230 |

## Stage 3 Run 5

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.900 | 495 | 550245 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.494 | 1658 | 664816 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.810 | 455 | 561478 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.374 | 340 | 908017 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.353 | 227 | 642378 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.321 | 764 | 578508 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.252 | 3939 | 630004 |

Aggregate elapsed times for all five processes: P1=6.263 ms, P2=6.200 ms, P3=6.308 ms, P4=6.249 ms, P5=6.252 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.258 | 28.258 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.266 | 28.266 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.215 | 72.215 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.215 | 72.215 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.270 | 72.270 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.270 | 72.270 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.270 | 72.270 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.270 | 72.270 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.270 | 72.270 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.270 | 72.270 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.270 | 72.270 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.270 | 72.270 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| final process state | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |

## Stage 3 perf

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.907 | 495 | 545947 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.530 | 1658 | 655222 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.855 | 455 | 532158 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.383 | 340 | 888366 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.364 | 227 | 623492 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.273 | 764 | 600117 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.312 | 3939 | 624047 |

Aggregate elapsed times for all five processes: P1=6.457 ms, P2=6.360 ms, P3=6.215 ms, P4=6.239 ms, P5=6.312 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.277 | 72.277 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.277 | 72.277 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.332 | 72.332 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.332 | 72.332 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.332 | 72.332 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.332 | 72.332 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.332 | 72.332 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.332 | 72.332 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.332 | 72.332 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.375 | 72.375 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| final process state | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |

## Stage 3 perf

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.880 | 495 | 562613 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.553 | 1658 | 649549 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.859 | 455 | 529559 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.390 | 340 | 871940 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.353 | 227 | 643254 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.296 | 764 | 589470 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.330 | 3939 | 622228 |

Aggregate elapsed times for all five processes: P1=6.247 ms, P2=6.330 ms, P3=6.336 ms, P4=6.395 ms, P5=6.202 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.281 | 72.281 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.281 | 72.281 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.336 | 72.336 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.336 | 72.336 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.336 | 72.336 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.336 | 72.336 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.336 | 72.336 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.336 | 72.336 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.336 | 72.336 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.336 | 72.336 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.379 | 72.379 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.383 | 72.383 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.383 | 72.383 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.383 | 72.383 | 2796 | 0 | 1241 | 230 |
| final process state | 72.383 | 72.383 | 2796 | 0 | 1241 | 230 |

## Stage 3 perf

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.902 | 495 | 548922 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.482 | 1658 | 668097 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.817 | 455 | 556950 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.400 | 340 | 849403 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.346 | 227 | 655122 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.273 | 764 | 600332 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.220 | 3939 | 633300 |

Aggregate elapsed times for all five processes: P1=6.196 ms, P2=6.290 ms, P3=6.196 ms, P4=6.220 ms, P5=6.365 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.250 | 28.250 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.262 | 28.262 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.211 | 72.211 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.211 | 72.211 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.266 | 72.266 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.266 | 72.266 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.266 | 72.266 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.266 | 72.266 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.266 | 72.266 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.266 | 72.266 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.266 | 72.266 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.266 | 72.266 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.309 | 72.309 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |
| final process state | 72.312 | 72.312 | 2796 | 0 | 1241 | 230 |

## Stage 3 perf

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.896 | 495 | 552353 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.506 | 1658 | 661723 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.839 | 455 | 542157 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.383 | 340 | 888004 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.353 | 227 | 642487 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.311 | 764 | 582855 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.288 | 3939 | 626434 |

Aggregate elapsed times for all five processes: P1=6.262 ms, P2=6.288 ms, P3=6.363 ms, P4=6.280 ms, P5=6.315 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.312 | 28.312 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.266 | 72.266 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.266 | 72.266 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.316 | 72.316 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.316 | 72.316 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.316 | 72.316 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.316 | 72.316 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.316 | 72.316 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.316 | 72.316 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.316 | 72.316 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.316 | 72.316 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.359 | 72.359 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.363 | 72.363 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.363 | 72.363 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2796 | 0 | 1241 | 230 |
| final process state | 72.363 | 72.363 | 2796 | 0 | 1241 | 230 |

## Stage 3 perf

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.924 | 495 | 535789 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.497 | 1658 | 663924 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.806 | 455 | 564799 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.369 | 340 | 921589 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.354 | 227 | 641047 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.283 | 764 | 595451 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.233 | 3939 | 631975 |

Aggregate elapsed times for all five processes: P1=6.233 ms, P2=6.277 ms, P3=6.322 ms, P4=6.217 ms, P5=6.224 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.262 | 28.262 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.270 | 28.270 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.223 | 72.223 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.223 | 72.223 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.273 | 72.273 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.273 | 72.273 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.273 | 72.273 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.273 | 72.273 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.273 | 72.273 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.273 | 72.273 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.273 | 72.273 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.273 | 72.273 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.316 | 72.316 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |
| final process state | 72.320 | 72.320 | 2796 | 0 | 1241 | 230 |

## profile_stage3

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.910 | 495 | 543665 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.524 | 1658 | 656908 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.838 | 455 | 543281 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.393 | 340 | 865820 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.360 | 227 | 629857 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.286 | 764 | 593884 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.311 | 3939 | 624102 |

Aggregate elapsed times for all five processes: P1=6.241 ms, P2=6.311 ms, P3=6.433 ms, P4=6.327 ms, P5=6.215 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.312 | 28.312 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.273 | 72.273 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.273 | 72.273 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.324 | 72.324 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.324 | 72.324 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.324 | 72.324 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.324 | 72.324 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.324 | 72.324 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.324 | 72.324 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.324 | 72.324 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.324 | 72.324 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.367 | 72.367 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |
| final process state | 72.371 | 72.371 | 2796 | 0 | 1241 | 230 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.952 | 495 | 520153 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.564 | 1658 | 646692 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.841 | 455 | 541335 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.390 | 340 | 872633 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.376 | 227 | 604413 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.321 | 764 | 578457 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.442 | 3939 | 611463 |

Aggregate elapsed times for all five processes: P1=6.644 ms, P2=7.237 ms, P3=6.442 ms, P4=6.376 ms, P5=6.392 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.234 | 28.234 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.242 | 28.242 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.195 | 72.195 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.195 | 72.195 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.250 | 72.250 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.250 | 72.250 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.250 | 72.250 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.250 | 72.250 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.250 | 72.250 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.250 | 72.250 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.250 | 72.250 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.250 | 72.250 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.293 | 72.293 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.297 | 72.297 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.297 | 72.297 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.297 | 72.297 | 2796 | 0 | 1241 | 230 |
| final process state | 72.297 | 72.297 | 2796 | 0 | 1241 | 230 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.898 | 495 | 551493 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.537 | 1658 | 653641 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.833 | 455 | 546469 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.380 | 340 | 895116 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.364 | 227 | 624406 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.283 | 764 | 595283 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.294 | 3939 | 625879 |

Aggregate elapsed times for all five processes: P1=6.294 ms, P2=6.348 ms, P3=6.222 ms, P4=6.201 ms, P5=6.389 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.227 | 28.227 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.234 | 28.234 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.305 | 72.305 | 460 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.305 | 72.305 | 460 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.355 | 72.355 | 1660 | 0 | 778 | 159 |
| warmup loaded: King safety | 72.355 | 72.355 | 1660 | 0 | 778 | 159 |
| warmup after search: King safety | 72.355 | 72.355 | 1952 | 0 | 909 | 189 |
| warmup loaded: Endgame | 72.355 | 72.355 | 1952 | 0 | 909 | 189 |
| warmup after search: Endgame | 72.355 | 72.355 | 2169 | 0 | 954 | 196 |
| warmup loaded: Promotion tactic | 72.355 | 72.355 | 2169 | 0 | 954 | 196 |
| warmup after search: Promotion tactic | 72.355 | 72.355 | 2287 | 0 | 1056 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.355 | 72.355 | 2287 | 0 | 1056 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| after complete warmup | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured loaded: Quiet middlegame | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured after search: Quiet middlegame | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured loaded: Kiwipete | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured after search: Kiwipete | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured loaded: King safety | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured after search: King safety | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured loaded: Endgame | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured after search: Endgame | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured loaded: Promotion tactic | 72.402 | 72.402 | 2796 | 0 | 1241 | 230 |
| measured after search: Promotion tactic | 72.406 | 72.406 | 2796 | 0 | 1241 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.406 | 72.406 | 2796 | 0 | 1241 | 230 |
| measured after search: Advanced pawns/check evasion | 72.406 | 72.406 | 2796 | 0 | 1241 | 230 |
| final process state | 72.406 | 72.406 | 2796 | 0 | 1241 | 230 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.962 | 494 | 513273 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.614 | 1658 | 634328 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.884 | 455 | 514572 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.409 | 323 | 789709 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.360 | 227 | 630046 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.360 | 795 | 584538 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.590 | 3952 | 599713 |

Aggregate elapsed times for all five processes: P1=6.615 ms, P2=6.590 ms, P3=8.009 ms, P4=6.463 ms, P5=6.538 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.316 | 28.316 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.301 | 72.301 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.301 | 72.301 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.301 | 72.301 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.301 | 72.301 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.301 | 72.301 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.301 | 72.301 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.301 | 72.301 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.301 | 72.301 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| after complete warmup | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured loaded: Quiet middlegame | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured after search: Quiet middlegame | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured loaded: Kiwipete | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured after search: Kiwipete | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured loaded: King safety | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured after search: King safety | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured loaded: Endgame | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured after search: Endgame | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured loaded: Promotion tactic | 72.336 | 72.336 | 2788 | 0 | 1250 | 230 |
| measured after search: Promotion tactic | 72.340 | 72.340 | 2788 | 0 | 1250 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.340 | 72.340 | 2788 | 0 | 1250 | 230 |
| measured after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2788 | 0 | 1250 | 230 |
| final process state | 72.340 | 72.340 | 2788 | 0 | 1250 | 230 |

## Stage 4 verification

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.949 | 494 | 520558 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.539 | 1658 | 652985 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.803 | 455 | 566303 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.386 | 323 | 835923 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.346 | 227 | 655430 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.342 | 795 | 592205 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.367 | 3952 | 620727 |

Aggregate elapsed times for all five processes: P1=7.040 ms, P2=6.367 ms, P3=6.337 ms, P4=6.451 ms, P5=6.321 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.336 | 28.336 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.344 | 28.344 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.270 | 72.270 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.270 | 72.270 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.320 | 72.320 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.320 | 72.320 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.320 | 72.320 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.320 | 72.320 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.320 | 72.320 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.320 | 72.320 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.320 | 72.320 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| after complete warmup | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured loaded: Quiet middlegame | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured after search: Quiet middlegame | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured loaded: Kiwipete | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured after search: Kiwipete | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured loaded: King safety | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured after search: King safety | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured loaded: Endgame | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured after search: Endgame | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured loaded: Promotion tactic | 72.355 | 72.355 | 2788 | 0 | 1250 | 230 |
| measured after search: Promotion tactic | 72.359 | 72.359 | 2788 | 0 | 1250 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.359 | 72.359 | 2788 | 0 | 1250 | 230 |
| measured after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2788 | 0 | 1250 | 230 |
| final process state | 72.359 | 72.359 | 2788 | 0 | 1250 | 230 |

## Stage 4 Baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.896 | 494 | 551643 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.489 | 1658 | 666259 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.802 | 455 | 567495 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.403 | 323 | 802408 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.366 | 227 | 619712 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.340 | 795 | 593471 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.294 | 3952 | 627879 |

Aggregate elapsed times for all five processes: P1=6.368 ms, P2=6.240 ms, P3=6.319 ms, P4=6.254 ms, P5=6.294 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.199 | 28.199 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.207 | 28.207 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.195 | 72.195 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.195 | 72.195 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.242 | 72.242 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.242 | 72.242 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.242 | 72.242 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.242 | 72.242 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.242 | 72.242 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.242 | 72.242 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.242 | 72.242 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.242 | 72.242 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| after complete warmup | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured loaded: Quiet middlegame | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured after search: Quiet middlegame | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured loaded: Kiwipete | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured after search: Kiwipete | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured loaded: King safety | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured after search: King safety | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured loaded: Endgame | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured after search: Endgame | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured loaded: Promotion tactic | 72.281 | 72.281 | 2788 | 0 | 1250 | 230 |
| measured after search: Promotion tactic | 72.285 | 72.285 | 2788 | 0 | 1250 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.285 | 72.285 | 2788 | 0 | 1250 | 230 |
| measured after search: Advanced pawns/check evasion | 72.285 | 72.285 | 2788 | 0 | 1250 | 230 |
| final process state | 72.285 | 72.285 | 2788 | 0 | 1250 | 230 |

## Baseline Stage 4

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.913 | 494 | 541344 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.498 | 1658 | 663660 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.814 | 455 | 558800 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.363 | 323 | 889522 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.341 | 227 | 665845 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.383 | 795 | 574988 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.312 | 3952 | 626136 |

Aggregate elapsed times for all five processes: P1=7.115 ms, P2=6.764 ms, P3=6.312 ms, P4=6.279 ms, P5=6.259 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| after complete warmup | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured loaded: King safety | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured after search: King safety | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured loaded: Endgame | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured after search: Endgame | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2788 | 0 | 1250 | 230 |
| measured after search: Promotion tactic | 72.348 | 72.348 | 2788 | 0 | 1250 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2788 | 0 | 1250 | 230 |
| measured after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2788 | 0 | 1250 | 230 |
| final process state | 72.348 | 72.348 | 2788 | 0 | 1250 | 230 |

## Perf Test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.899 | 494 | 549765 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.549 | 1658 | 650405 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.832 | 455 | 546567 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.369 | 323 | 874852 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.353 | 227 | 643942 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.353 | 795 | 587634 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.355 | 3952 | 621890 |

Aggregate elapsed times for all five processes: P1=6.672 ms, P2=6.307 ms, P3=6.412 ms, P4=6.355 ms, P5=6.304 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.289 | 28.289 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.223 | 72.223 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.223 | 72.223 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.270 | 72.270 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.270 | 72.270 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.270 | 72.270 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.270 | 72.270 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.270 | 72.270 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.270 | 72.270 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.270 | 72.270 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.270 | 72.270 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| after complete warmup | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured loaded: Quiet middlegame | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured after search: Quiet middlegame | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured loaded: Kiwipete | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured after search: Kiwipete | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured loaded: King safety | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured after search: King safety | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured loaded: Endgame | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured after search: Endgame | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured loaded: Promotion tactic | 72.305 | 72.305 | 2788 | 0 | 1250 | 230 |
| measured after search: Promotion tactic | 72.309 | 72.309 | 2788 | 0 | 1250 | 230 |
| measured loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2788 | 0 | 1250 | 230 |
| measured after search: Advanced pawns/check evasion | 72.309 | 72.309 | 2788 | 0 | 1250 | 230 |
| final process state | 72.309 | 72.309 | 2788 | 0 | 1250 | 230 |

## ASAN verification

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 2.416 | 494 | 204453 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 5.631 | 2042 | 362658 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 2.079 | 455 | 218868 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 1.226 | 323 | 263378 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 1.092 | 227 | 207962 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 3.262 | 875 | 268223 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 15.706 | 4416 | 281169 |

Aggregate elapsed times for all five processes: P1=15.751 ms, P2=15.766 ms, P3=15.658 ms, P4=15.706 ms, P5=15.542 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 34.441 | 34.523 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 34.641 | 34.652 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 86.543 | 86.562 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 86.609 | 86.621 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 87.379 | 87.391 | 1659 | 0 | 773 | 159 |
| warmup loaded: King safety | 87.414 | 87.426 | 1659 | 0 | 773 | 159 |
| warmup after search: King safety | 87.703 | 87.715 | 1951 | 0 | 904 | 189 |
| warmup loaded: Endgame | 87.738 | 87.758 | 1951 | 0 | 904 | 189 |
| warmup after search: Endgame | 88.070 | 88.082 | 2155 | 0 | 948 | 196 |
| warmup loaded: Promotion tactic | 88.105 | 88.121 | 2155 | 0 | 948 | 196 |
| warmup after search: Promotion tactic | 88.328 | 88.340 | 2273 | 0 | 1051 | 207 |
| warmup loaded: Advanced pawns/check evasion | 88.359 | 88.371 | 2273 | 0 | 1051 | 207 |
| warmup after search: Advanced pawns/check evasion | 88.906 | 88.922 | 2853 | 0 | 1265 | 233 |
| after complete warmup | 88.934 | 88.945 | 2853 | 0 | 1265 | 233 |
| measured loaded: Quiet middlegame | 88.980 | 88.992 | 2853 | 0 | 1265 | 233 |
| measured after search: Quiet middlegame | 89.246 | 89.258 | 2853 | 0 | 1265 | 233 |
| measured loaded: Kiwipete | 89.324 | 89.348 | 2853 | 0 | 1265 | 233 |
| measured after search: Kiwipete | 89.609 | 89.621 | 2853 | 0 | 1265 | 233 |
| measured loaded: King safety | 89.672 | 89.684 | 2853 | 0 | 1265 | 233 |
| measured after search: King safety | 89.805 | 89.820 | 2853 | 0 | 1265 | 233 |
| measured loaded: Endgame | 89.863 | 89.879 | 2853 | 0 | 1265 | 233 |
| measured after search: Endgame | 89.965 | 89.973 | 2853 | 0 | 1265 | 233 |
| measured loaded: Promotion tactic | 90.004 | 90.016 | 2853 | 0 | 1265 | 233 |
| measured after search: Promotion tactic | 90.113 | 90.125 | 2853 | 0 | 1265 | 233 |
| measured loaded: Advanced pawns/check evasion | 90.145 | 90.164 | 2853 | 0 | 1265 | 233 |
| measured after search: Advanced pawns/check evasion | 90.316 | 90.336 | 2853 | 0 | 1265 | 233 |
| final process state | 90.348 | 90.359 | 2853 | 0 | 1265 | 233 |

## Stage 5 Optimized SEE

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.767 | 494 | 644180 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.121 | 2042 | 962688 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.706 | 455 | 644554 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.365 | 323 | 885686 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.333 | 227 | 682413 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.262 | 875 | 693424 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.553 | 4416 | 795230 |

Aggregate elapsed times for all five processes: P1=5.936 ms, P2=5.532 ms, P3=5.553 ms, P4=5.534 ms, P5=5.560 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.355 | 28.355 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.332 | 72.332 | 1659 | 0 | 773 | 159 |
| warmup loaded: King safety | 72.332 | 72.332 | 1659 | 0 | 773 | 159 |
| warmup after search: King safety | 72.332 | 72.332 | 1951 | 0 | 904 | 189 |
| warmup loaded: Endgame | 72.332 | 72.332 | 1951 | 0 | 904 | 189 |
| warmup after search: Endgame | 72.332 | 72.332 | 2155 | 0 | 948 | 196 |
| warmup loaded: Promotion tactic | 72.332 | 72.332 | 2155 | 0 | 948 | 196 |
| warmup after search: Promotion tactic | 72.332 | 72.332 | 2273 | 0 | 1051 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2273 | 0 | 1051 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| after complete warmup | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured loaded: Quiet middlegame | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured after search: Quiet middlegame | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured loaded: Kiwipete | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured after search: Kiwipete | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured loaded: King safety | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured after search: King safety | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured loaded: Endgame | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured after search: Endgame | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured loaded: Promotion tactic | 72.367 | 72.367 | 2853 | 0 | 1265 | 233 |
| measured after search: Promotion tactic | 72.371 | 72.371 | 2853 | 0 | 1265 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.371 | 72.371 | 2853 | 0 | 1265 | 233 |
| measured after search: Advanced pawns/check evasion | 72.371 | 72.371 | 2853 | 0 | 1265 | 233 |
| final process state | 72.371 | 72.371 | 2853 | 0 | 1265 | 233 |

## Test Exact Match

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.821 | 494 | 601584 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.218 | 1658 | 747453 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.769 | 455 | 591488 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.370 | 323 | 872107 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.349 | 227 | 650819 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.353 | 875 | 646649 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.881 | 4032 | 685609 |

Aggregate elapsed times for all five processes: P1=5.890 ms, P2=5.925 ms, P3=5.878 ms, P4=5.881 ms, P5=5.848 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.246 | 28.246 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.254 | 28.254 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.180 | 72.180 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.180 | 72.180 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.230 | 72.230 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.230 | 72.230 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.230 | 72.230 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.230 | 72.230 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.230 | 72.230 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.230 | 72.230 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.230 | 72.230 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.230 | 72.230 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.266 | 72.266 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| final process state | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |

## Bench fast SeePosition

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.786 | 494 | 628861 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.100 | 1658 | 789529 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.720 | 455 | 631811 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.362 | 323 | 891851 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.339 | 227 | 668675 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.291 | 875 | 678010 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.598 | 4032 | 720273 |

Aggregate elapsed times for all five processes: P1=5.598 ms, P2=5.595 ms, P3=5.539 ms, P4=5.615 ms, P5=5.632 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.312 | 28.312 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.246 | 72.246 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.246 | 72.246 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.293 | 72.293 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.293 | 72.293 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.293 | 72.293 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.293 | 72.293 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.293 | 72.293 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.293 | 72.293 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.293 | 72.293 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.293 | 72.293 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| final process state | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |

## run_0

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.897 | 494 | 550690 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.188 | 1658 | 757659 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.748 | 455 | 608619 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.372 | 323 | 868057 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.346 | 227 | 656663 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.305 | 875 | 670276 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.856 | 4032 | 688503 |

Aggregate elapsed times for all five processes: P1=5.955 ms, P2=5.765 ms, P3=5.937 ms, P4=5.856 ms, P5=5.807 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.340 | 28.340 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| final process state | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |

## run_1

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.819 | 494 | 602837 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.173 | 1658 | 763172 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.746 | 455 | 609621 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.373 | 323 | 864899 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.345 | 227 | 657380 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.297 | 875 | 674451 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.754 | 4032 | 700675 |

Aggregate elapsed times for all five processes: P1=5.754 ms, P2=5.605 ms, P3=5.681 ms, P4=5.914 ms, P5=6.186 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.281 | 28.281 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.254 | 72.254 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.254 | 72.254 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| final process state | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |

## run_2

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.905 | 494 | 546106 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.198 | 1658 | 754402 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.755 | 455 | 602312 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.379 | 323 | 851528 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.351 | 227 | 647184 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.303 | 875 | 671632 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.891 | 4032 | 684476 |

Aggregate elapsed times for all five processes: P1=6.057 ms, P2=5.762 ms, P3=5.965 ms, P4=5.809 ms, P5=5.891 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.289 | 28.289 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.277 | 72.277 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.277 | 72.277 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.277 | 72.277 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.277 | 72.277 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.277 | 72.277 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.277 | 72.277 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.277 | 72.277 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.277 | 72.277 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| final process state | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |

## run_3

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.816 | 494 | 605061 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.179 | 1658 | 760836 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.785 | 455 | 579439 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.427 | 323 | 757151 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.381 | 227 | 595575 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.345 | 875 | 650724 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.933 | 4032 | 679558 |

Aggregate elapsed times for all five processes: P1=6.195 ms, P2=6.142 ms, P3=5.688 ms, P4=5.933 ms, P5=5.699 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.312 | 28.312 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.238 | 72.238 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.238 | 72.238 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.289 | 72.289 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.289 | 72.289 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.289 | 72.289 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.289 | 72.289 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.289 | 72.289 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.289 | 72.289 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.289 | 72.289 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.289 | 72.289 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| final process state | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |

## run_4

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.812 | 494 | 608081 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.148 | 1658 | 771967 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.728 | 455 | 624943 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.362 | 323 | 893074 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.340 | 227 | 668203 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.302 | 875 | 672090 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.692 | 4032 | 708423 |

Aggregate elapsed times for all five processes: P1=5.626 ms, P2=5.692 ms, P3=5.799 ms, P4=5.797 ms, P5=5.661 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.242 | 28.242 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.250 | 28.250 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.223 | 72.223 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.223 | 72.223 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.270 | 72.270 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.270 | 72.270 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.270 | 72.270 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.270 | 72.270 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.270 | 72.270 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.270 | 72.270 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.270 | 72.270 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.270 | 72.270 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.309 | 72.309 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.309 | 72.309 | 2853 | 0 | 1266 | 233 |
| final process state | 72.309 | 72.309 | 2853 | 0 | 1266 | 233 |

## run

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.797 | 494 | 619813 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.086 | 1658 | 794982 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.730 | 455 | 623463 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.371 | 323 | 871597 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.341 | 227 | 666126 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.284 | 875 | 681343 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.608 | 4032 | 718975 |

Aggregate elapsed times for all five processes: P1=5.730 ms, P2=5.608 ms, P3=5.602 ms, P4=5.605 ms, P5=5.639 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.258 | 28.258 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.254 | 72.254 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.254 | 72.254 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| final process state | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |

## asan_check

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 2.663 | 494 | 185494 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 5.552 | 1658 | 298634 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 2.108 | 455 | 215854 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 1.247 | 323 | 259023 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 1.144 | 227 | 198352 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 3.550 | 875 | 246480 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 16.264 | 4032 | 247903 |

Aggregate elapsed times for all five processes: P1=15.840 ms, P2=16.445 ms, P3=16.264 ms, P4=16.248 ms, P5=16.840 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 34.336 | 34.426 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 34.480 | 34.492 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 86.559 | 86.578 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 86.617 | 86.629 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 87.391 | 87.402 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 87.426 | 87.438 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 87.719 | 87.730 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 87.750 | 87.770 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 88.102 | 88.113 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 88.137 | 88.152 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 88.359 | 88.371 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 88.391 | 88.402 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 88.961 | 88.977 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 88.988 | 89.000 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 89.051 | 89.062 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 89.316 | 89.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 89.395 | 89.418 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 89.684 | 89.695 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 89.738 | 89.750 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 89.883 | 89.898 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 89.941 | 89.953 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 90.035 | 90.043 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 90.074 | 90.086 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 90.188 | 90.199 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 90.219 | 90.238 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 90.391 | 90.410 | 2853 | 0 | 1266 | 233 |
| final process state | 90.422 | 90.434 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.883 | 494 | 559184 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.134 | 1658 | 777007 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.743 | 455 | 612669 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.361 | 323 | 894055 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.338 | 227 | 671766 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.271 | 875 | 688510 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.730 | 4032 | 703669 |

Aggregate elapsed times for all five processes: P1=8.598 ms, P2=5.698 ms, P3=5.647 ms, P4=5.730 ms, P5=6.313 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| final process state | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.881 | 494 | 560606 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.162 | 1658 | 767059 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.766 | 455 | 593935 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.361 | 323 | 894196 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.346 | 227 | 655476 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.297 | 875 | 674827 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.813 | 4032 | 693626 |

Aggregate elapsed times for all five processes: P1=6.226 ms, P2=5.813 ms, P3=5.635 ms, P4=5.835 ms, P5=5.731 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.336 | 28.336 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.262 | 72.262 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.262 | 72.262 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.312 | 72.312 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.312 | 72.312 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.312 | 72.312 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.312 | 72.312 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.312 | 72.312 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.312 | 72.312 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.312 | 72.312 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.312 | 72.312 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.352 | 72.352 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.352 | 72.352 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.352 | 72.352 | 2853 | 0 | 1266 | 233 |
| final process state | 72.352 | 72.352 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.812 | 494 | 608184 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.093 | 1658 | 792007 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.733 | 455 | 620948 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.382 | 323 | 844925 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.362 | 227 | 626851 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.302 | 875 | 671979 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.685 | 4032 | 709241 |

Aggregate elapsed times for all five processes: P1=5.763 ms, P2=5.673 ms, P3=5.736 ms, P4=5.685 ms, P5=5.666 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| final process state | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.838 | 494 | 589578 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.092 | 1658 | 792419 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.708 | 455 | 642896 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.352 | 323 | 917835 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.327 | 227 | 693220 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.290 | 875 | 678174 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.608 | 4032 | 719030 |

Aggregate elapsed times for all five processes: P1=5.785 ms, P2=5.551 ms, P3=5.569 ms, P4=5.608 ms, P5=5.638 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.316 | 28.316 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.301 | 72.301 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.301 | 72.301 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.301 | 72.301 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.301 | 72.301 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.301 | 72.301 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.301 | 72.301 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.301 | 72.301 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.301 | 72.301 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| final process state | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.840 | 494 | 587770 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.128 | 1658 | 779273 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.757 | 455 | 601249 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.361 | 323 | 894100 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.333 | 227 | 680816 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.274 | 875 | 687042 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.693 | 4032 | 708225 |

Aggregate elapsed times for all five processes: P1=5.742 ms, P2=5.735 ms, P3=5.633 ms, P4=5.625 ms, P5=5.693 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.340 | 28.340 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.320 | 72.320 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.320 | 72.320 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.320 | 72.320 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.320 | 72.320 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.320 | 72.320 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.320 | 72.320 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.320 | 72.320 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| final process state | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.813 | 494 | 607660 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.110 | 1658 | 785637 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.752 | 455 | 605121 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.378 | 323 | 854314 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.343 | 227 | 661703 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.275 | 875 | 686290 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.671 | 4032 | 710940 |

Aggregate elapsed times for all five processes: P1=5.985 ms, P2=5.605 ms, P3=5.655 ms, P4=5.671 ms, P5=5.710 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| final process state | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.835 | 494 | 591594 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.094 | 1658 | 791941 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.709 | 455 | 641988 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.356 | 323 | 908433 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.329 | 227 | 689883 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.295 | 875 | 675515 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.617 | 4032 | 717787 |

Aggregate elapsed times for all five processes: P1=5.593 ms, P2=5.576 ms, P3=5.716 ms, P4=5.617 ms, P5=5.652 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.355 | 28.355 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.328 | 72.328 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.328 | 72.328 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.328 | 72.328 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.328 | 72.328 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.328 | 72.328 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.328 | 72.328 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.328 | 72.328 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| final process state | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.799 | 494 | 618463 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.069 | 1658 | 801504 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.814 | 455 | 558758 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.375 | 323 | 861471 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.340 | 227 | 667180 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.274 | 875 | 687004 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.670 | 4032 | 711049 |

Aggregate elapsed times for all five processes: P1=6.388 ms, P2=5.628 ms, P3=5.728 ms, P4=5.634 ms, P5=5.670 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.277 | 28.277 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.285 | 28.285 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.211 | 72.211 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.211 | 72.211 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.262 | 72.262 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.262 | 72.262 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.262 | 72.262 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.262 | 72.262 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.262 | 72.262 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.262 | 72.262 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.262 | 72.262 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.262 | 72.262 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| final process state | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.795 | 494 | 621108 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.098 | 1658 | 790412 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.723 | 455 | 629253 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.371 | 323 | 870730 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.354 | 227 | 640363 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.330 | 875 | 657740 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.672 | 4032 | 710882 |

Aggregate elapsed times for all five processes: P1=5.831 ms, P2=5.663 ms, P3=5.596 ms, P4=5.771 ms, P5=5.672 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.188 | 28.188 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.195 | 28.195 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.184 | 72.184 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.184 | 72.184 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.234 | 72.234 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.234 | 72.234 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.234 | 72.234 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.234 | 72.234 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.234 | 72.234 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.234 | 72.234 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.234 | 72.234 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.234 | 72.234 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.270 | 72.270 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| final process state | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |

## perf_test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.811 | 494 | 609139 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.090 | 1658 | 793259 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.721 | 455 | 630802 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.375 | 323 | 861246 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.336 | 227 | 674972 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.294 | 875 | 675972 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.628 | 4032 | 716395 |

Aggregate elapsed times for all five processes: P1=5.881 ms, P2=5.598 ms, P3=5.681 ms, P4=5.621 ms, P5=5.628 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.340 | 28.340 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.320 | 72.320 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.320 | 72.320 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.320 | 72.320 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.320 | 72.320 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.320 | 72.320 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.320 | 72.320 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.320 | 72.320 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| final process state | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |

## run_0

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 1.017 | 494 | 485573 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.231 | 1658 | 743094 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.776 | 455 | 586024 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.385 | 323 | 838416 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.341 | 227 | 665878 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.290 | 875 | 678431 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.041 | 4032 | 667453 |

Aggregate elapsed times for all five processes: P1=6.041 ms, P2=7.725 ms, P3=6.263 ms, P4=5.694 ms, P5=5.665 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.344 | 28.344 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.355 | 28.355 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.328 | 72.328 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.328 | 72.328 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.328 | 72.328 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.328 | 72.328 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.328 | 72.328 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.328 | 72.328 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.328 | 72.328 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| final process state | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |

## run_1

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.799 | 494 | 618577 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.200 | 1658 | 753497 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.718 | 455 | 633326 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.366 | 323 | 882788 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.331 | 227 | 684824 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.279 | 875 | 684126 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.694 | 4032 | 708138 |

Aggregate elapsed times for all five processes: P1=5.869 ms, P2=5.663 ms, P3=5.694 ms, P4=5.655 ms, P5=5.710 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.281 | 72.281 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.281 | 72.281 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.281 | 72.281 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.281 | 72.281 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.281 | 72.281 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.281 | 72.281 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.281 | 72.281 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.281 | 72.281 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |
| final process state | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |

## run_2

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.882 | 494 | 560307 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.140 | 1658 | 774645 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.719 | 455 | 632432 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.374 | 323 | 863391 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.337 | 227 | 673562 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.284 | 875 | 681205 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.737 | 4032 | 702801 |

Aggregate elapsed times for all five processes: P1=5.737 ms, P2=5.751 ms, P3=5.984 ms, P4=5.599 ms, P5=5.586 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.277 | 28.277 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.285 | 28.285 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.215 | 72.215 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.215 | 72.215 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.262 | 72.262 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.262 | 72.262 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.262 | 72.262 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.262 | 72.262 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.262 | 72.262 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.262 | 72.262 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.262 | 72.262 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.262 | 72.262 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| final process state | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |

## run_3

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.827 | 494 | 597434 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.093 | 1658 | 792276 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.715 | 455 | 636434 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.351 | 323 | 920172 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.329 | 227 | 689054 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.344 | 875 | 651175 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.659 | 4032 | 712534 |

Aggregate elapsed times for all five processes: P1=5.587 ms, P2=5.659 ms, P3=5.680 ms, P4=5.571 ms, P5=5.713 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.355 | 28.355 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.332 | 72.332 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.332 | 72.332 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.332 | 72.332 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.332 | 72.332 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.332 | 72.332 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.332 | 72.332 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.332 | 72.332 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.371 | 72.371 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.371 | 72.371 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.371 | 72.371 | 2853 | 0 | 1266 | 233 |
| final process state | 72.371 | 72.371 | 2853 | 0 | 1266 | 233 |

## run_4

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.797 | 494 | 620146 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.145 | 1658 | 773036 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.768 | 455 | 592707 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.360 | 323 | 898180 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.339 | 227 | 669353 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.282 | 875 | 682713 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.689 | 4032 | 708681 |

Aggregate elapsed times for all five processes: P1=5.654 ms, P2=5.689 ms, P3=5.880 ms, P4=5.587 ms, P5=5.707 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.355 | 28.355 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.332 | 72.332 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.332 | 72.332 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.332 | 72.332 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.332 | 72.332 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.332 | 72.332 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.332 | 72.332 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.332 | 72.332 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.371 | 72.371 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.371 | 72.371 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.371 | 72.371 | 2853 | 0 | 1266 | 233 |
| final process state | 72.371 | 72.371 | 2853 | 0 | 1266 | 233 |

## run_5

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.831 | 494 | 594235 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.108 | 1658 | 786563 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.717 | 455 | 634933 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.371 | 323 | 871244 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.329 | 227 | 690805 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.279 | 875 | 684088 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.634 | 4032 | 715623 |

Aggregate elapsed times for all five processes: P1=5.889 ms, P2=5.604 ms, P3=5.561 ms, P4=5.702 ms, P5=5.634 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.340 | 28.340 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| final process state | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |

## run_6

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.841 | 494 | 587161 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.107 | 1658 | 786994 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.723 | 455 | 628947 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.364 | 323 | 886330 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.331 | 227 | 685032 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.299 | 875 | 673404 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.667 | 4032 | 711527 |

Aggregate elapsed times for all five processes: P1=5.649 ms, P2=5.617 ms, P3=5.667 ms, P4=5.977 ms, P5=5.742 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.262 | 28.262 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.270 | 28.270 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| final process state | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |

## run_7

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.851 | 494 | 580683 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.139 | 1658 | 775199 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.731 | 455 | 622415 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.394 | 323 | 819451 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.342 | 227 | 663228 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.281 | 875 | 682998 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.738 | 4032 | 702672 |

Aggregate elapsed times for all five processes: P1=5.644 ms, P2=5.687 ms, P3=9.124 ms, P4=5.833 ms, P5=5.738 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.344 | 28.344 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.352 | 28.352 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| final process state | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |

## run_8

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.817 | 494 | 604620 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.084 | 1658 | 795749 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.714 | 455 | 636886 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.388 | 323 | 833352 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.336 | 227 | 675333 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.328 | 875 | 658926 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.667 | 4032 | 711529 |

Aggregate elapsed times for all five processes: P1=5.619 ms, P2=5.701 ms, P3=5.538 ms, P4=5.667 ms, P5=5.840 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.344 | 28.344 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.352 | 28.352 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.328 | 72.328 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.328 | 72.328 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.328 | 72.328 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.328 | 72.328 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.328 | 72.328 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.328 | 72.328 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.328 | 72.328 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |
| final process state | 72.367 | 72.367 | 2853 | 0 | 1266 | 233 |

## run_9

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.970 | 494 | 509414 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.241 | 1658 | 739957 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.805 | 455 | 565385 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.376 | 323 | 858686 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.358 | 227 | 634340 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.272 | 875 | 687840 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 6.021 | 4032 | 669625 |

Aggregate elapsed times for all five processes: P1=5.550 ms, P2=5.676 ms, P3=6.290 ms, P4=8.389 ms, P5=6.021 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.281 | 28.281 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.289 | 28.289 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.215 | 72.215 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.215 | 72.215 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.262 | 72.262 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.262 | 72.262 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.262 | 72.262 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.262 | 72.262 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.262 | 72.262 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.262 | 72.262 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.262 | 72.262 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.262 | 72.262 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| final process state | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |

## run_0

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.812 | 494 | 608215 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.145 | 1658 | 772918 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.823 | 455 | 552912 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.349 | 323 | 924643 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.349 | 227 | 649765 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.289 | 875 | 678957 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.768 | 4032 | 699069 |

Aggregate elapsed times for all five processes: P1=6.378 ms, P2=5.768 ms, P3=5.721 ms, P4=7.000 ms, P5=5.649 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.289 | 28.289 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.223 | 72.223 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.223 | 72.223 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.270 | 72.270 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.270 | 72.270 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.270 | 72.270 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.270 | 72.270 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.270 | 72.270 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.270 | 72.270 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.270 | 72.270 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.270 | 72.270 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.309 | 72.309 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.309 | 72.309 | 2853 | 0 | 1266 | 233 |
| final process state | 72.309 | 72.309 | 2853 | 0 | 1266 | 233 |

## run_1

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.881 | 494 | 560956 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.141 | 1658 | 774548 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.715 | 455 | 636466 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.373 | 323 | 865510 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.332 | 227 | 684576 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.265 | 875 | 691723 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.706 | 4032 | 706641 |

Aggregate elapsed times for all five processes: P1=5.644 ms, P2=5.676 ms, P3=5.792 ms, P4=5.777 ms, P5=5.706 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.281 | 72.281 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.281 | 72.281 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.281 | 72.281 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.281 | 72.281 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.281 | 72.281 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.281 | 72.281 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.281 | 72.281 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.281 | 72.281 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |
| final process state | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |

## run_2

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.887 | 494 | 556984 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.085 | 1658 | 795336 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.778 | 455 | 584865 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.367 | 323 | 879725 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.342 | 227 | 663085 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.267 | 875 | 690599 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.726 | 4032 | 704151 |

Aggregate elapsed times for all five processes: P1=5.726 ms, P2=5.789 ms, P3=5.786 ms, P4=5.708 ms, P5=5.683 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.316 | 28.316 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.297 | 72.297 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.297 | 72.297 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.297 | 72.297 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.297 | 72.297 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.297 | 72.297 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.297 | 72.297 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.297 | 72.297 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.297 | 72.297 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| final process state | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |

## run_3

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.836 | 494 | 590964 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.158 | 1658 | 768423 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.741 | 455 | 614121 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.366 | 323 | 881458 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.345 | 227 | 657389 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.280 | 875 | 683853 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.726 | 4032 | 704188 |

Aggregate elapsed times for all five processes: P1=5.776 ms, P2=5.726 ms, P3=5.604 ms, P4=5.550 ms, P5=6.517 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.281 | 28.281 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.254 | 72.254 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.254 | 72.254 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| final process state | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |

## run_4

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.785 | 494 | 629338 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.068 | 1658 | 801644 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.752 | 455 | 605198 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.385 | 323 | 838386 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.343 | 227 | 661541 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.272 | 875 | 688056 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.605 | 4032 | 719342 |

Aggregate elapsed times for all five processes: P1=5.605 ms, P2=5.576 ms, P3=5.730 ms, P4=5.587 ms, P5=5.724 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.309 | 28.309 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.316 | 28.316 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.289 | 72.289 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.289 | 72.289 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.340 | 72.340 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.340 | 72.340 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.340 | 72.340 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.340 | 72.340 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.340 | 72.340 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.340 | 72.340 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.340 | 72.340 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.340 | 72.340 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.375 | 72.375 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.379 | 72.379 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.379 | 72.379 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.379 | 72.379 | 2853 | 0 | 1266 | 233 |
| final process state | 72.379 | 72.379 | 2853 | 0 | 1266 | 233 |

## run_5

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.805 | 494 | 613843 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.075 | 1658 | 798981 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.742 | 455 | 613010 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.372 | 323 | 867866 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.349 | 227 | 650036 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.276 | 875 | 685835 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.619 | 4032 | 717520 |

Aggregate elapsed times for all five processes: P1=5.593 ms, P2=5.701 ms, P3=5.619 ms, P4=5.937 ms, P5=5.558 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.344 | 28.344 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.352 | 28.352 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| final process state | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |

## run_6

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.847 | 494 | 583511 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.136 | 1658 | 776164 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.739 | 455 | 615929 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.362 | 323 | 892206 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.336 | 227 | 675736 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.265 | 875 | 691962 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.684 | 4032 | 709367 |

Aggregate elapsed times for all five processes: P1=5.658 ms, P2=5.834 ms, P3=5.819 ms, P4=5.557 ms, P5=5.684 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.305 | 72.305 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.305 | 72.305 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.305 | 72.305 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.305 | 72.305 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.340 | 72.340 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| final process state | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |

## run_7

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.851 | 494 | 580300 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.154 | 1658 | 769738 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.738 | 455 | 616307 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.367 | 323 | 879454 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.337 | 227 | 673610 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.306 | 875 | 670078 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.754 | 4032 | 700777 |

Aggregate elapsed times for all five processes: P1=5.790 ms, P2=5.654 ms, P3=5.695 ms, P4=5.754 ms, P5=5.873 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| final process state | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |

## run_8

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.813 | 494 | 607762 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.135 | 1658 | 776515 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.719 | 455 | 632861 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.358 | 323 | 901386 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.340 | 227 | 668079 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.271 | 875 | 688698 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.636 | 4032 | 715453 |

Aggregate elapsed times for all five processes: P1=5.636 ms, P2=5.666 ms, P3=5.726 ms, P4=5.587 ms, P5=5.569 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.340 | 28.340 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.266 | 72.266 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.266 | 72.266 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.312 | 72.312 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.312 | 72.312 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.312 | 72.312 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.312 | 72.312 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.312 | 72.312 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.312 | 72.312 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.312 | 72.312 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.312 | 72.312 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.352 | 72.352 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.352 | 72.352 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.352 | 72.352 | 2853 | 0 | 1266 | 233 |
| final process state | 72.352 | 72.352 | 2853 | 0 | 1266 | 233 |

## run_9

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.916 | 494 | 539251 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.150 | 1658 | 771079 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.769 | 455 | 591996 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.364 | 323 | 887043 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.339 | 227 | 668748 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.276 | 875 | 685937 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.814 | 4032 | 693486 |

Aggregate elapsed times for all five processes: P1=5.666 ms, P2=5.814 ms, P3=6.116 ms, P4=6.107 ms, P5=5.635 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.250 | 28.250 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.258 | 28.258 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.188 | 72.188 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.188 | 72.188 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.238 | 72.238 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.238 | 72.238 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.238 | 72.238 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.238 | 72.238 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.238 | 72.238 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.238 | 72.238 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.238 | 72.238 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.238 | 72.238 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.273 | 72.273 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.277 | 72.277 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.277 | 72.277 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.277 | 72.277 | 2853 | 0 | 1266 | 233 |
| final process state | 72.277 | 72.277 | 2853 | 0 | 1266 | 233 |

## run_0

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.992 | 494 | 498124 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.205 | 1658 | 752022 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.744 | 455 | 611565 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.388 | 323 | 832907 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.367 | 227 | 618265 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.289 | 875 | 679077 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.984 | 4032 | 673808 |

Aggregate elapsed times for all five processes: P1=5.984 ms, P2=5.760 ms, P3=6.072 ms, P4=6.251 ms, P5=5.914 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.250 | 28.250 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.246 | 72.246 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.246 | 72.246 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.293 | 72.293 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.293 | 72.293 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.293 | 72.293 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.293 | 72.293 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.293 | 72.293 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.293 | 72.293 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.293 | 72.293 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.293 | 72.293 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| final process state | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |

## run_1

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.792 | 494 | 623965 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.151 | 1658 | 770734 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.754 | 455 | 603471 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.362 | 323 | 893439 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.347 | 227 | 653944 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.290 | 875 | 678515 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.695 | 4032 | 707976 |

Aggregate elapsed times for all five processes: P1=5.851 ms, P2=6.477 ms, P3=5.695 ms, P4=5.693 ms, P5=5.608 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.285 | 28.285 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.293 | 28.293 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.219 | 72.219 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.219 | 72.219 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.266 | 72.266 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.266 | 72.266 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.266 | 72.266 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.266 | 72.266 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.266 | 72.266 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.266 | 72.266 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.266 | 72.266 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.266 | 72.266 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.301 | 72.301 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |
| final process state | 72.305 | 72.305 | 2853 | 0 | 1266 | 233 |

## run_2

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.813 | 494 | 607613 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.145 | 1658 | 772943 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.728 | 455 | 625379 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.380 | 323 | 850960 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.341 | 227 | 666202 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.291 | 875 | 677915 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.697 | 4032 | 707784 |

Aggregate elapsed times for all five processes: P1=5.632 ms, P2=5.704 ms, P3=5.717 ms, P4=5.641 ms, P5=5.697 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.312 | 28.312 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.297 | 72.297 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.297 | 72.297 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.297 | 72.297 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.297 | 72.297 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.297 | 72.297 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.297 | 72.297 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.297 | 72.297 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.297 | 72.297 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| final process state | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |

## run_3

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.805 | 494 | 613508 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.083 | 1658 | 795907 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.714 | 455 | 637428 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.381 | 323 | 847673 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.336 | 227 | 675315 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.362 | 875 | 642641 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.681 | 4032 | 709744 |

Aggregate elapsed times for all five processes: P1=5.718 ms, P2=5.633 ms, P3=5.546 ms, P4=5.681 ms, P5=5.732 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.281 | 72.281 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.281 | 72.281 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.281 | 72.281 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.281 | 72.281 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.281 | 72.281 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.281 | 72.281 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.281 | 72.281 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.281 | 72.281 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |
| final process state | 72.320 | 72.320 | 2853 | 0 | 1266 | 233 |

## run_4

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.809 | 494 | 610327 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.183 | 1658 | 759664 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.707 | 455 | 643365 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.391 | 323 | 825628 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.409 | 227 | 555274 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.273 | 875 | 687474 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.772 | 4032 | 698549 |

Aggregate elapsed times for all five processes: P1=5.743 ms, P2=5.863 ms, P3=5.636 ms, P4=5.772 ms, P5=6.284 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.258 | 28.258 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.266 | 28.266 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.238 | 72.238 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.238 | 72.238 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.289 | 72.289 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.289 | 72.289 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.289 | 72.289 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.289 | 72.289 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.289 | 72.289 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.289 | 72.289 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.289 | 72.289 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.289 | 72.289 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| final process state | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |

## run_5

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.803 | 494 | 614919 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.064 | 1658 | 803131 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.769 | 455 | 591306 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.385 | 323 | 839615 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.345 | 227 | 658574 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.294 | 875 | 676418 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.660 | 4032 | 712339 |

Aggregate elapsed times for all five processes: P1=5.556 ms, P2=5.600 ms, P3=5.819 ms, P4=5.702 ms, P5=5.660 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.277 | 72.277 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.277 | 72.277 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.277 | 72.277 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.277 | 72.277 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.277 | 72.277 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.277 | 72.277 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.277 | 72.277 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.277 | 72.277 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| final process state | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |

## run_6

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.873 | 494 | 565610 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.068 | 1658 | 801660 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.781 | 455 | 582691 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.361 | 323 | 894305 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.356 | 227 | 637586 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.270 | 875 | 689123 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.709 | 4032 | 706204 |

Aggregate elapsed times for all five processes: P1=5.639 ms, P2=5.722 ms, P3=5.605 ms, P4=5.745 ms, P5=5.709 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.270 | 28.270 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.281 | 28.281 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.207 | 72.207 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.207 | 72.207 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.254 | 72.254 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.254 | 72.254 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.254 | 72.254 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.254 | 72.254 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.254 | 72.254 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.254 | 72.254 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.254 | 72.254 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.254 | 72.254 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.289 | 72.289 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| final process state | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |

## run_7

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.795 | 494 | 621647 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.118 | 1658 | 782986 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.749 | 455 | 607631 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.388 | 323 | 832435 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.344 | 227 | 659333 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.298 | 875 | 674004 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.692 | 4032 | 708422 |

Aggregate elapsed times for all five processes: P1=5.719 ms, P2=5.681 ms, P3=5.692 ms, P4=5.831 ms, P5=5.652 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.199 | 28.199 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.207 | 28.207 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.195 | 72.195 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.195 | 72.195 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.246 | 72.246 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.246 | 72.246 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.246 | 72.246 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.246 | 72.246 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.246 | 72.246 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.246 | 72.246 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.246 | 72.246 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.246 | 72.246 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.285 | 72.285 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.285 | 72.285 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.285 | 72.285 | 2853 | 0 | 1266 | 233 |
| final process state | 72.285 | 72.285 | 2853 | 0 | 1266 | 233 |

## run_8

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.846 | 494 | 583788 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.124 | 1658 | 780477 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.728 | 455 | 624768 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.361 | 323 | 895270 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.329 | 227 | 689661 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.287 | 875 | 679774 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.676 | 4032 | 710368 |

Aggregate elapsed times for all five processes: P1=5.676 ms, P2=5.750 ms, P3=5.636 ms, P4=5.658 ms, P5=5.783 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| final process state | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |

## run_9

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.909 | 494 | 543644 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.134 | 1658 | 776896 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.732 | 455 | 621214 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.365 | 323 | 886065 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.332 | 227 | 684365 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.295 | 875 | 675868 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.766 | 4032 | 699258 |

Aggregate elapsed times for all five processes: P1=5.854 ms, P2=5.808 ms, P3=5.589 ms, P4=5.766 ms, P5=5.696 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.312 | 28.312 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.238 | 72.238 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.238 | 72.238 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.289 | 72.289 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.289 | 72.289 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.289 | 72.289 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.289 | 72.289 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.289 | 72.289 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.289 | 72.289 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.289 | 72.289 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.289 | 72.289 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.324 | 72.324 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |
| final process state | 72.328 | 72.328 | 2853 | 0 | 1266 | 233 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.802 | 494 | 615906 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.109 | 1658 | 786194 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.722 | 455 | 630043 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.395 | 323 | 816739 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.336 | 227 | 674904 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.262 | 875 | 693150 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.627 | 4032 | 716506 |

Aggregate elapsed times for all five processes: P1=5.659 ms, P2=5.600 ms, P3=5.901 ms, P4=5.608 ms, P5=5.627 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.309 | 72.309 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.309 | 72.309 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.309 | 72.309 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |
| final process state | 72.348 | 72.348 | 2853 | 0 | 1266 | 233 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.797 | 494 | 619798 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.069 | 1658 | 801295 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.768 | 455 | 592442 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.358 | 323 | 903471 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.337 | 227 | 672648 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.293 | 875 | 676676 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.622 | 4032 | 717149 |

Aggregate elapsed times for all five processes: P1=5.641 ms, P2=5.548 ms, P3=5.622 ms, P4=5.588 ms, P5=5.723 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.234 | 28.234 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.242 | 28.242 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.230 | 72.230 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.277 | 72.277 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.277 | 72.277 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.277 | 72.277 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.277 | 72.277 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.277 | 72.277 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.277 | 72.277 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.277 | 72.277 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.277 | 72.277 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.312 | 72.312 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |
| final process state | 72.316 | 72.316 | 2853 | 0 | 1266 | 233 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.795 | 494 | 621623 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.068 | 1658 | 801773 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.707 | 455 | 643680 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.356 | 323 | 906926 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.324 | 227 | 701587 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.301 | 875 | 672684 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.550 | 4032 | 726494 |

Aggregate elapsed times for all five processes: P1=5.620 ms, P2=5.550 ms, P3=5.542 ms, P4=5.599 ms, P5=5.544 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.266 | 28.266 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.199 | 72.199 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.199 | 72.199 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.246 | 72.246 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.246 | 72.246 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.246 | 72.246 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.246 | 72.246 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.246 | 72.246 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.246 | 72.246 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.246 | 72.246 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.246 | 72.246 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.281 | 72.281 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.285 | 72.285 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.285 | 72.285 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.285 | 72.285 | 2853 | 0 | 1266 | 233 |
| final process state | 72.285 | 72.285 | 2853 | 0 | 1266 | 233 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.793 | 494 | 622984 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.091 | 1658 | 792800 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.710 | 455 | 641263 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.358 | 323 | 901892 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.327 | 227 | 693271 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.275 | 875 | 686509 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.554 | 4032 | 725970 |

Aggregate elapsed times for all five processes: P1=5.554 ms, P2=5.520 ms, P3=5.574 ms, P4=5.564 ms, P5=5.488 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.285 | 28.285 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.211 | 72.211 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.211 | 72.211 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.258 | 72.258 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.258 | 72.258 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.258 | 72.258 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.258 | 72.258 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.258 | 72.258 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.258 | 72.258 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.258 | 72.258 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.258 | 72.258 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.293 | 72.293 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |
| final process state | 72.297 | 72.297 | 2853 | 0 | 1266 | 233 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.793 | 494 | 623094 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.074 | 1658 | 799407 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.733 | 455 | 620753 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.377 | 323 | 856829 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.355 | 227 | 639085 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.274 | 875 | 686594 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.606 | 4032 | 719177 |

Aggregate elapsed times for all five processes: P1=5.643 ms, P2=5.545 ms, P3=5.606 ms, P4=5.571 ms, P5=5.617 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.281 | 28.281 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.352 | 28.352 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.324 | 72.324 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.324 | 72.324 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.324 | 72.324 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.324 | 72.324 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.359 | 72.359 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| final process state | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.804 | 494 | 614444 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.069 | 1658 | 801160 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.733 | 455 | 620702 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.362 | 323 | 892425 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.336 | 227 | 675593 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.257 | 875 | 695942 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.562 | 4032 | 724953 |

Aggregate elapsed times for all five processes: P1=5.507 ms, P2=5.542 ms, P3=5.667 ms, P4=5.562 ms, P5=5.578 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.316 | 28.316 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.297 | 72.297 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.297 | 72.297 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.297 | 72.297 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.297 | 72.297 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.297 | 72.297 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.297 | 72.297 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.297 | 72.297 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.297 | 72.297 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| final process state | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.844 | 494 | 585152 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.076 | 1658 | 798807 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.724 | 455 | 628429 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.365 | 323 | 885091 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.365 | 227 | 622204 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.297 | 875 | 674522 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.671 | 4032 | 711007 |

Aggregate elapsed times for all five processes: P1=5.600 ms, P2=5.676 ms, P3=5.593 ms, P4=5.867 ms, P5=5.671 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.312 | 28.312 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.246 | 72.246 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.246 | 72.246 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.297 | 72.297 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.297 | 72.297 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.297 | 72.297 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.297 | 72.297 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.297 | 72.297 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.297 | 72.297 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.297 | 72.297 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.297 | 72.297 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.332 | 72.332 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |
| final process state | 72.336 | 72.336 | 2853 | 0 | 1266 | 233 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.853 | 494 | 578889 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.122 | 1658 | 781510 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.735 | 455 | 618968 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.371 | 323 | 871004 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.325 | 227 | 698837 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.284 | 875 | 681723 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.689 | 4032 | 708716 |

Aggregate elapsed times for all five processes: P1=5.720 ms, P2=5.774 ms, P3=5.686 ms, P4=5.689 ms, P5=5.669 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.301 | 72.301 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.301 | 72.301 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.348 | 72.348 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.348 | 72.348 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.348 | 72.348 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.348 | 72.348 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.348 | 72.348 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.348 | 72.348 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.348 | 72.348 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.383 | 72.383 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.391 | 72.391 | 2853 | 0 | 1266 | 233 |
| final process state | 72.391 | 72.391 | 2853 | 0 | 1266 | 233 |

## test2

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.786 | 494 | 628424 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 2.044 | 1658 | 810974 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.730 | 455 | 622948 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.371 | 323 | 870965 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.330 | 227 | 687607 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.272 | 875 | 688026 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.534 | 4032 | 728628 |

Aggregate elapsed times for all five processes: P1=5.654 ms, P2=5.468 ms, P3=5.534 ms, P4=5.544 ms, P5=5.487 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.340 | 28.340 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.270 | 72.270 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.270 | 72.270 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.320 | 72.320 | 1659 | 0 | 775 | 159 |
| warmup loaded: King safety | 72.320 | 72.320 | 1659 | 0 | 775 | 159 |
| warmup after search: King safety | 72.320 | 72.320 | 1951 | 0 | 906 | 189 |
| warmup loaded: Endgame | 72.320 | 72.320 | 1951 | 0 | 906 | 189 |
| warmup after search: Endgame | 72.320 | 72.320 | 2155 | 0 | 950 | 196 |
| warmup loaded: Promotion tactic | 72.320 | 72.320 | 2155 | 0 | 950 | 196 |
| warmup after search: Promotion tactic | 72.320 | 72.320 | 2273 | 0 | 1052 | 207 |
| warmup loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2273 | 0 | 1052 | 207 |
| warmup after search: Advanced pawns/check evasion | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| after complete warmup | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Quiet middlegame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Quiet middlegame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Kiwipete | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Kiwipete | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: King safety | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: King safety | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Endgame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Endgame | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Promotion tactic | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Promotion tactic | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured loaded: Advanced pawns/check evasion | 72.355 | 72.355 | 2853 | 0 | 1266 | 233 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |
| final process state | 72.363 | 72.363 | 2853 | 0 | 1266 | 233 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.808 | 494 | 611397 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.933 | 1430 | 739753 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.721 | 455 | 631477 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.367 | 323 | 879835 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.349 | 227 | 650113 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.251 | 875 | 699446 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.429 | 3804 | 700698 |

Aggregate elapsed times for all five processes: P1=5.461 ms, P2=5.373 ms, P3=5.549 ms, P4=5.429 ms, P5=5.411 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.352 | 28.352 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.359 | 28.359 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.281 | 72.281 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.332 | 72.332 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.332 | 72.332 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.332 | 72.332 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.332 | 72.332 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.332 | 72.332 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.332 | 72.332 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.332 | 72.332 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.367 | 72.367 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| final process state | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.836 | 494 | 591069 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.860 | 1430 | 769002 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.720 | 455 | 631682 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.405 | 323 | 797779 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.336 | 227 | 675689 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.264 | 875 | 692433 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.420 | 3804 | 701830 |

Aggregate elapsed times for all five processes: P1=5.442 ms, P2=5.364 ms, P3=5.372 ms, P4=5.420 ms, P5=5.422 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.266 | 28.266 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.227 | 72.227 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.227 | 72.227 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.273 | 72.273 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.273 | 72.273 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.273 | 72.273 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.273 | 72.273 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.273 | 72.273 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.273 | 72.273 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.273 | 72.273 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.273 | 72.273 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| final process state | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.895 | 494 | 552008 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.928 | 1430 | 741725 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.731 | 455 | 622779 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.370 | 323 | 873265 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.336 | 227 | 676596 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.270 | 875 | 688989 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.529 | 3804 | 688033 |

Aggregate elapsed times for all five processes: P1=8.561 ms, P2=5.586 ms, P3=5.529 ms, P4=5.375 ms, P5=5.330 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.391 | 28.391 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.398 | 28.398 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.324 | 72.324 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.324 | 72.324 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.371 | 72.371 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.371 | 72.371 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.371 | 72.371 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.371 | 72.371 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.371 | 72.371 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.371 | 72.371 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.371 | 72.371 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.371 | 72.371 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.406 | 72.406 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.414 | 72.414 | 2821 | 0 | 1229 | 210 |
| final process state | 72.414 | 72.414 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.796 | 494 | 620464 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.916 | 1430 | 746353 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.731 | 455 | 622205 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.365 | 323 | 883875 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.331 | 227 | 686065 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.254 | 875 | 697582 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.394 | 3804 | 705218 |

Aggregate elapsed times for all five processes: P1=5.310 ms, P2=5.392 ms, P3=5.421 ms, P4=5.488 ms, P5=5.394 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.344 | 28.344 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.352 | 28.352 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.320 | 72.320 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.320 | 72.320 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.320 | 72.320 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.320 | 72.320 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.320 | 72.320 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.320 | 72.320 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.320 | 72.320 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| final process state | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.827 | 494 | 597664 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.858 | 1430 | 769638 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.701 | 455 | 649381 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.346 | 323 | 934260 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.330 | 227 | 687120 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.279 | 875 | 684056 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.340 | 3804 | 712298 |

Aggregate elapsed times for all five processes: P1=5.328 ms, P2=5.340 ms, P3=5.359 ms, P4=5.268 ms, P5=5.351 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.227 | 72.227 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.227 | 72.227 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.273 | 72.273 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.273 | 72.273 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.273 | 72.273 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.273 | 72.273 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.273 | 72.273 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.273 | 72.273 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.273 | 72.273 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.273 | 72.273 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| final process state | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.794 | 494 | 621906 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.895 | 1430 | 754670 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.719 | 455 | 632639 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.367 | 323 | 880639 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.334 | 227 | 678917 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.284 | 875 | 681724 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.393 | 3804 | 705352 |

Aggregate elapsed times for all five processes: P1=5.393 ms, P2=5.350 ms, P3=5.403 ms, P4=5.396 ms, P5=5.306 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.297 | 72.297 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.297 | 72.297 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.297 | 72.297 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.297 | 72.297 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.297 | 72.297 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.297 | 72.297 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.297 | 72.297 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.297 | 72.297 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| final process state | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.800 | 494 | 617379 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.868 | 1430 | 765359 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.720 | 455 | 632028 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.366 | 323 | 882566 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.331 | 227 | 686036 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.247 | 875 | 701523 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.333 | 3804 | 713346 |

Aggregate elapsed times for all five processes: P1=5.333 ms, P2=5.325 ms, P3=5.383 ms, P4=5.256 ms, P5=5.476 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.281 | 28.281 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.234 | 72.234 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.234 | 72.234 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.285 | 72.285 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.285 | 72.285 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.285 | 72.285 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.285 | 72.285 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.285 | 72.285 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.285 | 72.285 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.285 | 72.285 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.285 | 72.285 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| final process state | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.862 | 494 | 573218 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.932 | 1430 | 740048 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.760 | 455 | 599027 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.367 | 323 | 879421 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.341 | 227 | 666060 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.269 | 875 | 689721 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.530 | 3804 | 687834 |

Aggregate elapsed times for all five processes: P1=5.870 ms, P2=5.385 ms, P3=5.501 ms, P4=5.530 ms, P5=5.535 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.336 | 28.336 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.293 | 72.293 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.293 | 72.293 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.340 | 72.340 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.340 | 72.340 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.340 | 72.340 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.340 | 72.340 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.340 | 72.340 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.340 | 72.340 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.340 | 72.340 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.340 | 72.340 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| final process state | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.871 | 494 | 567222 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.958 | 1430 | 730325 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.738 | 455 | 616512 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.373 | 323 | 865330 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.345 | 227 | 657763 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.302 | 875 | 672167 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.587 | 3804 | 680854 |

Aggregate elapsed times for all five processes: P1=5.607 ms, P2=5.587 ms, P3=5.652 ms, P4=5.555 ms, P5=5.354 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.312 | 28.312 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.234 | 72.234 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.234 | 72.234 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.281 | 72.281 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.281 | 72.281 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.281 | 72.281 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.281 | 72.281 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.281 | 72.281 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.281 | 72.281 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.281 | 72.281 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.281 | 72.281 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.324 | 72.324 | 2821 | 0 | 1229 | 210 |
| final process state | 72.324 | 72.324 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.806 | 494 | 612830 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.867 | 1430 | 766114 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.714 | 455 | 637129 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.371 | 323 | 870986 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.326 | 227 | 696071 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.249 | 875 | 700310 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.333 | 3804 | 713267 |

Aggregate elapsed times for all five processes: P1=5.405 ms, P2=5.330 ms, P3=5.679 ms, P4=5.330 ms, P5=5.333 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.305 | 72.305 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.305 | 72.305 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.305 | 72.305 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.305 | 72.305 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.305 | 72.305 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.305 | 72.305 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.305 | 72.305 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.305 | 72.305 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.352 | 72.352 | 2821 | 0 | 1229 | 210 |
| final process state | 72.352 | 72.352 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.827 | 494 | 597389 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.860 | 1430 | 768819 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.707 | 455 | 643510 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.363 | 323 | 890918 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.321 | 227 | 707828 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.260 | 875 | 694550 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.337 | 3804 | 712754 |

Aggregate elapsed times for all five processes: P1=5.322 ms, P2=5.252 ms, P3=5.393 ms, P4=5.386 ms, P5=5.337 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.309 | 72.309 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.309 | 72.309 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.309 | 72.309 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.309 | 72.309 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.309 | 72.309 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.309 | 72.309 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.309 | 72.309 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.352 | 72.352 | 2821 | 0 | 1229 | 210 |
| final process state | 72.352 | 72.352 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.819 | 494 | 603116 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.896 | 1430 | 754169 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.727 | 455 | 625896 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.372 | 323 | 868048 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.332 | 227 | 684353 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.265 | 875 | 691703 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.411 | 3804 | 703018 |

Aggregate elapsed times for all five processes: P1=5.456 ms, P2=5.922 ms, P3=5.375 ms, P4=5.411 ms, P5=5.391 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.285 | 28.285 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.293 | 28.293 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.266 | 72.266 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.266 | 72.266 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.312 | 72.312 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.312 | 72.312 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.312 | 72.312 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.312 | 72.312 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.312 | 72.312 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.312 | 72.312 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.312 | 72.312 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.312 | 72.312 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| final process state | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.800 | 494 | 617516 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.852 | 1430 | 772236 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.704 | 455 | 646563 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.359 | 323 | 900466 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.343 | 227 | 662514 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.265 | 875 | 691958 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.321 | 3804 | 714859 |

Aggregate elapsed times for all five processes: P1=5.426 ms, P2=5.397 ms, P3=5.321 ms, P4=5.314 ms, P5=5.317 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.344 | 28.344 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.352 | 28.352 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.273 | 72.273 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.320 | 72.320 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.320 | 72.320 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.320 | 72.320 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.320 | 72.320 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.320 | 72.320 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.320 | 72.320 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.320 | 72.320 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| final process state | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.822 | 494 | 601303 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.851 | 1430 | 772517 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.711 | 455 | 640150 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.391 | 323 | 826021 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.356 | 227 | 636931 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.289 | 875 | 678591 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.420 | 3804 | 701809 |

Aggregate elapsed times for all five processes: P1=5.399 ms, P2=5.495 ms, P3=5.513 ms, P4=5.420 ms, P5=5.340 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.277 | 28.277 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.285 | 28.285 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.242 | 72.242 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.242 | 72.242 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.293 | 72.293 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.293 | 72.293 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.293 | 72.293 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.293 | 72.293 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.293 | 72.293 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.293 | 72.293 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.293 | 72.293 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.293 | 72.293 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| final process state | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.799 | 494 | 618249 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.892 | 1430 | 755827 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.726 | 455 | 627066 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.381 | 323 | 848385 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.333 | 227 | 681818 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.268 | 875 | 690020 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.398 | 3804 | 704662 |

Aggregate elapsed times for all five processes: P1=5.411 ms, P2=5.392 ms, P3=5.354 ms, P4=5.398 ms, P5=5.427 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.352 | 28.352 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.359 | 28.359 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.324 | 72.324 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.324 | 72.324 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.375 | 72.375 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.375 | 72.375 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.375 | 72.375 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.375 | 72.375 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.375 | 72.375 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.375 | 72.375 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.375 | 72.375 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.375 | 72.375 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.410 | 72.410 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.418 | 72.418 | 2821 | 0 | 1229 | 210 |
| final process state | 72.418 | 72.418 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.842 | 494 | 586628 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.908 | 1430 | 749477 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.738 | 455 | 616322 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.360 | 323 | 896313 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.327 | 227 | 694903 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.264 | 875 | 691982 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.440 | 3804 | 699283 |

Aggregate elapsed times for all five processes: P1=5.542 ms, P2=5.440 ms, P3=5.279 ms, P4=5.413 ms, P5=5.529 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.348 | 28.348 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.355 | 28.355 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.328 | 72.328 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.328 | 72.328 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.328 | 72.328 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.328 | 72.328 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.328 | 72.328 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.328 | 72.328 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.328 | 72.328 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| final process state | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.789 | 494 | 625729 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.856 | 1430 | 770458 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.698 | 455 | 651854 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.363 | 323 | 889358 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.320 | 227 | 709022 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.289 | 875 | 678575 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.316 | 3804 | 715530 |

Aggregate elapsed times for all five processes: P1=5.659 ms, P2=5.264 ms, P3=5.268 ms, P4=5.316 ms, P5=5.344 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.301 | 72.301 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.301 | 72.301 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.348 | 72.348 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.348 | 72.348 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.348 | 72.348 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.348 | 72.348 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.348 | 72.348 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.348 | 72.348 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.348 | 72.348 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.391 | 72.391 | 2821 | 0 | 1229 | 210 |
| final process state | 72.391 | 72.391 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.777 | 494 | 635880 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.833 | 1430 | 780124 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.736 | 455 | 617895 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.363 | 323 | 890963 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.327 | 227 | 693377 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.254 | 875 | 697879 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.290 | 3804 | 719092 |

Aggregate elapsed times for all five processes: P1=5.303 ms, P2=5.303 ms, P3=5.274 ms, P4=5.269 ms, P5=5.290 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.301 | 72.301 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.301 | 72.301 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.301 | 72.301 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.301 | 72.301 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.301 | 72.301 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.301 | 72.301 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.301 | 72.301 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.301 | 72.301 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.336 | 72.336 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| final process state | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.770 | 494 | 641395 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.853 | 1430 | 771801 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.697 | 455 | 652330 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.381 | 323 | 847279 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.316 | 227 | 719378 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.252 | 875 | 699021 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.269 | 3804 | 721955 |

Aggregate elapsed times for all five processes: P1=5.234 ms, P2=5.282 ms, P3=5.284 ms, P4=5.269 ms, P5=5.258 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.312 | 28.312 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.234 | 72.234 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.234 | 72.234 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.281 | 72.281 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.281 | 72.281 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.281 | 72.281 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.281 | 72.281 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.281 | 72.281 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.281 | 72.281 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.281 | 72.281 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.281 | 72.281 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.316 | 72.316 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.324 | 72.324 | 2821 | 0 | 1229 | 210 |
| final process state | 72.324 | 72.324 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.790 | 494 | 625681 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.851 | 1430 | 772636 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.708 | 455 | 642806 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.349 | 323 | 925398 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.323 | 227 | 703267 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.248 | 875 | 701193 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.268 | 3804 | 722113 |

Aggregate elapsed times for all five processes: P1=5.215 ms, P2=5.308 ms, P3=5.268 ms, P4=5.282 ms, P5=5.258 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.262 | 72.262 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.262 | 72.262 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.312 | 72.312 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.312 | 72.312 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.312 | 72.312 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.312 | 72.312 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.312 | 72.312 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.312 | 72.312 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.312 | 72.312 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.312 | 72.312 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |
| final process state | 72.355 | 72.355 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.789 | 494 | 626078 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.853 | 1430 | 771576 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.704 | 455 | 646533 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.350 | 323 | 923368 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.326 | 227 | 695403 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.264 | 875 | 692329 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.286 | 3804 | 719606 |

Aggregate elapsed times for all five processes: P1=6.990 ms, P2=5.265 ms, P3=5.315 ms, P4=5.286 ms, P5=5.284 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.340 | 28.340 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.262 | 72.262 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.262 | 72.262 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.309 | 72.309 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.309 | 72.309 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.309 | 72.309 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.309 | 72.309 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.309 | 72.309 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.309 | 72.309 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.309 | 72.309 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.352 | 72.352 | 2821 | 0 | 1229 | 210 |
| final process state | 72.352 | 72.352 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.790 | 494 | 625081 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.861 | 1430 | 768553 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.708 | 455 | 642994 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.350 | 323 | 923611 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.322 | 227 | 705106 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.320 | 875 | 662856 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.350 | 3804 | 710993 |

Aggregate elapsed times for all five processes: P1=5.337 ms, P2=5.371 ms, P3=5.702 ms, P4=5.298 ms, P5=5.350 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.273 | 28.273 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.281 | 28.281 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.234 | 72.234 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.234 | 72.234 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.285 | 72.285 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.285 | 72.285 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.285 | 72.285 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.285 | 72.285 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.285 | 72.285 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.285 | 72.285 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.285 | 72.285 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.285 | 72.285 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.320 | 72.320 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |
| final process state | 72.328 | 72.328 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.791 | 494 | 624288 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.865 | 1430 | 766909 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.708 | 455 | 643016 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.357 | 323 | 905220 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.320 | 227 | 710307 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.268 | 875 | 689826 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.308 | 3804 | 716605 |

Aggregate elapsed times for all five processes: P1=5.566 ms, P2=5.281 ms, P3=5.268 ms, P4=5.442 ms, P5=5.308 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.301 | 28.301 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.309 | 28.309 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.262 | 72.262 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.262 | 72.262 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.309 | 72.309 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.309 | 72.309 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.309 | 72.309 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.309 | 72.309 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.309 | 72.309 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.309 | 72.309 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.309 | 72.309 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.309 | 72.309 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.344 | 72.344 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.352 | 72.352 | 2821 | 0 | 1229 | 210 |
| final process state | 72.352 | 72.352 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.777 | 494 | 636088 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.846 | 1430 | 774472 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.740 | 455 | 614861 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.356 | 323 | 908145 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.324 | 227 | 701140 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.251 | 875 | 699635 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.293 | 3804 | 718668 |

Aggregate elapsed times for all five processes: P1=5.293 ms, P2=5.342 ms, P3=5.260 ms, P4=5.254 ms, P5=5.328 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.320 | 28.320 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.301 | 72.301 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.301 | 72.301 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.348 | 72.348 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.348 | 72.348 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.348 | 72.348 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.348 | 72.348 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.348 | 72.348 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.348 | 72.348 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.348 | 72.348 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.391 | 72.391 | 2821 | 0 | 1229 | 210 |
| final process state | 72.391 | 72.391 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.819 | 494 | 603470 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.820 | 1430 | 785812 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.711 | 455 | 639495 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.354 | 323 | 911574 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.321 | 227 | 708078 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.298 | 875 | 674287 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.322 | 3804 | 714708 |

Aggregate elapsed times for all five processes: P1=5.348 ms, P2=5.294 ms, P3=5.472 ms, P4=5.285 ms, P5=5.322 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.309 | 28.309 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.316 | 28.316 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.277 | 72.277 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.328 | 72.328 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.328 | 72.328 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.328 | 72.328 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.328 | 72.328 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.328 | 72.328 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.328 | 72.328 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.328 | 72.328 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.328 | 72.328 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.363 | 72.363 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| final process state | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.781 | 494 | 632540 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.828 | 1430 | 782125 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.708 | 455 | 642435 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.353 | 323 | 915115 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.328 | 227 | 691192 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.274 | 875 | 686787 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.273 | 3804 | 721411 |

Aggregate elapsed times for all five processes: P1=5.237 ms, P2=5.229 ms, P3=5.344 ms, P4=5.273 ms, P5=5.280 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.316 | 28.316 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.328 | 28.328 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.289 | 72.289 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.289 | 72.289 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.336 | 72.336 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.336 | 72.336 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.336 | 72.336 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.336 | 72.336 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.336 | 72.336 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.336 | 72.336 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.336 | 72.336 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.336 | 72.336 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.371 | 72.371 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.379 | 72.379 | 2821 | 0 | 1229 | 210 |
| final process state | 72.379 | 72.379 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.783 | 494 | 630808 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.854 | 1430 | 771106 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.702 | 455 | 648526 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.351 | 323 | 920718 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.323 | 227 | 703834 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.252 | 875 | 698844 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.265 | 3804 | 722563 |

Aggregate elapsed times for all five processes: P1=5.305 ms, P2=5.245 ms, P3=5.326 ms, P4=5.263 ms, P5=5.265 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.254 | 28.254 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.262 | 28.262 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.246 | 72.246 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.246 | 72.246 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.297 | 72.297 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.297 | 72.297 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.297 | 72.297 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.297 | 72.297 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.297 | 72.297 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.297 | 72.297 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.297 | 72.297 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.297 | 72.297 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| final process state | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.783 | 494 | 630512 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.892 | 1430 | 755944 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.719 | 455 | 632511 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.365 | 323 | 886040 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.326 | 227 | 696797 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.278 | 875 | 684600 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.363 | 3804 | 709310 |

Aggregate elapsed times for all five processes: P1=5.335 ms, P2=5.401 ms, P3=5.332 ms, P4=5.363 ms, P5=5.380 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.324 | 28.324 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.332 | 28.332 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.301 | 72.301 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.301 | 72.301 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.348 | 72.348 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.348 | 72.348 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.348 | 72.348 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.348 | 72.348 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.348 | 72.348 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.348 | 72.348 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.348 | 72.348 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.348 | 72.348 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.391 | 72.391 | 2821 | 0 | 1229 | 210 |
| final process state | 72.391 | 72.391 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.771 | 494 | 640870 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.853 | 1430 | 771671 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.704 | 455 | 645941 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.370 | 323 | 873041 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.324 | 227 | 700113 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.240 | 875 | 705485 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.263 | 3804 | 722805 |

Aggregate elapsed times for all five processes: P1=5.256 ms, P2=5.263 ms, P3=5.281 ms, P4=5.270 ms, P5=5.258 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.258 | 28.258 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.266 | 28.266 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.250 | 72.250 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.297 | 72.297 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.297 | 72.297 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.297 | 72.297 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.297 | 72.297 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.297 | 72.297 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.297 | 72.297 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.297 | 72.297 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.297 | 72.297 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.332 | 72.332 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| final process state | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.778 | 494 | 635327 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.826 | 1430 | 783325 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.725 | 455 | 627673 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.361 | 323 | 894999 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.331 | 227 | 686325 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.268 | 875 | 690140 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.288 | 3804 | 719432 |

Aggregate elapsed times for all five processes: P1=5.372 ms, P2=5.274 ms, P3=5.288 ms, P4=5.339 ms, P5=5.216 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.297 | 28.297 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.305 | 28.305 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.258 | 72.258 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.305 | 72.305 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.305 | 72.305 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.305 | 72.305 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.305 | 72.305 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.305 | 72.305 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.305 | 72.305 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.305 | 72.305 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.305 | 72.305 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.340 | 72.340 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |
| final process state | 72.348 | 72.348 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.797 | 494 | 619603 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.903 | 1430 | 751296 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.738 | 455 | 616834 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.363 | 323 | 889464 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.328 | 227 | 692172 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.250 | 875 | 700077 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.379 | 3804 | 707161 |

Aggregate elapsed times for all five processes: P1=5.304 ms, P2=5.379 ms, P3=5.343 ms, P4=5.813 ms, P5=5.417 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.363 | 28.363 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.371 | 28.371 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.293 | 72.293 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.293 | 72.293 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.340 | 72.340 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.340 | 72.340 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.340 | 72.340 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.340 | 72.340 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.340 | 72.340 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.340 | 72.340 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.340 | 72.340 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.340 | 72.340 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.375 | 72.375 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |
| final process state | 72.383 | 72.383 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.783 | 494 | 630800 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.800 | 1430 | 794598 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.751 | 455 | 605715 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.358 | 323 | 901645 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.319 | 227 | 711048 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.218 | 875 | 718506 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.229 | 3804 | 727447 |

Aggregate elapsed times for all five processes: P1=5.260 ms, P2=5.161 ms, P3=5.229 ms, P4=5.150 ms, P5=5.239 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.598 | 28.598 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.859 | 28.859 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.738 | 72.738 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.734 | 72.738 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.781 | 72.781 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.781 | 72.781 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.781 | 72.781 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.781 | 72.781 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.781 | 72.781 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.781 | 72.781 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.781 | 72.781 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.781 | 72.781 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| final process state | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.810 | 494 | 610035 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.785 | 1430 | 801120 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.724 | 455 | 628068 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.353 | 323 | 914395 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.310 | 227 | 732567 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.206 | 875 | 725578 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.188 | 3804 | 733191 |

Aggregate elapsed times for all five processes: P1=5.759 ms, P2=5.400 ms, P3=5.188 ms, P4=5.142 ms, P5=5.126 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.590 | 28.590 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.852 | 28.852 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.730 | 72.730 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.727 | 72.730 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.777 | 72.777 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.777 | 72.777 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.777 | 72.777 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.777 | 72.777 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.777 | 72.777 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.777 | 72.777 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.777 | 72.777 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.777 | 72.777 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |
| final process state | 72.812 | 72.812 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.794 | 494 | 622189 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.785 | 1430 | 801227 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.681 | 455 | 668315 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.334 | 323 | 966455 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.310 | 227 | 732326 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.197 | 875 | 731190 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.100 | 3804 | 745822 |

Aggregate elapsed times for all five processes: P1=5.117 ms, P2=5.100 ms, P3=5.125 ms, P4=5.057 ms, P5=5.063 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.531 | 28.531 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.793 | 28.793 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.719 | 72.719 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.715 | 72.719 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.766 | 72.766 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.766 | 72.766 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.766 | 72.766 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.766 | 72.766 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.766 | 72.766 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.766 | 72.766 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.766 | 72.766 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.766 | 72.766 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| final process state | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.757 | 494 | 652872 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.792 | 1430 | 798118 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.691 | 455 | 658444 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.344 | 323 | 939256 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.316 | 227 | 717287 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.206 | 875 | 725356 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.106 | 3804 | 744997 |

Aggregate elapsed times for all five processes: P1=5.154 ms, P2=5.118 ms, P3=5.075 ms, P4=5.083 ms, P5=5.106 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.574 | 28.574 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.836 | 28.836 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.758 | 72.758 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.754 | 72.758 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.801 | 72.801 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.801 | 72.801 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.801 | 72.801 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.801 | 72.801 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.801 | 72.801 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.801 | 72.801 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.801 | 72.801 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.801 | 72.801 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| final process state | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.764 | 494 | 646244 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.770 | 1430 | 807871 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.685 | 455 | 664015 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.344 | 323 | 938432 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.313 | 227 | 725891 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.189 | 875 | 735666 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.066 | 3804 | 750883 |

Aggregate elapsed times for all five processes: P1=5.063 ms, P2=5.055 ms, P3=5.103 ms, P4=5.082 ms, P5=5.066 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.582 | 28.582 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.844 | 28.844 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.758 | 72.758 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.754 | 72.758 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.801 | 72.801 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.801 | 72.801 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.801 | 72.801 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.801 | 72.801 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.801 | 72.801 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.801 | 72.801 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.801 | 72.801 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.801 | 72.801 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| final process state | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.759 | 494 | 650736 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.768 | 1430 | 808916 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.687 | 455 | 662345 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.343 | 323 | 941787 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.309 | 227 | 734580 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.200 | 875 | 728926 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.066 | 3804 | 750848 |

Aggregate elapsed times for all five processes: P1=5.087 ms, P2=5.065 ms, P3=5.115 ms, P4=5.035 ms, P5=5.066 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.613 | 28.613 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.879 | 28.879 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.758 | 72.758 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.754 | 72.758 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.801 | 72.801 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.801 | 72.801 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.801 | 72.801 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.801 | 72.801 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.801 | 72.801 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.801 | 72.801 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.801 | 72.801 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.801 | 72.801 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| final process state | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.758 | 494 | 651875 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.785 | 1430 | 801149 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.686 | 455 | 662968 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.343 | 323 | 942091 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.307 | 227 | 738417 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.204 | 875 | 726680 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.083 | 3804 | 748313 |

Aggregate elapsed times for all five processes: P1=5.242 ms, P2=5.115 ms, P3=5.063 ms, P4=5.083 ms, P5=5.075 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.555 | 28.555 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.816 | 28.816 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.727 | 72.727 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.723 | 72.727 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.773 | 72.773 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.773 | 72.773 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.773 | 72.773 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.773 | 72.773 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.773 | 72.773 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.773 | 72.773 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.773 | 72.773 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.773 | 72.773 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| final process state | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.761 | 494 | 649004 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.803 | 1430 | 793082 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.687 | 455 | 662546 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.341 | 323 | 948460 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.307 | 227 | 738276 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.210 | 875 | 722892 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.109 | 3804 | 744503 |

Aggregate elapsed times for all five processes: P1=5.055 ms, P2=5.054 ms, P3=5.172 ms, P4=5.117 ms, P5=5.109 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.617 | 28.617 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.879 | 28.879 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.758 | 72.758 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.754 | 72.758 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.801 | 72.801 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.801 | 72.801 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.801 | 72.801 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.801 | 72.801 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.801 | 72.801 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.801 | 72.801 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.801 | 72.801 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.801 | 72.801 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |
| final process state | 72.836 | 72.836 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.759 | 494 | 650465 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.770 | 1430 | 808031 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.680 | 455 | 668670 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.360 | 323 | 898435 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.320 | 227 | 710371 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.196 | 875 | 731565 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.085 | 3804 | 748115 |

Aggregate elapsed times for all five processes: P1=5.071 ms, P2=5.572 ms, P3=5.084 ms, P4=5.085 ms, P5=5.109 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.523 | 28.523 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.785 | 28.785 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.695 | 72.695 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.691 | 72.695 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.742 | 72.742 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.742 | 72.742 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.742 | 72.742 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.742 | 72.742 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.742 | 72.742 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.742 | 72.742 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.742 | 72.742 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.742 | 72.742 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| final process state | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.761 | 494 | 649313 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.771 | 1430 | 807533 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.686 | 455 | 663256 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.347 | 323 | 932095 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.317 | 227 | 716830 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.245 | 875 | 702957 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.126 | 3804 | 742159 |

Aggregate elapsed times for all five processes: P1=5.158 ms, P2=5.074 ms, P3=5.215 ms, P4=5.126 ms, P5=5.073 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.605 | 28.605 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.867 | 28.867 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.750 | 72.750 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.746 | 72.750 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.793 | 72.793 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.793 | 72.793 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.793 | 72.793 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.793 | 72.793 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.793 | 72.793 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.793 | 72.793 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.793 | 72.793 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.793 | 72.793 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| final process state | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.764 | 494 | 646844 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.782 | 1430 | 802573 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.677 | 455 | 671987 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.348 | 323 | 928643 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.321 | 227 | 707685 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.199 | 875 | 729766 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.090 | 3804 | 747323 |

Aggregate elapsed times for all five processes: P1=5.090 ms, P2=5.201 ms, P3=5.089 ms, P4=5.055 ms, P5=5.110 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.547 | 28.547 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.809 | 28.809 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.719 | 72.719 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.715 | 72.719 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.762 | 72.762 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.762 | 72.762 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.762 | 72.762 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.762 | 72.762 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.762 | 72.762 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.762 | 72.762 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.762 | 72.762 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.762 | 72.762 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| final process state | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.789 | 494 | 626183 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.801 | 1430 | 794174 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.694 | 455 | 655341 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.344 | 323 | 939592 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.322 | 227 | 705293 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.205 | 875 | 726397 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.154 | 3804 | 738066 |

Aggregate elapsed times for all five processes: P1=5.272 ms, P2=5.154 ms, P3=5.206 ms, P4=5.137 ms, P5=5.105 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.516 | 28.516 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.777 | 28.777 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.688 | 72.688 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.684 | 72.688 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.734 | 72.734 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.734 | 72.734 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.734 | 72.734 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.734 | 72.734 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.734 | 72.734 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.734 | 72.734 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.734 | 72.734 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.734 | 72.734 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |
| final process state | 72.770 | 72.770 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.767 | 494 | 644275 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.775 | 1430 | 805779 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.703 | 455 | 647604 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.350 | 323 | 923730 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.330 | 227 | 687004 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.201 | 875 | 728756 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.125 | 3804 | 742274 |

Aggregate elapsed times for all five processes: P1=5.161 ms, P2=5.039 ms, P3=5.125 ms, P4=5.138 ms, P5=5.107 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.578 | 28.578 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.840 | 28.840 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.719 | 72.719 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.715 | 72.719 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.766 | 72.766 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.766 | 72.766 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.766 | 72.766 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.766 | 72.766 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.766 | 72.766 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.766 | 72.766 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.766 | 72.766 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.766 | 72.766 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| final process state | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.767 | 494 | 644476 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.762 | 1430 | 811702 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.683 | 455 | 665876 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.336 | 323 | 962117 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.308 | 227 | 737269 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.219 | 875 | 717593 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.075 | 3804 | 749627 |

Aggregate elapsed times for all five processes: P1=5.054 ms, P2=5.075 ms, P3=5.071 ms, P4=5.104 ms, P5=5.136 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.543 | 28.543 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.805 | 28.805 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.719 | 72.719 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.715 | 72.719 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.766 | 72.766 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.766 | 72.766 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.766 | 72.766 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.766 | 72.766 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.766 | 72.766 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.766 | 72.766 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.766 | 72.766 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.766 | 72.766 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| final process state | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.757 | 494 | 652232 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.774 | 1430 | 806202 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.694 | 455 | 655992 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.338 | 323 | 955166 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.307 | 227 | 739941 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.211 | 875 | 722402 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.081 | 3804 | 748681 |

Aggregate elapsed times for all five processes: P1=5.081 ms, P2=5.031 ms, P3=5.138 ms, P4=5.128 ms, P5=5.060 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.473 | 28.473 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.734 | 28.734 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.676 | 72.676 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.672 | 72.676 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.723 | 72.723 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.723 | 72.723 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.723 | 72.723 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.723 | 72.723 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.723 | 72.723 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.723 | 72.723 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.723 | 72.723 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.723 | 72.723 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| final process state | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.768 | 494 | 643067 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.749 | 1430 | 817595 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.683 | 455 | 666453 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.343 | 323 | 942672 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.365 | 227 | 621909 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.218 | 875 | 718466 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.125 | 3804 | 742176 |

Aggregate elapsed times for all five processes: P1=5.108 ms, P2=5.109 ms, P3=5.125 ms, P4=5.189 ms, P5=5.134 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.602 | 28.602 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.863 | 28.863 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.742 | 72.742 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.738 | 72.742 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.789 | 72.789 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.789 | 72.789 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.789 | 72.789 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.789 | 72.789 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.789 | 72.789 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.789 | 72.789 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.789 | 72.789 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.789 | 72.789 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| final process state | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.755 | 494 | 654675 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.768 | 1430 | 808672 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.683 | 455 | 665933 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.349 | 323 | 924288 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.313 | 227 | 726086 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.202 | 875 | 727809 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.070 | 3804 | 750224 |

Aggregate elapsed times for all five processes: P1=5.136 ms, P2=5.056 ms, P3=5.070 ms, P4=5.116 ms, P5=4.979 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.566 | 28.566 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.828 | 28.828 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.758 | 72.758 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.754 | 72.758 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.805 | 72.805 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.805 | 72.805 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.805 | 72.805 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.805 | 72.805 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.805 | 72.805 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.805 | 72.805 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.805 | 72.805 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.805 | 72.805 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| final process state | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.844 | 494 | 585020 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.916 | 1430 | 746211 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.752 | 455 | 604963 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.377 | 323 | 857591 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.333 | 227 | 682593 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.248 | 875 | 701374 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.470 | 3804 | 695478 |

Aggregate elapsed times for all five processes: P1=6.235 ms, P2=5.196 ms, P3=5.470 ms, P4=5.345 ms, P5=5.573 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.621 | 28.621 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.883 | 28.883 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.809 | 72.809 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.805 | 72.809 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.855 | 72.855 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.855 | 72.855 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.855 | 72.855 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.855 | 72.855 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.855 | 72.855 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.855 | 72.855 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.855 | 72.855 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.855 | 72.855 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |
| final process state | 72.891 | 72.891 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.763 | 494 | 647680 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.765 | 1430 | 810289 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.689 | 455 | 660844 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.341 | 323 | 947486 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.333 | 227 | 680741 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.189 | 875 | 736106 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.079 | 3804 | 748953 |

Aggregate elapsed times for all five processes: P1=5.079 ms, P2=5.121 ms, P3=5.050 ms, P4=5.124 ms, P5=5.067 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.527 | 28.527 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.789 | 28.789 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.699 | 72.699 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.695 | 72.699 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.746 | 72.746 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.746 | 72.746 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.746 | 72.746 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.746 | 72.746 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.746 | 72.746 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.746 | 72.746 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.746 | 72.746 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.746 | 72.746 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |
| final process state | 72.781 | 72.781 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.762 | 494 | 648184 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.791 | 1430 | 798265 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.682 | 455 | 666763 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.344 | 323 | 939387 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.306 | 227 | 742536 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.203 | 875 | 727354 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.088 | 3804 | 747574 |

Aggregate elapsed times for all five processes: P1=5.041 ms, P2=5.990 ms, P3=5.088 ms, P4=5.249 ms, P5=5.045 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.613 | 28.613 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.875 | 28.875 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.754 | 72.754 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.750 | 72.754 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.797 | 72.797 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.797 | 72.797 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.797 | 72.797 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.797 | 72.797 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.797 | 72.797 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.797 | 72.797 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.797 | 72.797 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.797 | 72.797 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |
| final process state | 72.832 | 72.832 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.759 | 494 | 651075 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.755 | 1430 | 814687 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.691 | 455 | 658092 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.344 | 323 | 940221 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.316 | 227 | 718345 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.203 | 875 | 727353 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.068 | 3804 | 750600 |

Aggregate elapsed times for all five processes: P1=5.094 ms, P2=5.057 ms, P3=5.029 ms, P4=5.072 ms, P5=5.068 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.523 | 28.523 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.785 | 28.785 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.695 | 72.695 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.691 | 72.695 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.738 | 72.738 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.738 | 72.738 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.738 | 72.738 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.738 | 72.738 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.738 | 72.738 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.738 | 72.738 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.738 | 72.738 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.738 | 72.738 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |
| final process state | 72.773 | 72.773 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.759 | 494 | 650678 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.795 | 1430 | 796563 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.714 | 455 | 637603 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.349 | 323 | 926356 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.309 | 227 | 733637 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.184 | 875 | 739194 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.110 | 3804 | 744445 |

Aggregate elapsed times for all five processes: P1=5.128 ms, P2=5.026 ms, P3=5.220 ms, P4=5.093 ms, P5=5.110 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.543 | 28.543 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.879 | 28.879 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.758 | 72.758 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.754 | 72.758 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.805 | 72.805 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.805 | 72.805 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.805 | 72.805 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.805 | 72.805 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.805 | 72.805 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.805 | 72.805 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.805 | 72.805 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.805 | 72.805 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| final process state | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.792 | 494 | 623354 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.772 | 1430 | 806793 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.709 | 455 | 641888 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.338 | 323 | 956090 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.310 | 227 | 732310 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.214 | 875 | 720895 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.135 | 3804 | 740746 |

Aggregate elapsed times for all five processes: P1=5.120 ms, P2=5.135 ms, P3=5.162 ms, P4=5.082 ms, P5=5.221 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.598 | 28.598 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.859 | 28.859 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.738 | 72.738 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.734 | 72.738 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.781 | 72.781 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.781 | 72.781 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.781 | 72.781 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.781 | 72.781 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.781 | 72.781 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.781 | 72.781 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.781 | 72.781 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.781 | 72.781 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| final process state | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.768 | 494 | 643467 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.770 | 1430 | 808060 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.679 | 455 | 669816 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.336 | 323 | 960443 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.315 | 227 | 720118 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.183 | 875 | 739833 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.051 | 3804 | 753132 |

Aggregate elapsed times for all five processes: P1=5.066 ms, P2=5.019 ms, P3=5.028 ms, P4=5.105 ms, P5=5.051 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.578 | 28.578 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.840 | 28.840 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.766 | 72.766 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.762 | 72.766 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.812 | 72.812 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.812 | 72.812 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.812 | 72.812 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.812 | 72.812 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.812 | 72.812 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.812 | 72.812 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.812 | 72.812 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.812 | 72.812 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |
| final process state | 72.848 | 72.848 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.785 | 494 | 629034 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.777 | 1430 | 804630 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.680 | 455 | 669532 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.342 | 323 | 944939 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.307 | 227 | 739172 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.199 | 875 | 729788 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.090 | 3804 | 747344 |

Aggregate elapsed times for all five processes: P1=5.135 ms, P2=5.090 ms, P3=5.121 ms, P4=5.072 ms, P5=5.058 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.566 | 28.566 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.828 | 28.828 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.742 | 72.742 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.738 | 72.742 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.789 | 72.789 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.789 | 72.789 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.789 | 72.789 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.789 | 72.789 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.789 | 72.789 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.789 | 72.789 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.789 | 72.789 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.789 | 72.789 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| final process state | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.781 | 494 | 632199 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.776 | 1430 | 805288 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.684 | 455 | 665026 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.327 | 323 | 987293 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.311 | 227 | 728891 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.259 | 875 | 694956 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.139 | 3804 | 740221 |

Aggregate elapsed times for all five processes: P1=5.124 ms, P2=5.005 ms, P3=5.139 ms, P4=5.141 ms, P5=5.492 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.555 | 28.555 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.816 | 28.816 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.695 | 72.695 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.691 | 72.695 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.742 | 72.742 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.742 | 72.742 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.742 | 72.742 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.742 | 72.742 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.742 | 72.742 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.742 | 72.742 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.742 | 72.742 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.742 | 72.742 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |
| final process state | 72.777 | 72.777 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.765 | 494 | 645571 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.789 | 1430 | 799299 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.684 | 455 | 665678 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.338 | 323 | 955369 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.306 | 227 | 741827 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.205 | 875 | 726130 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.087 | 3804 | 747803 |

Aggregate elapsed times for all five processes: P1=5.113 ms, P2=5.087 ms, P3=5.451 ms, P4=4.982 ms, P5=5.031 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.512 | 28.512 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.773 | 28.773 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.715 | 72.715 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.711 | 72.715 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.762 | 72.762 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.762 | 72.762 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.762 | 72.762 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.762 | 72.762 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.762 | 72.762 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.762 | 72.762 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.762 | 72.762 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.762 | 72.762 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| final process state | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.772 | 494 | 639739 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.802 | 1430 | 793722 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.691 | 455 | 658171 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.340 | 323 | 949095 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.302 | 227 | 750909 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.201 | 875 | 728272 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.109 | 3804 | 744534 |

Aggregate elapsed times for all five processes: P1=5.207 ms, P2=5.012 ms, P3=5.109 ms, P4=5.192 ms, P5=5.107 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.500 | 28.500 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.762 | 28.762 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.672 | 72.672 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.668 | 72.672 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.715 | 72.715 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.715 | 72.715 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.715 | 72.715 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.715 | 72.715 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.715 | 72.715 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.715 | 72.715 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.715 | 72.715 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.715 | 72.715 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |
| final process state | 72.750 | 72.750 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.797 | 494 | 619629 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.773 | 1430 | 806645 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.672 | 455 | 676859 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.341 | 323 | 946026 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.318 | 227 | 713827 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.209 | 875 | 723571 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.111 | 3804 | 744283 |

Aggregate elapsed times for all five processes: P1=5.155 ms, P2=5.111 ms, P3=5.189 ms, P4=5.047 ms, P5=5.084 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.598 | 28.598 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.859 | 28.859 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.738 | 72.738 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.734 | 72.738 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.781 | 72.781 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.781 | 72.781 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.781 | 72.781 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.781 | 72.781 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.781 | 72.781 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.781 | 72.781 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.781 | 72.781 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.781 | 72.781 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |
| final process state | 72.816 | 72.816 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.768 | 494 | 643127 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.775 | 1430 | 805784 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.758 | 455 | 600381 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.341 | 323 | 946021 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.322 | 227 | 705937 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.192 | 875 | 734008 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.156 | 3804 | 737822 |

Aggregate elapsed times for all five processes: P1=5.218 ms, P2=5.115 ms, P3=5.210 ms, P4=5.156 ms, P5=5.110 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.477 | 28.477 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.738 | 28.738 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.680 | 72.680 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.676 | 72.680 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.723 | 72.723 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.723 | 72.723 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.723 | 72.723 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.723 | 72.723 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.723 | 72.723 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.723 | 72.723 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.723 | 72.723 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.723 | 72.723 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| final process state | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.743 | 494 | 665260 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.758 | 1430 | 813469 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.687 | 455 | 661997 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.348 | 323 | 928916 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.312 | 227 | 728311 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.204 | 875 | 726712 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.051 | 3804 | 753083 |

Aggregate elapsed times for all five processes: P1=5.135 ms, P2=5.020 ms, P3=5.040 ms, P4=5.051 ms, P5=5.126 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.547 | 28.547 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.809 | 28.809 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.688 | 72.688 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.684 | 72.688 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.730 | 72.730 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.730 | 72.730 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.730 | 72.730 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.730 | 72.730 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.730 | 72.730 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.730 | 72.730 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.730 | 72.730 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.730 | 72.730 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| final process state | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.746 | 494 | 662114 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.784 | 1430 | 801460 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.696 | 455 | 653839 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.340 | 323 | 948883 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.312 | 227 | 726613 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.191 | 875 | 734749 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.070 | 3804 | 750308 |

Aggregate elapsed times for all five processes: P1=5.070 ms, P2=5.339 ms, P3=5.034 ms, P4=5.087 ms, P5=5.031 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.516 | 28.516 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.777 | 28.777 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.719 | 72.719 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.715 | 72.719 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.762 | 72.762 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.762 | 72.762 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.762 | 72.762 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.762 | 72.762 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.762 | 72.762 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.762 | 72.762 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.762 | 72.762 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.762 | 72.762 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| final process state | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.753 | 494 | 656257 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.806 | 1430 | 791729 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.711 | 455 | 640031 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.359 | 323 | 899598 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.313 | 227 | 724117 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.213 | 875 | 721581 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.155 | 3804 | 737927 |

Aggregate elapsed times for all five processes: P1=5.124 ms, P2=5.334 ms, P3=5.128 ms, P4=5.155 ms, P5=5.967 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.516 | 28.516 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.777 | 28.777 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.719 | 72.719 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.715 | 72.719 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.766 | 72.766 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.766 | 72.766 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.766 | 72.766 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.766 | 72.766 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.766 | 72.766 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.766 | 72.766 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.766 | 72.766 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.766 | 72.766 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |
| final process state | 72.801 | 72.801 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.740 | 494 | 667944 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.772 | 1430 | 806947 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.677 | 455 | 671705 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.339 | 323 | 951976 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.316 | 227 | 718661 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.200 | 875 | 729152 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.044 | 3804 | 754125 |

Aggregate elapsed times for all five processes: P1=5.041 ms, P2=5.044 ms, P3=5.051 ms, P4=5.057 ms, P5=5.043 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.656 | 28.656 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.918 | 28.918 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.797 | 72.797 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.793 | 72.797 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.844 | 72.844 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.844 | 72.844 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.844 | 72.844 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.844 | 72.844 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.844 | 72.844 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.844 | 72.844 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.844 | 72.844 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.844 | 72.844 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |
| final process state | 72.879 | 72.879 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.803 | 494 | 614886 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.829 | 1430 | 781798 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.681 | 455 | 668007 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.342 | 323 | 944436 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.324 | 227 | 701175 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.219 | 875 | 718005 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.198 | 3804 | 731813 |

Aggregate elapsed times for all five processes: P1=5.277 ms, P2=5.417 ms, P3=5.112 ms, P4=5.198 ms, P5=5.151 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.500 | 28.500 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.762 | 28.762 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.676 | 72.676 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.672 | 72.676 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.723 | 72.723 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.723 | 72.723 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.723 | 72.723 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.723 | 72.723 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.723 | 72.723 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.723 | 72.723 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.723 | 72.723 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.723 | 72.723 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |
| final process state | 72.758 | 72.758 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.762 | 494 | 648350 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.767 | 1430 | 809379 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.690 | 455 | 659124 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.345 | 323 | 936082 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.318 | 227 | 712800 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.197 | 875 | 730751 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.080 | 3804 | 748827 |

Aggregate elapsed times for all five processes: P1=5.053 ms, P2=5.080 ms, P3=5.152 ms, P4=5.063 ms, P5=5.102 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.605 | 28.605 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.867 | 28.867 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.746 | 72.746 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.742 | 72.746 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.793 | 72.793 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.793 | 72.793 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.793 | 72.793 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.793 | 72.793 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.793 | 72.793 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.793 | 72.793 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.793 | 72.793 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.793 | 72.793 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| final process state | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.758 | 494 | 651567 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.804 | 1430 | 792535 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.701 | 455 | 649030 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.347 | 323 | 931420 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.318 | 227 | 714848 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.206 | 875 | 725310 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.134 | 3804 | 740904 |

Aggregate elapsed times for all five processes: P1=5.081 ms, P2=5.070 ms, P3=5.198 ms, P4=5.134 ms, P5=5.142 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.512 | 28.512 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.773 | 28.773 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.688 | 72.688 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.684 | 72.688 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.730 | 72.730 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.730 | 72.730 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.730 | 72.730 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.730 | 72.730 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.730 | 72.730 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.730 | 72.730 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.730 | 72.730 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.730 | 72.730 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |
| final process state | 72.766 | 72.766 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.752 | 494 | 656516 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.759 | 1430 | 813138 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.691 | 455 | 658634 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.346 | 323 | 933903 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.311 | 227 | 730236 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.196 | 875 | 731667 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.055 | 3804 | 752594 |

Aggregate elapsed times for all five processes: P1=5.127 ms, P2=5.055 ms, P3=5.064 ms, P4=5.031 ms, P5=5.025 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.605 | 28.605 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.867 | 28.867 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.746 | 72.746 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.742 | 72.746 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.793 | 72.793 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.793 | 72.793 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.793 | 72.793 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.793 | 72.793 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.793 | 72.793 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.793 | 72.793 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.793 | 72.793 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.793 | 72.793 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |
| final process state | 72.828 | 72.828 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.758 | 494 | 651959 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.779 | 1430 | 803701 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.670 | 455 | 678679 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.345 | 323 | 935348 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.317 | 227 | 716312 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.213 | 875 | 721503 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.082 | 3804 | 748468 |

Aggregate elapsed times for all five processes: P1=5.082 ms, P2=5.054 ms, P3=5.107 ms, P4=5.099 ms, P5=5.029 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.637 | 28.637 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.898 | 28.898 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.777 | 72.777 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.773 | 72.777 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.820 | 72.820 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.820 | 72.820 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.820 | 72.820 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.820 | 72.820 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.820 | 72.820 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.820 | 72.820 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.820 | 72.820 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.820 | 72.820 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |
| final process state | 72.855 | 72.855 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.744 | 494 | 664111 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.764 | 1430 | 810630 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.685 | 455 | 664093 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.346 | 323 | 933296 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.317 | 227 | 716248 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.207 | 875 | 724896 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.063 | 3804 | 751313 |

Aggregate elapsed times for all five processes: P1=5.104 ms, P2=5.008 ms, P3=5.035 ms, P4=5.070 ms, P5=5.063 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.586 | 28.586 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.848 | 28.848 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.727 | 72.727 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.723 | 72.727 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.773 | 72.773 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.773 | 72.773 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.773 | 72.773 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.773 | 72.773 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.773 | 72.773 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.773 | 72.773 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.773 | 72.773 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.773 | 72.773 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |
| final process state | 72.809 | 72.809 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 2 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.763 | 494 | 647350 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.748 | 1430 | 818036 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.684 | 455 | 664852 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.352 | 323 | 918432 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.312 | 227 | 728666 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.208 | 875 | 724505 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.066 | 3804 | 750814 |

Aggregate elapsed times for all five processes: P1=5.054 ms, P2=5.066 ms, P3=5.760 ms, P4=5.009 ms, P5=5.129 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.504 | 28.504 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.766 | 28.766 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.676 | 72.676 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.672 | 72.676 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.719 | 72.719 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.719 | 72.719 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.719 | 72.719 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.719 | 72.719 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.719 | 72.719 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.719 | 72.719 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.719 | 72.719 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.719 | 72.719 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |
| final process state | 72.754 | 72.754 | 2821 | 0 | 1229 | 210 |

## test

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.751 | 494 | 657933 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.743 | 1430 | 820502 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.688 | 455 | 661072 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.382 | 323 | 846152 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.308 | 227 | 737182 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.210 | 875 | 723162 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.082 | 3804 | 748588 |

Aggregate elapsed times for all five processes: P1=5.082 ms, P2=5.079 ms, P3=5.083 ms, P4=5.065 ms, P5=5.170 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.535 | 28.535 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.797 | 28.797 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.727 | 72.727 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.723 | 72.727 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.770 | 72.770 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.770 | 72.770 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.770 | 72.770 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.770 | 72.770 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.770 | 72.770 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.770 | 72.770 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.770 | 72.770 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.770 | 72.770 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| final process state | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 3 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.767 | 494 | 644208 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.796 | 1430 | 796046 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.699 | 455 | 650712 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.342 | 323 | 943757 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.313 | 227 | 724667 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.210 | 875 | 723183 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.128 | 3804 | 741828 |

Aggregate elapsed times for all five processes: P1=5.223 ms, P2=5.278 ms, P3=5.128 ms, P4=5.055 ms, P5=5.054 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.574 | 28.574 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.836 | 28.836 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.715 | 72.715 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.711 | 72.715 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.762 | 72.762 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.762 | 72.762 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.762 | 72.762 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.762 | 72.762 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.762 | 72.762 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.762 | 72.762 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.762 | 72.762 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.762 | 72.762 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |
| final process state | 72.797 | 72.797 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 5 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.764 | 494 | 646312 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.763 | 1430 | 810952 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.683 | 455 | 666374 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.347 | 323 | 931829 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.314 | 227 | 721849 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.204 | 875 | 726640 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.076 | 3804 | 749443 |

Aggregate elapsed times for all five processes: P1=5.154 ms, P2=5.188 ms, P3=5.045 ms, P4=5.068 ms, P5=5.076 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.574 | 28.574 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.836 | 28.836 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.762 | 72.762 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.758 | 72.762 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.805 | 72.805 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.805 | 72.805 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.805 | 72.805 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.805 | 72.805 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.805 | 72.805 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.805 | 72.805 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.805 | 72.805 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.805 | 72.805 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |
| final process state | 72.840 | 72.840 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.755 | 494 | 654495 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.746 | 1430 | 819131 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.707 | 455 | 643752 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.360 | 323 | 896353 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.328 | 227 | 693062 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.204 | 875 | 726622 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.099 | 3804 | 745968 |

Aggregate elapsed times for all five processes: P1=5.099 ms, P2=5.028 ms, P3=5.173 ms, P4=5.031 ms, P5=5.119 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.555 | 28.555 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.820 | 28.820 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.746 | 72.746 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.742 | 72.746 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.789 | 72.789 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.789 | 72.789 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.789 | 72.789 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.789 | 72.789 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.789 | 72.789 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.789 | 72.789 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.789 | 72.789 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.789 | 72.789 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |
| final process state | 72.824 | 72.824 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 1 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.750 | 494 | 659042 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.776 | 1430 | 805212 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.686 | 455 | 663711 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.360 | 323 | 896714 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.314 | 227 | 722102 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.199 | 875 | 730072 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.084 | 3804 | 748212 |

Aggregate elapsed times for all five processes: P1=5.084 ms, P2=5.088 ms, P3=5.029 ms, P4=5.145 ms, P5=5.038 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.508 | 28.508 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.770 | 28.770 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.711 | 72.711 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.707 | 72.711 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.758 | 72.758 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.758 | 72.758 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.758 | 72.758 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.758 | 72.758 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.758 | 72.758 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.758 | 72.758 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.758 | 72.758 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.758 | 72.758 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |
| final process state | 72.793 | 72.793 | 2821 | 0 | 1229 | 210 |

## baseline

- Compiler: g++ 13.3.0
- Build type: Release
- Expected flags: `-O3 -DNDEBUG -std=gnu++17`
- Protocol: five independent processes; one complete warmup and one measured corpus per process
- Selected process: 4 (median aggregate elapsed time)

| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |
|---|---:|---:|---:|---:|---|---:|---|
| Quiet middlegame | 4 | 0.746 | 494 | 661902 | d6d5 | 2 | d6d5 d2b3 d5e4 f3e5 |
| Kiwipete | 4 | 1.783 | 1430 | 802048 | e2a6 | 68 | e2a6 b4c3 d2c3 e6d5 |
| King safety | 4 | 0.668 | 455 | 681060 | c3d5 | 455 | c3d5 f6d5 g5e7 d5e7 |
| Endgame | 5 | 0.334 | 323 | 967639 | b4f4 | 253 | b4f4 h4g3 f4f7 g3g2 f7c7 |
| Promotion tactic | 4 | 0.309 | 227 | 735434 | d7c8q | 564 | d7c8q d8c8 e1f2 e7c5 e2d4 c8f5 f2e2 c5d4 d1d4 f5c2 b1d2 |
| Advanced pawns/check evasion | 5 | 1.204 | 875 | 726567 | c4c5 | -670 | c4c5 b6c5 b4c5 a3c5 f1f2 b2a1q d1a1 f6e4 |

| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |
|---:|---:|---:|
| 5.044 | 3804 | 754148 |

Aggregate elapsed times for all five processes: P1=5.090 ms, P2=5.077 ms, P3=5.026 ms, P4=5.044 ms, P5=5.020 ms

| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |
|---|---:|---:|---:|---:|---:|---:|
| after engine initialization | 28.582 | 28.582 | 0 | 0 | 0 | 0 |
| warmup loaded: Quiet middlegame | 28.844 | 28.844 | 0 | 0 | 0 | 0 |
| warmup after search: Quiet middlegame | 72.723 | 72.723 | 459 | 0 | 301 | 64 |
| warmup loaded: Kiwipete | 72.719 | 72.723 | 459 | 0 | 301 | 64 |
| warmup after search: Kiwipete | 72.770 | 72.770 | 1627 | 0 | 729 | 131 |
| warmup loaded: King safety | 72.770 | 72.770 | 1627 | 0 | 729 | 131 |
| warmup after search: King safety | 72.770 | 72.770 | 1919 | 0 | 862 | 163 |
| warmup loaded: Endgame | 72.770 | 72.770 | 1919 | 0 | 862 | 163 |
| warmup after search: Endgame | 72.770 | 72.770 | 2123 | 0 | 906 | 171 |
| warmup loaded: Promotion tactic | 72.770 | 72.770 | 2123 | 0 | 906 | 171 |
| warmup after search: Promotion tactic | 72.770 | 72.770 | 2241 | 0 | 1008 | 182 |
| warmup loaded: Advanced pawns/check evasion | 72.770 | 72.770 | 2241 | 0 | 1008 | 182 |
| warmup after search: Advanced pawns/check evasion | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| after complete warmup | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Quiet middlegame | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Quiet middlegame | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Kiwipete | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Kiwipete | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: King safety | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: King safety | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Endgame | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Endgame | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Promotion tactic | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Promotion tactic | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured loaded: Advanced pawns/check evasion | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| measured after search: Advanced pawns/check evasion | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
| final process state | 72.805 | 72.805 | 2821 | 0 | 1229 | 210 |
