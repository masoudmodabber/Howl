#!/usr/bin/env python3
import argparse,glob,hashlib,os,sqlite3,pyarrow as pa,pyarrow.parquet as pq
def main():
 p=argparse.ArgumentParser();p.add_argument('--input',nargs='+',required=True);p.add_argument('--output',required=True);p.add_argument('--db',default='/tmp/howl-lichess-v2.sqlite');p.add_argument('--min-knodes',type=int,default=3000);p.add_argument('--validation-fraction',type=float,default=.05);a=p.parse_args(); shards=[]
 for x in a.input: shards.extend(sorted(glob.glob(x)))
 con=sqlite3.connect(a.db);con.execute('pragma journal_mode=wal');con.execute('create table if not exists d(fen text primary key,knodes integer,depth integer,cp integer,mate integer)');con.commit()
 for path in shards:
  for batch in pq.ParquetFile(path).iter_batches(columns=['fen','knodes','depth','cp','mate'],batch_size=65536):
   cols=[x.to_pylist() for x in batch.columns]
   for row in zip(*cols):
    fen,kn,dep,cp,mate=row; old=con.execute('select knodes from d where fen=?',(fen,)).fetchone()
    if old is None or kn>=old[0]: con.execute('insert or replace into d values(?,?,?,?,?)',(fen,kn,dep,cp,mate))
  con.commit();print('completed',path,flush=True)
 rows=con.execute('select fen,knodes,depth,cp,mate from d where knodes>=?',(a.min_knodes,)).fetchall(); tr=[];va=[]
 for r in rows: (va if int.from_bytes(hashlib.sha256(r[0].encode()).digest()[:8],'little')/2**64<a.validation_fraction else tr).append(r)
 os.makedirs(os.path.dirname(a.output) or '.',exist_ok=True)
 for name,data in [('train',tr),('validation',va)]: pq.write_table(pa.table({'fen':[r[0] for r in data],'knodes':[r[1] for r in data],'depth':[r[2] for r in data],'cp':[r[3] for r in data],'mate':[r[4] for r in data]}),os.path.join(a.output,f'{name}.parquet'))
 print('shards',len(shards),'unique',con.execute('select count(*) from d').fetchone()[0],'retained',len(rows),'train',len(tr),'validation',len(va))
if __name__=='__main__':main()
