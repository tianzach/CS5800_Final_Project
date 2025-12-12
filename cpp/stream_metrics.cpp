#include "dsu_2pass.hpp"
#include "algorithms.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <set>
#include <algorithm>
#include <cstring>
#include <sys/resource.h>
#include <unordered_map>
#include <unordered_set>

// Stream DSU implementation
class StreamDSU {
private:
    std::vector<int32_t> parent;
    std::vector<int8_t> rank;
    std::unordered_map<int, int32_t> coord_to_label;
    int32_t next_label;
    int H, W;
    bool eight_conn;

    int coord_to_idx(int y, int x) const {
        return y * W + x;
    }

    int32_t find(int32_t x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }

    void union_set(int32_t a, int32_t b) {
        if (a == b) return;
        int32_t ra = find(a);
        int32_t rb = find(b);
        if (ra == rb) return;
        
        if (rank[ra] < rank[rb]) {
            parent[ra] = rb;
        } else if (rank[ra] > rank[rb]) {
            parent[rb] = ra;
        } else {
            parent[rb] = ra;
            rank[ra]++;
        }
    }

    std::vector<int32_t> get_neighbors(int y, int x) {
        std::vector<int32_t> neighbors;
        int idx = coord_to_idx(y, x);
        
        if (x - 1 >= 0) {
            int left_idx = coord_to_idx(y, x - 1);
            if (coord_to_label.count(left_idx)) {
                neighbors.push_back(find(coord_to_label[left_idx]));
            }
        }
        if (y - 1 >= 0) {
            int top_idx = coord_to_idx(y - 1, x);
            if (coord_to_label.count(top_idx)) {
                neighbors.push_back(find(coord_to_label[top_idx]));
            }
        }
        
        if (eight_conn) {
            if (x - 1 >= 0 && y - 1 >= 0) {
                int tl_idx = coord_to_idx(y - 1, x - 1);
                if (coord_to_label.count(tl_idx)) {
                    neighbors.push_back(find(coord_to_label[tl_idx]));
                }
            }
            if (x + 1 < W && y - 1 >= 0) {
                int tr_idx = coord_to_idx(y - 1, x + 1);
                if (coord_to_label.count(tr_idx)) {
                    neighbors.push_back(find(coord_to_label[tr_idx]));
                }
            }
        }
        
        std::sort(neighbors.begin(), neighbors.end());
        neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
        return neighbors;
    }

public:
    StreamDSU(int h, int w, bool eight_connectivity = false) 
        : H(h), W(w), eight_conn(eight_connectivity), next_label(1) {
        parent.resize(H * W + 1);
        rank.resize(H * W + 1, 0);
        for (size_t i = 0; i < parent.size(); ++i) {
            parent[i] = i;
        }
    }

    void add_pixel(int y, int x) {
        int idx = coord_to_idx(y, x);
        if (coord_to_label.count(idx)) {
            return;
        }

        auto neighbors = get_neighbors(y, x);
        
        if (neighbors.empty()) {
            coord_to_label[idx] = next_label;
            next_label++;
        } else {
            int32_t min_label = *std::min_element(neighbors.begin(), neighbors.end());
            coord_to_label[idx] = min_label;
            
            for (int32_t nb : neighbors) {
                if (nb != min_label) {
                    union_set(min_label, nb);
                }
            }
        }
    }

    int get_component_count() {
        std::unordered_set<int32_t> roots;
        for (const auto& pair : coord_to_label) {
            roots.insert(find(pair.second));
        }
        return roots.size();
    }

    size_t get_memory_usage() {
        return coord_to_label.size() * (sizeof(int) + sizeof(int32_t)) + 
               parent.size() * (sizeof(int32_t) + sizeof(int8_t));
    }
};

struct StreamMetrics {
    double time_us;
    size_t memory_bytes;
    int components;
    double throughput_pixels_per_sec;
    int num_pixels;
};

