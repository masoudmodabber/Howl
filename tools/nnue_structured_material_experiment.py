#!/usr/bin/env python3
import random, argparse, os
import numpy as np, chess, torch
import pyarrow.parquet as pq
from nnue_material_pairs import generate as generate_material_pairs
from torch import nn
from nnue_trainer import item, parse, CLASSES

VAL={'P':100,'N':320,'B':330,'R':500,'Q':900}

def make_positions(n,seed):
    rng=random.Random(seed); out=[]
    while len(out)<n:
        b=chess.Board(None); b.turn=bool(rng.getrandbits(1)); b.castling_rights=0
        free=list(range(64)); wk=rng.choice(free);free.remove(wk)
        ok=[s for s in free if chess.square_distance(wk,s)>1]
        if not ok: continue
        bk=rng.choice(ok);free.remove(bk); b.set_piece_at(wk,chess.Piece(chess.KING,1));b.set_piece_at(bk,chess.Piece(chess.KING,0))
        for color in (1,0):
            for typ in [rng.choice((1,2,3,4,5)) for _ in range(5)]:
                avail=[s for s in free if not (typ==1 and chess.square_rank(s) in (0,7))]
                if not avail: break
                s=rng.choice(avail);free.remove(s);b.set_piece_at(s,chess.Piece(typ,color))
        if not b.is_valid(): continue
        cp=sum((1 if p.color==b.turn else -1)*VAL[p.symbol().upper()] for p in b.piece_map().values() if p.piece_type!=6)
        out.append((b.fen(),cp))
    return out

class Structured(nn.Module):
    def __init__(self):
        super().__init__(); self.real=nn.Embedding(40960,256);self.base=nn.Embedding(640,256)
        self.scalar=nn.Embedding(640,1);self.sres=nn.Embedding(40960,1)
        self.b=nn.Parameter(torch.zeros(256));self.f1=nn.Linear(512,32);self.f2=nn.Linear(32,32);self.out=nn.Linear(32,1)
        self.real.weight.data.normal_(0,.0001);self.base.weight.data.normal_(0,.05);self.scalar.weight.data.zero_();self.sres.weight.data.zero_()
        for c,v in enumerate((100,320,330,500,900)):
            self.scalar.weight.data[c*64:(c+1)*64,0].fill_(v/600)
            self.scalar.weight.data[(c+5)*64:(c+6)*64,0].fill_(-v/600)
        for x in (self.f1,self.f2,self.out): x.bias.data.zero_()
        self.out.weight.data.zero_()
    def forward(self,x,black,mask=None):
        p=x.shape[1]//2; real=self.real(x); base=self.base(torch.remainder(x,640)); e=real+base
        if mask is not None: e=e*mask.unsqueeze(-1)
        a=e[:,:p].sum(1)+self.b;o=e[:,p:].sum(1)+self.b;s=torch.where(black[:,None],o,a);q=torch.where(black[:,None],a,o)
        nnv=self.out(torch.relu(self.f2(torch.relu(self.f1(torch.cat((s,q),1)))))).squeeze(1)
        sc=self.scalar(torch.remainder(x,640))+self.sres(x)
        if mask is not None: sc=sc*mask.unsqueeze(-1)
        sa=sc[:,:p].sum(1).squeeze(1);so=sc[:,p:].sum(1).squeeze(1)
        return nnv+torch.tanh(torch.where(black,so,sa))

