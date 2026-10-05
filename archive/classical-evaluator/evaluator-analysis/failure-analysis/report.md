# Systematic Evaluator Failure Analysis Report

## Executive Summary
- **Total candidate positions analyzed:** 100
- **Likely evaluation related:** 36
- **Likely search related:** 31
- **Mixed:** 16
- **Unresolved:** 17
- **Total causal ablation probes conducted:** 416
- **Preference reversals observed via ablation:** 33

## Search vs Evaluation Diagnostic Ladder Breakdown

| Classification | Count | Percentage |
| :--- | :--- | :--- |
| likely search related | 31 | 31.0% |
| likely evaluation related | 36 | 36.0% |
| mixed | 16 | 16.0% |
| unresolved | 17 | 17.0% |

## Causal Ablation Key Findings
Controlled family scalings (0.0x, 0.5x, 1.0x, 1.5x) revealed that in 33 cases, scaling down a suspect family (primarily KingSafety, Attack, or PieceSquare) directly reversed Howl's preference away from the blunder and restored the reference move ordering.
