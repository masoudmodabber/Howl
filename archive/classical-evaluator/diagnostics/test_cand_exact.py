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

prod_mg = interpolate([0, 4, 8, 12, 15], decode_anchors(mg_params)) + [20]*12
prod_eg = interpolate([0, 3, 6, 9, 12], decode_anchors(eg_params)) + [131]*15

# Candidate 2: 7 parameters:
# MG: p0 = Base (-6), p1 = Low-Mid slope (3), p2 = Active slope (15), p3 = Saturation slope (8)
# EG: p4 = Base (10), p5 = Central slope (94), p6 = Saturation slope (27)
# Let's test candidate generators:
def generate_cand_mg(p):
    anchors = [p[0], p[0] + max(0, p[1]), p[0] + max(0, p[1]) + max(0, p[2]), p[0] + max(0, p[1]) + max(0, p[2]) + max(0, p[3])]
    # buckets: 0, 4, 8, 15
    out = [0] * 28
    buckets = [0, 4, 8, 15]
    for a in range(3):
        firstB = buckets[a]
        span = buckets[a+1] - firstB
        firstV = anchors[a]
        delta = anchors[a+1] - firstV
        for offset in range(span):
            out[firstB + offset] = firstV + round_divide(delta * offset, span)
    out[15] = anchors[3]
    for i in range(16, 28):
        out[i] = anchors[3]
    return out

cand_p_mg = [-6, 3, 15, 8]
cand_mg = generate_cand_mg(cand_p_mg)
print("MG diff:", [cand_mg[i] - prod_mg[i] for i in range(28)])

# Notice: between bucket 8 and 15:
# prod_mg has anchor at 12 (16) and 15 (20):
# 8: 12, 9: 13, 10: 14, 11: 15, 12: 16, 13: 17, 14: 19, 15: 20
# With a single span 8 to 15 (span = 7, delta = 8):
# offset 0: 12 + round(0/7) = 12
# offset 1: 12 + round(8/7) = 13
# offset 2: 12 + round(16/7) = 14
# offset 3: 12 + round(24/7) = 15
# offset 4: 12 + round(32/7) = 17 (vs 16 in prod!)
# offset 5: 12 + round(40/7) = 18 (vs 17 in prod!)
# offset 6: 12 + round(48/7) = 19 (vs 19 in prod!)
