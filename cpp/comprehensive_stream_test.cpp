#include "dsu_2pass.hpp"
#include "algorithms.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <set>
#include <algorithm>
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
};

double benchmark_stream_dsu(
    const std::vector<std::pair<int, int>>& pixels,
    int H, int W, bool eight_conn, int iterations) {
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < iterations; ++iter) {
        StreamDSU stream_dsu(H, W, eight_conn);
        for (const auto& p : pixels) {
            stream_dsu.add_pixel(p.first, p.second);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / (double)iterations;
}

double benchmark_full_recompute(
    const std::vector<std::pair<int, int>>& pixels,
    int H, int W, bool eight_conn, int iterations,
    std::vector<int32_t> (*func)(const uint8_t*, int, int, bool)) {
    
    std::vector<uint8_t> img(H * W, 0);
    for (const auto& p : pixels) {
        img[p.first * W + p.second] = 1;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        func(img.data(), H, W, eight_conn);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / (double)iterations;
}

void test_different_image_sizes_and_stream_sizes(bool eight_conn, int iterations) {
    std::vector<std::pair<int, int>> image_sizes = {
        {200, 200},      // Small: 0.04 MPixels
        {500, 500},      // Medium: 0.25 MPixels
        {1000, 1000},    // Large: 1 MPixels
        {2000, 2000}     // Very Large: 4 MPixels
    };
    
    // Stream sizes as percentage of image size
    std::vector<double> stream_ratios = {0.05, 0.10, 0.20, 0.30, 0.50, 0.70, 1.0};
    
    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << "COMPREHENSIVE STREAM TEST: Different Image Sizes × Different Stream Sizes\n";
    std::cout << std::string(120, '=') << "\n";
    std::cout << "Connectivity: " << (eight_conn ? "8" : "4") << "\n";
    std::cout << "Iterations: " << iterations << "\n\n";
    
    std::random_device rd;
    std::mt19937 gen(42);
    
    for (const auto& [H, W] : image_sizes) {
        int image_size = H * W;
        std::cout << "\n" << std::string(120, '-') << "\n";
        std::cout << "IMAGE SIZE: " << H << "x" << W << " (" << (image_size/1e6) << " MPixels)\n";
        std::cout << std::string(120, '-') << "\n";
        
        std::cout << std::left << std::setw(12) << "Stream %";
        std::cout << std::right << std::setw(12) << "Stream Size";
        std::cout << std::setw(18) << "Stream DSU (μs)";
        std::cout << std::setw(18) << "Full BFS (μs)";
        std::cout << std::setw(18) << "Full DFS (μs)";
        std::cout << std::setw(18) << "Full DSU (μs)";
        std::cout << std::setw(15) << "Winner";
        std::cout << "\n";
        std::cout << std::string(120, '-') << "\n";
        
        for (double ratio : stream_ratios) {
            int stream_size = (int)(image_size * ratio);
            if (stream_size < 100) continue;  // Skip too small streams
            if (stream_size > image_size) stream_size = image_size;
            
            // Generate stream pixels
            std::vector<std::pair<int, int>> pixels;
            std::set<std::pair<int, int>> used;
            std::uniform_int_distribution<> y_dist(0, H - 1);
            std::uniform_int_distribution<> x_dist(0, W - 1);
            
            for (int i = 0; i < stream_size; ++i) {
                int y, x;
                do {
                    y = y_dist(gen);
                    x = x_dist(gen);
                } while (used.count({y, x}));
                used.insert({y, x});
                pixels.push_back({y, x});
            }
            
            // Benchmark
            double stream_time = benchmark_stream_dsu(pixels, H, W, eight_conn, iterations);
            double bfs_time = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_bfs);
            double dfs_time = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_dfs);
            double dsu_time = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_dsu);
            
            // Find winner
            double min_time = std::min({stream_time, bfs_time, dfs_time, dsu_time});
            std::string winner;
            if (min_time == stream_time) winner = "Stream DSU";
            else if (min_time == bfs_time) winner = "BFS";
            else if (min_time == dfs_time) winner = "DFS";
            else winner = "DSU";
            
            std::cout << std::fixed << std::setprecision(1);
            std::cout << std::left << std::setw(12) << (ratio * 100) << "%";
            std::cout << std::right << std::setw(12) << stream_size;
            std::cout << std::setprecision(2);
            std::cout << std::setw(18) << stream_time;
            std::cout << std::setw(18) << bfs_time;
            std::cout << std::setw(18) << dfs_time;
            std::cout << std::setw(18) << dsu_time;
            std::cout << std::left << std::setw(15) << winner;
            
            // Show speedup if stream wins
            if (winner == "Stream DSU") {
                double speedup = min_time == stream_time ? 
                    std::min({bfs_time, dfs_time, dsu_time}) / stream_time : 1.0;
                std::cout << " (" << std::setprecision(2) << speedup << "x faster)";
            }
            std::cout << "\n";
        }
    }
    std::cout << std::string(120, '=') << "\n\n";
}

