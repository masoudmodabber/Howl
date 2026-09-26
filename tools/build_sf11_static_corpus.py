#!/usr/bin/env python3
import argparse, concurrent.futures, os, re, subprocess, time, random
import pyarrow.parquet as pq

SF='/home/masoud/Downloads/Stockfish-sf_11/src/stockfish'
SRC='/tmp/howl-nnue-raw-teacher/train.parquet'; OUT='/tmp/howl-nnue-sf11-static.tsv'

def select(path,n,seed):
    rows=[]
    for b in pq.ParquetFile(path).iter_batches(batch_size=50000,columns=['fen']): rows.extend(b.column('fen').to_pylist())
    random.Random(seed).shuffle(rows); return list(dict.fromkeys(rows))[:n]

def worker(fens):
    p=subprocess.Popen([SF],stdin=subprocess.PIPE,stdout=subprocess.PIPE,text=True,bufsize=1)
    payload='bulkeval\n'+'\n'.join(fens)+'\n'
    stdout,_=p.communicate(payload)
    values=[int(line) for line in stdout.splitlines() if re.fullmatch(r'-?\d+',line.strip())]
    if len(values)!=len(fens): raise RuntimeError(f'bulk evaluation count mismatch: {len(values)} != {len(fens)}')
    return [(fen, val) for fen,val in zip(fens,values)]

def main():
    a=argparse.ArgumentParser();a.add_argument('--source',default=SRC);a.add_argument('--output',default=OUT);a.add_argument('--count',type=int,default=1000000);a.add_argument('--limit',type=int);a.add_argument('--workers',type=int,default=8);a.add_argument('--seed',type=int,default=20260926);x=a.parse_args()
    target=x.limit or x.count; t=time.time(); f=select(x.source,target,x.seed); done=set()
    if os.path.exists(x.output):
        with open(x.output,encoding='utf-8') as z:
            for line in z:
                if line.strip() and not line.startswith('fen\t'): done.add(line.split('\t',1)[0])
    pending=[fen for fen in f if fen not in done]; new_file=not os.path.exists(x.output) or os.path.getsize(x.output)==0
    with open(x.output,'a',encoding='utf-8',buffering=1) as out:
        if new_file: out.write('fen\tstatic_cp\n')
        with concurrent.futures.ThreadPoolExecutor(max_workers=x.workers) as pool:
            for base in range(0,len(pending),1000*x.workers):
                chunks=[pending[i:i+1000] for i in range(base,min(base+1000*x.workers,len(pending)),1000)]
                for part in pool.map(worker,chunks):
                    for fen,cp in part:
                        if fen not in done: out.write(f'{fen}\t{cp}\n'); done.add(fen)
                    elapsed=time.time()-t; rate=len(done)/elapsed if elapsed else 0
                    print(f'completed {len(done)}/{target} ({100*len(done)/target:.2f}%) {rate:.2f} positions/sec elapsed {elapsed:.1f}s',flush=True)
    print(f'selected {len(done)} seconds {time.time()-t:.2f} output {x.output}',flush=True)
if __name__=='__main__':main()
