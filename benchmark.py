#!/usr/bin/env python3
"""
Performance comparison between Python and C++ implementations
of Connected Component Labeling algorithms.
"""
import numpy as np
import time
import subprocess
import sys
import os
from pathlib import Path

# Add python directory to path
sys.path.insert(0, str(Path(__file__).parent / "python"))

from dsu_2pass import label_cc_2pass
from algorithms import label_cc_bfs, label_cc_dfs, label_cc_dsu


def generate_test_image(H, W, density=0.3):
    """Generate random binary test image"""
    return (np.random.rand(H, W) < density).astype(np.uint8)


def benchmark_python(func, img, iterations=10):
    """Benchmark Python function"""
    times = []
    for _ in range(iterations):
        start = time.perf_counter()
        result = func(img)
        end = time.perf_counter()
        times.append((end - start) * 1e6)  # Convert to microseconds
    return np.mean(times), result


def benchmark_cpp(H, W, density, iterations, eight_conn):
    """Run C++ benchmark and parse results"""
    cpp_build_dir = Path(__file__).parent / "cpp" / "build"
    benchmark_exe = cpp_build_dir / "benchmark"
    
    if not benchmark_exe.exists():
        print("C++ benchmark not found. Please compile first:")
        print("  cd cpp && mkdir -p build && cd build && cmake .. && make")
        return None
    
    cmd = [
        str(benchmark_exe),
        str(H), str(W), str(density), str(iterations), 
        "1" if eight_conn else "0"
    ]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error running C++ benchmark: {e}")
        print(f"stderr: {e.stderr}")
        return None


def main():
    # Test parameters
    H, W = 1000, 1000
    density = 0.3
    iterations = 10
    eight_conn = False
    
    if len(sys.argv) > 1:
        H = int(sys.argv[1])
    if len(sys.argv) > 2:
        W = int(sys.argv[2])
    if len(sys.argv) > 3:
        density = float(sys.argv[3])
    if len(sys.argv) > 4:
        iterations = int(sys.argv[4])
    if len(sys.argv) > 5:
        eight_conn = bool(int(sys.argv[5]))
    
    print("=" * 60)
    print("Connected Component Labeling - Performance Comparison")
    print("=" * 60)
    print(f"Image size: {H}x{W}")
    print(f"Density: {density}")
    print(f"Connectivity: {'8' if eight_conn else '4'}")
    print(f"Iterations: {iterations}")
    print()
    
    # Generate test image
    print("Generating test image...")
    img = generate_test_image(H, W, density)
    print(f"Foreground pixels: {np.sum(img)} ({100*np.sum(img)/(H*W):.1f}%)")
    print()
    
    # Python benchmarks
    print("Python Implementation:")
    print("-" * 60)
    
    py_2pass_time, result_2pass = benchmark_python(
        lambda x: label_cc_2pass(x, eight_conn), img, iterations
    )
    py_bfs_time, result_bfs = benchmark_python(
        lambda x: label_cc_bfs(x, eight_conn), img, iterations
    )
    py_dfs_time, result_dfs = benchmark_python(
        lambda x: label_cc_dfs(x, eight_conn), img, iterations
    )
    py_dsu_time, result_dsu = benchmark_python(
        lambda x: label_cc_dsu(x, eight_conn), img, iterations
    )
    
    print(f"2-Pass (DSU): {py_2pass_time:.2f} μs")
    print(f"BFS:          {py_bfs_time:.2f} μs")
    print(f"DFS:          {py_dfs_time:.2f} μs")
    print(f"DSU (1-pass): {py_dsu_time:.2f} μs")
    print()
    
    # Verify results are consistent
    num_components_2pass = len(np.unique(result_2pass[result_2pass > 0]))
    num_components_bfs = len(np.unique(result_bfs[result_bfs > 0]))
    print(f"Number of components (2-pass): {num_components_2pass}")
    print(f"Number of components (BFS):    {num_components_bfs}")
    assert num_components_2pass == num_components_bfs, "Results don't match!"
    print()
    
    # C++ benchmarks
    print("C++ Implementation:")
    print("-" * 60)
    cpp_output = benchmark_cpp(H, W, density, iterations, eight_conn)
    if cpp_output:
        print(cpp_output)
        
        # Parse C++ results for comparison
        lines = cpp_output.strip().split('\n')
        cpp_times = {}
        for line in lines:
            if 'μs' in line:
                parts = line.split(':')
                if len(parts) == 2:
                    algo = parts[0].strip()
                    time_str = parts[1].strip().replace('μs', '').strip()
                    try:
                        cpp_times[algo] = float(time_str)
                    except ValueError:
                        pass
        
        # Comparison
        if cpp_times:
            print("\n" + "=" * 60)
            print("Speedup Comparison (C++ / Python):")
            print("=" * 60)
            if "2-Pass (DSU)" in cpp_times:
                speedup = py_2pass_time / cpp_times["2-Pass (DSU)"]
                print(f"2-Pass (DSU): {speedup:.2f}x faster in C++")
            if "BFS" in cpp_times:
                speedup = py_bfs_time / cpp_times["BFS"]
                print(f"BFS:          {speedup:.2f}x faster in C++")
            if "DFS" in cpp_times:
                speedup = py_dfs_time / cpp_times["DFS"]
                print(f"DFS:          {speedup:.2f}x faster in C++")
            if "DSU (1-pass)" in cpp_times:
                speedup = py_dsu_time / cpp_times["DSU (1-pass)"]
                print(f"DSU (1-pass): {speedup:.2f}x faster in C++")
    else:
        print("C++ benchmark not available. Please compile first.")
        print("  cd cpp && mkdir -p build && cd build && cmake .. && make")
    
    print("\n" + "=" * 60)


if __name__ == "__main__":
    main()

