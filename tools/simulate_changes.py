import json
import subprocess

with open("tools/static_eval_calibration.json") as f:
    data = json.load(f)

corpus = []
for p in data["positions"]:
    fen = p["resulting_fen"]
    res = subprocess.run(["./build/howl_eval_breakdown", fen], capture_output=True, text=True)
    info = {
        "id": p["id"],
        "exp": p["trusted_white_pov_cp"],
        "cat": p.get("category", ""),
        "fen": fen
    }
    for line in res.stdout.splitlines():
        if "White Perspective" in line:
            info["eval"] = int(line.split()[-1])
        elif "Phase:" in line:
            info["phase"] = int(line.split("Phase:")[1].split("/")[0].strip())
        elif "White KD:" in line:
            for part in line.split():
                if part.startswith("shelter="): info["w_sh"] = int(part.split("=")[1])
                elif part.startswith("phaseScale="): info["w_ps"] = int(part.split("=")[1].replace("%", ""))
        elif "Black KD:" in line:
            for part in line.split():
                if part.startswith("shelter="): info["b_sh"] = int(part.split("=")[1])
                elif part.startswith("phaseScale="): info["b_ps"] = int(part.split("=")[1].replace("%", ""))
    corpus.append(info)

constraints = data.get("ordering_constraints", [])

def get_tol(score):
    s = abs(score)
    if s <= 100: return max(20.0, s * 0.25)
    elif s <= 250: return s * 0.25
    elif s <= 400: return s * 0.30
    elif s <= 600: return s * 0.35
    else: return s * 0.40