StreamMetrics benchmark_stream_dsu(
    const std::vector<std::pair<int, int>>& pixels,
    int H, int W, bool eight_conn, int iterations) {
    
    StreamMetrics m;
    m.num_pixels = pixels.size();
    
    auto start = std::chrono::high_resolution_clock::now();
    StreamDSU* stream_dsu = nullptr;
    
    for (int iter = 0; iter < iterations; ++iter) {
        if (stream_dsu) delete stream_dsu;
        stream_dsu = new StreamDSU(H, W, eight_conn);
        for (const auto& p : pixels) {
            stream_dsu->add_pixel(p.first, p.second);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    m.time_us = duration.count() / (double)iterations;
    m.components = stream_dsu->get_component_count();
    m.memory_bytes = stream_dsu->get_memory_usage();
    m.throughput_pixels_per_sec = (m.num_pixels / (m.time_us / 1e6)) / 1e6; // MPixels/sec
    
    delete stream_dsu;
    return m;
}

StreamMetrics benchmark_full_recompute(
    const std::vector<std::pair<int, int>>& pixels,
    int H, int W, bool eight_conn, int iterations,
    std::vector<int32_t> (*func)(const uint8_t*, int, int, bool),
    const char* name) {
    
    std::vector<uint8_t> img(H * W, 0);
    for (const auto& p : pixels) {
        img[p.first * W + p.second] = 1;
    }
    
    StreamMetrics m;
    m.num_pixels = pixels.size();
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        func(img.data(), H, W, eight_conn);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    m.time_us = duration.count() / (double)iterations;
    
    auto result = func(img.data(), H, W, eight_conn);
    std::set<int32_t> comps;
    for (int i = 0; i < H * W; ++i) {
        if (result[i] > 0) comps.insert(result[i]);
    }
    m.components = comps.size();
    m.memory_bytes = H * W * sizeof(uint8_t) + H * W * sizeof(int32_t); // Approximate
    m.throughput_pixels_per_sec = (m.num_pixels / (m.time_us / 1e6)) / 1e6;
    
    return m;
}

void print_stream_metrics_table(
    const std::vector<std::pair<std::string, StreamMetrics>>& results,
    int H, int W, int num_pixels) {
    
    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "STREAM PROCESSING METRICS COMPARISON\n";
    std::cout << std::string(100, '=') << "\n";
    std::cout << "Image Size: " << H << "x" << W << "\n";
    std::cout << "Stream Size: " << num_pixels << " pixels (" << (num_pixels/1e6) << " MPixels)\n\n";
    
    double best_time = results[0].second.time_us;
    for (const auto& r : results) {
        if (r.second.time_us < best_time) best_time = r.second.time_us;
    }
    
    std::cout << std::left << std::setw(20) << "Method";
    std::cout << std::right << std::setw(15) << "Time (μs)";
    std::cout << std::setw(15) << "Speedup";
    std::cout << std::setw(15) << "Memory (MB)";
    std::cout << std::setw(15) << "Throughput";
    std::cout << std::setw(12) << "Components";
    std::cout << "\n";
    std::cout << std::string(100, '-') << "\n";
    
    for (const auto& [name, m] : results) {
        std::cout << std::left << std::setw(20) << name;
        std::cout << std::right << std::fixed << std::setprecision(2);
        std::cout << std::setw(15) << m.time_us;
        
        double speedup = best_time / m.time_us;
        std::cout << std::setw(14) << std::setprecision(2) << speedup << "x";
        
        std::cout << std::setw(14) << std::setprecision(2) << (m.memory_bytes / 1e6) << " MB";
        std::cout << std::setw(14) << std::setprecision(2) << m.throughput_pixels_per_sec << " MP/s";
        std::cout << std::setw(12) << m.components;
        std::cout << "\n";
    }
    std::cout << std::string(100, '=') << "\n\n";
}

void test_stream_vs_batch(int H, int W, bool eight_conn, int iterations) {
    std::vector<int> stream_sizes = {10000, 30000, 50000, 100000};
    double density = 0.3;
    
    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "STREAM vs BATCH PROCESSING COMPARISON\n";
    std::cout << std::string(100, '=') << "\n";
    std::cout << "Image Size: " << H << "x" << W << "\n";
    std::cout << "Connectivity: " << (eight_conn ? "8" : "4") << "\n\n";
    
    std::cout << std::left << std::setw(12) << "Stream Size";
    std::cout << std::right << std::setw(20) << "Stream DSU (μs)";
    std::cout << std::setw(20) << "Full BFS (μs)";
    std::cout << std::setw(20) << "Speedup";
    std::cout << "\n";
    std::cout << std::string(72, '-') << "\n";
    
    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_int_distribution<> y_dist(0, H - 1);
    std::uniform_int_distribution<> x_dist(0, W - 1);
    
    for (int stream_size : stream_sizes) {
        if (stream_size > H * W) continue;
        
        // Generate stream pixels
        std::vector<std::pair<int, int>> pixels;
        std::set<std::pair<int, int>> used;
        
        for (int i = 0; i < stream_size; ++i) {
            int y, x;
            do {
                y = y_dist(gen);
                x = x_dist(gen);
            } while (used.count({y, x}));
            used.insert({y, x});
            pixels.push_back({y, x});
        }
        
        auto stream_metrics = benchmark_stream_dsu(pixels, H, W, eight_conn, iterations);
        auto batch_metrics = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_bfs, "BFS");
        
        std::cout << std::left << std::setw(12) << stream_size;
        std::cout << std::right << std::fixed << std::setprecision(2);
        std::cout << std::setw(20) << stream_metrics.time_us;
        std::cout << std::setw(20) << batch_metrics.time_us;
        double speedup = batch_metrics.time_us / stream_metrics.time_us;
        std::cout << std::setw(19) << std::setprecision(2) << speedup << "x";
        std::cout << (speedup > 1.0 ? " (Stream faster)" : " (Batch faster)") << "\n";
    }
    std::cout << std::string(72, '=') << "\n\n";
}

int main(int argc, char* argv[]) {
    int H = 1000;
    int W = 1000;
    int num_pixels = 50000;
    int iterations = 10;
    bool eight_conn = false;
    bool full_analysis = false;

    if (argc > 1) H = std::stoi(argv[1]);
    if (argc > 2) W = std::stoi(argv[2]);
    if (argc > 3) num_pixels = std::stoi(argv[3]);
    if (argc > 4) iterations = std::stoi(argv[4]);
    if (argc > 5) eight_conn = (std::stoi(argv[5]) != 0);
    if (argc > 6) full_analysis = (std::stoi(argv[6]) != 0);

    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "STREAM INPUT METRICS COMPARISON\n";
    std::cout << std::string(100, '=') << "\n";
    std::cout << "Test Configuration:\n";
    std::cout << "  Image Size: " << H << "x" << W << "\n";
    std::cout << "  Stream Size: " << num_pixels << " pixels\n";
    std::cout << "  Connectivity: " << (eight_conn ? "8" : "4") << "\n";
    std::cout << "  Iterations: " << iterations << "\n";

    // Generate stream pixels (random order)
    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_int_distribution<> y_dist(0, H - 1);
    std::uniform_int_distribution<> x_dist(0, W - 1);
    
    std::vector<std::pair<int, int>> pixels;
    std::set<std::pair<int, int>> used;
    
    for (int i = 0; i < num_pixels; ++i) {
        int y, x;
        do {
            y = y_dist(gen);
            x = x_dist(gen);
        } while (used.count({y, x}));
        used.insert({y, x});
        pixels.push_back({y, x});
    }

    std::cout << "\nGenerated " << pixels.size() << " unique pixels in random order\n";
    std::cout << "Running benchmarks...\n";

    // Benchmark all methods
    std::vector<std::pair<std::string, StreamMetrics>> results;
    
    results.push_back({"Stream DSU", benchmark_stream_dsu(pixels, H, W, eight_conn, iterations)});
    results.push_back({"Full 2-Pass", benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_2pass, "2-Pass")});
    results.push_back({"Full BFS", benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_bfs, "BFS")});
    results.push_back({"Full DFS", benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_dfs, "DFS")});
    results.push_back({"Full DSU", benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_dsu, "DSU")});

    print_stream_metrics_table(results, H, W, num_pixels);

    // Additional analysis
    if (full_analysis) {
        test_stream_vs_batch(H, W, eight_conn, iterations);
    }

    // Summary
    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "SUMMARY FOR STREAM PROCESSING\n";
    std::cout << std::string(100, '=') << "\n";
    
    double min_time = results[0].second.time_us;
    std::string fastest = results[0].first;
    for (const auto& [name, m] : results) {
        if (m.time_us < min_time) {
            min_time = m.time_us;
            fastest = name;
        }
    }
    
    std::cout << "Fastest for Stream: " << fastest << " (" << min_time << " μs)\n";
    
    // Compare stream vs batch
    double stream_time = results[0].second.time_us;
    double batch_time = results[2].second.time_us; // BFS
    std::cout << "Stream DSU vs Full BFS: " << (batch_time / stream_time) << "x ";
    std::cout << (stream_time < batch_time ? "(Stream faster)" : "(Batch faster)") << "\n";
    
    std::cout << std::string(100, '=') << "\n\n";

    return 0;
}

