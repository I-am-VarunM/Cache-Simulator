import subprocess
import re
import matplotlib.pyplot as plt
import numpy as np

# Constants
block_size = 32  # Fixed block size
num_of_vc_blocks = 0  # No victim cache
l2_size = 0  # No L2 cache
l2_assoc = 0  # No L2 cache
trace_file_name = 'gcc_trace.txt'  # Replace with actual trace file name

# Cache sizes from 2KB to 1MB in bytes
sizes_kb = [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
sizes_bytes = [size * 1024 for size in sizes_kb]

# Associativities to test
associativities = [1, 2, 4, 8, 'fully-associative']

# Regular expression to capture L1 miss rate from output.txt
miss_rate_pattern = re.compile(r"h\. combined L1\+VC miss rate:\s+([0-9]*\.[0-9]*)")

# Function to run the cache simulator and extract the L1 miss rate
def run_cache_sim(L1_size, L1_assoc):
    # Determine associativity parameter
    if L1_assoc == 'fully-associative':
        assoc_arg = L1_size // block_size  # Associativity is Size / Block Size for fully-associative
    else:
        assoc_arg = L1_assoc

    command = f'./cache_sim {L1_size} {assoc_arg} {block_size} {num_of_vc_blocks} {l2_size} {l2_assoc} {trace_file_name} > output.txt'
    subprocess.run(command, shell=True)
    
    # Read and parse the output file
    with open('output.txt', 'r') as f:
        output = f.read()
        match = miss_rate_pattern.search(output)
        if match:
            return float(match.group(1))
        else:
            return None

# Store results in a dictionary, one entry per associativity
miss_rates = {assoc: [] for assoc in associativities}

# Run experiments for each combination of SIZE and ASSOCIATIVITY
for size in sizes_bytes:
    for assoc in associativities:
        miss_rate = run_cache_sim(size, assoc)
        if miss_rate is not None:
            miss_rates[assoc].append(miss_rate)

# Plotting the results
plt.figure(figsize=(10, 6))
log2_sizes = np.log2(sizes_bytes)
for assoc in associativities:
    label = f'{assoc}-way set associative' if assoc != 'fully-associative' else 'Fully-associative'
    plt.plot(log2_sizes, miss_rates[assoc], label=label, marker='o')

# Add labels and title
plt.xlabel('log2(Size of L1 Cache in Bytes)')
plt.ylabel('L1 Miss Rate')
plt.title('L1 Miss Rate vs log2(L1 Cache Size)')
plt.legend()
plt.grid(True)
plt.show()
