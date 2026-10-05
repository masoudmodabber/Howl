#!/usr/bin/env python3
"""Generate resumable side-to-move UCI teacher scores for a tuner corpus."""

import argparse
import subprocess
from pathlib import Path


def wait_for(process, token):
    while True:
        line = process.stdout.readline()
        if line == "":
            raise RuntimeError("UCI engine terminated unexpectedly")
        if line.strip() == token:
            return


def send(process, command):
    process.stdin.write(command + "\n")
    process.stdin.flush()


def corpus_positions(path):
    for line in path.read_text().splitlines():
        if not line:
            continue
        fen, separator, _ = line.rpartition("\t")
        if separator and fen:
            yield fen


def completed_keys(path):
    completed = set()
    if not path.exists():
        return completed
    for line in path.read_text().splitlines():
        key, separator, _ = line.rpartition("\t")
        if separator and key:
            completed.add(key)
    return completed


def parse_score(tokens):
    try:
        index = tokens.index("score")
        kind = tokens[index + 1]
        value = int(tokens[index + 2])
    except (ValueError, IndexError):
        return None
    if kind == "cp":
        return value
    if kind == "mate":
        return 10000 if value > 0 else -10000
    return None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--engine", required=True, type=Path)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--depth", type=int, default=14)
    args = parser.parse_args()
    if args.depth < 1:
        parser.error("--depth must be positive")

    done = completed_keys(args.output)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    process = subprocess.Popen(
        [str(args.engine)], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL, text=True, bufsize=1)
    try:
        send(process, "uci")
        wait_for(process, "uciok")
        send(process, "isready")
        wait_for(process, "readyok")
        with args.output.open("a", buffering=1) as output:
            written = 0
            for fen in corpus_positions(args.input):
                if fen in done:
                    continue
                send(process, "position fen " + fen)
                send(process, f"go depth {args.depth}")
                deepest_depth = -1
                deepest_score = None
                while True:
                    line = process.stdout.readline()
                    if line == "":
                        raise RuntimeError("UCI engine terminated unexpectedly")
                    tokens = line.split()
                    if tokens and tokens[0] == "bestmove":
                        break
                    if not tokens or tokens[0] != "info":
                        continue
                    score = parse_score(tokens)
                    if score is None:
                        continue
                    try:
                        depth = int(tokens[tokens.index("depth") + 1])
                    except (ValueError, IndexError):
                        continue
                    if depth >= deepest_depth:
                        deepest_depth = depth
                        deepest_score = score
                if deepest_score is None:
                    raise RuntimeError(f"No score reported for position: {fen}")
                output.write(f"{fen}\t{deepest_score}\n")
                written += 1
                if written % 25 == 0:
                    output.flush()
    finally:
        if process.poll() is None:
            send(process, "quit")
            process.wait(timeout=10)


if __name__ == "__main__":
    main()
