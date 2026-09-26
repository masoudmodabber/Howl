#!/usr/bin/env python3
import argparse, concurrent.futures, csv, os, time, sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from external_move_quality_pilot import StockfishUCI

ENGINE = None
NODES = 0

def init(path, nodes, hash_mb):
    global ENGINE, NODES
    NODES = nodes
    ENGINE = StockfishUCI(path, threads=1, hash_mb=hash_mb)

def work(task):
    index, fen = task
    ENGINE.send("setoption name Clear Hash")
    ENGINE.send("setoption name MultiPV value 1")
    ENGINE.send("isready"); ENGINE.wait("readyok")
    ENGINE.send("ucinewgame")
    ENGINE.send(f"position fen {fen}")
    ENGINE.send(f"go nodes {NODES}")
    latest = None
    while True:
        line = ENGINE.readline()
        if line.startswith("info "):
            tokens = line.split()
            for i, token in enumerate(tokens[:-2]):
                if token == "score" and tokens[i + 1] in ("cp", "mate"):
                    latest = (tokens[i + 1], int(tokens[i + 2]))
                    break
        if line.startswith("bestmove"):
            break
    if latest is None:
        score_type, score = 'cp', 0
    else:
        score_type, score = latest
    if score_type == 'mate':
        score = 100000 if score > 0 else -100000
    return index, fen, int(score)

def rows(path):
    with open(path, encoding='utf-8') as stream:
        return [(i, line.rstrip().split('\t', 1)[0]) for i, line in enumerate(stream)]

def label(input_path, output_path, engine, nodes, workers, hash_mb, limit=None):
    source = rows(input_path)
    if limit is not None: source = source[:limit]
    done = {}
    if Path(output_path).exists():
        with open(output_path, encoding='utf-8') as stream:
            for row in csv.DictReader(stream, delimiter='\t'):
                done[int(row['index'])] = row
    tasks = [(i, fen) for i, fen in source if i not in done]
    start = time.monotonic()
    if tasks:
        with concurrent.futures.ProcessPoolExecutor(workers, initializer=init,
                initargs=(engine, nodes, hash_mb)) as pool:
            for i, fen, score in pool.map(work, tasks, chunksize=1):
                done[i] = {'index': str(i), 'fen': fen, 'score_cp_stm': str(score)}
                if len(done) % 100 == 0:
                    Path(output_path).parent.mkdir(parents=True, exist_ok=True)
                    tmp = str(output_path) + '.tmp'
                    with open(tmp, 'w', encoding='utf-8', newline='') as out:
                        writer = csv.DictWriter(out, fieldnames=['index','fen','score_cp_stm'], delimiter='\t')
                        writer.writeheader(); writer.writerows(done[i] for i, _ in source if i in done)
                    os.replace(tmp, output_path)
    elapsed = time.monotonic() - start
    Path(output_path).parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, 'w', encoding='utf-8', newline='') as out:
        writer = csv.DictWriter(out, fieldnames=['index','fen','score_cp_stm'], delimiter='\t')
        writer.writeheader(); writer.writerows(done[i] for i, _ in source if i in done)
    return elapsed, len(source), len(tasks)

def main():
    p = argparse.ArgumentParser(); p.add_argument('--engine', required=True); p.add_argument('--train', required=True); p.add_argument('--validation', required=True); p.add_argument('--out-dir', required=True); p.add_argument('--nodes', type=int, default=3000000); p.add_argument('--workers', type=int, default=8); p.add_argument('--hash', type=int, default=32); p.add_argument('--benchmark', type=int, default=100); p.add_argument('--run-full', action='store_true')
    a = p.parse_args(); out = Path(a.out_dir); out.mkdir(parents=True, exist_ok=True)
    t, count, _ = label(a.train, out/'train.tsv', a.engine, a.nodes, a.workers, a.hash, a.benchmark)
    total = len(rows(a.train)) + len(rows(a.validation)); rate = t / a.benchmark
    print(f'benchmark_seconds_per_position={rate:.6f} estimated_total_seconds={rate * total / a.workers:.1f}')
    if a.run_full and rate * total / a.workers < 7200:
        for inp, name in ((a.train, 'train.tsv'), (a.validation, 'validation.tsv')):
            elapsed, count, pending = label(inp, out/name, a.engine, a.nodes, a.workers, a.hash)
            print(f'{name}_count={count} pending={pending} seconds={elapsed:.1f}')

if __name__ == '__main__': main()