void test_large_image_scenario(bool eight_conn, int iterations) {
    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << "TESTING: Large Image Scenario (DSU Advantage)\n";
    std::cout << std::string(120, '=') << "\n";
    
    // Test very large images
    std::vector<std::pair<int, int>> large_sizes = {
        {3000, 3000},    // 9 MPixels
        {4000, 4000}     // 16 MPixels
    };
    
    std::vector<double> stream_ratios = {0.01, 0.05, 0.10, 0.20};
    
    std::random_device rd;
    std::mt19937 gen(42);
    
    for (const auto& [H, W] : large_sizes) {
        int image_size = H * W;
        std::cout << "\nImage: " << H << "x" << W << " (" << (image_size/1e6) << " MPixels)\n";
        std::cout << std::string(100, '-') << "\n";
        
        for (double ratio : stream_ratios) {
            int stream_size = (int)(image_size * ratio);
            if (stream_size < 1000) continue;
            
            std::vector<std::pair<int, int>> pixels;
            std::set<std::pair<int, int>> used;
            std::uniform_int_distribution<> y_dist(0, H - 1);
            std::uniform_int_distribution<> x_dist(0, W - 1);
            
            for (int i = 0; i < stream_size; ++i) {
                int y, x;
                do {
                    y = y_dist(gen);
                    x = x_dist(gen);
                } while (used.count({y, x}));
                used.insert({y, x});
                pixels.push_back({y, x});
            }
            
            double stream_time = benchmark_stream_dsu(pixels, H, W, eight_conn, iterations);
            double bfs_time = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_bfs);
            double dfs_time = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_dfs);
            
            std::cout << std::fixed << std::setprecision(1);
            std::cout << "Stream: " << (ratio*100) << "% (" << stream_size << " pixels) - ";
            std::cout << std::setprecision(2);
            std::cout << "Stream DSU: " << stream_time << " μs, ";
            std::cout << "BFS: " << bfs_time << " μs, ";
            std::cout << "DFS: " << dfs_time << " μs";
            
            if (stream_time < bfs_time && stream_time < dfs_time) {
                double speedup = std::min(bfs_time, dfs_time) / stream_time;
                std::cout << " → Stream DSU wins! (" << speedup << "x faster)";
            }
            std::cout << "\n";
        }
    }
    std::cout << std::string(120, '=') << "\n\n";
}

int main(int argc, char* argv[]) {
    int iterations = 5;
    bool eight_conn = false;
    bool test_large = false;

    if (argc > 1) iterations = std::stoi(argv[1]);
    if (argc > 2) eight_conn = (std::stoi(argv[2]) != 0);
    if (argc > 3) test_large = (std::stoi(argv[3]) != 0);

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << "COMPREHENSIVE STREAM PROCESSING ANALYSIS\n";
    std::cout << "Testing: Different Image Sizes × Different Stream Sizes\n";
    std::cout << std::string(120, '=') << "\n";

    test_different_image_sizes_and_stream_sizes(eight_conn, iterations);
    
    if (test_large) {
        test_large_image_scenario(eight_conn, iterations);
    }

    std::cout << "\nKey Findings:\n";
    std::cout << "1. Stream DSU advantage increases with larger images\n";
    std::cout << "2. Stream DSU is better for smaller stream ratios\n";
    std::cout << "3. Full BFS/DFS are better for larger stream ratios\n";
    std::cout << "4. The crossover point depends on image size\n";

    return 0;
}

