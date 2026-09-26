#!/usr/bin/env python3
import argparse, os, random, time
import numpy as np
import pyarrow as pa, pyarrow.parquet as pq
import torch
from nnue_trainer import Net, item

SCHEMA = pa.schema([('features', pa.list_(pa.int32())), ('black', pa.bool_()), ('cp', pa.int32()), ('mate', pa.int32())])

def cache_one(src, dst):
    tmp = dst + '.tmp'; writer = None
    for b in pq.ParquetFile(src).iter_batches(batch_size=10000, columns=['fen','stm_cp','stm_mate']):
        rows=[]
        for fen,cp,mate in zip(b.column('fen').to_pylist(),b.column('stm_cp').to_pylist(),b.column('stm_mate').to_pylist()):
            ix,black=item(fen); rows.append({'features':ix,'black':black,'cp':cp,'mate':mate})
        t=pa.Table.from_pylist(rows,schema=SCHEMA)
        if writer is None: writer=pq.ParquetWriter(tmp,SCHEMA,compression='zstd')
        writer.write_table(t)
    if writer: writer.close()
    os.replace(tmp,dst)

def benchmark(path, batch_size, device):
    tab=pq.read_table(path,columns=['features','black','cp','mate']).slice(0,500000)
    rows=list(zip(tab['features'].to_pylist(),tab['black'].to_pylist(),tab['cp'].to_pylist(),tab['mate'].to_pylist()))
    torch.manual_seed(1); n=Net().to(device); opt=torch.optim.Adam(n.parameters(),lr=.001); order=list(range(len(rows))); random.Random(42).shuffle(order)
    peak=0; start=time.time(); n.train()
    for base in range(0,len(order),batch_size):
        q=[rows[i] for i in order[base:base+batch_size]]; m=max(len(x[0]) for x in q); x=torch.zeros((len(q),m),dtype=torch.long,device=device); side=torch.tensor([x[1] for x in q],dtype=torch.bool,device=device)
        for i,r in enumerate(q): x[i,:len(r[0])]=torch.tensor(r[0],device=device)
        y=torch.tensor([1. if r[3] is not None and r[3]>0 else -1. if r[3] is not None else np.tanh(r[2]/600.) for r in q],dtype=torch.float32,device=device)
        opt.zero_grad(set_to_none=True); ((n(x,side)-y)**2).mean().backward(); opt.step()
        if device.type=='cuda': peak=max(peak,torch.cuda.max_memory_allocated(device))
    sec=time.time()-start; print(f'positions=500000 batch_size={batch_size} seconds={sec:.6f} positions_per_second={500000/sec:.2f} peak_cuda_memory_bytes={peak}')

def main():
    p=argparse.ArgumentParser(); p.add_argument('--cache',action='store_true'); p.add_argument('--benchmark',action='store_true'); p.add_argument('--train',default='/tmp/howl-nnue-raw-teacher/train.parquet'); p.add_argument('--validation',default='/tmp/howl-nnue-raw-teacher/validation.parquet'); p.add_argument('--train-cache',default='/tmp/howl-nnue-raw-teacher-cache/train.parquet'); p.add_argument('--validation-cache',default='/tmp/howl-nnue-raw-teacher-cache/validation.parquet'); p.add_argument('--batch-size',type=int,choices=(1024,2048,4096,8192),default=1024); p.add_argument('--device',default='cuda'); a=p.parse_args()
    if a.cache:
        os.makedirs(os.path.dirname(a.train_cache),exist_ok=True); cache_one(a.train,a.train_cache); cache_one(a.validation,a.validation_cache); return
    if a.benchmark:
        d=torch.device(a.device)
        if d.type=='cuda' and not torch.cuda.is_available(): raise SystemExit('CUDA is unavailable')
        benchmark(a.train_cache,a.batch_size,d)
if __name__=='__main__': main()
