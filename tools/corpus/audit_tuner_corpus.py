#!/usr/bin/env python3
import argparse, collections, math
import chess

VALUES = {chess.PAWN:100, chess.KNIGHT:350, chess.BISHOP:350, chess.ROOK:550, chess.QUEEN:975}

def stable_hash(text):
    value=1469598103934665603
    for byte in text.encode(): value=((value ^ byte)*1099511628211)&((1<<64)-1)
    return value

def passed(board, square, color):
    file=chess.square_file(square); rank=chess.square_rank(square)
    for enemy in board.pieces(chess.PAWN, not color):
        ef=chess.square_file(enemy); er=chess.square_rank(enemy)
        if abs(ef-file)<=1 and ((color and er>rank) or (not color and er<rank)): return False
    return True

def row(fen, result):
    b=chess.Board(fen); pieces=len(b.piece_map()); pawns=len(b.pieces(chess.PAWN,True))+len(b.pieces(chess.PAWN,False))
    npm=sum(VALUES.get(p.piece_type,0) for p in b.piece_map().values() if p.piece_type not in (chess.PAWN,chess.KING))
    balance=sum((1 if p.color else -1)*VALUES.get(p.piece_type,0) for p in b.piece_map().values() if p.piece_type!=chess.KING)
    phase=min(24,sum(({chess.KNIGHT:1,chess.BISHOP:1,chess.ROOK:2,chess.QUEEN:4}.get(p.piece_type,0)) for p in b.piece_map().values()))
    captures=promotions=0
    for m in b.legal_moves:
        captures += b.is_capture(m); promotions += bool(m.promotion)
    any_passed=any(passed(b,s,c) for c in (True,False) for s in b.pieces(chess.PAWN,c))
    stm_result=(1-result) if b.turn==chess.BLACK else result
    return dict(result=result,stm_result=stm_result,white_to_move=int(b.turn),pieces=pieces,pawns=pawns,
      nonpawn_material=npm,material_balance=balance,phase=phase,queens=int(bool(b.queens)),
      both_queens_absent=int(not b.queens),castling=int(bool(b.castling_rights)),check=int(b.is_check()),
      captures=captures,has_capture=int(captures>0),promotion=int(promotions>0),passed=int(any_passed))

def summarize(rows):
    keys=rows[0].keys(); out={}
    for k in keys:
        vals=[r[k] for r in rows]; out[k]=(sum(vals)/len(vals),min(vals),max(vals))
    return out

def ks(a,b):
    a=sorted(a);b=sorted(b);i=j=0;best=0
    while i<len(a) or j<len(b):
        x=min(a[i] if i<len(a) else math.inf,b[j] if j<len(b) else math.inf)
        while i<len(a) and a[i]<=x:i+=1
        while j<len(b) and b[j]<=x:j+=1
        best=max(best,abs(i/len(a)-j/len(b)))
    return best

def main():
    p=argparse.ArgumentParser();p.add_argument('--corpus',default='tuner-train.tsv');p.add_argument('--output-prefix',default='.corpus-audit/corpus');a=p.parse_args()
    entries=[]; exact=collections.Counter(); states=collections.Counter()
    with open(a.corpus) as f:
        for line in f:
            fen,res=line.rstrip().rsplit('\t',1); result=float(res); entries.append((fen,result)); exact[fen]+=1; states[' '.join(fen.split()[:4])]+=1
    subset=[]
    for e in entries:
        if stable_hash(e[0])%100<70:
            subset.append(e)
            if len(subset)==500:break
    fullrows=[row(*e) for e in entries]; subrows=[row(*e) for e in subset]
    import os;os.makedirs(os.path.dirname(a.output_prefix),exist_ok=True)
    for suffix,rows in [('full',fullrows),('subset500',subrows)]:
        with open(a.output_prefix+'-'+suffix+'.tsv','w') as o:
            o.write('metric\tmean\tminimum\tmaximum\n')
            for k,v in summarize(rows).items():o.write(f'{k}\t{v[0]:.12g}\t{v[1]}\t{v[2]}\n')
    numeric=['pieces','pawns','nonpawn_material','material_balance','phase','captures']
    categorical=['result','stm_result','white_to_move','queens','both_queens_absent','castling','check','has_capture','promotion','passed']
    with open(a.output_prefix+'-comparison.tsv','w') as o:
        o.write('metric\tfull_mean\tsubset_mean\tdifference\tstandardized_mean_difference\tks_distance\n')
        for k in numeric+categorical:
            x=[r[k] for r in fullrows];y=[r[k] for r in subrows];mx=sum(x)/len(x);my=sum(y)/len(y)
            vx=sum((z-mx)**2 for z in x)/len(x);vy=sum((z-my)**2 for z in y)/len(y);pooled=math.sqrt((vx+vy)/2)
            o.write(f'{k}\t{mx:.12g}\t{my:.12g}\t{my-mx:.12g}\t{((my-mx)/pooled if pooled else 0):.12g}\t{ks(x,y):.12g}\n')
    labels=collections.Counter(r for _,r in entries);stm=collections.Counter(r['stm_result'] for r in fullrows)
    print('positions',len(entries),'subset',len(subset),'labels',dict(labels),'stm',dict(stm))
    print('exact_duplicate_extra',sum(v-1 for v in exact.values()),'state_duplicate_extra',sum(v-1 for v in states.values()))

if __name__=='__main__':main()
