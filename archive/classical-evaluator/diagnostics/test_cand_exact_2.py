# If we define Candidate 2 with 6 parameters:
# MG: 3 anchors or quadratic/piecewise
# What if Candidate 2 preserves exact baseline values by direct table mapping or exact anchor mapping?
# How can we achieve exact table identity?
# Let's inspect the baseline parameters:
# MG: [-6, 3, 15, 4, 4]
# EG: [10, 30, 33, 31, 27]
# Notice:
# In MG, Increment_3 = 4 and Increment_4 = 4.
# In EG:
# Increment_1 = 30, Increment_2 = 33, Increment_3 = 31, Increment_4 = 27.
# Notice the anchors in EG:
# 0: 10
# 3: 40 (span 3, delta 30 -> slope 10.0/bucket)
# 6: 73 (span 3, delta 33 -> slope 11.0/bucket)
# 9: 104 (span 3, delta 31 -> slope 10.33/bucket)
# 12: 131 (span 3, delta 27 -> slope 9.0/bucket)
# EG table:
# [10, 20, 30, 40, 51, 62, 73, 83, 94, 104, 113, 122, 131, 131, ...]
# Look at the increments per bucket in EG:
# 10, 10, 10, 11, 11, 11, 10, 11, 10, 9, 9, 9!
# It is essentially a STRAIGHT LINE from 10 to 131 with slope ~10.08 per bucket!
