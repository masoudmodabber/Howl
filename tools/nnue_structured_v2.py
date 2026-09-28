#!/usr/bin/env python3
import argparse, math, os, random, time, glob, queue, threading, multiprocessing as mp
import numpy as np
import pyarrow.parquet as pq
import torch
from torch import nn
from nnue_trainer import item

FEATURES=40960; FACTOR=640; ACC=256; WDL_SCALE=400.0
PIECE_ID={ord('P'):1,ord('N'):2,ord('B'):3,ord('R'):4,ord('Q'):5,ord('p'):9,ord('n'):10,ord('b'):11,ord('r'):12,ord('q'):13}
def fast_item(fen):
    placement=fen.split(' ',1)[0]; wk=bk=-1; pieces=[]; sq=0
    rows=placement.split('/')
    for row in reversed(rows):
        for ch in row:
            o=ord(ch)
            if 49<=o<=56: sq+=o-48; continue
            if ch=='K': wk=sq
            elif ch=='k': bk=sq
            else: pieces.append((sq,PIECE_ID[o]))
            sq+=1
    white=[];black=[]
    for s,p in pieces:
        c=(p-1 if p<9 else p-9); white.append(((wk*10+(c if p<9 else c+5))*64+s)); black.append((((bk^63)*10+(c if p>8 else c+5))*64+(s^63)))
    return white+black, fen.split()[1]=='b'
def normalize_teacher_pov(cp,mate,black):
    """Convert Lichess white-POV labels to the model's side-to-move POV."""
    if black:
        cp = -cp if cp is not None else None
        mate = -mate if mate is not None else None
    return cp,mate

def target_wdl(cp,mate,black):
    cp,mate=normalize_teacher_pov(cp,mate,black)
    if mate is not None: return 1. if mate>0 else 0.
    return 1./(1.+math.exp(-cp/WDL_SCALE))

class V2(nn.Module):
    def __init__(self, seed=1):
        super().__init__(); torch.manual_seed(seed)
        self.real=nn.Embedding(FEATURES,ACC); self.factor=nn.Embedding(FACTOR,ACC)
        self.ft_bias=nn.Parameter(torch.full((ACC,),.5))
        self.f1=nn.Linear(512,32); self.f2=nn.Linear(32,32); self.out=nn.Linear(32,1)
        nn.init.normal_(self.real.weight,0,.01); nn.init.normal_(self.factor.weight,0,.01)
        for x in (self.f1,self.f2,self.out): nn.init.xavier_uniform_(x.weight); nn.init.constant_(x.bias,0)
    @staticmethod
    def clip(x): return torch.clamp(x,0.,1.)
    def forward(self,x,black,mask=None):
        p=x.shape[1]//2; e=self.real(x)+self.factor(torch.remainder(x,FACTOR))
        if mask is not None: e=e*mask.unsqueeze(-1)
        a=self.clip(e[:,:p].sum(1)+self.ft_bias); o=self.clip(e[:,p:].sum(1)+self.ft_bias)
        s=torch.where(black[:,None],o,a); q=torch.where(black[:,None],a,o)
        h1=self.clip(self.f1(torch.cat((s,q),1))); h2=self.clip(self.f2(h1)); return self.out(h2).squeeze(1)

