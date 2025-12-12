#include "dsu_2pass.hpp"
#include "algorithms.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>
#include <sys/resource.h>

struct Metrics {
    double time_us;
    size_t memory_kb;
    int components;
    int num_operations;
    double throughput_mpixels_per_sec;
};

// Get memory usage in KB
size_t get_memory_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;  // KB on macOS/Linux
}

Metrics benchmark_with_metrics(
    std::vector<int32_t> (*func)(const uint8_t*, int, int, bool),
    const uint8_t* img, int H, int W, bool eight_conn,
    int iterations, const char* name) {
    
    Metrics m;
    m.num_operations = iterations;
    
    // Warm up
    func(img, H, W, eight_conn);
    
    // Measure memory before
    size_t mem_before = get_memory_usage();
    
    // Time measurement
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func(img, H, W, eight_conn);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    // Measure memory after
    size_t mem_after = get_memory_usage();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    m.time_us = duration.count() / (double)iterations;
    m.memory_kb = mem_after > mem_before ? (mem_after - mem_before) : 0;
    
    // Get component count
    auto result = func(img, H, W, eight_conn);
    std::set<int32_t> comps;
    for (int i = 0; i < H * W; ++i) {
        if (result[i] > 0) comps.insert(result[i]);
    }
    m.components = comps.size();
    
    // Throughput: megapixels per second
    double total_pixels = (double)H * W / 1e6;
    m.throughput_mpixels_per_sec = total_pixels / (m.time_us / 1e6);
    
    return m;
}

void print_metrics_table(const std::vector<std::pair<std::string, Metrics>>& results, int H, int W) {
    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "DETAILED METRICS COMPARISON\n";
    std::cout << std::string(100, '=') << "\n";
    std::cout << "Image Size: " << H << "x" << W << " (" << (H*W/1e6) << " MPixels)\n\n";
    
    // Find best time
    double best_time = results[0].second.time_us;
    for (const auto& r : results) {
        if (r.second.time_us < best_time) best_time = r.second.time_us;
    }
    
    std::cout << std::left << std::setw(20) << "Algorithm";
    std::cout << std::right << std::setw(12) << "Time (μs)";
    std::cout << std::setw(15) << "Speedup";
    std::cout << std::setw(12) << "Memory (KB)";
    std::cout << std::setw(15) << "Throughput";
    std::cout << std::setw(12) << "Components";
    std::cout << "\n";
    std::cout << std::string(100, '-') << "\n";
    
    for (const auto& [name, m] : results) {
        std::cout << std::left << std::setw(20) << name;
        std::cout << std::right << std::fixed << std::setprecision(2);
        std::cout << std::setw(12) << m.time_us;
        
        double speedup = best_time / m.time_us;
        std::cout << std::setw(14) << std::setprecision(2) << speedup << "x";
        
        std::cout << std::setw(12) << m.memory_kb;
        std::cout << std::setw(13) << std::setprecision(2) << m.throughput_mpixels_per_sec << " MP/s";
        std::cout << std::setw(12) << m.components;
        std::cout << "\n";
    }
    std::cout << std::string(100, '=') << "\n\n";
}

