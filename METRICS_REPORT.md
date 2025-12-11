# Comprehensive Metrics Comparison Report

## Overview

This report provides detailed performance metrics comparison for all Connected Component Labeling (CCL) algorithms implemented in both Python and C++.

## Test Results Summary

### Base Configuration
- **Image Size**: 1000x1000 (1 MPixel)
- **Density**: 0.3 (30% foreground pixels)
- **Connectivity**: 4-connected
- **Iterations**: 10

### Detailed Metrics Table

| Algorithm | Time (μs) | Speedup | Memory (KB) | Throughput (MP/s) | Components |
|-----------|-----------|---------|-------------|-------------------|------------|
| **2-Pass (DSU)** | 33,790.80 | 0.36x | 638,976 | 29.59 | 128,241 |
| **BFS** | 12,164.00 | 0.99x | 0 | 82.21 | 128,241 |
| **DFS** | 12,072.60 | **1.00x** | 0 | **82.83** | 128,241 |
| **DSU (1-pass)** | 13,929.70 | 0.87x | 0 | 71.79 | 128,241 |

### Key Findings

1. **Fastest Algorithm**: DFS (12,072.60 μs)
2. **Slowest Algorithm**: 2-Pass DSU (33,790.80 μs) - **2.80x slower**
3. **Most Memory Efficient**: BFS/DFS/DSU (0 KB overhead)
4. **Least Memory Efficient**: 2-Pass DSU (638,976 KB overhead)

## Performance vs Density Analysis

| Density | 2-Pass (μs) | BFS (μs) | DFS (μs) | DSU (μs) | Best |
|---------|-------------|----------|----------|----------|------|
| 0.1 | 14,155.00 | 5,298.80 | 5,353.30 | 6,310.40 | BFS |
| 0.3 | 32,785.10 | 12,077.40 | 12,376.00 | 13,596.90 | BFS |
| 0.5 | 43,529.80 | 16,053.00 | 16,023.80 | 16,980.50 | DFS |
| 0.7 | 45,775.10 | 19,240.90 | 15,745.20 | 12,084.00 | **DSU** |
| 0.9 | 48,866.70 | 11,164.50 | 11,107.20 | 10,009.30 | **DSU** |

**Observations**:
- At **low densities (0.1-0.5)**: BFS/DFS perform best
- At **high densities (0.7-0.9)**: DSU (1-pass) becomes fastest
- 2-Pass DSU is consistently slower across all densities

## Performance vs Image Size Analysis

| Size | MPixels | 2-Pass (μs) | BFS (μs) | DFS (μs) | DSU (μs) | Best |
|------|---------|--------------|----------|----------|----------|------|
| 100x100 | 0.01 | 307.30 | 111.70 | 116.50 | 118.40 | BFS |
| 500x500 | 0.25 | 8,184.40 | 3,164.50 | 3,297.60 | 3,548.10 | BFS |
| 1000x1000 | 1.00 | 33,045.60 | 12,401.60 | 12,282.70 | 13,944.20 | DFS |
| 2000x2000 | 4.00 | 134,976.50 | 49,434.90 | 50,294.90 | 78,167.00 | BFS |

**Observations**:
- **Scaling**: All algorithms scale approximately linearly with image size
- **Best for small images**: BFS
- **Best for large images**: BFS/DFS (similar performance)
- **2-Pass DSU**: Shows worse scaling, especially for large images

## Throughput Analysis

| Algorithm | Throughput (MP/s) | Relative Performance |
|-----------|-------------------|---------------------|
| DFS | 82.83 | 100% (baseline) |
| BFS | 82.21 | 99.3% |
| DSU (1-pass) | 71.79 | 86.7% |
| 2-Pass (DSU) | 29.59 | 35.7% |

## Memory Usage Analysis

- **BFS/DFS/DSU (1-pass)**: Minimal memory overhead (0 KB measured)
- **2-Pass DSU**: Significant memory overhead (638,976 KB)
  - This is due to maintaining DSU data structures for all possible labels
  - Memory scales with potential label count (H*W/2)

## Algorithm Characteristics

### 2-Pass (DSU)
- **Pros**: 
  - Handles complex connectivity patterns well
  - Can be optimized for incremental updates
- **Cons**: 
  - Slowest overall performance
  - High memory usage
  - More complex implementation

### BFS
- **Pros**: 
  - Fast and efficient
  - Low memory overhead
  - Good cache locality
- **Cons**: 
  - Requires queue data structure
  - Slightly slower than DFS for some cases

### DFS
- **Pros**: 
  - Fastest overall performance
  - Low memory overhead
  - Simple stack-based implementation
- **Cons**: 
  - Potential stack overflow for very deep components
  - Slightly worse cache behavior than BFS

### DSU (1-pass)
- **Pros**: 
  - Good performance at high densities
  - Efficient for incremental updates
  - Single pass algorithm
- **Cons**: 
  - Slightly slower than BFS/DFS at low densities
  - Requires union-find data structure

## Recommendations

### For General Use:
1. **Use DFS** for best overall performance
2. **Use BFS** if you prefer breadth-first traversal
3. Both BFS and DFS are nearly equivalent in performance

### For High Density Images (>70% foreground):
- **Use DSU (1-pass)** - becomes fastest at very high densities

### For Incremental/Stream Updates:
- **Use DSU-based algorithms** - better suited for maintaining state

### For Memory-Constrained Environments:
- **Avoid 2-Pass DSU** - uses significant memory
- **Use BFS/DFS** - minimal memory overhead

### For Large Images (>2000x2000):
- **Use BFS** - best scaling performance
- **Avoid 2-Pass DSU** - poor scaling

## Python vs C++ Performance

From previous benchmarks (500x500 image):
- **C++ is 50-150x faster** than Python
- **BFS/DFS show best speedup** (~140x)
- **2-Pass DSU shows lower speedup** (~52x)

## Conclusion

1. **DFS is the fastest** algorithm for most scenarios
2. **BFS is nearly as fast** and sometimes faster for large images
3. **DSU (1-pass)** excels at high densities and incremental updates
4. **2-Pass DSU** is slower but useful for specific use cases (stream processing, complex connectivity)

The choice of algorithm should depend on:
- Image characteristics (size, density)
- Update patterns (one-time vs incremental)
- Memory constraints
- Specific use case requirements

