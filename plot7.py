import matplotlib.pyplot as plt
import numpy as np

data = {
    "(1 way, 4 entry VC)": [(10.0, 0.7905), (11.0, 0.7839), (12.0, 0.7824), (13.0, 0.7857), (14.0, 0.8022), (15.0, 0.832)],
    "(1 way, 8 entry VC)": [(10.0, 0.7845), (11.0, 0.78), (12.0, 0.7799), (13.0, 0.7838), (14.0, 0.8014), (15.0, 0.8317)],
    "(1 way, 16 entry VC)": [(10.0, 0.7769), (11.0, 0.7751), (12.0, 0.7752), (13.0, 0.7811), (14.0, 0.8007), (15.0, 0.8313)],
    "(1 way, no VC)": [(10.0, 0.7861), (11.0, 0.7802), (12.0, 0.7771), (13.0, 0.782), (14.0, 0.7996), (15.0, 0.8306)],
    "(2 way, no VC)": [(11.0, 0.7986), (12.0, 0.7918), (13.0, 0.8021), (14.0, 0.8171), (15.0, 0.8562)],
    "(4 way, no VC)": [(12.0, 0.8021), (13.0, 0.8039), (14.0, 0.8242), (15.0, 0.8616)]
}

# Create the plot
plt.figure(figsize=(12, 8))

# Plot each line
for label, coordinates in data.items():
    x, y = zip(*coordinates)
    plt.plot(x, y, marker='o', label=label)

# Set labels and title
plt.xlabel('log2(L1 SIZE)')
plt.ylabel('Average Access Time (AAT)')
plt.title('AAT vs. log2(L1 SIZE) for Different Cache Configurations')

# Add legend
plt.legend()

# Add grid
plt.grid(True)

# Set y-axis limits to match the data range
plt.ylim(0.77, 0.87)

# Save and show the plot
plt.savefig('cache_performance_plot.png')
plt.show()