#!/usr/bin/env python3
import argparse,random,struct,time
import numpy as np
import torch
from torch import nn
FEATURES,ACC,DENSE=40960,256,32
STATIC_LINEAR_SCALE=3000.
MAGIC,VERSION=0x3145554E,1
CLASSES={1:0,2:1,3:2,4:3,5:4,9:5,10:6,11:7,12:8,13:9}
def dev(x): return torch.device('cuda' if x=='auto' and torch.cuda.is_available() else ('cpu' if x=='auto' else x))
def parse(f):
 b,t=f.split()[:2]; ps=[]; wk=bk=-1
 for r,row in enumerate(reversed(b.split('/'))):
  c=0
  for x in row:
   if x.isdigit(): c+=int(x); continue
   s=r*8+c;c+=1
   if x=='K':wk=s
   elif x=='k':bk=s
   else:ps.append((s,{'P':1,'N':2,'B':3,'R':4,'Q':5,'p':9,'n':10,'b':11,'r':12,'q':13}[x]))
 return wk,bk,ps,t=='b'
def row(k,w,s,p):
 c=CLASSES[p]%5; own=p<9 if w else p>8; c+=0 if own else 5
 return ((k if w else k^63)*10+c)*64+(s if w else s^63)
def item(f):
 wk,bk,ps,black=parse(f); return [row(wk,1,s,p) for s,p in ps]+[row(bk,0,s,p) for s,p in ps],black
class Net(nn.Module):
 def __init__(self,factorized=False):
  super().__init__(); self.factorized=factorized; self.w1=nn.Embedding(FEATURES,ACC);self.p1=nn.Embedding(640,ACC) if factorized else None;self.b1=nn.Parameter(torch.zeros(ACC));self.fc1=nn.Linear(512,32);self.fc2=nn.Linear(32,32);self.out=nn.Linear(32,1)
 def forward(self,x,b):
  p=x.shape[1]//2;e=self.w1(x); 
  if self.factorized:e=e+self.p1(torch.remainder(x,640))
  a=e[:,:p].sum(1)+self.b1;o=e[:,p:].sum(1)+self.b1;s=torch.where(b[:,None],o,a);q=torch.where(b[:,None],a,o);return self.out(torch.relu(self.fc2(torch.relu(self.fc1(torch.cat((s,q),1)))))).squeeze(1)
def data(path,limit=None):
 z=[]
 with open(path) as f:
  static=False
  for l in f:
   a=l.rstrip().split('\t');
   if a[0]=='index':continue
   if a[0]=='fen' and len(a)>1 and a[1]=='static_cp': static=True; continue
   fens=a[0] if len(a)==2 else a[1]; v=float(a[-1]);
   if len(a)>2:
    s=int(a[2]);v=1. if s==100000 else -1. if s==-100000 else np.tanh(s/1000.)
   else:v=np.clip(v/STATIC_LINEAR_SCALE,-1.,1.) if static else (1-v if fens.split()[1]=='b' else v)
   ix,b=item(fens);z.append((fens,ix,b,np.float32(v)))
   if limit and len(z)>=limit:break
 return z
def batch(d,i,n,device):
 q=d[i:i+n];m=max(len(x[1]) for x in q);x=torch.zeros(len(q),m,dtype=torch.long,device=device);bs=[];y=[]
 for j,(_,ix,b,v) in enumerate(q):x[j,:len(ix)]=torch.tensor(ix,device=device);bs.append(b);y.append(v)
 return x,torch.tensor(bs,device=device),torch.tensor(y,device=device)
def save(path,n):
  with open(path,'wb') as f:
   f.write(struct.pack('<II',MAGIC,VERSION))
   w=n.w1.weight.detach().cpu()
   if n.factorized:w=w+n.p1.weight.detach().cpu()[torch.arange(FEATURES)%640]
   for x in (w,n.b1,n.fc1.weight,n.fc1.bias,n.fc2.weight,n.fc2.bias,n.out.weight,n.out.bias):f.write(x.numpy().astype('<f4').tobytes())
def epoch(n,opt,d,device,bs):
 random.Random(42).shuffle(d);n.train();tot=0
 for i in range(0,len(d),bs):
  x,b,y=batch(d,i,bs,device);opt.zero_grad(set_to_none=True);loss=((n(x,b)-y)**2).mean();loss.backward();opt.step();tot+=float(loss)*len(y)
 return tot/len(d)
def main():
 p=argparse.ArgumentParser();p.add_argument('--device',default='auto');p.add_argument('--batch-size',type=int,default=1024);p.add_argument('--epochs',type=int,default=5);p.add_argument('--train',required=True);p.add_argument('--validation');p.add_argument('--output',default='/tmp/howl_nnue_pytorch.weights');p.add_argument('--benchmark',type=int);p.add_argument('--overfit-512',action='store_true');p.add_argument('--factorized',action='store_true');a=p.parse_args();device=dev(a.device);torch.manual_seed(1);d=data(a.train,512 if a.overfit_512 else a.benchmark);n=Net(a.factorized).to(device);opt=torch.optim.Adam(n.parameters(),lr=.001)
 if a.benchmark:
  t=time.time();epoch(n,opt,d,device,a.batch_size);s=time.time()-t;print(f'device={device} positions={len(d)} seconds={s:.6f} positions_per_second={len(d)/s:.2f}');return
 v=data(a.validation) if a.validation else d;best=1e99
 for e in range(1,a.epochs+1):
  tr=epoch(n,opt,d,device,a.batch_size);n.eval();pr=[]
  with torch.no_grad():
   for i in range(0,len(v),a.batch_size):pr.extend(n(*batch(v,i,a.batch_size,device)[:2]).cpu().numpy())
  y=np.array([x[3] for x in v]);m=float(np.mean((np.array(pr)-y)**2));print(f'epoch={e} train_mse={tr:.8f} validation_mse={m:.8f}')
  if m<best:best=m;save(a.output,n)
 print(f'best_validation_mse={best:.8f} overfit={best<.02 if a.overfit_512 else "n/a"} saved={a.output}')
if __name__=='__main__':main()
