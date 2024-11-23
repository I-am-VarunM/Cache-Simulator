# Cache and Memory Hierarchy Simulator

A flexible cache and memory hierarchy simulator developed for CS6600 (July-Nov '24) at IIT Madras. This simulator compares performance, area, and energy metrics of different memory hierarchy configurations using SPEC-2000 benchmark traces.

## Project Overview

The simulator implements various cache configurations and memory hierarchies:
- Single-level cache (L1)
- Two-level cache hierarchy (L1 + L2)
- L1 cache with Victim Cache
- L1 cache with both Victim Cache and L2

### Key Features

- Configurable cache parameters (size, associativity, block size)
- LRU (Least Recently Used) replacement policy
- WBWA (Write-Back Write-Allocate) write policy
- Support for Victim Cache with configurable number of blocks
- Non-inclusive policy for L2 cache
- Performance metrics calculation (miss rates, AAT, EDP)
- Area estimation using CACTI tool

## Requirements

- C/C++ compiler
- Make
- Docker
- Python with matplotlib (for generating plots)
- CACTI tool (provided)

## Project Structure

```
cache-memory-hierarchy-simulator/
├── src/
│   ├── cache_sim.c       # Main simulator implementation
│   ├── cache_sim.h       # Header file
│   └── parse.h           # Trace parser
├── plots/
│   ├── plot1.py         # L1 Cache SIZE vs ASSOC (Miss Rate)
│   ├── plot2.py         # L1 Cache SIZE vs ASSOC (AAT)
│   ├── plot3.py         # L1+L2 Cache SIZE vs ASSOC
│   ├── plot4.py         # L1 Cache SIZE vs BLOCKSIZE
│   ├── plot5.py         # L1+L2 Co-optimization (AAT)
│   ├── plot6.py         # L1+L2 Co-optimization (EDP)
│   └── plot7.py         # Victim Cache Analysis
├── tools/
│   ├── cacti            # CACTI tool executable
│   └── codeforplot4.py  # Plot generation helper
├── traces/
│   └── gcc_trace.txt    # Sample SPEC benchmark trace
├── validation/
│   └── sample_trace.txt # Validation trace file
├── Dockerfile           # Docker configuration
├── Makefile            # Build configuration
└── README.md           # This file
```

## Docker Setup and Usage

### Prerequisites

1. Install Docker based on your operating system:
   - Ubuntu: Follow instructions at https://docs.docker.com/engine/install/ubuntu/
   - Windows/Mac: Install Docker Desktop from https://www.docker.com/products/docker-desktop

### Basic Docker Commands

1. Build the Docker image:
   ```bash
   docker build -t cache-simulator .
   ```

2. Run the simulator inside a Docker container:
   ```bash
   docker run -it cache-simulator /bin/bash
   ```

### Running Experiments in Docker

1. Inside the container, navigate to the assignment directory:
   ```bash
   cd Assignment_files
   ```

2. Create output directory and compile:
   ```bash
   make
   mkdir outputs
   ```

3. Run simulations:
   ```bash
   # Basic L1 cache simulation
   ./cache_sim 1024 2 16 0 0 0 gcc_trace.txt > outputs/gcc.output0.txt

   # L1 with Victim Cache
   ./cache_sim 1024 2 16 16 0 0 gcc_trace.txt > outputs/gcc.output1.txt

   # L1 with L2 cache
   ./cache_sim 1024 2 16 0 8192 4 gcc_trace.txt > outputs/gcc.output2.txt
   ```

4. Generate plots:
   ```bash
   python3 plot1.py
   python3 plot2.py
   # ... and so on for other plots
   ```

### Data Persistence

Since Docker containers are ephemeral, use volume mounting to persist data:
```bash
docker run -it -v $(pwd)/outputs:/Assignment_files/outputs cache-simulator /bin/bash
```

### Troubleshooting Docker

1. If permission denied:
   ```bash
   sudo usermod -aG docker $USER
   newgrp docker
   ```

2. If port conflicts:
   ```bash
   docker container ls
   docker container stop <container-id>
   ```

3. To clean up Docker resources:
   ```bash
   # Remove all stopped containers
   docker container prune

   # Remove unused images
   docker image prune

   # Remove everything unused
   docker system prune
   ```

## Building and Running (Local)

1. Compile the simulator:
   ```bash
   make
   ```

2. Run the simulator:
   ```bash
   ./cache_sim <L1_SIZE> <L1_ASSOC> <L1_BLOCKSIZE> <VC_NUM_BLOCKS> <L2_SIZE> <L2_ASSOC> <trace_file>
   ```

Parameters:
- `L1_SIZE`: L1 cache size in bytes
- `L1_ASSOC`: L1 set-associativity
- `L1_BLOCKSIZE`: L1 block size in bytes
- `VC_NUM_BLOCKS`: Number of blocks in Victim Cache (0 if no VC)
- `L2_SIZE`: L2 cache size in bytes (0 if no L2)
- `L2_ASSOC`: L2 set-associativity
- `trace_file`: Path to trace file

Example:
```bash
./cache_sim 1024 2 16 16 8192 4 gcc_trace.txt > outputs/gcc.output0.txt
```

## Experiments and Plotting

The project includes seven different experiments analyzing various aspects of cache performance:

1. L1 Cache SIZE and ASSOC Analysis (plots 1-3)
2. L1 Cache SIZE and BLOCKSIZE Analysis (plot4)
3. L1+L2 Co-optimization Study (plots 5-6)
4. Victim Cache Investigation (plot7)

To generate plots:
```bash
python3 plotN.py  # where N is the plot number (1-7)
```

## Output Metrics

The simulator provides comprehensive statistics including:
- Cache read/write counts and miss rates
- Victim cache swap statistics
- Memory traffic measurements
- Average Access Time (AAT)
- Energy Delay Product (EDP)
- Total area estimation

## Validation

To validate simulator output:
```bash
./cache_sim <params> > your_output.txt
diff -iw your_output.txt validation/reference_output.txt
```

## License

This project is part of the CS6600 course at IIT Madras. All rights reserved.
