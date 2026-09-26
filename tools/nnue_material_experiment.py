#!/usr/bin/env python3
import random, argparse
import numpy as np
import chess, torch
from nnue_trainer import Net, item
from nnue_init_lr_experiment import material_sanity

VALUES = {'P':100,'N':320,'B':330,'R':500,'Q':900}

def make_positions(n, seed):
    rng=random.Random(seed); out=[]
    kinds='PNBRQpnbrq'
    while len(out)<n:
        b=chess.Board(None); b.turn=bool(rng.getrandbits(1)); b.castling_rights=0
        squares=list(range(64)); wk=rng.choice(squares); squares.remove(wk)
        bk=rng.choice([s for s in squares if chess.square_distance(wk,s)>1]); squares.remove(bk)
        b.set_piece_at(wk,chess.Piece(chess.KING,chess.WHITE)); b.set_piece_at(bk,chess.Piece(chess.KING,chess.BLACK))
        for color in (chess.WHITE,chess.BLACK):
            for typ in (chess.PAWN,chess.KNIGHT,chess.BISHOP,chess.ROOK,chess.QUEEN):
                if rng.random()<.72:
                    count=rng.randrange(0,3 if typ!=chess.QUEEN else 2)
                    for _ in range(count):
                        avail=[s for s in squares if not (typ==chess.PAWN and chess.square_rank(s) in (0,7))]
                        if not avail: break
                        s=rng.choice(avail); squares.remove(s); b.set_piece_at(s,chess.Piece(typ,color))
        if b.is_valid():
            fen=b.fen(); wk,bk,ps,black=(None,None,None,None)
            _,_,_,black = __import__('nnue_trainer').parse(fen)
            cp=0
            for s,p in ps if False else []: pass
            for sq,p in b.piece_map().items():
                if p.piece_type!=chess.KING:
                    cp += (1 if p.color==b.turn else -1)*VALUES[p.symbol().upper()]
            out.append((fen,cp))
    return out

def batch(data, device):
    parsed=[(item(fen),cp) for fen,cp in data]
    m=max(len(z[0][0]) for z in parsed); x=torch.zeros(len(data),m,dtype=torch.long,device=device); side=[]; y=[]
    for i,((ix,black),cp) in enumerate(parsed):
        x[i,:len(ix)]=torch.tensor(ix,device=device); side.append(black); y.append(np.tanh(cp/600.))
    return x,torch.tensor(side,dtype=torch.bool,device=device),torch.tensor(y,dtype=torch.float32,device=device)

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--device',default='cpu'); ap.add_argument('--train-count',type=int,default=100000); ap.add_argument('--validation-count',type=int,default=20000); a=ap.parse_args()
    random.seed(1); np.random.seed(1); torch.manual_seed(1); d=torch.device(a.device)
    tr=make_positions(a.train_count,11); va=make_positions(a.validation_count,12); n=Net().to(d); n.w1.weight.data.normal_(0,.05); n.b1.data.zero_(); n.fc1.bias.data.zero_(); n.fc2.bias.data.zero_(); n.out.bias.data.zero_(); opt=torch.optim.Adam(n.parameters(),lr=.0003)
    for e in range(1,6):
        rng=random.Random(100+e); rng.shuffle(tr); total=0
        n.train()
        for i in range(0,len(tr),4096):
            x,s,y=batch(tr[i:i+4096],d); opt.zero_grad(set_to_none=True); loss=((n(x,s)-y)**2).mean(); loss.backward(); opt.step(); total+=float(loss.detach())*len(y)
        n.eval(); pred=[]; target=[]
        with torch.no_grad():
            for i in range(0,len(va),4096):
                x,s,y=batch(va[i:i+4096],d); pred.extend(n(x,s).cpu().numpy()); target.extend(y.cpu().numpy())
        pred=np.asarray(pred);target=np.asarray(target); mse=np.mean((pred-target)**2); corr=np.corrcoef(pred,target)[0,1]; rem,syn,_,_=material_sanity(n)
        print(f'epoch {e} train_mse {total/len(tr):.8f} validation_mse {mse:.8f} pearson {corr:.6f} removal {rem}/10 synthetic {syn}/6',flush=True)
    print(f'training_positions {len(tr)} validation_positions {len(va)}')
if __name__=='__main__': main()
