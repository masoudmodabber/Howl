#!/usr/bin/env python3
import argparse, os, random, time
import numpy as np
import pyarrow.parquet as pq
import torch
from nnue_trainer import Net, save

def rows(path):
    t=pq.read_table(path,columns=['features','black','cp','mate'])
    return list(zip(t['features'].to_pylist(),t['black'].to_pylist(),t['cp'].to_pylist(),t['mate'].to_pylist()))

def batch(q, device):
    m=max(len(r[0]) for r in q); x=torch.zeros((len(q),m),dtype=torch.long,device=device)
    for i,r in enumerate(q): x[i,:len(r[0])]=torch.tensor(r[0],device=device)
    side=torch.tensor([r[1] for r in q],dtype=torch.bool,device=device)
    y=torch.tensor([1. if r[3] is not None and r[3]>0 else -1. if r[3] is not None else np.tanh(r[2]/600.) for r in q],dtype=torch.float32,device=device)
    return x,side,y

def evaluate(n, data, bs, device):
    n.eval(); pred=[]; target=[]
    with torch.no_grad():
        for i in range(0,len(data),bs):
            q=data[i:i+bs]; pred.extend(n(*batch(q,device)[:2]).cpu().numpy()); target.extend(batch(q,device)[2].cpu().numpy())
    p=np.asarray(pred); y=np.asarray(target); mse=float(np.mean((p-y)**2)); corr=float(np.corrcoef(p,y)[0,1])
    cm=np.array([r[3] is None for r in data]); true=np.array([r[2] for r in data if r[3] is None],float); z=np.clip(p[cm],-.999999,.999999); err=np.abs(np.arctanh(z)*600.-true)
    mm=~cm; mate=np.array([r[3] for r in data if r[3] is not None],float)
    return mse,corr,float(err.mean()),float(np.median(err)),float(np.mean(np.sign(np.arctanh(z)*600.)==np.sign(true))),float(np.mean(np.sign(p[mm])==np.sign(mate)))

def main():
    p=argparse.ArgumentParser(); p.add_argument('--train',default='/tmp/howl-nnue-raw-teacher-cache/train.parquet'); p.add_argument('--validation',default='/tmp/howl-nnue-raw-teacher-cache/validation.parquet'); p.add_argument('--device',default='cuda'); p.add_argument('--batch-size',type=int,default=4096); p.add_argument('--epochs',type=int,default=5); p.add_argument('--checkpoint-dir',default='/tmp/howl-nnue-full-run'); p.add_argument('--resume',action='store_true'); a=p.parse_args()
    d=torch.device(a.device)
    if d.type=='cuda' and not torch.cuda.is_available(): raise SystemExit('CUDA is unavailable')
    os.makedirs(a.checkpoint_dir,exist_ok=True); tr=rows(a.train); va=rows(a.validation); n=Net().to(d); opt=torch.optim.Adam(n.parameters(),lr=.001); start=0; best=1e99
    if a.resume:
        paths=sorted([x for x in os.listdir(a.checkpoint_dir) if x.startswith('epoch-') and x.endswith('.pt')])
        if paths:
            ck=torch.load(os.path.join(a.checkpoint_dir,paths[-1]),map_location=d); n.load_state_dict(ck['model']);opt.load_state_dict(ck['optimizer']);start=ck['epoch'];best=ck['best']
    for e in range(start+1,a.epochs+1):
        begin=time.time(); order=list(range(len(tr)));random.Random(42+e).shuffle(order);n.train(); total=0.
        for base in range(0,len(order),a.batch_size):
            q=[tr[i] for i in order[base:base+a.batch_size]]; x,s,y=batch(q,d);opt.zero_grad(set_to_none=True); loss=((n(x,s)-y)**2).mean();loss.backward();opt.step();total+=float(loss)*len(q)
        metrics=evaluate(n,va,a.batch_size,d); sec=time.time()-begin; val=metrics[0]
        ck={'epoch':e,'model':n.state_dict(),'optimizer':opt.state_dict(),'best':min(best,val)};torch.save(ck,os.path.join(a.checkpoint_dir,f'epoch-{e}.pt'));torch.save(n.state_dict(),os.path.join(a.checkpoint_dir,'final.pt'))
        if val<best: best=val;torch.save(n.state_dict(),os.path.join(a.checkpoint_dir,'best.pt'))
        print(f'epoch={e} train_loss={total/len(tr):.8f} validation_mse={metrics[0]:.8f} validation_pearson={metrics[1]:.8f} cp_mae={metrics[2]:.4f} cp_median_ae={metrics[3]:.4f} cp_sign={metrics[4]:.6f} mate_sign={metrics[5]:.6f} seconds={sec:.2f}',flush=True)
    n.load_state_dict(torch.load(os.path.join(a.checkpoint_dir,'best.pt'),map_location=d));save(os.path.join(a.checkpoint_dir,'best.weights'),n)
if __name__=='__main__': main()
