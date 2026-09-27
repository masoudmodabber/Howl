#!/usr/bin/env python3
"""Structured NNUE V3: V2 with unclipped ReLU hidden layers."""
import argparse
import os
import sys
import time

import torch
from torch import nn

import nnue_structured_v2 as v2


class V3(v2.V2):
    def forward(self, x, black, mask=None):
        p = x.shape[1] // 2
        e = self.real(x) + self.factor(torch.remainder(x, v2.FACTOR))
        if mask is not None:
            e = e * mask.unsqueeze(-1)
        a = self.clip(e[:, :p].sum(1) + self.ft_bias)
        o = self.clip(e[:, p:].sum(1) + self.ft_bias)
        s = torch.where(black[:, None], o, a)
        q = torch.where(black[:, None], a, o)
        return self.out(torch.relu(self.f2(torch.relu(self.f1(torch.cat((s, q), 1)))))).squeeze(1)


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--train', nargs='+', required=True)
    p.add_argument('--validation', dest='validation_paths', nargs='+', required=True)
    p.add_argument('--output', required=True)
    p.add_argument('--device', default='cpu')
    p.add_argument('--batch-size', type=int, default=4096)
    p.add_argument('--loader-workers', type=int, default=8)
    p.add_argument('--prefetch', type=int, default=16)
    p.add_argument('--arrow-batch-size', type=int, default=131072)
    p.add_argument('--epochs', type=int, default=1)
    p.add_argument('--smoke', type=int, default=5_000_000)
    p.add_argument('--max-validation', type=int, default=100_000)
    p.add_argument('--min-knodes', type=int, default=3000)
    p.add_argument('--lr', type=float, default=3e-4)
    p.add_argument('--seed', type=int, default=1)
    a = p.parse_args()
    d = torch.device(a.device)
    if d.type == 'cuda' and not torch.cuda.is_available():
        raise RuntimeError('CUDA requested but unavailable')
    os.makedirs(a.output, exist_ok=True)
    n = V3(a.seed).to(d)
    opt = torch.optim.Adam(n.parameters(), lr=a.lr)
    paths = v2.stream_paths(a.train)
    for epoch in range(1, a.epochs + 1):
        a.validation = False
        st, _, loss, seconds = v2.stream_epoch(n, opt, paths, d, a, epoch, True)
        a.validation = True
        a.smoke = a.max_validation
        vst, _, _, _ = v2.stream_epoch(n, opt, v2.stream_paths(a.validation_paths), d, a, epoch, False)
        torch.save({'epoch': epoch, 'model': n.state_dict(), 'optimizer': opt.state_dict()},
                   os.path.join(a.output, f'epoch-{epoch}.pt'))
        print('epoch', epoch, 'train_wdl', loss, 'train_seconds', seconds,
              'accepted', st['accepted'], 'validation_rows', vst['validation'], flush=True)


if __name__ == '__main__':
    main()
