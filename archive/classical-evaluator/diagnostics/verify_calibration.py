import json
import subprocess
import sys
from statistics import mean, median

def get_tolerance(score):
    s = abs(score)
    if s <= 100:
        return max(20.0, s * 0.25)
    elif s <= 250:
        return s * 0.25
    elif s <= 400:
        return s * 0.30
    elif s <= 600:
        return s * 0.35
    else:
        return s * 0.40

def run_calibration(corpus_path='tools/static_eval_calibration.json', verbose=True):
    with open(corpus_path) as f:
        data = json.load(f)

    positions = data['positions']
    constraints = data.get('ordering_constraints', [])
    
    results = {}
    errors = []
    sign_errors = []
    passed_count = 0
    
    if verbose:
        print(f"{'ID':<25} | {'Exp':>5} | {'Actual':>6} | {'Diff':>5} | {'Tol':>5} | {'Status':<6} | {'Category'}")
        print("-" * 80)

    for p in positions:
        pid = p['id']
        fen = p['resulting_fen']
        exp = p['trusted_white_pov_cp']
        cat = p.get('category', '')
        
        proc = subprocess.run(['./build/howl_eval_breakdown', fen], capture_output=True, text=True)
        white_eval = None
        for line in proc.stdout.splitlines():
            if "White Perspective" in line:
                white_eval = int(line.split()[-1])
                break
        
        if white_eval is None:
            print(f"Error parsing eval for {pid}")
            continue
            
        results[pid] = white_eval
        diff = abs(white_eval - exp)
        errors.append(diff)
        tol = get_tolerance(exp)
        ok = diff <= tol
        if ok:
            passed_count += 1
            status = "PASS"
        else:
            status = "FAIL"
            
        # Check sign error: opposite sign when expected magnitude is non-trivial (>= 15 cp)
        if (exp >= 15 and white_eval < 0) or (exp <= -15 and white_eval > 0):
            sign_errors.append((pid, exp, white_eval))

        if verbose:
            print(f"{pid:<25} | {exp:>5} | {white_eval:>6} | {diff:>5} | {tol:>5.1f} | {status:<6} | {cat}")

    total = len(positions)
    pass_pct = (passed_count / total) * 100 if total else 0
    mae = mean(errors) if errors else 0
    med_err = median(errors) if errors else 0
    
    # Check ordering constraints
    constraint_passes = 0
    constraint_fails = []
    for c in constraints:
        cid = c.get('id', '')
        hid = c['higher_id']
        lid = c['lower_id']
        rel = c.get('relation', '>')
        if hid in results and lid in results:
            val_h = results[hid]
            val_l = results[lid]
            if rel == '>' and val_h > val_l:
                constraint_passes += 1
            elif rel == '<' and val_h < val_l:
                constraint_passes += 1
            else:
                constraint_fails.append((cid, hid, val_h, lid, val_l, c.get('notes', '')))

    print("\n" + "="*50)
    print(f"SUMMARY: {corpus_path}")
    print(f"Positions: {passed_count}/{total} passed ({pass_pct:.1f}%)")
    print(f"Mean Absolute Error: {mae:.2f} cp")
    print(f"Median Absolute Error: {med_err:.2f} cp")
    print(f"Sign Errors: {len(sign_errors)}")
    for se in sign_errors:
        print(f"  Sign Error in {se[0]}: expected {se[1]}, got {se[2]}")
    print(f"Ordering Constraints: {constraint_passes}/{len(constraints)} passed")
    for cf in constraint_fails:
        print(f"  FAILED Constraint {cf[0]}: {cf[1]} ({cf[2]}) not > {cf[3]} ({cf[4]}) [{cf[5]}]")
    print("="*50 + "\n")

    return {
        'total': total,
        'passed': passed_count,
        'pass_pct': pass_pct,
        'mae': mae,
        'med_err': med_err,
        'sign_errors': sign_errors,
        'constraint_passes': constraint_passes,
        'total_constraints': len(constraints),
        'constraint_fails': constraint_fails,
        'results': results
    }

if __name__ == '__main__':
    run_calibration()
