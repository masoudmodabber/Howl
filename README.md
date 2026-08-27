I had to make the previous repo private since it contained some personal information. This one continues a project that’s been evolving for 17 years!

## Azure Cloud Self-Match Runner

A disposable parallel cloud runner for evaluating Howl engine versions using Azure Container Instances (ACI).

### Prerequisites
- Python 3.10+ with `python-chess` installed (`pip install chess`)
- Azure CLI (`az`) installed and authenticated
- Docker installed locally (for image building when running non-dry matches)

### Azure Login
```bash
az login
```

### Running Cloud Self Matches
Run 20 games (10 openings played twice with colours swapped) in parallel on ACI at 5m + 1.5s time control:
```bash
python tools/cloud_self_match.py \
  ./match-engines/howl-old \
  ./build/howl \
  --time 300 \
  --inc 1.5 \
  --games 20 \
  --pgn cloud-match.pgn
```

Preview resource provisioning and opening assignments without deploying:
```bash
python tools/cloud_self_match.py ./match-engines/howl-old ./build/howl --time 300 --inc 1.5 --games 20 --dry-run
```

### Expected Output
- Live single-line game progress for each finished cloud worker instance
- Aggregated win/draw/loss scores and per-opening breakdown matching `tools/self_match.py`
- Combined PGN saved locally to `--pgn` destination

### Disposable Cleanup
All cloud resources are created inside a dedicated temporary resource group (`howl-match-rg-<id>`). Upon match completion, failure, or cancellation (`Ctrl+C`), the temporary resource group is asynchronously deleted via `az group delete`.
