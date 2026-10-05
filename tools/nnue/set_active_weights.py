#!/usr/bin/env python3
"""Set the root active NNUE weight alias while preserving the versioned model."""

import hashlib
import shutil
import sys
from pathlib import Path


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {Path(sys.argv[0]).name} <versioned-weight-file>", file=sys.stderr)
        return 2

    source = Path(sys.argv[1])
    destination = Path.cwd() / "nnue.weights"
    if not source.is_file():
        print(f"Source weight file not found: {source}", file=sys.stderr)
        return 1

    shutil.copyfile(source, destination)
    source_hash = sha256(source)
    destination_hash = sha256(destination)
    if source_hash != destination_hash:
        print("SHA256 verification failed", file=sys.stderr)
        return 1

    print(f"source: {source}")
    print(f"destination: {destination}")
    print(f"sha256: {source_hash}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
