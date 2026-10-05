# Verification of Candidate 2 table generation
def round_divide(num, den):
    if num >= 0:
        return (num + den // 2) // den
    else:
        return -((-num + den // 2) // den)

def decode_anchors(params):
    anchors = [params[0]]
    for p in params[1:]:
        anchors.append(anchors[-1] + max(0, p))
    return anchors

def interpolate(buckets, anchors):
    out = [0] * (buckets[-1] + 1)
    for a in range(len(anchors) - 1):
        firstB = buckets[a]
        span = buckets[a+1] - firstB
        firstV = anchors[a]
        delta = anchors[a+1] - firstV
        for offset in range(span):
            out[firstB + offset] = firstV + round_divide(delta * offset, span)
    out[buckets[-1]] = anchors[-1]
    return out

mg_params = [-6, 3, 15, 4, 4]
eg_params = [10, 30, 33, 31, 27]

mg_anchors = decode_anchors(mg_params)
eg_anchors = decode_anchors(eg_params)

mg_buckets = [0, 4, 8, 12, 15]
eg_buckets = [0, 3, 6, 9, 12]

prod_mg = interpolate(mg_buckets, mg_anchors) + [mg_anchors[-1]] * 12
prod_eg = interpolate(eg_buckets, eg_anchors) + [eg_anchors[-1]] * 15

# Candidate 2:
# In Candidate 2, we directly store the 4 key anchor points or linear increments for MG:
# P0: Base (moveCount 0) = -6
# P1: Low-Mid slope increment (moveCount 4: -6 + 3 = -3) -> increment 3
# P2: Active central slope increment (moveCount 8: -3 + 15 = 12) -> increment 15
# P3: Saturation cap increment (moveCount 12: 12 + 4 = 16, count 15: 16 + 4 = 20) -> increment 8 (or 4,4)
# And for EG:
# P4: EG Base = 10
# P5: EG Central Slope = 63
# P6: EG Saturation Cap = 58