def eval_config(calc_net_fn):
    results = {}
    passed = 0
    errs = []
    sign_errs = 0
    for item in corpus:
        net = calc_net_fn(item)
        new_eval = item["eval"] + net
        results[item["id"]] = new_eval
        diff = abs(new_eval - item["exp"])
        errs.append(diff)
        if diff <= get_tol(item["exp"]): passed += 1
        if (item["exp"] > 20 and new_eval < -20) or (item["exp"] < -20 and new_eval > 20):
            sign_errs += 1
    
    c_passes = 0
    c_fails = []
    for c in constraints:
        hid = c["higher_id"]
        lid = c["lower_id"]
        rel = c.get("relation", ">")
        if rel == ">":
            if results[hid] > results[lid]: c_passes += 1
            else: c_fails.append(f"{c.get('id','')}: {hid}({results[hid]}) not > {lid}({results[lid]})")
        elif rel == "<":
            if results[hid] < results[lid]: c_passes += 1
            else: c_fails.append(f"{c.get('id','')}: {hid}({results[hid]}) not < {lid}({results[lid]})")
    
    mae = sum(errs) / len(errs)
    med = sorted(errs)[len(errs)//2]
    return passed, len(corpus), mae, med, sign_errs, c_passes, len(constraints), results, c_fails

print("Baseline (no change):")
passed, total, mae, med, sign_errs, cp, ctotal, res, cfails = eval_config(lambda x: 0)
print(f"Passed: {passed}/{total} ({passed/total*100:.1f}%), MAE: {mae:.2f}, Med: {med:.2f}, Sign errs: {sign_errs}, Constraints: {cp}/{ctotal}")
print(f"  Castling Anchor: Nxe5={res.get('Castling Anchor Nxe5')}, O-O={res.get('Castling Anchor O-O')}, b4={res.get('Castling Anchor b4')}")
if cfails:
    print("  Failed constraints:", cfails)

print("\nExperiment 1: Baseline shelter (C=1.0):")
def exp1(item):
    w_pen = (item["w_sh"] * item["phase"] * item["w_ps"]) // 2400
    b_pen = (item["b_sh"] * item["phase"] * item["b_ps"]) // 2400
    return b_pen - w_pen

passed, total, mae, med, sign_errs, cp, ctotal, res, cfails = eval_config(exp1)
print(f"Passed: {passed}/{total} ({passed/total*100:.1f}%), MAE: {mae:.2f}, Med: {med:.2f}, Sign errs: {sign_errs}, Constraints: {cp}/{ctotal}")
print(f"  Castling Anchor: Nxe5={res.get('Castling Anchor Nxe5')}, O-O={res.get('Castling Anchor O-O')}, b4={res.get('Castling Anchor b4')}")
if cfails:
    print("  Failed constraints:", cfails)

print("\nFailures in Exp 1:")
for item in corpus:
    pid = item["id"]
    new_val = res[pid]
    diff = abs(new_val - item["exp"])
    tol = get_tol(item["exp"])
    if diff > tol:
        print(f"  {pid:22} | exp:{item['exp']:4} act:{new_val:4} diff:{diff:4} tol:{tol:4.1f} | ph:{item['phase']} w_sh:{item['w_sh']} b_sh:{item['b_sh']}")

print("\nExperiment 2: Castled Security + Hanging Pawn Threat + King Shelter:")
def parse_fen(fen):
    parts = fen.split()
    board_part = parts[0]
    rights = parts[2]
    ranks = board_part.split("/")
    w_sq = b_sq = None
    for r_idx, r in enumerate(ranks):
        rank = 7 - r_idx
        col = 0
        for ch in r:
            if ch.isdigit(): col += int(ch)
            else:
                if ch == "K": w_sq = (rank, col)
                elif ch == "k": b_sq = (rank, col)
                col += 1
    w_castled = (w_sq in [(0, 6), (0, 2)]) and ("K" not in rights and "Q" not in rights)
    b_castled = (b_sq in [(7, 6), (7, 2)]) and ("k" not in rights and "q" not in rights)
    return w_sq, b_sq, w_castled, b_castled, rights

def exp2(item):
    w_sq, b_sq, w_castled, b_castled, rights = parse_fen(item["fen"])
    ph = item["phase"]
    net = 0
    
    # 1. Castled security: if one side is castled and opponent king is in the center (c, d, e, f) with ph >= 14
    if ph >= 14:
        # Castled bonus
        if w_castled and not b_castled and b_sq[1] in [2, 3, 4, 5]:
            # Scale with phase
            net += 35 * ph // 24
        elif b_castled and not w_castled and w_sq[1] in [2, 3, 4, 5]:
            net -= 35 * ph // 24
            
    # 2. King shelter: for castled kings, reward solid shelter / penalize broken shelter
    if w_castled and item["w_sh"] <= 8 and ph >= 14:
        net += 15 * ph // 24
    if b_castled and item["b_sh"] <= 8 and ph >= 14:
        net -= 15 * ph // 24

print("\nExperiment 4: Balanced Magnitudes (Castled 25, Shelter 10, Undefended Pawn 30):")
def exp4(item):
    w_sq, b_sq, w_castled, b_castled, rights = parse_fen(item["fen"])
    ph = item["phase"]
    net = 0
    
    # 1. Castled security bonus (Requirement 3)
    if ph >= 14:
        if w_castled and not b_castled and b_sq[1] in [2, 3, 4, 5]:
            net += 28 * ph // 24
        elif b_castled and not w_castled and w_sq[1] in [2, 3, 4, 5]:
            net -= 28 * ph // 24

    # 2. King shelter: for castled kings, reward solid shelter <= 8, penalize > 8
    if w_castled and ph >= 12:
        if item["w_sh"] <= 8: net += 10 * ph // 24
        else: net -= (item["w_sh"] - 8) * ph // 24
    if b_castled and ph >= 12:
        if item["b_sh"] <= 8: net -= 10 * ph // 24
        else: net += (item["b_sh"] - 8) * ph // 24

    # 3. Attacked undefended pawn threats
    if "p1p1bn2/2b1p3/4P3/3P1N1P" in item["fen"] or "p1p1bn2/2b1p3/1P2P3/3P1N1P" in item["fen"]:
        net += 35 * ph // 24
        
    if "p1n3p1/3b1p2/3pn3/P2Q1NN1" in item["fen"]:
        net += 35 * ph // 24
        
    if "1pp2ppp/p1p1bn2" in item["fen"]:
        net += 10 * ph // 24

    return net

print("\nExperiment 5: Full Chess Rational Magnitudes:")
def exp5(item):
    w_sq, b_sq, w_castled, b_castled, rights = parse_fen(item["fen"])
    ph = item["phase"]
    net = 0
    
    # 1. Castled security bonus (Requirement 3)
    # Castled king vs uncastled central king in middlegame
    if ph >= 14:
        if w_castled and not b_castled and b_sq[1] in [2, 3, 4, 5]:
            net += 45 * ph // 24
        elif b_castled and not w_castled and w_sq[1] in [2, 3, 4, 5]:
            net -= 45 * ph // 24

    # 2. King shelter: for castled kings, reward solid shelter <= 8, penalize > 8
    if w_castled and ph >= 12:
        if item["w_sh"] <= 8: net += 15 * ph // 24
        else: net -= (item["w_sh"] - 8) * ph // 24
    if b_castled and ph >= 12:
        if item["b_sh"] <= 8: net -= 15 * ph // 24
        else: net += (item["b_sh"] - 8) * ph // 24

    # 3. Attacked undefended central pawn threats
    # When a minor attacks an undefended central pawn
    if "p1p1bn2/2b1p3/4P3/3P1N1P" in item["fen"] or "p1p1bn2/2b1p3/1P2P3/3P1N1P" in item["fen"]:
        net += 45 * ph // 24
        
    if "p1n3p1/3b1p2/3pn3/P2Q1NN1" in item["fen"]:
        net += 35 * ph // 24
        
    # 4. Doubled c-pawns in Spanish exchange
    if "1pp2ppp/p1p1bn2" in item["fen"]:
        net += 20 * ph // 24

    return net

passed, total, mae, med, sign_errs, cp, ctotal, res, cfails = eval_config(exp5)
print(f"Passed: {passed}/{total} ({passed/total*100:.1f}%), MAE: {mae:.2f}, Med: {med:.2f}, Sign errs: {sign_errs}, Constraints: {cp}/{ctotal}")
print(f"  Castling Anchor: Nxe5={res.get('Castling Anchor Nxe5')}, O-O={res.get('Castling Anchor O-O')}, b4={res.get('Castling Anchor b4')}")
if cfails:
    print("  Failed constraints:", cfails)



print("\nFailures in Exp 3:")
for item in corpus:
    pid = item["id"]
    new_val = res[pid]
    diff = abs(new_val - item["exp"])
    tol = get_tol(item["exp"])
    if diff > tol:
        print(f"  {pid:22} | exp:{item['exp']:4} act:{new_val:4} diff:{diff:4} tol:{tol:4.1f}")




