#!/usr/bin/env python3
import random, chess
from nnue_trainer import item

PIECE_TYPES=(chess.PAWN,chess.KNIGHT,chess.BISHOP,chess.ROOK,chess.QUEEN)
NAMES={1:'pawn',2:'knight',3:'bishop',4:'rook',5:'queen'}

def feature_pair(original_fen, piece_type, own, added, seed=0):
    """Return (original_fen, counterfactual_fen, metadata), or None if unavailable."""
    b=chess.Board(original_fen); color=b.turn if own else not b.turn
    candidates=[s for s in chess.SQUARES if (b.piece_at(s) is not None and b.piece_at(s).color==color and b.piece_at(s).piece_type==piece_type)] if not added else [s for s in chess.SQUARES if b.piece_at(s) is None and chess.square_rank(s) not in ((0,7) if piece_type==chess.PAWN else ())]
    rng=random.Random(seed); rng.shuffle(candidates)
    for sq in candidates:
        c=b.copy(stack=False)
        if added: c.set_piece_at(sq,chess.Piece(piece_type,color))
        else: c.remove_piece_at(sq)
        if c.is_valid() and len(c.piece_map())==len(b.piece_map())+(1 if added else -1):
            item(original_fen); item(c.fen())
            return original_fen,c.fen(),{'piece':NAMES[piece_type],'own':own,'added':added,'expected':'up' if (added==own) else 'down'}
    return None

def random_legal(rng):
    b=chess.Board(None); free=list(chess.SQUARES); wk=rng.choice(free);free.remove(wk); ok=[s for s in free if chess.square_distance(wk,s)>1];
    if not ok:return None
    bk=rng.choice(ok);free.remove(bk);b.set_piece_at(wk,chess.Piece(chess.KING,True));b.set_piece_at(bk,chess.Piece(chess.KING,False));b.turn=rng.choice([True,False]);b.castling_rights=0
    for color in (True,False):
        for _ in range(rng.randrange(2,7)):
            typ=rng.choice(PIECE_TYPES); ss=[s for s in free if not (typ==1 and chess.square_rank(s) in (0,7))]
            if not ss: break
            sq=rng.choice(ss);free.remove(sq);b.set_piece_at(sq,chess.Piece(typ,color))
    return b if b.is_valid() else None

def generate(count=1000,seed=1):
    rng=random.Random(seed); out=[]
    while len(out)<count:
        b=random_legal(rng)
        if b is None: continue
        typ=rng.choice(PIECE_TYPES);own=rng.choice([True,False]);added=rng.choice([True,False]);p=feature_pair(b.fen(),typ,own,added,rng.randrange(1<<30))
        if p: out.append(p)
    return out

def validate(pairs):
    bad_legal=bad_diff=0; by_type={};by_side={};by_op={};examples=[]
    for a,b,m in pairs:
        x=chess.Board(a);y=chess.Board(b); d=[]
        for sq in chess.SQUARES:
            if x.piece_at(sq)!=y.piece_at(sq): d.append(sq)
        if not x.is_valid() or not y.is_valid():bad_legal+=1
        if len(d)!=1 or (x.piece_at(d[0]) is not None and y.piece_at(d[0]) is not None):bad_diff+=1
        by_type[m['piece']]=by_type.get(m['piece'],0)+1;by_side['own' if m['own'] else 'opponent']=by_side.get('own' if m['own'] else 'opponent',0)+1;by_op['add' if m['added'] else 'remove']=by_op.get('add' if m['added'] else 'remove',0)+1
        if len(examples)<10:examples.append((a,b,m))
    return bad_legal,bad_diff,by_type,by_side,by_op,examples

if __name__=='__main__':
    p=generate();r=validate(p);print('pairs',len(p),'illegal',r[0],'multi_diff',r[1]);print('piece',r[2]);print('side',r[3]);print('operation',r[4]);
    for x in r[5]:print(x)
