#!/usr/bin/env python3
import argparse, collections, numpy as np, pyarrow.parquet as pq
def main():
 p=argparse.ArgumentParser();p.add_argument('path');p.add_argument('--batch-size',type=int,default=65536);a=p.parse_args(); c=np.zeros(40960,dtype=np.int64); rows=0; stm=[0,0]; cps=[]; mates=0; total=0; king=collections.Counter(); cls=collections.Counter(); sq=collections.Counter()
 for b in pq.ParquetFile(a.path).iter_batches(batch_size=a.batch_size,columns=['features','black','cp','mate']):
  for ix,black,cp,mate in zip(b.column('features').to_pylist(),b.column('black').to_pylist(),b.column('cp').to_pylist(),b.column('mate').to_pylist()):
   rows+=1;stm[int(bool(black))]+=1; c[np.asarray(ix,dtype=np.int64)]+=1; total+=len(ix); 
   for f in ix: king[f//640]+=1; cls[(f%640)//64]+=1; sq[f%64]+=1
   if mate is None:cps.append(cp)
   else:mates+=1
 nz=c[c>0];print('rows',rows,'active_occurrences',total,'seen_rows',int((c>0).sum()),'unseen_rows',int((c==0).sum()));print('frequency_min_nonzero',nz.min() if len(nz) else 0,'p1',np.percentile(nz,1) if len(nz) else 0,'p5',np.percentile(nz,5) if len(nz) else 0,'p10',np.percentile(nz,10) if len(nz) else 0,'median',np.median(nz) if len(nz) else 0,'p90',np.percentile(nz,90) if len(nz) else 0,'p95',np.percentile(nz,95) if len(nz) else 0,'p99',np.percentile(nz,99) if len(nz) else 0,'max',nz.max() if len(nz) else 0);print('lt10',int((c<10).sum()),'lt100',int((c<100).sum()),'lt1000',int((c<1000).sum()),'lt10000',int((c<10000).sum()));print('stm_white',stm[0],'stm_black',stm[1],'cp',len(cps),'mate',mates,'class',dict(cls),'king',dict(king),'square',dict(sq))
if __name__=='__main__':main()
