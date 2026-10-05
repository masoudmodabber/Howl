# Let's test a lossless formulation:
# Can we represent QueenMobility with fewer parameters and 100% exact table identity?
# Let's check:
# MG has 5 parameters: p0=-6, p1=3, p2=15, p3=4, p4=4
# Notice p3 == p4 == 4!
# If p3 and p4 are tied: p_sat = 4, then MG has 4 parameters: Base (-6), Incr1 (3), Incr2 (15), IncrSat (4).
# Anchor 3 = Anchor 2 + IncrSat = 12 + 4 = 16.
# Anchor 4 = Anchor 3 + IncrSat = 16 + 4 = 20.
# Table generated from [Base, Incr1, Incr2, IncrSat]:
# Exactly: [-6, -5, -4, -4, -3, 1, 5, 8, 12, 13, 14, 15, 16, 17, 19, 20, 20...] -> 100.000% EXACT MATCH!

# Now what about EG?
# In EG, the anchors are:
# a0 = 10
# a1 = 10 + 30 = 40
# a2 = 40 + 33 = 73
# a3 = 73 + 31 = 104
# a4 = 104 + 27 = 131
# Can EG be parameterized cleanly?
# If we keep EG anchors or a direct model:
# What if EG has Base (10), Linear slope / midpoint anchor, and Terminal cap (131)?
# If we do that, does it match exactly?
# Let's check:
