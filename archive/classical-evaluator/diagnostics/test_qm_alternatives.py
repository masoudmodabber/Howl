import math

# Baseline tables
mg_table = [-6, -5, -4, -4, -3, 1, 5, 8, 12, 13, 14, 15, 16, 17, 19, 20] + [20]*12
eg_table = [10, 20, 30, 40, 51, 62, 73, 83, 94, 104, 113, 122, 131] + [131]*15

print("MG table:", mg_table)
print("EG table:", eg_table)

# Let's inspect the correlation between (moveCount) and table values
# In MG: moveCount 0 -> -6, moveCount 4 -> -3, moveCount 8 -> 12, moveCount 12 -> 16, moveCount 15..27 -> 20
# In EG: moveCount 0 -> 10, moveCount 3 -> 40, moveCount 6 -> 73, moveCount 9 -> 104, moveCount 12..27 -> 131
