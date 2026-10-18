#!/usr/bin/env python3
"""Prepares train and validation Parquet datasets from Lichess position evaluations."""
import argparse
import glob
import hashlib
import os
import sqlite3
import pyarrow as pa
import pyarrow.parquet as pq


def main():
    p = argparse.ArgumentParser(description="Extract and deduplicate positions from Lichess shards.")
    p.add_argument('--input', nargs='+', required=True, help="Input parquet shard glob/paths")
    p.add_argument('--output', required=True, help="Output directory for train.parquet and validation.parquet")
    p.add_argument('--db', default='/tmp/howl-lichess-v2.sqlite', help="Path to temporary SQLite database")
    p.add_argument('--min-knodes', type=int, default=3000, help="Minimum Stockfish analysis search nodes (in thousands)")
    p.add_argument('--validation-fraction', type=float, default=0.05, help="Fraction of positions reserved for validation")
    a = p.parse_args()

    shards = []
    for x in a.input:
        shards.extend(sorted(glob.glob(x)))
    if not shards:
        raise ValueError("No input files matched --input")

    con = sqlite3.connect(a.db)
    con.execute('pragma journal_mode=wal')
    con.execute('create table if not exists d(fen text primary key,knodes integer,depth integer,cp integer,mate integer)')
    con.commit()

    for path in shards:
        for batch in pq.ParquetFile(path).iter_batches(columns=['fen', 'knodes', 'depth', 'cp', 'mate'], batch_size=65536):
            cols = [x.to_pylist() for x in batch.columns]
            for row in zip(*cols):
                fen, kn, dep, cp, mate = row
                old = con.execute('select knodes from d where fen=?', (fen,)).fetchone()
                if old is None or kn >= old[0]:
                    con.execute('insert or replace into d values(?,?,?,?,?)', (fen, kn, dep, cp, mate))
        con.commit()
        print('completed', path, flush=True)

    rows = con.execute('select fen,knodes,depth,cp,mate from d where knodes>=?', (a.min_knodes,)).fetchall()
    tr = []
    va = []
    for r in rows:
        h = int.from_bytes(hashlib.sha256(r[0].encode()).digest()[:8], 'little') / 2**64
        if h < a.validation_fraction:
            va.append(r)
        else:
            tr.append(r)

    os.makedirs(a.output, exist_ok=True)
    for name, data in [('train', tr), ('validation', va)]:
        tbl = pa.table({
            'fen': [r[0] for r in data],
            'knodes': [r[1] for r in data],
            'depth': [r[2] for r in data],
            'cp': [r[3] for r in data],
            'mate': [r[4] for r in data]
        })
        pq.write_table(tbl, os.path.join(a.output, f'{name}.parquet'))

    total_unique = con.execute('select count(*) from d').fetchone()[0]
    print(f"shards: {len(shards)}, unique: {total_unique}, retained: {len(rows)}, train: {len(tr)}, validation: {len(va)}")


if __name__ == '__main__':
    main()
