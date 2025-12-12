#include "dsu_2pass.hpp"
#include "algorithms.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <numeric>

// Stream-based DSU implementation for incremental updates
class StreamDSU {
private:
    std::vector<int32_t> parent;
    std::vector<int8_t> rank;
    std::unordered_map<int, int32_t> coord_to_label;  // (y*W+x) -> label
    std::unordered_map<int32_t, int> label_count;      // label -> count
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
            label_count[rb] += label_count[ra];
            label_count.erase(ra);
        } else if (rank[ra] > rank[rb]) {
            parent[rb] = ra;
            label_count[ra] += label_count[rb];
            label_count.erase(rb);
        } else {
            parent[rb] = ra;
            rank[ra]++;
            label_count[ra] += label_count[rb];
            label_count.erase(rb);
        }
    }

    std::vector<int32_t> get_neighbors(int y, int x) {
        std::vector<int32_t> neighbors;
        int idx = coord_to_idx(y, x);
        
        // left
        if (x - 1 >= 0) {
            int left_idx = coord_to_idx(y, x - 1);
            if (coord_to_label.count(left_idx)) {
                int32_t root = find(coord_to_label[left_idx]);
                neighbors.push_back(root);
            }
        }
        // top
        if (y - 1 >= 0) {
            int top_idx = coord_to_idx(y - 1, x);
            if (coord_to_label.count(top_idx)) {
                int32_t root = find(coord_to_label[top_idx]);
                neighbors.push_back(root);
            }
        }
        
        if (eight_conn) {
            // top-left
            if (x - 1 >= 0 && y - 1 >= 0) {
                int tl_idx = coord_to_idx(y - 1, x - 1);
                if (coord_to_label.count(tl_idx)) {
                    int32_t root = find(coord_to_label[tl_idx]);
                    neighbors.push_back(root);
                }
            }
            // top-right
            if (x + 1 < W && y - 1 >= 0) {
                int tr_idx = coord_to_idx(y - 1, x + 1);
                if (coord_to_label.count(tr_idx)) {
                    int32_t root = find(coord_to_label[tr_idx]);
                    neighbors.push_back(root);
                }
            }
        }
        
        // Remove duplicates
        std::sort(neighbors.begin(), neighbors.end());
        neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
        
        return neighbors;
    }

public:
    StreamDSU(int h, int w, bool eight_connectivity = false) 
        : H(h), W(w), eight_conn(eight_connectivity), next_label(1) {
        // Reserve space for labels (upper bound: H*W)
        parent.resize(H * W + 1);
        rank.resize(H * W + 1, 0);
        for (size_t i = 0; i < parent.size(); ++i) {
            parent[i] = i;
        }
    }

    // Add a new pixel at (y, x) and update labels incrementally
    void add_pixel(int y, int x) {
        int idx = coord_to_idx(y, x);
        if (coord_to_label.count(idx)) {
            return;  // Already added
        }

        auto neighbors = get_neighbors(y, x);
        
        if (neighbors.empty()) {
            // New component
            coord_to_label[idx] = next_label;
            label_count[next_label] = 1;
            next_label++;
        } else {
            // Merge with existing neighbors - use minimum root label
            int32_t min_label = *std::min_element(neighbors.begin(), neighbors.end());
            coord_to_label[idx] = min_label;
            label_count[min_label]++;
            
            // Union with other neighbors
            for (int32_t nb : neighbors) {
                if (nb != min_label) {
                    union_set(min_label, nb);
                }
            }
        }
    }

    // Get current number of components
    int get_component_count() {
        // Count unique root labels
        std::unordered_set<int32_t> roots;
        for (const auto& pair : coord_to_label) {
            roots.insert(find(pair.second));
        }
        return roots.size();
    }

    // Get full label map (for verification)
    std::vector<int32_t> get_labels() {
        std::vector<int32_t> labels(H * W, 0);
        std::unordered_map<int32_t, int32_t> root_to_final;
        int final_label = 1;
        
        // Map roots to final labels
        std::unordered_set<int32_t> roots;
        for (const auto& pair : coord_to_label) {
            roots.insert(find(pair.second));
        }
        std::vector<int32_t> roots_sorted(roots.begin(), roots.end());
        std::sort(roots_sorted.begin(), roots_sorted.end());
        for (int32_t r : roots_sorted) {
            root_to_final[r] = final_label++;
        }
        
        // Apply final labels
        for (const auto& pair : coord_to_label) {
            int idx = pair.first;
            int32_t root = find(pair.second);
            labels[idx] = root_to_final[root];
        }
        
        return labels;
    }
};

