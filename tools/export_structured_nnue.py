#!/usr/bin/env python3
import argparse, struct
from pathlib import Path
import numpy as np
import torch

MAGIC=b'HOWLSTRC'; VERSION=2
ORDER=('ft.weight','ft_bias','f1.weight','f1.bias','f2.weight','f2.bias','out.weight','out.bias')

def write(path,tensors):
    with open(path,'wb') as f:
        f.write(struct.pack('<8sII',MAGIC,VERSION,len(ORDER)))
        for name in ORDER:
            a=tensors[name].detach().cpu().numpy().astype('<f4',copy=False)
            nb=name.encode(); f.write(struct.pack('<II',len(nb),a.ndim)); f.write(nb)
            f.write(struct.pack('<'+'I'*a.ndim,*a.shape)); f.write(a.tobytes(order='C'))

def read(path):
    out={}
    with open(path,'rb') as f:
        magic,version,count=struct.unpack('<8sII',f.read(16))
        if magic!=MAGIC or version!=VERSION or count!=len(ORDER): raise ValueError('invalid structured NNUE header')
        for _ in range(count):
            n,nd=struct.unpack('<II',f.read(8)); name=f.read(n).decode(); shape=struct.unpack('<'+'I'*nd,f.read(4*nd)); count=int(np.prod(shape)); out[name]=np.frombuffer(f.read(4*count),dtype='<f4').reshape(shape).copy()
    return out

def main():
    p=argparse.ArgumentParser();p.add_argument('checkpoint');p.add_argument('output');a=p.parse_args()
    ck=torch.load(a.checkpoint,map_location='cpu',weights_only=False); tensors=ck['model'].copy()
    if 'real.weight' in tensors and 'factor.weight' in tensors:
        tensors['ft.weight']=tensors.pop('real.weight')+tensors.pop('factor.weight').repeat(64,1)
    missing=[x for x in ORDER if x not in tensors]
    if missing: raise ValueError(f'missing tensors: {missing}')
    print('tensors:')
    for name in ORDER: print(name,tuple(tensors[name].shape),tensors[name].numel(),tensors[name].dtype)
    write(a.output,tensors); loaded=read(a.output); diff=max(float(np.max(np.abs(loaded[n]-tensors[n].detach().cpu().numpy()))) for n in ORDER)
    params=sum(tensors[n].numel() for n in ORDER); expected=16+sum(8+len(n.encode())+4*tensors[n].ndim+4*tensors[n].numel() for n in ORDER)
    print('parameter_count',params);print('expected_size',expected);print('actual_size',Path(a.output).stat().st_size);print('roundtrip_max_difference',diff)
if __name__=='__main__': main()