void test_different_densities(int H, int W, bool eight_conn, int iterations) {
    std::vector<double> densities = {0.1, 0.3, 0.5, 0.7, 0.9};
    
    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "PERFORMANCE vs DENSITY ANALYSIS\n";
    std::cout << std::string(100, '=') << "\n";
    std::cout << "Image Size: " << H << "x" << W << "\n";
    std::cout << "Connectivity: " << (eight_conn ? "8" : "4") << "\n\n";
    
    std::cout << std::left << std::setw(10) << "Density";
    std::cout << std::right << std::setw(15) << "2-Pass (μs)";
    std::cout << std::setw(15) << "BFS (μs)";
    std::cout << std::setw(15) << "DFS (μs)";
    std::cout << std::setw(15) << "DSU (μs)";
    std::cout << "\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (double density : densities) {
        // Generate test image
        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        std::vector<uint8_t> img(H * W);
        for (int i = 0; i < H * W; ++i) {
            img[i] = (dis(gen) < density) ? 1 : 0;
        }
        
        // Benchmark all algorithms
        auto m_2pass = benchmark_with_metrics(label_cc_2pass, img.data(), H, W, eight_conn, iterations, "2-Pass");
        auto m_bfs = benchmark_with_metrics(label_cc_bfs, img.data(), H, W, eight_conn, iterations, "BFS");
        auto m_dfs = benchmark_with_metrics(label_cc_dfs, img.data(), H, W, eight_conn, iterations, "DFS");
        auto m_dsu = benchmark_with_metrics(label_cc_dsu, img.data(), H, W, eight_conn, iterations, "DSU");
        
        std::cout << std::fixed << std::setprecision(1);
        std::cout << std::left << std::setw(10) << density;
        std::cout << std::right << std::setprecision(2);
        std::cout << std::setw(15) << m_2pass.time_us;
        std::cout << std::setw(15) << m_bfs.time_us;
        std::cout << std::setw(15) << m_dfs.time_us;
        std::cout << std::setw(15) << m_dsu.time_us;
        std::cout << "\n";
    }
    std::cout << std::string(70, '=') << "\n\n";
}

void test_different_sizes(bool eight_conn, int iterations) {
    std::vector<std::pair<int, int>> sizes = {{100, 100}, {500, 500}, {1000, 1000}, {2000, 2000}};
    double density = 0.3;
    
    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "PERFORMANCE vs IMAGE SIZE ANALYSIS\n";
    std::cout << std::string(100, '=') << "\n";
    std::cout << "Density: " << density << "\n";
    std::cout << "Connectivity: " << (eight_conn ? "8" : "4") << "\n\n";
    
    std::cout << std::left << std::setw(15) << "Size";
    std::cout << std::right << std::setw(12) << "MPixels";
    std::cout << std::setw(15) << "2-Pass (μs)";
    std::cout << std::setw(15) << "BFS (μs)";
    std::cout << std::setw(15) << "DFS (μs)";
    std::cout << std::setw(15) << "DSU (μs)";
    std::cout << "\n";
    std::cout << std::string(87, '-') << "\n";
    
    for (const auto& [H, W] : sizes) {
        // Generate test image
        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        std::vector<uint8_t> img(H * W);
        for (int i = 0; i < H * W; ++i) {
            img[i] = (dis(gen) < density) ? 1 : 0;
        }
        
        // Benchmark all algorithms
        auto m_2pass = benchmark_with_metrics(label_cc_2pass, img.data(), H, W, eight_conn, iterations, "2-Pass");
        auto m_bfs = benchmark_with_metrics(label_cc_bfs, img.data(), H, W, eight_conn, iterations, "BFS");
        auto m_dfs = benchmark_with_metrics(label_cc_dfs, img.data(), H, W, eight_conn, iterations, "DFS");
        auto m_dsu = benchmark_with_metrics(label_cc_dsu, img.data(), H, W, eight_conn, iterations, "DSU");
        
        std::cout << std::left << std::setw(15) << (std::to_string(H) + "x" + std::to_string(W));
        std::cout << std::right << std::fixed << std::setprecision(3);
        std::cout << std::setw(12) << (H * W / 1e6);
        std::cout << std::setprecision(2);
        std::cout << std::setw(15) << m_2pass.time_us;
        std::cout << std::setw(15) << m_bfs.time_us;
        std::cout << std::setw(15) << m_dfs.time_us;
        std::cout << std::setw(15) << m_dsu.time_us;
        std::cout << "\n";
    }
    std::cout << std::string(87, '=') << "\n\n";
}

