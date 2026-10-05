#!/usr/bin/env python3
"""Corrected independent-opening SPSA signal study. Never updates parameters."""
import argparse,csv,io,json,math,os,random,re,statistics,subprocess,sys,time
from concurrent.futures import ProcessPoolExecutor,as_completed
from pathlib import Path
import chess.pgn
HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE))
from fishtest_spsa import atomic_json,build_variant,read_manifest,stochastic_round
from spsa_signal_study import bootstrap_ci,sign,write_tsv
from spsa_tune import SingleGameTask,UCIEngineProcess,_run_single_game_worker

SCALES=(.25,.125,.0625,.03125,.015625); PREFIXES=(8,16,32,64)
def read_tsv(p):
 with Path(p).open(newline='',encoding='utf-8') as f:return list(csv.DictReader(f,delimiter='\t'))
def vector(manifest,theta,direction,scale,seed=42):
 rng=random.Random(seed+direction*1000003);p={};m={};changed=0;disp=[]
 factor=4000**.101*scale
 for r in manifest:
  flip=rng.choice((-1,1));v=float(theta[r['name']]);lo=float(r['minimum']);hi=float(r['maximum']);c=float(r['natural_scale'])*factor
  p[r['name']]=stochastic_round(min(max(v+c*flip,lo),hi),rng);m[r['name']]=stochastic_round(min(max(v-c*flip,lo),hi),rng)
  changed+=p[r['name']]!=m[r['name']];disp.append(abs(p[r['name']]-m[r['name']])/2)
 return p,m,changed/len(manifest),statistics.mean(disp)
def root_worker(t):
 idx,fen,plus,minus,nodes=t; moves=[];scores=[]
 for path in (plus,minus):
  e=UCIEngineProcess(path,{})
  try:e.new_game();moves.append(e.get_move(fen,[],1000,1000,0,0,30,nodes) or '')
  finally:e.close()
 return idx,moves
def eval_build(repo,build,out,vec,label):
 from tune_evaluator_move_regret import replace_option_source
 d=out/'eval-builds'/label;b=d/'howl_eval_breakdown'
 if b.exists():return b
 d.mkdir(parents=True,exist_ok=True);src=d/'Option.cpp';obj=d/'Option.cpp.o';replace_option_source(vec,src)
 subprocess.run(['c++','-O3','-DNDEBUG','-std=gnu++17',f'-I{repo}',f'-I{repo/"third_party/fathom/src"}','-c',str(src),'-o',str(obj)],check=True)
 objs=[str(p) for p in (build/'CMakeFiles/howl_eval_breakdown.dir').rglob('*.o') if p.name!='Option.cpp.o']
 subprocess.run(['c++','-O3','-DNDEBUG',*objs,str(obj),str(build/'libfathom.a'),'-lpthread','-o',str(b)],check=True);return b
def static_score(binary,fen):
 p=subprocess.run([str(binary),fen],text=True,capture_output=True,check=True);m=re.search(r'White Perspective\s+-\s+-\s+(-?\d+)',p.stdout)
 if not m:raise RuntimeError('static score missing')
 return int(m.group(1))
def game_tasks(openings,plus,minus,nodes,workers):
 tasks=[]
 for pair,o in enumerate(openings):
  for color in range(2):
   i=pair*2+color;tasks.append(SingleGameTask(i,o['id'],o['fen'],'plus' if color==0 else 'minus','minus' if color==0 else 'plus',str(plus),str(minus),1,0,i%workers,{},nodes))
 return tasks
def games(tasks,workers):
 out=[None]*len(tasks);start=time.monotonic()
 with ProcessPoolExecutor(max_workers=workers) as ex:
  fs={ex.submit(_run_single_game_worker,t):t.game_index for t in tasks}
  for f in as_completed(fs):out[fs[f]]=f.result()
 bad=[r for r in out if not r.is_valid]
 return out,time.monotonic()-start,bad