def cached_batch(rows,device):
    p=max(len(r[0])//2 for r in rows); xa=np.zeros((len(rows),2*p),dtype=np.int64); ma=np.zeros((len(rows),2*p),dtype=np.float32); black=[]; y=[]
    for i,(ix,b,cp,mate) in enumerate(rows):
        k=len(ix)//2; xa[i,:k]=ix[:k]; xa[i,p:p+k]=ix[k:]; ma[i,:k]=1; ma[i,p:p+k]=1; black.append(b)
        y.append(target_wdl(cp,mate,b))
    return torch.from_numpy(xa).to(device),torch.tensor(black,dtype=torch.bool,device=device),torch.tensor(y,dtype=torch.float32,device=device),torch.from_numpy(ma).to(device)

def load(path,limit,seed):
    t=pq.read_table(path,columns=['features','black','cp','mate']); rows=list(zip(t['features'].to_pylist(),t['black'].to_pylist(),t['cp'].to_pylist(),t['mate'].to_pylist())); random.Random(seed).shuffle(rows); return rows[:limit]
def metrics(n,rows,d,batch):
    n.eval(); pr=[]; yt=[]; cps=[]; mates=[]
    with torch.no_grad():
        for i in range(0,len(rows),batch):
            x,b,y,m=cached_batch(rows[i:i+batch],d); z=n(x,b,m); pr.extend(z.cpu().numpy()); yt.extend(y.cpu().numpy())
    pr=np.asarray(pr); yt=np.asarray(yt); pred_cp=pr; norm=np.asarray([normalize_teacher_pov(r[2],r[3],r[1]) for r in rows],object); target=np.asarray([x[0] for x in norm],float); valid=np.asarray([x[1] is None for x in norm]); cpmae=float(np.mean(np.abs(pred_cp[valid]-target[valid]))) if valid.any() else 0.; med=float(np.median(np.abs(pred_cp[valid]-target[valid]))) if valid.any() else 0.; sign=float(np.mean(np.sign(pred_cp[valid])==np.sign(target[valid]))) if valid.any() else 0.; mateok=float(np.mean(np.sign(pred_cp[~valid])==np.sign(np.asarray([x[1] for x in norm if x[1] is not None],float)))) if (~valid).any() else 0.
    return float(np.mean((1/(1+np.exp(-pr))-yt)**2)),float(np.corrcoef(pred_cp[valid],target[valid])[0,1]) if valid.sum()>1 else 0.,cpmae,med,sign,mateok,float(pred_cp.min()),float(pred_cp.max()),float(pred_cp.mean()),int(np.sum(~np.isfinite(pred_cp)))

def stream_paths(values):
    out=[]
    for v in values:
        out.extend(sorted(glob.glob(v)))
    if not out: raise ValueError('no parquet files matched --train/--validation')
    return out

def stream_rows(paths, min_knodes, validation, max_validation=None, seed=1, epoch=1, batch_rows=131072):
    """Yield filtered rows directly from Arrow; split identity is shard/batch/row."""
    order=list(range(len(paths))); random.Random(seed+epoch).shuffle(order)
    count=[0]*8; coverage=np.zeros(FEATURES,dtype=np.int64); emitted=0
    for si in order:
        pf=pq.ParquetFile(paths[si]); groups=list(range(pf.num_row_groups)); random.Random(seed+epoch*1009+si).shuffle(groups)
        for gi in groups:
            for bi,b in enumerate(pf.iter_batches(batch_size=batch_rows,columns=['fen','knodes','cp','mate'])):
                fens=b.column('fen').to_pylist(); kn=b.column('knodes').to_pylist(); cps=b.column('cp').to_pylist(); mates=b.column('mate').to_pylist()
                for ri,(fen,k,cp,mate) in enumerate(zip(fens,kn,cps,mates)):
                    count[0]+=1
                    if k is None or k<min_knodes: count[1]+=1; continue
                    if cp is None and mate is None: continue
                    is_val=((si*1000003+gi*9176+ri*37+17)%50)==0
                    if is_val!=validation: continue
                    if validation and max_validation is not None and emitted>=max_validation: return count,coverage,emitted
                    ix,black=fast_item(fen); count[2]+=1; count[3 if validation else 4]+=1; count[5 if mate is None else 6]+=1; count[7 if black else 4]+=0
                    if black: count[7]+=1
                    else: count[4]+=1
                    coverage[np.asarray(ix,dtype=np.int64)]+=1
                    yield (ix,black,cp,mate); emitted+=1
    return count,coverage,emitted

def stream_epoch_single(n,opt,paths,device,args,epoch,training=True):
    stats={'raw':0,'rejected':0,'accepted':0,'train':0,'validation':0,'cp':0,'mate':0,'white':0,'black':0}; cov=np.zeros(FEATURES,dtype=np.int64); start=time.time()
    accepted_limit = getattr(args, 'validation_limit' if args.validation else 'train_limit', None)
    batches=queue.Queue(maxsize=max(1,getattr(args,'prefetch',4))); sentinel=object()
    def producer():
        rows=[]
        try:
            for si,path in enumerate(paths):
                pf=pq.ParquetFile(path); groups=list(range(pf.num_row_groups)); random.Random(args.seed+epoch*1009+si).shuffle(groups)
                for gi in groups:
                    for b in pf.iter_batches(batch_size=args.arrow_batch_size,columns=['fen','knodes','cp','mate'],row_groups=[gi]):
                        for ri,(fen,k,cp,mate) in enumerate(zip(b.column('fen').to_pylist(),b.column('knodes').to_pylist(),b.column('cp').to_pylist(),b.column('mate').to_pylist())):
                            stats['raw']+=1
                            if k is None or k<args.min_knodes: stats['rejected']+=1; continue
                            if cp is None and mate is None: continue
                            is_val=((si*1000003+gi*9176+ri*37+17)%50)==0
                            if is_val!=args.validation: continue
                            ix,black=fast_item(fen); rows.append((ix,black,cp,mate)); stats['accepted']+=1; stats['validation' if is_val else 'train']+=1; stats['mate' if mate is not None else 'cp']+=1; stats['black' if black else 'white']+=1; cov[np.asarray(ix,dtype=np.int64)]+=1
                            if len(rows)>=args.batch_size:
                                tensors=list(cached_batch(rows,'cpu'))
                                if device.type=='cuda': tensors=[t.pin_memory() if t.device.type=='cpu' else t for t in tensors]
                                batches.put((tensors,len(rows))); rows=[]
                            if accepted_limit is not None and stats['accepted'] >= accepted_limit:
                                raise StopIteration
                            if args.smoke and stats['accepted']>=args.smoke: raise StopIteration
            if rows:
                tensors=list(cached_batch(rows,'cpu'))
                if device.type=='cuda': tensors=[t.pin_memory() if t.device.type=='cpu' else t for t in tensors]
                batches.put((tensors,len(rows)))
        except StopIteration:
            pass
        finally:
            batches.put(sentinel)
    thread=threading.Thread(target=producer,daemon=True); thread.start()
    total_loss=0.; total_n=0
    while True:
        item=batches.get()
        if item is sentinel: break
        tensors,count=item; x,bl,y,m=[t.to(device,non_blocking=device.type=='cuda') for t in tensors]
        if training:
            opt.zero_grad(set_to_none=True); z=n(x,bl,m); loss=((torch.sigmoid(z/WDL_SCALE)-y)**2).mean(); loss.backward(); opt.step(); total_loss+=float(loss.detach().cpu())*count
        else:
            with torch.no_grad(): total_loss+=float(((torch.sigmoid(n(x,bl,m)/WDL_SCALE)-y)**2).mean().cpu())*count
        total_n+=count
    thread.join()
    return stats,cov,total_loss/max(1,total_n),time.time()-start

def _preprocess_chunk(task):
    chunk_id, rows = task
    flat=[]; offsets=[0]; blacks=[]; cps=[]; mates=[]
    for fen,cp,mate in rows:
        ix,black=fast_item(fen)
        flat.extend(ix); offsets.append(len(flat)); blacks.append(black)
        cps.append(np.nan if cp is None else cp)
        mates.append(np.nan if mate is None else mate)
    return chunk_id, (np.asarray(flat,dtype=np.int32), np.asarray(offsets,dtype=np.int32),
                      np.asarray(blacks,dtype=np.bool_), np.asarray(cps,dtype=np.float64),
                      np.asarray(mates,dtype=np.float64))

def _unpack_chunk(payload):
    flat,offsets,blacks,cps,mates=payload
    out=[]
    for i in range(len(blacks)):
        cp=None if np.isnan(cps[i]) else cps[i].item()
        mate=None if np.isnan(mates[i]) else mates[i].item()
        out.append((flat[offsets[i]:offsets[i+1]].tolist(), bool(blacks[i]), cp, mate))
    return out

def stream_epoch(n,opt,paths,device,args,epoch,training=True):
    workers=int(getattr(args,'loader_workers',0))
    if workers<=0:
        return stream_epoch_single(n,opt,paths,device,args,epoch,training)
    stats={'raw':0,'rejected':0,'accepted':0,'train':0,'validation':0,'cp':0,'mate':0,'white':0,'black':0}; cov=np.zeros(FEATURES,dtype=np.int64); start=time.time()
    accepted_limit = getattr(args, 'validation_limit' if args.validation else 'train_limit', None)
    ctx=mp.get_context('fork'); tasks=ctx.Queue(maxsize=max(1,int(getattr(args,'prefetch',8)))); results=ctx.Queue(maxsize=max(1,int(getattr(args,'prefetch',8))))
    stop=threading.Event(); workers_list=[]
    def worker():
        while True:
            task=tasks.get()
            if task is None: return
            try: results.put(('ok',_preprocess_chunk(task)))
            except BaseException as e: results.put(('error',repr(e)))
    for _ in range(workers):
        p=ctx.Process(target=worker); p.start(); workers_list.append(p)
    def reader():
        chunk_id=0
        try:
            for si,path in enumerate(paths):
                pf=pq.ParquetFile(path); groups=list(range(pf.num_row_groups)); random.Random(args.seed+epoch*1009+si).shuffle(groups)
                for gi in groups:
                    for b in pf.iter_batches(batch_size=args.arrow_batch_size,columns=['fen','knodes','cp','mate'],row_groups=[gi]):
                        rows=[]
                        stop_after_chunk=False
                        for ri,(fen,k,cp,mate) in enumerate(zip(b.column('fen').to_pylist(),b.column('knodes').to_pylist(),b.column('cp').to_pylist(),b.column('mate').to_pylist())):
                            stats['raw']+=1
                            if k is None or k<args.min_knodes: stats['rejected']+=1; continue
                            if cp is None and mate is None: continue
                            is_val=((si*1000003+gi*9176+ri*37+17)%50)==0
                            if is_val!=args.validation: continue
                            rows.append((fen,cp,mate)); stats['accepted']+=1; stats['validation' if is_val else 'train']+=1; stats['mate' if mate is not None else 'cp']+=1; stats['black' if fen.split()[1]=='b' else 'white']+=1
                            if accepted_limit is not None and stats['accepted']>=accepted_limit:
                                stop_after_chunk=True
                                break
                        if rows:
                            if stop.is_set(): return
                            tasks.put((chunk_id,rows)); chunk_id+=1
                        if stop_after_chunk: raise StopIteration
                        if args.smoke and stats['accepted']>=args.smoke: raise StopIteration
        except StopIteration: pass
        finally:
            for _ in workers_list:
                if not stop.is_set(): tasks.put(None)
            # This is the authoritative end-of-input marker.  Its payload is
            # the number of chunks submitted, not a queue-state heuristic.
            if not stop.is_set(): results.put(('done',chunk_id))
    reader_thread=threading.Thread(target=reader,daemon=True); reader_thread.start()
    pending={}; next_id=0; carry=[]; total_loss=0.; total_n=0; done=False; expected_chunks=None
    try:
        while not done:
            msg=results.get()
            if msg[0]=='error': raise RuntimeError(msg[1])
            if msg[0]=='done':
                expected_chunks=msg[1]
                if next_id==expected_chunks and not pending: done=True
                continue
            cid,payload=msg[1]; pending[cid]=_unpack_chunk(payload)
            while next_id in pending:
                carry.extend(pending.pop(next_id)); next_id+=1
                while len(carry)>=args.batch_size:
                    batch,carry=carry[:args.batch_size],carry[args.batch_size:]
                    for ix,black,cp,mate in batch: cov[np.asarray(ix,dtype=np.int64)]+=1
                    x,bl,y,m=cached_batch(batch,'cpu')
                    if device.type=='cuda': x,bl,y,m=[t.pin_memory() for t in (x,bl,y,m)]
                    x,bl,y,m=[t.to(device,non_blocking=device.type=='cuda') for t in (x,bl,y,m)]
                    if training:
                        opt.zero_grad(set_to_none=True); z=n(x,bl,m); loss=((torch.sigmoid(z/WDL_SCALE)-y)**2).mean(); loss.backward(); opt.step(); total_loss+=float(loss.detach().cpu())*len(batch)
                    else:
                        with torch.no_grad(): total_loss+=float(((torch.sigmoid(n(x,bl,m)/WDL_SCALE)-y)**2).mean().cpu())*len(batch)
                    total_n+=len(batch)
            if expected_chunks is not None and next_id==expected_chunks and not pending: done=True
        reader_thread.join()
        if carry:
            for ix,black,cp,mate in carry: cov[np.asarray(ix,dtype=np.int64)]+=1
            x,bl,y,m=cached_batch(carry,'cpu')
            if device.type=='cuda': x,bl,y,m=[t.pin_memory() for t in (x,bl,y,m)]
            x,bl,y,m=[t.to(device,non_blocking=device.type=='cuda') for t in (x,bl,y,m)]
            if training:
                opt.zero_grad(set_to_none=True); z=n(x,bl,m); loss=((torch.sigmoid(z/WDL_SCALE)-y)**2).mean(); loss.backward(); opt.step(); total_loss+=float(loss.detach().cpu())*len(carry)
            else:
                with torch.no_grad(): total_loss+=float(((torch.sigmoid(n(x,bl,m)/WDL_SCALE)-y)**2).mean().cpu())*len(carry)
            total_n+=len(carry)
    finally:
        stop.set(); reader_thread.join(timeout=5)
        for p in workers_list:
            if p.is_alive(): p.terminate()
            p.join()
    return stats,cov,total_loss/max(1,total_n),time.time()-start
def coalesce(state):
    state=state.copy(); state['ft.weight']=state.pop('real.weight')+state.pop('factor.weight').repeat(64,1); return state
def run_stream(a):
    d=torch.device(a.device); train_paths=stream_paths(a.train); val_paths=stream_paths(a.validation or a.train); os.makedirs(a.output,exist_ok=True); n=V2(a.seed).to(d); opt=torch.optim.Adam(n.parameters(),lr=a.lr); best=float('inf')
    for epoch in range(1,a.epochs+1):
        args=a; args.validation=False; st,cov,loss,seconds=stream_epoch(n,opt,train_paths,d,args,epoch,True)
        # Validation streams in source order and are not persisted.
        args.validation=True; args.smoke=a.smoke; vst,_,_,_=stream_epoch(n,opt,val_paths,d,args,epoch,False)
        payload={'epoch':epoch,'model':n.state_dict(),'optimizer':opt.state_dict(),'validation_wdl_loss':float('nan'),'step':0}; torch.save(payload,os.path.join(a.output,f'epoch-{epoch}.pt'))
        print('epoch',epoch,'train_wdl',loss,'train_seconds',seconds,'raw_rows',st['raw'],'rejected_knodes',st['rejected'],'accepted',st['accepted'],'train_rows',st['train'],'validation_rows',vst['validation'],'cp_rows',st['cp'],'mate_rows',st['mate'],'white_stm',st['white'],'black_stm',st['black'],'features_seen',int((cov>0).sum()),flush=True)
        if a.smoke: break
def evaluate_stream(n,paths,a):
    rows=[]; preds=[]; targets=[]; cps=[]; mates=[]; accepted=0
    validation_limit = getattr(a, 'validation_limit', None)
    limit = validation_limit if validation_limit is not None else a.max_validation
    for si,path in enumerate(paths):
        pf=pq.ParquetFile(path)
        for gi in range(pf.num_row_groups):
            for b in pf.iter_batches(batch_size=a.arrow_batch_size,columns=['fen','knodes','cp','mate'],row_groups=[gi]):
                for ri,(fen,k,cp,mate) in enumerate(zip(b.column('fen').to_pylist(),b.column('knodes').to_pylist(),b.column('cp').to_pylist(),b.column('mate').to_pylist())):
                    if k is None or k<a.min_knodes or (cp is None and mate is None): continue
                    if ((si*1000003+gi*9176+ri*37+17)%50)!=0: continue
                    ix,black=fast_item(fen); rows.append((ix,black,cp,mate)); accepted+=1
                    if len(rows)>=a.batch_size:
                        x,bl,y,m=cached_batch(rows,'cpu')
                        with torch.no_grad(): preds.extend(n(x,bl,m).numpy()); targets.extend(y.numpy())
                        cps.extend([normalize_teacher_pov(r[2],r[3],r[1])[0] for r in rows]); mates.extend([normalize_teacher_pov(r[2],r[3],r[1])[1] for r in rows]); rows.clear()
                    if limit and accepted>=limit: break
                if limit and accepted>=limit: break
            if limit and accepted>=limit: break
        if limit and accepted>=limit: break
    if rows:
        x,bl,y,m=cached_batch(rows,'cpu')
        with torch.no_grad(): preds.extend(n(x,bl,m).numpy()); targets.extend(y.numpy())
        cps.extend([normalize_teacher_pov(r[2],r[3],r[1])[0] for r in rows]); mates.extend([normalize_teacher_pov(r[2],r[3],r[1])[1] for r in rows])
    pred=np.asarray(preds); target=np.asarray(targets); cp=np.asarray(cps,float); valid=np.asarray([m is None for m in mates]); predcp=pred; finite=np.isfinite(predcp)
    print('validation_positions',len(pred),'validation_wdl_loss',float(np.mean((1/(1+np.exp(-predcp/WDL_SCALE))-target)**2)),'cp_mae',float(np.mean(np.abs(predcp[valid]-cp[valid]))),'cp_median_abs_error',float(np.median(np.abs(predcp[valid]-cp[valid]))),'cp_sign_accuracy',float(np.mean(np.sign(predcp[valid])==np.sign(cp[valid]))),'mate_sign_accuracy',float(np.mean(np.sign(predcp[~valid])==np.where(np.asarray(mates)[~valid]>0,1,-1))) if (~valid).any() else 0.,'prediction_min',float(predcp.min()),'prediction_max',float(predcp.max()),'prediction_mean',float(predcp.mean()),'nan_count',int(np.isnan(predcp).sum()),'inf_count',int(np.isinf(predcp).sum()))
def evaluate_sanity(n):
    base='rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'
    cases={'pawn':('P',6),'knight':('N',7),'bishop':('B',7),'rook':('R',7),'queen':('Q',7)}
    def score(f):
        ix,b=fast_item(f); x,bl,_,m=cached_batch([(ix,b,0,None)],'cpu')
        with torch.no_grad(): return float(n(x,bl,m)[0])
    z=score(base)
    for name,(piece,row) in cases.items():
        r=base.split(); a=list(r[0].split('/')[row]); a[a.index(piece)]='1'; r[0]='/'.join(r[0].split('/')[:row]+[''.join(a)]+r[0].split('/')[row+1:]); d=score(' '.join(r)); print('material',name,'correct_direction',float(d<z),'delta_cp',d-z)
    for label,f in [('A','r1bqk2r/ppppbppp/2n5/3p4/4R3/5N2/PPPP1PPP/R1BQ1BK1 w kq - 0 10'),('B','r1bqk2r/ppp1bppp/2n5/3p4/3R4/5N2/PPPP1PPP/R1BQ1BK1 b kq - 1 10'),('C','r1bqk2r/ppp1bppp/8/3p4/3n4/5N2/PPPP1PPP/R1BQ1BK1 w kq - 0 11')]: print('rook_loss',label,'cp',score(f))
def main():
    p=argparse.ArgumentParser(); p.add_argument('--train',required=False,nargs='+');p.add_argument('--validation',nargs='+');p.add_argument('--output',required=False);p.add_argument('--device',default='cpu');p.add_argument('--batch-size',type=int,default=4096);p.add_argument('--prefetch',type=int,default=4);p.add_argument('--loader-workers',type=int,default=0);p.add_argument('--arrow-batch-size',type=int,default=131072);p.add_argument('--epochs',type=int,default=5);p.add_argument('--train-limit',type=int);p.add_argument('--validation-limit',type=int);p.add_argument('--max-validation',type=int);p.add_argument('--min-knodes',type=int,default=3000);p.add_argument('--lr',type=float,default=3e-4);p.add_argument('--warmup-steps',type=int,default=1000);p.add_argument('--final-lr-multiplier',type=float,default=.1);p.add_argument('--seed',type=int,default=1);p.add_argument('--resume',type=str);p.add_argument('--stream',action='store_true');p.add_argument('--smoke',type=int);p.add_argument('--evaluate-checkpoint');a=p.parse_args()
    if a.evaluate_checkpoint:
        ck=torch.load(a.evaluate_checkpoint,map_location='cpu',weights_only=False); n=V2(a.seed); n.load_state_dict(ck['model']); n.eval(); evaluate_stream(n,stream_paths(a.validation),a); evaluate_sanity(n); return
    if a.stream: return run_stream(a)
    d=torch.device(a.device); tr=load(a.train[0],a.train_limit or 10**18,11); va=load(a.validation[0],a.validation_limit or 10**18,12); os.makedirs(a.output,exist_ok=True); n=V2(a.seed).to(d); opt=torch.optim.Adam(n.parameters(),lr=a.lr); total=max(1,a.epochs*((len(tr)+a.batch_size-1)//a.batch_size)); step=0; best=float('inf')
    if a.resume:
        ck=torch.load(a.resume,map_location=d,weights_only=False); n.load_state_dict(ck['model']); opt.load_state_dict(ck['optimizer']); step=ck.get('step',0)
    for epoch in range(1,a.epochs+1):
        random.Random(a.seed+epoch).shuffle(tr); n.train(); loss_sum=0.; start=time.time()
        for i in range(0,len(tr),a.batch_size):
            x,b,y,m=cached_batch(tr[i:i+a.batch_size],d); opt.zero_grad(set_to_none=True); z=n(x,b,m); pred=torch.sigmoid(z/WDL_SCALE); loss=((pred-y)**2).mean(); loss.backward(); step+=1
            warm=min(1.,step/max(1,a.warmup_steps)); decay=max(a.final_lr_multiplier,1-(step/max(1,total))*(1-a.final_lr_multiplier));
            for g in opt.param_groups:g['lr']=a.lr*warm*decay
            opt.step(); loss_sum+=float(loss.detach())*len(y)
        vm=metrics(n,va,d,a.batch_size); payload={'epoch':epoch,'model':n.state_dict(),'optimizer':opt.state_dict(),'step':step,'validation_wdl_loss':vm[0]}; torch.save(payload,os.path.join(a.output,f'epoch-{epoch}.pt')); 
        if vm[0]<best: best=vm[0]; torch.save(payload,os.path.join(a.output,'best.pt'))
        print('epoch',epoch,'train_wdl',loss_sum/len(tr),'validation_wdl',vm[0],'cp_pearson',vm[1],'cp_mae',vm[2],'cp_median',vm[3],'cp_sign',vm[4],'mate_sign',vm[5],'pred_min',vm[6],'pred_max',vm[7],'pred_mean',vm[8],'nan_inf',vm[9],'seconds',time.time()-start,flush=True)
if __name__=='__main__': main()