int main(int argc, char* argv[]) {
    int H = 1000;
    int W = 1000;
    double density = 0.3;
    int iterations = 10;
    bool eight_conn = false;
    bool full_analysis = false;

    if (argc > 1) H = std::stoi(argv[1]);
    if (argc > 2) W = std::stoi(argv[2]);
    if (argc > 3) density = std::stod(argv[3]);
    if (argc > 4) iterations = std::stoi(argv[4]);
    if (argc > 5) eight_conn = (std::stoi(argv[5]) != 0);
    if (argc > 6) full_analysis = (std::stoi(argv[6]) != 0);

    // Generate test image
    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    std::vector<uint8_t> img(H * W);
    int foreground_count = 0;
    for (int i = 0; i < H * W; ++i) {
        img[i] = (dis(gen) < density) ? 1 : 0;
        if (img[i] == 1) foreground_count++;
    }

    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "COMPREHENSIVE METRICS COMPARISON\n";
    std::cout << std::string(100, '=') << "\n";
    std::cout << "Test Configuration:\n";
    std::cout << "  Image Size: " << H << "x" << W << " (" << (H*W/1e6) << " MPixels)\n";
    std::cout << "  Density: " << density << " (" << foreground_count << " foreground pixels)\n";
    std::cout << "  Connectivity: " << (eight_conn ? "8" : "4") << "\n";
    std::cout << "  Iterations: " << iterations << "\n";

    // Benchmark all algorithms
    std::vector<std::pair<std::string, Metrics>> results;
    
    std::cout << "\nRunning benchmarks...\n";
    results.push_back({"2-Pass (DSU)", benchmark_with_metrics(label_cc_2pass, img.data(), H, W, eight_conn, iterations, "2-Pass")});
    results.push_back({"BFS", benchmark_with_metrics(label_cc_bfs, img.data(), H, W, eight_conn, iterations, "BFS")});
    results.push_back({"DFS", benchmark_with_metrics(label_cc_dfs, img.data(), H, W, eight_conn, iterations, "DFS")});
    results.push_back({"DSU (1-pass)", benchmark_with_metrics(label_cc_dsu, img.data(), H, W, eight_conn, iterations, "DSU")});

    // Print detailed metrics table
    print_metrics_table(results, H, W);

    // Additional analysis
    if (full_analysis || argc <= 6) {
        test_different_densities(H, W, eight_conn, iterations);
        test_different_sizes(eight_conn, iterations);
    }

    // Summary statistics
    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "SUMMARY STATISTICS\n";
    std::cout << std::string(100, '=') << "\n";
    
    double min_time = results[0].second.time_us;
    double max_time = results[0].second.time_us;
    std::string fastest = results[0].first;
    std::string slowest = results[0].first;
    
    for (const auto& [name, m] : results) {
        if (m.time_us < min_time) {
            min_time = m.time_us;
            fastest = name;
        }
        if (m.time_us > max_time) {
            max_time = m.time_us;
            slowest = name;
        }
    }
    
    std::cout << "Fastest Algorithm: " << fastest << " (" << min_time << " μs)\n";
    std::cout << "Slowest Algorithm: " << slowest << " (" << max_time << " μs)\n";
    std::cout << "Speed Difference: " << (max_time / min_time) << "x\n";
    
    // Memory comparison
    size_t min_mem = results[0].second.memory_kb;
    size_t max_mem = results[0].second.memory_kb;
    std::string most_mem = results[0].first;
    std::string least_mem = results[0].first;
    
    for (const auto& [name, m] : results) {
        if (m.memory_kb < min_mem) {
            min_mem = m.memory_kb;
            least_mem = name;
        }
        if (m.memory_kb > max_mem) {
            max_mem = m.memory_kb;
            most_mem = name;
        }
    }
    
    std::cout << "Most Memory Efficient: " << least_mem << " (" << min_mem << " KB)\n";
    std::cout << "Least Memory Efficient: " << most_mem << " (" << max_mem << " KB)\n";
    
    std::cout << std::string(100, '=') << "\n\n";

    return 0;
}

