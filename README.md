# Connected Component Labeling - Python vs C++ Performance Comparison

This project implements Connected Component Labeling (CCL) algorithms in both Python and C++, and compares their performance.

## Project Structure

```
CS5800_Final_Project/
├── python/              # Python implementations
│   ├── dsu_2pass.py     # Two-pass algorithm with DSU
│   ├── algorithms.py    # BFS, DFS, and DSU algorithms
│   ├── crop_paste_back.py
│   └── make_mask.py
├── cpp/                 # C++ implementations
│   ├── dsu_2pass.hpp/cpp
│   ├── algorithms.hpp/cpp
│   ├── benchmark.cpp
│   └── CMakeLists.txt
└── benchmark.py         # Performance comparison script
```

## Algorithms Implemented

1. **Two-Pass Algorithm (DSU)**: Rosenfeld's two-pass algorithm using Union-Find
2. **BFS**: Breadth-First Search based labeling
3. **DFS**: Depth-First Search (stack-based) labeling
4. **DSU One-Pass**: Single-pass union-then-assign algorithm

## Setup

### Python Requirements

```bash
pip install numpy pillow scipy
```

### C++ Build

```bash
cd cpp
mkdir -p build
cd build
cmake ..
make
```

## Testing

### Run Python Tests

```bash
python python/test_algorithms.py
```

### Run C++ Tests

```bash
cd cpp/build
make test_algorithms
./test_algorithms
```

## Usage

### Run Performance Comparison

```bash
python benchmark.py [H] [W] [density] [iterations] [eight_conn]

# Example:
python benchmark.py 1000 1000 0.3 10 0
```

Parameters:
- `H`: Image height (default: 1000)
- `W`: Image width (default: 1000)
- `density`: Foreground pixel density (default: 0.3)
- `iterations`: Number of benchmark iterations (default: 10)
- `eight_conn`: Use 8-connectivity? 1 for yes, 0 for no (default: 0)

### Run C++ Benchmark Only

```bash
cd cpp/build
./benchmark [H] [W] [density] [iterations] [eight_conn]
```

## Expected Performance Differences

C++ implementations are typically **10-100x faster** than Python implementations for this type of compute-intensive task, depending on:

1. **Image size**: Larger images show bigger speedup
2. **Algorithm**: BFS/DFS may show different speedups than DSU-based algorithms
3. **Optimization level**: C++ compiled with `-O3` vs Python's interpreted execution
4. **NumPy overhead**: Python uses NumPy (C-based) but still has Python overhead

### Why C++ is Faster

1. **Compiled vs Interpreted**: C++ is compiled to machine code, Python is interpreted
2. **Type Safety**: C++ has no runtime type checking overhead
3. **Memory Management**: Direct control over memory layout and allocation
4. **Optimization**: Compiler can apply aggressive optimizations (`-O3`, `-march=native`)
5. **No GIL**: C++ has no Global Interpreter Lock

### Typical Speedup Range

- **Small images (100x100)**: 5-20x speedup
- **Medium images (1000x1000)**: 20-50x speedup
- **Large images (5000x5000)**: 50-100x+ speedup

The exact speedup depends on your hardware, compiler, and Python version.

## Notes

- Both implementations produce identical results
- Python uses NumPy arrays (C-based backend) but still has Python overhead
- C++ uses `-O3` optimization and native architecture flags for best performance
- For production use, consider using optimized libraries like OpenCV or scikit-image

