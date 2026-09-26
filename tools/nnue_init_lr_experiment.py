#!/usr/bin/env python3
import argparse, os, random
import numpy as np
import pyarrow.parquet as pq
import torch
import chess
from nnue_trainer import Net

def load(path, limit, seed):
    t=pq.read_table(path,columns=['features','black','cp','mate']); rows=[]
    for ix,b,c,m in zip(t['features'].to_pylist(),t['black'].to_pylist(),t['cp'].to_pylist(),t['mate'].to_pylist()):
        rows.append((ix,b,c,m))
    random.Random(seed).shuffle(rows); return rows[:limit]

def batch(q, d):
    m=max(len(r[0]) for r in q); x=torch.zeros(len(q),m,dtype=torch.long,device=d)
    for i,r in enumerate(q): x[i,:len(r[0])]=torch.tensor(r[0],device=d)
    s=torch.tensor([r[1] for r in q],dtype=torch.bool,device=d)
    y=torch.tensor([1. if r[3] is not None and r[3]>0 else -1. if r[3] is not None else np.tanh(r[2]/600.) for r in q],dtype=torch.float32,device=d)
    return x,s,y

def evaluate(n,data,bs,d):
    n.eval();p=[];y=[]
    with torch.no_grad():
        for i in range(0,len(data),bs):
            q=data[i:i+bs];z=batch(q,d);p.extend(n(z[0],z[1]).cpu().numpy());y.extend(z[2].cpu().numpy())
    p=np.asarray(p);y=np.asarray(y); cm=np.array([r[3] is None for r in data]); cp=np.array([r[2] for r in data if r[3] is None],float); z=np.clip(p[cm],-.999999,.999999); pred=np.arctanh(z)*600
    mm=~cm; mate=np.array([r[3] for r in data if r[3] is not None],float)
    return np.mean((p-y)**2),np.corrcoef(p,y)[0,1],np.mean(abs(pred-cp)),np.median(abs(pred-cp)),np.mean(np.sign(pred)==np.sign(cp)),np.mean(np.sign(p[mm])==np.sign(mate))

def score(n, fen):
    from nnue_trainer import item
    ix, black = item(fen); x=torch.tensor(ix,dtype=torch.long)[None,:]; b=torch.tensor([black])
    with torch.no_grad(): return float(n(x,b)[0])

def material_sanity(n):
    base='rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'
    rows=[('P',6),('N',7),('B',7),('R',7),('Q',7),('p',1),('n',0),('b',0),('r',0),('q',0)]
    fields=base.split(); b=fields[0].split('/'); suffix=fields[1:]; baseline=score(n,base); good=0; deltas=[]
    for ch,rank in rows:
        r=b.copy(); z=list(r[rank]);z[z.index(ch)]='1';r[rank]=''.join(z);d=score(n,'/'.join(r)+' '+' '.join(suffix));delta=d-baseline;deltas.append(delta);good+=delta<0 if ch.isupper() else delta>0
    syn=[('white pawn','4k3/8/8/8/8/8/4P3/4K3 w - - 0 1',1),('black pawn','4k3/4p3/8/8/8/8/8/4K3 w - - 0 1',-1),('white knight','4k3/8/8/8/2N5/8/8/4K3 w - - 0 1',1),('black knight','4k3/2n5/8/8/8/8/8/4K3 w - - 0 1',-1),('white queen','4k3/8/8/8/8/8/3Q4/4K3 w - - 0 1',1),('black queen','3qk3/8/8/8/8/8/8/4K3 w - - 0 1',-1)]
    sb=score(n,'4k3/8/8/8/8/8/8/4K3 w - - 0 1');sg=0;sd=[]
    for name,fen,sign in syn:
        x=score(n,fen)-sb;sd.append((name,x));sg+=x*sign>0
    return good,sg,deltas,sd

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--device',default='auto');ap.add_argument('--batch-size',type=int,default=4096);ap.add_argument('--output-dir',default='/tmp/howl-nnue-init-lr-experiment');ap.add_argument('--train',default='/tmp/howl-nnue-raw-teacher-cache/train.parquet');ap.add_argument('--validation',default='/tmp/howl-nnue-raw-teacher-cache/validation.parquet');ap.add_argument('--diagnostics',action='store_true');a=ap.parse_args();d=torch.device('cuda' if a.device=='auto' and torch.cuda.is_available() else a.device if a.device!='auto' else 'cpu');os.makedirs(a.output_dir,exist_ok=True)
    if a.diagnostics:
        for name in ('A','B','C'):
            for e in (1,2,3):
                n=Net().to(d);ck=torch.load(os.path.join(a.output_dir,f'{name}-epoch-{e}.pt'),map_location=d);n.load_state_dict(ck['model']);r=material_sanity(n);print(name,e,r[0],r[1]);
                if name=='A' and e==3:print('best_raw_deltas',r[2],r[3])
        return
    tr=load(a.train,1000000,20260926);va=load(a.validation,100000,20260927)
    for name,std,lr in [('A',.05,.0003),('B',.05,.0001),('C',.10,.0003)]:
        torch.manual_seed(1);n=Net().to(d);n.w1.weight.data.normal_(0,std);n.b1.data.zero_();n.fc1.bias.data.zero_();n.fc2.bias.data.zero_();n.out.bias.data.zero_();opt=torch.optim.Adam(n.parameters(),lr=lr)
        for e in range(1,4):
            order=list(range(len(tr)));random.Random(42+e).shuffle(order);n.train();total=0
            for k in range(0,len(order),a.batch_size):
                q=[tr[i] for i in order[k:k+a.batch_size]];x,s,y=batch(q,d);opt.zero_grad(set_to_none=True);loss=((n(x,s)-y)**2).mean();loss.backward();opt.step();total+=float(loss)*len(q)
            metrics=evaluate(n,va,a.batch_size,d);torch.save({'config':name,'epoch':e,'model':n.state_dict(),'optimizer':opt.state_dict()},os.path.join(a.output_dir,f'{name}-epoch-{e}.pt'))
            print(name,e,total/len(tr),*metrics, 'material_removal=not-implemented', 'synthetic_material=not-implemented',flush=True)
if __name__=='__main__':main()
