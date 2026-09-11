# Syzygy tablebases

Set `SyzygyPath` to a directory containing Syzygy files and optionally set
`SyzygyProbeLimit` (default 5, maximum 7). An empty path disables probing.

The focused WDL tests require only the small `KQvK.rtbw` file. Fathom handles
the bare-kings draw internally.

Place them together in any directory, then run:

```sh
HOWL_SYZYGY_PATH=/path/to/syzygy ctest --test-dir build -R tablebase --output-on-failure
```

The tests skip their file-backed cases when `HOWL_SYZYGY_PATH` is unset. They
never download tablebases. A normal build and all non-tablebase tests require
no Syzygy files.

This first integration uses interior WDL probing only. Fathom's WDL API cannot
represent a nonzero 50-move halfmove clock, so Howl probes only positions whose
clock is zero and accepts only unconditional win, draw, and loss results.
Cursed-win and blessed-loss results are not treated as exact. Root DTZ move
selection is intentionally deferred.