// Simulate stream input: pixels arrive one by one
double benchmark_stream_dsu(const std::vector<std::pair<int, int>>& pixels, 
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

// Benchmark full recomputation (simulating traditional approach)
double benchmark_full_recompute(const std::vector<std::pair<int, int>>& pixels,
                                int H, int W, bool eight_conn, int iterations,
                                std::vector<int32_t> (*func)(const uint8_t*, int, int, bool)) {
    // Build full image from pixels
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

int main(int argc, char* argv[]) {
    int H = 500;
    int W = 500;
    int num_pixels = 50000;  // Number of pixels in stream
    int iterations = 10;
    bool eight_conn = false;

    if (argc > 1) H = std::stoi(argv[1]);
    if (argc > 2) W = std::stoi(argv[2]);
    if (argc > 3) num_pixels = std::stoi(argv[3]);
    if (argc > 4) iterations = std::stoi(argv[4]);
    if (argc > 5) eight_conn = (std::stoi(argv[5]) != 0);

    std::cout << "============================================================\n";
    std::cout << "Stream Processing Performance Comparison\n";
    std::cout << "============================================================\n";
    std::cout << "Image size: " << H << "x" << W << "\n";
    std::cout << "Stream size: " << num_pixels << " pixels\n";
    std::cout << "Connectivity: " << (eight_conn ? "8" : "4") << "\n";
    std::cout << "Iterations: " << iterations << "\n\n";

    // Generate random stream of pixels
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> y_dist(0, H - 1);
    std::uniform_int_distribution<> x_dist(0, W - 1);
    
    std::vector<std::pair<int, int>> pixels;
    std::set<std::pair<int, int>> seen;  // Avoid duplicates
    
    for (int i = 0; i < num_pixels; ++i) {
        int y, x;
        do {
            y = y_dist(gen);
            x = x_dist(gen);
        } while (seen.count({y, x}));
        seen.insert({y, x});
        pixels.push_back({y, x});
    }

    std::cout << "Generated " << pixels.size() << " unique pixels\n\n";

    // Benchmark Stream DSU (incremental)
    std::cout << "Testing Stream DSU (incremental update)...\n";
    double stream_time = benchmark_stream_dsu(pixels, H, W, eight_conn, iterations);
    
    // Verify correctness
    StreamDSU verify_dsu(H, W, eight_conn);
    for (const auto& p : pixels) {
        verify_dsu.add_pixel(p.first, p.second);
    }
    int stream_components = verify_dsu.get_component_count();
    std::cout << "  Time: " << stream_time << " μs\n";
    std::cout << "  Components: " << stream_components << "\n\n";

    // Benchmark full recomputation methods
    std::cout << "Testing Full Recomputation Methods...\n";
    
    double time_2pass = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_2pass);
    double time_bfs = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_bfs);
    double time_dfs = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_dfs);
    double time_dsu = benchmark_full_recompute(pixels, H, W, eight_conn, iterations, label_cc_dsu);

    // Verify correctness
    std::vector<uint8_t> img(H * W, 0);
    for (const auto& p : pixels) {
        img[p.first * W + p.second] = 1;
    }
    auto result_2pass = label_cc_2pass(img.data(), H, W, eight_conn);
    std::set<int32_t> components_2pass;
    for (int i = 0; i < H * W; ++i) {
        if (result_2pass[i] > 0) components_2pass.insert(result_2pass[i]);
    }
    int full_components = components_2pass.size();

    std::cout << "  2-Pass (DSU): " << time_2pass << " μs\n";
    std::cout << "  BFS:          " << time_bfs << " μs\n";
    std::cout << "  DFS:          " << time_dfs << " μs\n";
    std::cout << "  DSU (1-pass): " << time_dsu << " μs\n";
    std::cout << "  Components:  " << full_components << "\n\n";

    // Verify results match
    if (stream_components != full_components) {
        std::cerr << "WARNING: Component count mismatch!\n";
        std::cerr << "  Stream DSU: " << stream_components << "\n";
        std::cerr << "  Full recompute: " << full_components << "\n";
    } else {
        std::cout << "✓ Results verified: " << stream_components << " components\n\n";
    }

    // Comparison
    std::cout << "============================================================\n";
    std::cout << "Performance Comparison:\n";
    std::cout << "============================================================\n";
    std::cout << "Stream DSU (incremental): " << stream_time << " μs\n";
    std::cout << "Full 2-Pass:              " << time_2pass << " μs (";
    std::cout << (time_2pass / stream_time) << "x slower)\n";
    std::cout << "Full BFS:                 " << time_bfs << " μs (";
    std::cout << (time_bfs / stream_time) << "x slower)\n";
    std::cout << "Full DFS:                 " << time_dfs << " μs (";
    std::cout << (time_dfs / stream_time) << "x slower)\n";
    std::cout << "Full DSU (1-pass):        " << time_dsu << " μs (";
    std::cout << (time_dsu / stream_time) << "x slower)\n\n";

    if (stream_time < time_2pass && stream_time < time_bfs && 
        stream_time < time_dfs && stream_time < time_dsu) {
        std::cout << "✅ Stream DSU is FASTER for incremental updates!\n";
        std::cout << "   Advantage: " << std::min({time_2pass, time_bfs, time_dfs, time_dsu}) / stream_time;
        std::cout << "x speedup\n";
    } else {
        std::cout << "⚠️  Stream DSU may not be faster in this scenario.\n";
        std::cout << "   Consider: stream overhead vs full recomputation cost\n";
    }

    return 0;
}