def batch(q,d):
    parsed=[(item(f),cp) for f,cp in q];p=max(len(x[0][0])//2 for x in parsed);x=torch.zeros(len(q),2*p,dtype=torch.long,device=d);mask=torch.zeros(len(q),2*p,dtype=torch.float32,device=d);bs=[];y=[]
    for i,((ix,b),cp) in enumerate(parsed):k=len(ix)//2;x[i,:k]=torch.tensor(ix[:k],device=d);x[i,p:p+k]=torch.tensor(ix[k:],device=d);mask[i,:k]=1;mask[i,p:p+k]=1;bs.append(b);y.append(np.tanh(cp/600))
    return x,torch.tensor(bs,dtype=torch.bool,device=d),torch.tensor(y,dtype=torch.float32,device=d),mask

def load_cached(path, limit, seed):
    t=pq.read_table(path,columns=['features','black','cp','mate'])
    q=list(zip(t['features'].to_pylist(),t['black'].to_pylist(),t['cp'].to_pylist(),t['mate'].to_pylist()))
    random.Random(seed).shuffle(q)
    return q[:limit]

def cached_batch(q,d):
    p=max(len(r[0])//2 for r in q); x=torch.zeros(len(q),2*p,dtype=torch.long,device=d);mask=torch.zeros(len(q),2*p,dtype=torch.float32,device=d);bs=[];y=[]
    for i,(ix,b,cp,mate) in enumerate(q):
        k=len(ix)//2;x[i,:k]=torch.tensor(ix[:k],device=d);x[i,p:p+k]=torch.tensor(ix[k:],device=d);mask[i,:k]=1;mask[i,p:p+k]=1;bs.append(b);y.append(1. if mate is not None and mate>0 else -1. if mate is not None else np.tanh(cp/600.))
    return x,torch.tensor(bs,dtype=torch.bool,device=d),torch.tensor(y,dtype=torch.float32,device=d),mask

def cached_metrics(n,q,d):
    pred=[];target=[];cp=[];mate=[]
    n.eval()
    with torch.no_grad():
        for i in range(0,len(q),4096):
            x,b,y,m=cached_batch(q[i:i+4096],d);pred.extend(n(x,b,m).cpu().numpy());target.extend(y.cpu().numpy())
    pred=np.asarray(pred);target=np.asarray(target); valid=np.array([r[3] is None for r in q]); cps=np.array([r[2] for r in q if r[3] is None],float); pp=np.clip(pred[valid],-.999999,.999999)*0
    pp=np.arctanh(np.clip(pred[valid],-.999999,.999999))*600
    mates=np.array([r[3] for r in q if r[3] is not None],float); mp=pred[~valid]
    return (np.mean((pred-target)**2),np.corrcoef(pred,target)[0,1],np.mean(abs(pp-cps)),np.median(abs(pp-cps)),np.mean(np.sign(pp)==np.sign(cps)),np.mean(np.sign(mp)==np.sign(mates)))

def pair_score_batch(n, pairs, d):
    q=[(a,0) for a,b,m in pairs]+[(b,0) for a,b,m in pairs]
    x,side,_,m=batch(q,d); return n(x,side,m).reshape(2,len(pairs))

def material_loss(n, pairs, d):
    z=pair_score_batch(n,pairs,d); loss=[]
    for i,(_,_,m) in enumerate(pairs):
        a,b=z[0,i],z[1,i]
        if m['added']: v=torch.relu(a-b) if m['own'] else torch.relu(b-a)
        else: v=torch.relu(b-a) if m['own'] else torch.relu(a-b)
        loss.append(v)
    return torch.stack(loss).mean()

def run_calibration(a):
    d=torch.device(a.device); tr=load_cached(a.train,1000000,11);va=load_cached(a.validation,100000,12); pool=generate_material_pairs(50000,20260926); os.makedirs(a.output,exist_ok=True)
    for lam in a.lambdas:
        out=os.path.join(a.output,f'lambda-{lam:g}');os.makedirs(out,exist_ok=True);n=Structured().to(d);o=torch.optim.Adam(n.parameters(),lr=.0003);ck=torch.load(a.resume_from,map_location=d,weights_only=False);n.load_state_dict(ck['model']);o.load_state_dict(ck['optimizer'])
        for e in range(1,4):
            random.Random(700+e).shuffle(tr);n.train();tm=ml=0
            for i in range(0,len(tr),4096):
                x,b,y,m=cached_batch(tr[i:i+4096],d);q=random.Random(900000+i+e).sample(pool,64);o.zero_grad(set_to_none=True);teacher=((n(x,b,m)-y)**2).mean();aux=material_loss(n,q,d);loss=teacher+lam*aux;loss.backward();o.step();tm+=float(teacher.detach())*len(y);ml+=float(aux.detach())
            m=cached_metrics(n,va,d);rm,sy=sanity(n,d);payload={'epoch':e+7,'lambda':lam,'model':n.state_dict(),'optimizer':o.state_dict(),'validation_mse':m[0]};torch.save(payload,os.path.join(out,f'epoch-{e+7}.pt'))
            print('lambda',lam,'epoch',e+7,'teacher_mse',tm/len(tr),'material_loss',ml/(len(tr)//4096+1),'validation_mse',m[0],'pearson',m[1],'cp_mae',m[2],'cp_median_abs_error',m[3],'cp_sign_accuracy',m[4],'mate_sign_accuracy',m[5],'material_removal',f'{rm}/10','synthetic_material',f'{sy}/6',flush=True)

def run_cached(a):
    d=torch.device(a.device); torch.manual_seed(1)
    tr=load_cached(a.train,a.train_limit,11);va=load_cached(a.validation,a.validation_limit,12);n=Structured().to(d);o=torch.optim.Adam(n.parameters(),lr=.0003);os.makedirs(a.output,exist_ok=True)
    start=1;best=float('inf')
    if a.resume:
        done=[int(x.split('-')[1].split('.')[0]) for x in os.listdir(a.output) if x.startswith('epoch-') and x.endswith('.pt')]
        if done:
            e=max(done);ck=torch.load(os.path.join(a.output,f'epoch-{e}.pt'),map_location=d,weights_only=False);n.load_state_dict(ck['model']);o.load_state_dict(ck['optimizer']);start=e+1
            if os.path.exists(os.path.join(a.output,'best.pt')):
                best=float(torch.load(os.path.join(a.output,'best.pt'),map_location='cpu',weights_only=False).get('validation_mse',best))
    for e in range(start,a.epochs+1):
        random.Random(100+e).shuffle(tr);n.train();tot=0
        for i in range(0,len(tr),4096):
            x,b,y,m=cached_batch(tr[i:i+4096],d);o.zero_grad(set_to_none=True);l=((n(x,b,m)-y)**2).mean();l.backward();o.step();tot+=float(l.detach())*len(y)
        m=cached_metrics(n,va,d);rm,sy=sanity(n,d);payload={'epoch':e,'model':n.state_dict(),'optimizer':o.state_dict(),'validation_mse':m[0]};torch.save(payload,os.path.join(a.output,f'epoch-{e}.pt'))
        print('epoch',e,'train_mse',tot/len(tr),'validation_mse',m[0],'pearson',m[1],'cp_mae',m[2],'cp_median_abs_error',m[3],'cp_sign_accuracy',m[4],'mate_sign_accuracy',m[5], 'material_removal',f'{rm}/10','synthetic_material',f'{sy}/6',flush=True)
        if m[0]<best: best=m[0];torch.save(payload,os.path.join(a.output,'best.pt'))
        if rm<9 or sy<6: print('material_gate_failed',flush=True);break

def sanity(n,d):
    def sc(f):
        x,b,y,m=batch([(f,0)],d)
        with torch.no_grad():return float(n(x,b,m)[0])
    base='rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'; z=sc(base); rem=0
    for ch,ri in [('P',6),('N',7),('B',7),('R',7),('Q',7),('p',1),('n',0),('b',0),('r',0),('q',0)]:
        r=base.split();rows=r[0].split('/');a=list(rows[ri]);a[a.index(ch)]='1';rows[ri]=''.join(a);v=sc('/'.join(rows)+' '+' '.join(r[1:]));rem+=((v-z)<0 if ch.isupper() else (v-z)>0)
    b='4k3/8/8/8/8/8/8/4K3 w - - 0 1';z=sc(b);syn=[]
    for f,sg in [('4k3/8/8/8/8/8/4P3/4K3 w - - 0 1',1),('4k3/4p3/8/8/8/8/8/4K3 w - - 0 1',-1),('4k3/8/8/8/2N5/8/8/4K3 w - - 0 1',1),('4k3/2n5/8/8/8/8/8/4K3 w - - 0 1',-1),('4k3/8/8/8/8/8/3Q4/4K3 w - - 0 1',1),('3qk3/8/8/8/8/8/8/4K3 w - - 0 1',-1)]:syn.append((sc(f)-z)*sg>0)
    return rem,sum(syn)

def main():
    p=argparse.ArgumentParser();p.add_argument('--device',default='cpu');p.add_argument('--stockfish',action='store_true');p.add_argument('--material-calibration',action='store_true');p.add_argument('--train');p.add_argument('--validation');p.add_argument('--output',default='/tmp/howl-nnue-structured-stockfish');p.add_argument('--train-limit',type=int,default=1000000);p.add_argument('--validation-limit',type=int,default=100000);p.add_argument('--epochs',type=int,default=3);p.add_argument('--resume',action='store_true');p.add_argument('--resume-from',default='/tmp/howl-nnue-structured-full/epoch-7.pt');p.add_argument('--lambdas',type=float,nargs='+',default=[.01,.05,.10]);a=p.parse_args()
    if a.material_calibration:return run_calibration(a)
    if a.stockfish:return run_cached(a)
    d=torch.device(a.device);torch.manual_seed(1);tr=make_positions(100000,11);va=make_positions(20000,12);n=Structured().to(d);o=torch.optim.Adam(n.parameters(),lr=.0003)
    n.eval();pred=[];ys=[]
    with torch.no_grad():
        for i in range(0,len(va),4096):x,b,y,m=batch(va[i:i+4096],d);pred.extend(n(x,b,m).cpu().numpy().reshape(-1));ys.extend(y.cpu().numpy().reshape(-1))
    print('initial',np.mean((np.array(pred)-ys)**2),np.corrcoef(pred,ys)[0,1],*sanity(n,d),flush=True)
    for e in range(1,4):
        random.Random(100+e).shuffle(tr);tot=0;n.train()
        for i in range(0,len(tr),4096):
            x,b,y,m=batch(tr[i:i+4096],d);o.zero_grad(set_to_none=True);l=((n(x,b,m)-y)**2).mean();l.backward();o.step();tot+=float(l.detach())*len(y)
        n.eval();pred=[];ys=[]
        with torch.no_grad():
            for i in range(0,len(va),4096):x,b,y,m=batch(va[i:i+4096],d);pred.extend(n(x,b,m).cpu().numpy().reshape(-1));ys.extend(y.cpu().numpy().reshape(-1))
        print(e,tot/len(tr),np.mean((np.array(pred)-ys)**2),np.corrcoef(pred,ys)[0,1],*sanity(n,d),flush=True)
if __name__=='__main__':main()
