# Howl NNUE Training Pipeline

This folder contains the complete toolchain to train and export an NNUE network for the Howl chess engine.

## Requirements

Install Python dependencies:

```bash
pip install -r requirements.txt
```

## 1. Prepare Training Data

Prepare training and validation datasets from Lichess position evaluations Parquet shards:

```bash
python3 prepare_data.py \
  --input "/path/to/lichess/*.parquet" \
  --output data/ \
  --min-knodes 3000 \
  --validation-fraction 0.05
```

This deduplicates positions and retains evaluations backed by at least 3M searched nodes, producing `data/train.parquet` and `data/validation.parquet`.

## 2. Train the Network

Train the NNUE architecture:

```bash
python3 train.py \
  --train data/train.parquet \
  --validation data/validation.parquet \
  --output checkpoints/ \
  --epochs 5 \
  --batch-size 4096 \
  --device cuda
```

## 3. Export Weights for Howl

Convert the PyTorch checkpoint to Howl's binary weight file (`HOWLSTRC` format):

```bash
python3 export.py checkpoints/best.pt ../nnue.weights
```

The exported `nnue.weights` in the repository root will be loaded automatically by Howl.
