# Vendored Fathom

This directory contains the probing sources from `jdart1/Fathom`, pinned at
commit `c9c6fef0dddc05d2e242c183acf5833149ab676d`.

Howl compiles `src/tbprobe.c` as a static library. That translation unit
includes `tbchess.c`, matching Fathom's own build. Engine code must use
`Tablebase.h`; it must not include or call the Fathom API directly.
