#!/usr/bin/env python3
import argparse, csv, hashlib, os, sqlite3, sys, time
import chess.pgn

def split_for_game(game_id):
    # Stable 95/5 assignment without knowing the total game count.
    return 1 if int.from_bytes(hashlib.blake2b(str(game_id).encode(), digest_size=8).digest(), 'big') % 20 == 0 else 0

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('pgn')
    ap.add_argument('--out-dir', default='/tmp/howl-nnue-lumbras')
    args = ap.parse_args()
    os.makedirs(args.out_dir, exist_ok=True)
    train_path = os.path.join(args.out_dir, 'train.tsv')
    val_path = os.path.join(args.out_dir, 'validation.tsv')
    db_path = os.path.join(args.out_dir, 'fen-dedup.sqlite3')
    db = sqlite3.connect(db_path)
    db.execute('PRAGMA journal_mode=OFF')
    db.execute('PRAGMA synchronous=OFF')
    db.execute('CREATE TABLE IF NOT EXISTS seen (split INTEGER NOT NULL, fen TEXT NOT NULL, PRIMARY KEY(split, fen))')
    db.commit()
    files = [open(train_path, 'w', newline=''), open(val_path, 'w', newline='')]
    writers = [csv.writer(f, delimiter='\t', lineterminator='\n') for f in files]
    for w in writers: w.writerow(('game_id', 'ply', 'fen', 'result'))
    games = 0; accepted = [0, 0]; duplicates = 0; results = [{}, {}]; started = time.time()
    try:
        with open(args.pgn, encoding='utf-8', errors='replace') as stream:
            while accepted[0] < 1_000_000 or accepted[1] < 75_000:
                game = chess.pgn.read_game(stream)
                if game is None: break
                result = game.headers.get('Result')
                if result not in ('1-0', '0-1', '1/2-1/2'): continue
                game_id = games; games += 1; split = split_for_game(game_id)
                board = game.board(); next_ply = 13 + (game_id % 6)
                for ply, move in enumerate(game.mainline_moves(), 1):
                    board.push(move)
                    if ply < next_ply: continue
                    next_ply += 6
                    if board.is_game_over(claim_draw=False): continue
                    if accepted[split] >= (75_000 if split else 1_000_000): continue
                    fen = board.fen()
                    cur = db.execute('INSERT OR IGNORE INTO seen(split, fen) VALUES (?, ?)', (split, fen))
                    if cur.rowcount == 0: duplicates += 1; continue
                    accepted[split] += 1; writers[split].writerow((game_id, ply, fen, result))
                    results[split][result] = results[split].get(result, 0) + 1
                    if sum(accepted) % 10_000 == 0:
                        db.commit(); print(f'accepted={sum(accepted)} train={accepted[0]} validation={accepted[1]} games={games}', flush=True)
                    if accepted[0] >= 1_000_000 and accepted[1] >= 75_000: break
                if accepted[0] >= 1_000_000 and accepted[1] >= 75_000: break
        db.commit()
    finally:
        for f in files: f.close()
        db.close()
    print(f'games={games} train={accepted[0]} validation={accepted[1]} duplicates={duplicates} results={results} wall_seconds={time.time()-started:.3f}', flush=True)

if __name__ == '__main__': main()