def pairs(results):
 a=[]
 for i in range(0,len(results),2):
  s=results[i].plus_score+results[i+1].plus_score;a.append(s-(2-s))
 return a
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--repo-root',default='.');ap.add_argument('--workers',type=int,default=16);ap.add_argument('--output',default='move-quality-tuning/spsa-signal-study');args=ap.parse_args()
 repo=Path(args.repo_root).resolve();build=repo/'build';out=repo/args.output; openings=read_tsv(out/'openings-128.tsv');manifest=read_manifest(repo/'move-quality-tuning/parameter-manifest.tsv');theta={r['name']:int(r['current_value']) for r in manifest}
 screen_path=out/'corrected-c-screen.tsv';screens=read_tsv(screen_path) if screen_path.exists() else []
 completed_scales={float(r['scale']) for r in screens}
 for scale in SCALES:
  if scale in completed_scales:continue
  disagreements=[];staticdiff=[];fractions=[];disps=[];fail=0
  for direction in range(20):
   pv,mv,fraction,disp=vector(manifest,theta,direction,scale);fractions.append(fraction);disps.append(disp)
   pb=build_variant(repo,build,out,pv,f'corrected-{scale}-{direction}-plus');mb=build_variant(repo,build,out,mv,f'corrected-{scale}-{direction}-minus')
   pe=eval_build(repo,build,out,pv,f'{scale}-{direction}-plus');me=eval_build(repo,build,out,mv,f'{scale}-{direction}-minus')
   with ProcessPoolExecutor(max_workers=args.workers) as ex:
    roots=list(ex.map(root_worker,[(i,o['fen'],str(pb),str(mb),1000) for i,o in enumerate(openings)]))
   disagreements.extend(int(x[1][0]!=x[1][1]) for x in roots);fail+=sum(not all(x[1]) for x in roots)
   with ProcessPoolExecutor(max_workers=args.workers) as ex:
    ps=list(ex.map(static_score,[pe]*len(openings),[o['fen'] for o in openings]));ms=list(ex.map(static_score,[me]*len(openings),[o['fen'] for o in openings]))
   staticdiff.extend(int(a!=b) for a,b in zip(ps,ms))
  screens.append({'scale':scale,'root_disagreement':statistics.mean(disagreements),'static_difference':statistics.mean(staticdiff),'mean_parameter_fraction_changed':statistics.mean(fractions),'mean_absolute_displacement':statistics.mean(disps),'failures':fail})
  write_tsv(screen_path,screens)
  if .05<=screens[-1]['root_disagreement']<=.25 and screens[-1]['mean_parameter_fraction_changed']>=.05 and fail==0:break
 selected=float(screens[-1]['scale']); signal=[];pairraw=[];times=[]
 for direction in range(20):
  pv,mv,_,_=vector(manifest,theta,direction,selected);pb=build_variant(repo,build,out,pv,f'corrected-{selected}-{direction}-plus');mb=build_variant(repo,build,out,mv,f'corrected-{selected}-{direction}-minus')
  rs,elapsed,bad=games(game_tasks(openings[:64],pb,mb,1000,args.workers),args.workers);times.append(elapsed)
  if bad:raise RuntimeError(f'{len(bad)} failures at selected scale')
  ds=pairs(rs);d64=sum(ds);ci=bootstrap_ci(ds,42000+direction)
  for i,d in enumerate(ds):pairraw.append({'direction':direction,'nodes':1000,'pair':i,'D':d})
  for n in PREFIXES:
   d=sum(ds[:n]);signal.append({'direction':direction,'pairs':n,'D':d,'sign':sign(d),'reference_D':d64,'agreement':int(sign(d)==sign(d64)),'ci_low':ci[0] if n==64 else '','ci_high':ci[1] if n==64 else ''})
 write_tsv(out/'corrected-signal.tsv',signal);write_tsv(out/'corrected-pairs.tsv',pairraw)
 budget=[]
 for nodes in (5000,20000):
  for direction in range(10):
   pv,mv,_,_=vector(manifest,theta,direction,selected);pb=build_variant(repo,build,out,pv,f'corrected-{selected}-{direction}-plus');mb=build_variant(repo,build,out,mv,f'corrected-{selected}-{direction}-minus')
   rs,elapsed,bad=games(game_tasks(openings[:64],pb,mb,nodes,args.workers),args.workers)
   if bad:
    raise RuntimeError(f'{len(bad)} failures at {nodes} nodes')
   d=sum(pairs(rs))
   base=sum(float(r['D']) for r in pairraw if int(r['direction'])==direction and int(r['nodes'])==1000)
   budget.append({'nodes':nodes,'direction':direction,'D':d,'sign':sign(d),'sign_vs_1000':int(sign(d)==sign(base)),'seconds':elapsed})
   write_tsv(out/'corrected-node-budget.tsv',budget)
 agreements={n:statistics.mean(int(r['agreement']) for r in signal if int(r['pairs'])==n) for n in PREFIXES[:-1]}; recpairs=next((n for n in PREFIXES[:-1] if agreements[n]>=.75),64)
 b20=[r for r in budget if int(r['nodes'])==20000];recnodes=20000 if statistics.mean(int(r['sign_vs_1000']) for r in b20)>=.75 else 5000
 sec=statistics.mean(float(r['seconds']) for r in budget if int(r['nodes'])==recnodes)*recpairs/64
 summary={'unique_openings':len({o['fen'] for o in openings}),'selected_scale':selected,'screen':screens[-1],'sign_agreement':agreements,'recommended_nodes':recnodes,'recommended_pairs':recpairs,'seconds_per_update':sec,'projections_seconds':{str(k):sec*k for k in (500,1000,2000)},'parameters':len(manifest)};atomic_json(out/'corrected-summary.json',summary);print(json.dumps(summary,sort_keys=True))
if __name__=='__main__':main()
