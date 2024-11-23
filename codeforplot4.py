import subprocess
import re
import matplotlib.pyplot as plt
import numpy as np

# Define constants
L1_Assoc = 4
L2_size = 0
L2_Assoc = 0
Num_of_VC_Blocks = 0
trace_file_name = 'gcc_trace.txt'  # Replace with actual trace file name

# L1 sizes in KB and corresponding values in bytes
sizes_kb = [1, 2, 4, 8, 16, 32]
sizes_bytes = [size * 1024 for size in sizes_kb]

# Block sizes in bytes
block_sizes = [16, 32, 64, 128]

# Regular expression to capture L1 miss rate from output.txt
miss_rate_pattern = re.compile(r"h\. combined L1\+VC miss rate:\s+([0-9]*\.[0-9]*)")

# Function to run the cache simulator and extract the L1 miss rate
def run_cache_sim(L1_size, block_size):
    command = f'./cache_sim {L1_size} {L1_Assoc} {block_size} {Num_of_VC_Blocks} {L2_size} {L2_Assoc} {trace_file_name} > output.txt'
    subprocess.run(command, shell=True)
    
    # Read and parse the output file
    with open('output.txt', 'r') as f:
        output = f.read()
        match = miss_rate_pattern.search(output)
        if match:
            return float(match.group(1))
        else:
            return None

# Store results in a dictionary
miss_rates = {size: [] for size in sizes_bytes}

# Run experiments for each combination of SIZE and BLOCKSIZE
for size in sizes_bytes:
    for block_size in block_sizes:
        miss_rate = run_cache_sim(size, block_size)
        if miss_rate is not None:
            miss_rates[size].append(miss_rate)

# Plotting the results
plt.figure(figsize=(10, 6))
for size_kb, size_bytes in zip(sizes_kb, sizes_bytes):
    plt.plot(np.log2(block_sizes), miss_rates[size_bytes], label=f'L1 Size = {size_kb}KB', marker='o')

# Add labels and title
plt.xlabel('log2(Block Size)')
plt.ylabel('L1 Miss Rate')
plt.title('L1 Miss Rate vs log2(Block Size)')
plt.legend()
plt.grid(True)
plt.show()