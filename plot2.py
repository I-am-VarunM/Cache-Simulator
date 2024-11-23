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

# Regular expression to capture average access time from output.txt
access_time_pattern = re.compile(r"1\. average access time:\s+([0-9]*\.[0-9]*)")

# Function to run the cache simulator and extract the average access time
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
        match = access_time_pattern.search(output)
        if match:
            return float(match.group(1))
        else:
            return None  # Return None if no valid access time is found

# Store results in a dictionary, one entry per associativity
access_times = {assoc: {'sizes': [], 'times': []} for assoc in associativities}

# Run experiments for each combination of SIZE and ASSOCIATIVITY
for size in sizes_bytes:
    for assoc in associativities:
        access_time = run_cache_sim(size, assoc)
        if access_time is not None:
            access_times[assoc]['sizes'].append(np.log2(size))  # Store log2(size)
            access_times[assoc]['times'].append(access_time)  # Store average access time

# Plotting the results
plt.figure(figsize=(10, 6))
for assoc in associativities:
    label = f'{assoc}-way set associative' if assoc != 'fully-associative' else 'Fully-associative'
    
    # Only plot if there are valid data points for this associativity
    if access_times[assoc]['sizes'] and access_times[assoc]['times']:
        plt.plot(access_times[assoc]['sizes'], access_times[assoc]['times'], label=label, marker='o')

# Add labels and title
plt.xlabel('log2(Size of L1 Cache in Bytes)')
plt.ylabel('Average Access Time (in cycles)')
plt.title('Average Access Time vs log2(L1 Cache Size)')
plt.legend()
plt.grid(True)
plt.show()