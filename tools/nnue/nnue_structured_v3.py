#!/usr/bin/env python3
"""Structured NNUE V3: V2 with unclipped ReLU hidden layers."""
import argparse
import os
import random
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
    p.add_argument('--train', nargs='+')
    p.add_argument('--validation', dest='validation_paths', nargs='+')
    p.add_argument('--output')
    p.add_argument('--device', default='cpu')
    p.add_argument('--batch-size', type=int, default=4096)
    p.add_argument('--loader-workers', type=int, default=8)
    p.add_argument('--prefetch', type=int, default=16)
    p.add_argument('--arrow-batch-size', type=int, default=131072)
    p.add_argument('--epochs', type=int, default=5)
    p.add_argument('--train-limit', type=int)
    p.add_argument('--validation-limit', type=int)
    p.add_argument('--max-validation', type=int)
    p.add_argument('--warmup-steps', type=int, default=1000)
    p.add_argument('--final-lr-multiplier', type=float, default=.1)
    p.add_argument('--resume')
    p.add_argument('--stream', action='store_true')
    p.add_argument('--smoke', type=int)
    p.add_argument('--evaluate-checkpoint')
    p.add_argument('--min-knodes', type=int, default=3000)
    p.add_argument('--lr', type=float, default=3e-4)
    p.add_argument('--seed', type=int, default=1)
    a = p.parse_args()
    d = torch.device(a.device)
    if d.type == 'cuda' and not torch.cuda.is_available():
        raise RuntimeError('CUDA requested but unavailable')
    if a.evaluate_checkpoint:
        ck = torch.load(a.evaluate_checkpoint, map_location='cpu', weights_only=False)
        n = V3(a.seed); n.load_state_dict(ck['model']); n.eval()
        v2.evaluate_stream(n, v2.stream_paths(a.validation_paths), a)
        v2.evaluate_sanity(n)
        return
    if not a.train or not a.validation_paths or not a.output:
        p.error('--train, --validation, and --output are required for training')
    os.makedirs(a.output, exist_ok=True)
    n = V3(a.seed).to(d)
    opt = torch.optim.Adam(n.parameters(), lr=a.lr)
    if not a.stream:
        train = v2.load(a.train[0], a.train_limit or 10**18, 11)
        val = v2.load(a.validation_paths[0], a.validation_limit or 10**18, 12)
        step = 0
        if a.resume:
            ck = torch.load(a.resume, map_location=d, weights_only=False)
            n.load_state_dict(ck['model']); opt.load_state_dict(ck['optimizer']); step = ck.get('step', 0)
        total = max(1, a.epochs * ((len(train) + a.batch_size - 1) // a.batch_size))
        best = float('inf')
        for epoch in range(1, a.epochs + 1):
            random.Random(a.seed + epoch).shuffle(train); n.train()
            for i in range(0, len(train), a.batch_size):
                x, b, y, mask = v2.cached_batch(train[i:i+a.batch_size], d)
                opt.zero_grad(set_to_none=True); z = n(x, b, mask)
                loss = ((torch.sigmoid(z / v2.WDL_SCALE) - y) ** 2).mean()
                loss.backward(); step += 1
                warm = min(1., step / max(1, a.warmup_steps))
                decay = max(a.final_lr_multiplier, 1 - (step / total) * (1 - a.final_lr_multiplier))
                for g in opt.param_groups: g['lr'] = a.lr * warm * decay
                opt.step()
            vm = v2.metrics(n, val, d, a.batch_size)
            torch.save({'epoch': epoch, 'model': n.state_dict(), 'optimizer': opt.state_dict(),
                        'validation_wdl_loss': vm[0], 'step': step}, os.path.join(a.output, f'epoch-{epoch}.pt'))
            if vm[0] < best:
                best = vm[0]
                torch.save({'epoch': epoch, 'model': n.state_dict(), 'optimizer': opt.state_dict(),
                            'validation_wdl_loss': vm[0], 'step': step}, os.path.join(a.output, 'best.pt'))
            print('epoch', epoch, 'validation_wdl', vm[0], flush=True)
        return
    paths = v2.stream_paths(a.train)
    start_epoch = 1
    if a.resume:
        ck = torch.load(a.resume, map_location=d, weights_only=False)
        n.load_state_dict(ck['model'])
        opt.load_state_dict(ck['optimizer'])
        start_epoch = int(ck.get('epoch', 0)) + 1
    for epoch in range(start_epoch, start_epoch + a.epochs):
        a.validation = False
        st, _, loss, seconds = v2.stream_epoch(n, opt, paths, d, a, epoch, True)
        a.validation = True
        a.smoke = a.max_validation
        vst, _, _, _ = v2.stream_epoch(n, opt, v2.stream_paths(a.validation_paths), d, a, epoch, False)
        torch.save({'epoch': epoch, 'model': n.state_dict(), 'optimizer': opt.state_dict()},
                   os.path.join(a.output, f'epoch-{epoch}.pt'))
        print('epoch', epoch, 'train_wdl', loss, 'train_seconds', seconds,
              'accepted', st['accepted'], 'positions_per_second', st['accepted'] / max(seconds, 1e-9),
              'batches_per_second', st.get('batches', 0) / max(seconds, 1e-9),
              'validation_rows', vst['validation'], flush=True)


if __name__ == '__main__':
    main()
