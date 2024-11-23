import subprocess
import re
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Constants
block_size = 32  # Fixed block size
num_of_vc_blocks = 0  # No victim cache
trace_file_name = 'gcc_trace.txt'  # Replace with actual trace file name

# Cache sizes: L1 from 1KB to 64KB and L2 from 32KB to 1MB
l1_sizes_kb = [1, 2, 4, 8, 16, 32, 64]  # In KB
l2_sizes_kb = [32, 64, 128, 256, 512, 1024]  # In KB
l1_sizes_bytes = [size * 1024 for size in l1_sizes_kb]
l2_sizes_bytes = [size * 1024 for size in l2_sizes_kb]

# Regular expression to capture average access time from output.txt
access_time_pattern = re.compile(r"1\. average access time:\s+([0-9]*\.[0-9]*)")

# Function to run the cache simulator and extract the average access time
def run_cache_sim(L1_size, L2_size):
    # Associativities
    L1_assoc = 4  # 4-way set associative for L1
    L2_assoc = 8  # 8-way set associative for L2

    command = f'./cache_sim {L1_size} {L1_assoc} {block_size} {num_of_vc_blocks} {L2_size} {L2_assoc} {trace_file_name} > output.txt'
    subprocess.run(command, shell=True)
    
    # Read and parse the output file
    with open('output.txt', 'r') as f:
        output = f.read()
        match = access_time_pattern.search(output)
        if match:
            return float(match.group(1))
        else:
            return None  # Return None if no valid access time is found

# Prepare data for 3D surface plot
X = []  # log2(L1 sizes)
Y = []  # log2(L2 sizes)
Z = []  # Average Access Times
a = 10
b = 10
c = 10
# Iterate through L1 and L2 sizes
for L1_size in l1_sizes_bytes:
    for L2_size in l2_sizes_bytes:
        if L1_size < L2_size:  # Ensure L1 is smaller than L2
            access_time = run_cache_sim(L1_size, L2_size)
            if access_time is not None:
                X.append(np.log2(L1_size))  # log2(L1 cache size)
                Y.append(np.log2(L2_size))  # log2(L2 cache size)
                Z.append(access_time)  # Average access time
                t = a
                a = min(a,access_time)
                if t != a:
                    b = np.log2(L1_size)
                    c = np.log2(L2_size)
print("L1 value")
print(b)
print("L2 value")
print(c)
print("AAT")
print(a)
# Convert lists to numpy arrays for plotting
X = np.array(X)
Y = np.array(Y)
Z = np.array(Z)

# Create a 3D surface plot
fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

# Create a grid for the surface plot
X_grid, Y_grid = np.meshgrid(np.unique(X), np.unique(Y))
Z_grid = np.zeros_like(X_grid)

# Populate the grid with average access times
for i in range(len(X)):
    x_idx = np.where(np.unique(X) == X[i])[0][0]
    y_idx = np.where(np.unique(Y) == Y[i])[0][0]
    Z_grid[y_idx, x_idx] = Z[i]

# Plot the surface
surf = ax.plot_surface(X_grid, Y_grid, Z_grid, cmap='viridis', edgecolor='none')

# Plot data points on the surface
ax.scatter(X, Y, Z, color='red', label='Data Points', s=50)  # Mark data points with red markers

# Add labels and title
ax.set_xlabel('log2(L1 Cache Size in Bytes)')
ax.set_ylabel('log2(L2 Cache Size in Bytes)')
ax.set_zlabel('Average Access Time (in ns)')
ax.set_title('3D Surface Plot of Average Access Time vs L1 and L2 Cache Sizes')
ax.set_zlim(0.6, 0.95)
# Show color bar
fig.colorbar(surf, ax=ax, shrink=0.5, aspect=10)

# Add legend for data points
ax.legend()

plt.show()
