#!/usr/bin/env python3
import argparse, random, time
import numpy as np
import pyarrow.parquet as pq
import torch
from nnue_trainer import Net, item

def load(path, limit):
    t = pq.read_table(path, columns=['fen', 'stm_cp', 'stm_mate'])
    out = []
    for fen, cp, mate in zip(t['fen'].to_pylist(), t['stm_cp'].to_pylist(), t['stm_mate'].to_pylist()):
        ix, black = item(fen); out.append((ix, black, cp, mate))
        if len(out) == limit: break
    return out

def batch(rows, device):
    m = max(len(x[0]) for x in rows)
    x = torch.zeros((len(rows), m), dtype=torch.long, device=device)
    side = torch.zeros(len(rows), dtype=torch.bool, device=device)
    for i, (features, black, _, _) in enumerate(rows):
        x[i, :len(features)] = torch.tensor(features, device=device); side[i] = black
    return x, side

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--device', default='cuda'); p.add_argument('--batch-size', type=int, default=1024)
    p.add_argument('--train', default='/tmp/howl-nnue-raw-teacher/train.parquet')
    p.add_argument('--validation', default='/tmp/howl-nnue-raw-teacher/validation.parquet')
    a = p.parse_args(); device = torch.device(a.device)
    if device.type == 'cuda' and not torch.cuda.is_available(): raise SystemExit('CUDA is unavailable')
    torch.manual_seed(1); np.random.seed(1)
    train = load(a.train, 500000); valid = load(a.validation, 50000)
    for scale in (600., 1000., 1500.):
        torch.manual_seed(1); n = Net().to(device); opt = torch.optim.Adam(n.parameters(), lr=.001); start = time.time()
        for epoch in range(2):
            order = list(range(len(train))); random.Random(42 + epoch).shuffle(order); n.train()
            for base in range(0, len(order), a.batch_size):
                rows = [train[i] for i in order[base:base+a.batch_size]]; x, side = batch(rows, device)
                y = torch.tensor([1. if m is not None and m > 0 else -1. if m is not None else np.tanh(cp/scale) for _,_,cp,m in rows], dtype=torch.float32, device=device)
                opt.zero_grad(set_to_none=True); loss = ((n(x, side) - y) ** 2).mean(); loss.backward(); opt.step()
        n.eval(); pred=[]; target=[]
        with torch.no_grad():
            for base in range(0, len(valid), a.batch_size):
                rows=valid[base:base+a.batch_size]; pred.extend(n(*batch(rows,device)).cpu().numpy())
                target.extend(1. if m is not None and m > 0 else -1. if m is not None else np.tanh(cp/scale) for _,_,cp,m in rows)
        pred=np.asarray(pred); target=np.asarray(target); mse=np.mean((pred-target)**2); corr=np.corrcoef(pred,target)[0,1]
        cp_mask=np.array([m is None for _,_,_,m in valid]); true_cp=np.array([cp for _,_,cp,m in valid if m is None],float); est=np.clip(pred[cp_mask],-0.999999,0.999999); err=np.abs(np.arctanh(est)*scale-true_cp)
        mate_mask=~cp_mask; mate_acc=np.mean(np.sign(pred[mate_mask])==np.sign(np.array([m for _,_,_,m in valid if m is not None])))
        print(f'scale={int(scale)} validation_mse={mse:.8f} pearson={corr:.8f} cp_mae={err.mean():.4f} cp_median_ae={np.median(err):.4f} cp_sign={np.mean(np.sign(np.arctanh(est)*scale)==np.sign(true_cp)):.6f} mate_sign={mate_acc:.6f} seconds={time.time()-start:.2f}', flush=True)
if __name__ == '__main__': main()
