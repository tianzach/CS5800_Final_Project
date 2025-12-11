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

// Incremental DSU for updating existing image
class IncrementalDSU {
private:
    std::vector<int32_t> parent;
    std::vector<int8_t> rank;
    std::vector<int32_t> labels;  // Full label map
    int H, W;
    bool eight_conn;
    int next_label;

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

    std::vector<int32_t> get_neighbor_labels(int y, int x) {
        std::vector<int32_t> neighbor_labels;
        
        // left
        if (x - 1 >= 0 && labels[coord_to_idx(y, x - 1)] > 0) {
            neighbor_labels.push_back(labels[coord_to_idx(y, x - 1)]);
        }
        // top
        if (y - 1 >= 0 && labels[coord_to_idx(y - 1, x)] > 0) {
            neighbor_labels.push_back(labels[coord_to_idx(y - 1, x)]);
        }
        
        if (eight_conn) {
            // top-left
            if (x - 1 >= 0 && y - 1 >= 0 && labels[coord_to_idx(y - 1, x - 1)] > 0) {
                neighbor_labels.push_back(labels[coord_to_idx(y - 1, x - 1)]);
            }
            // top-right
            if (x + 1 < W && y - 1 >= 0 && labels[coord_to_idx(y - 1, x + 1)] > 0) {
                neighbor_labels.push_back(labels[coord_to_idx(y - 1, x + 1)]);
            }
        }
        
        // Get roots and remove duplicates
        std::set<int32_t> roots;
        for (int32_t lab : neighbor_labels) {
            roots.insert(find(lab));
        }
        return std::vector<int32_t>(roots.begin(), roots.end());
    }

public:
    IncrementalDSU(int h, int w, bool eight_connectivity = false) 
        : H(h), W(w), eight_conn(eight_connectivity), next_label(1) {
        labels.resize(H * W, 0);
        parent.resize(H * W + 1);
        rank.resize(H * W + 1, 0);
        for (size_t i = 0; i < parent.size(); ++i) {
            parent[i] = i;
        }
    }

    // Initialize from existing image
    void initialize(const uint8_t* img) {
        // First pass: assign temporary labels
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                if (img[coord_to_idx(y, x)] == 0) {
                    continue;
                }

                auto neighbor_roots = get_neighbor_labels(y, x);
                
                if (neighbor_roots.empty()) {
                    labels[coord_to_idx(y, x)] = next_label;
                    next_label++;
                } else {
                    int32_t min_label = *std::min_element(neighbor_roots.begin(), neighbor_roots.end());
                    labels[coord_to_idx(y, x)] = min_label;
                    
                    for (int32_t nb : neighbor_roots) {
                        if (nb != min_label) {
                            union_set(min_label, nb);
                        }
                    }
                }
            }
        }
        
        // Second pass: compress labels
        compress_labels();
    }

    void compress_labels() {
        std::unordered_set<int32_t> used_roots;
        for (int i = 0; i < H * W; ++i) {
            if (labels[i] > 0) {
                used_roots.insert(find(labels[i]));
            }
        }
        
        std::vector<int32_t> roots_sorted(used_roots.begin(), used_roots.end());
        std::sort(roots_sorted.begin(), roots_sorted.end());
        
        std::unordered_map<int32_t, int32_t> root_to_final;
        for (size_t i = 0; i < roots_sorted.size(); ++i) {
            root_to_final[roots_sorted[i]] = i + 1;
        }
        
        for (int i = 0; i < H * W; ++i) {
            if (labels[i] > 0) {
                labels[i] = root_to_final[find(labels[i])];
            }
        }
        
        next_label = roots_sorted.size() + 1;
    }

    // Add a new pixel incrementally
    void add_pixel(int y, int x) {
        int idx = coord_to_idx(y, x);
        if (labels[idx] > 0) {
            return;  // Already foreground
        }

        labels[idx] = 0;  // Temporarily mark
        auto neighbor_roots = get_neighbor_labels(y, x);
        
        if (neighbor_roots.empty()) {
            labels[idx] = next_label;
            next_label++;
        } else {
            int32_t min_label = *std::min_element(neighbor_roots.begin(), neighbor_roots.end());
            labels[idx] = min_label;
            
            for (int32_t nb : neighbor_roots) {
                if (nb != min_label) {
                    union_set(min_label, nb);
                }
            }
        }
    }

    // Get component count
    int get_component_count() {
        std::unordered_set<int32_t> roots;
        for (int i = 0; i < H * W; ++i) {
            if (labels[i] > 0) {
                roots.insert(find(labels[i]));
            }
        }
        return roots.size();
    }

    std::vector<int32_t> get_labels() {
        compress_labels();
        return labels;
    }
};

double benchmark_incremental_updates(
    const std::vector<uint8_t>& base_img,
    const std::vector<std::pair<int, int>>& new_pixels,
    int H, int W, bool eight_conn, int iterations) {
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < iterations; ++iter) {
        IncrementalDSU inc_dsu(H, W, eight_conn);
        inc_dsu.initialize(base_img.data());
        
        for (const auto& p : new_pixels) {
            inc_dsu.add_pixel(p.first, p.second);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / (double)iterations;
}

double benchmark_full_recompute(
    const std::vector<uint8_t>& base_img,
    const std::vector<std::pair<int, int>>& new_pixels,
    int H, int W, bool eight_conn, int iterations,
    std::vector<int32_t> (*func)(const uint8_t*, int, int, bool)) {
    
    std::vector<uint8_t> full_img = base_img;
    for (const auto& p : new_pixels) {
        full_img[p.first * W + p.second] = 1;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        func(full_img.data(), H, W, eight_conn);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / (double)iterations;
}

int main(int argc, char* argv[]) {
    int H = 500;
    int W = 500;
    int base_pixels = 30000;  // Initial pixels
    int new_pixels_count = 5000;  // Pixels to add incrementally
    int iterations = 10;
    bool eight_conn = false;

    if (argc > 1) H = std::stoi(argv[1]);
    if (argc > 2) W = std::stoi(argv[2]);
    if (argc > 3) base_pixels = std::stoi(argv[3]);
    if (argc > 4) new_pixels_count = std::stoi(argv[4]);
    if (argc > 5) iterations = std::stoi(argv[5]);
    if (argc > 6) eight_conn = (std::stoi(argv[6]) != 0);

    std::cout << "============================================================\n";
    std::cout << "Incremental Update Performance Comparison\n";
    std::cout << "============================================================\n";
    std::cout << "Image size: " << H << "x" << W << "\n";
    std::cout << "Base pixels: " << base_pixels << "\n";
    std::cout << "New pixels to add: " << new_pixels_count << "\n";
    std::cout << "Connectivity: " << (eight_conn ? "8" : "4") << "\n";
    std::cout << "Iterations: " << iterations << "\n\n";

    // Generate base image
    std::random_device rd;
    std::mt19937 gen(42);  // Fixed seed for reproducibility
    std::uniform_int_distribution<> y_dist(0, H - 1);
    std::uniform_int_distribution<> x_dist(0, W - 1);
    
    std::vector<uint8_t> base_img(H * W, 0);
    std::set<std::pair<int, int>> used;
    
    for (int i = 0; i < base_pixels; ++i) {
        int y, x;
        do {
            y = y_dist(gen);
            x = x_dist(gen);
        } while (used.count({y, x}));
        used.insert({y, x});
        base_img[y * W + x] = 1;
    }

    // Generate new pixels to add
    std::vector<std::pair<int, int>> new_pixels;
    for (int i = 0; i < new_pixels_count; ++i) {
        int y, x;
        do {
            y = y_dist(gen);
            x = x_dist(gen);
        } while (used.count({y, x}));
        used.insert({y, x});
        new_pixels.push_back({y, x});
    }

    std::cout << "Generated " << base_pixels << " base pixels and ";
    std::cout << new_pixels.size() << " new pixels\n\n";

    // Benchmark incremental DSU
    std::cout << "Testing Incremental DSU (update existing image)...\n";
    double inc_time = benchmark_incremental_updates(base_img, new_pixels, H, W, eight_conn, iterations);
    
    IncrementalDSU verify_inc(H, W, eight_conn);
    verify_inc.initialize(base_img.data());
    for (const auto& p : new_pixels) {
        verify_inc.add_pixel(p.first, p.second);
    }
    int inc_components = verify_inc.get_component_count();
    std::cout << "  Time: " << inc_time << " μs\n";
    std::cout << "  Components: " << inc_components << "\n\n";

    // Benchmark full recomputation
    std::cout << "Testing Full Recomputation Methods...\n";
    double time_2pass = benchmark_full_recompute(base_img, new_pixels, H, W, eight_conn, iterations, label_cc_2pass);
    double time_bfs = benchmark_full_recompute(base_img, new_pixels, H, W, eight_conn, iterations, label_cc_bfs);
    double time_dfs = benchmark_full_recompute(base_img, new_pixels, H, W, eight_conn, iterations, label_cc_dfs);
    double time_dsu = benchmark_full_recompute(base_img, new_pixels, H, W, eight_conn, iterations, label_cc_dsu);

    // Verify correctness
    std::vector<uint8_t> full_img = base_img;
    for (const auto& p : new_pixels) {
        full_img[p.first * W + p.second] = 1;
    }
    auto result_2pass = label_cc_2pass(full_img.data(), H, W, eight_conn);
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
    if (inc_components != full_components) {
        std::cerr << "WARNING: Component count mismatch!\n";
        std::cerr << "  Incremental: " << inc_components << "\n";
        std::cerr << "  Full recompute: " << full_components << "\n";
    } else {
        std::cout << "✓ Results verified: " << inc_components << " components\n\n";
    }

    // Comparison
    std::cout << "============================================================\n";
    std::cout << "Performance Comparison:\n";
    std::cout << "============================================================\n";
    std::cout << "Incremental DSU:        " << inc_time << " μs\n";
    std::cout << "Full 2-Pass:            " << time_2pass << " μs (";
    std::cout << (time_2pass / inc_time) << "x ";
    std::cout << (time_2pass > inc_time ? "slower" : "faster") << ")\n";
    std::cout << "Full BFS:               " << time_bfs << " μs (";
    std::cout << (time_bfs / inc_time) << "x ";
    std::cout << (time_bfs > inc_time ? "slower" : "faster") << ")\n";
    std::cout << "Full DFS:               " << time_dfs << " μs (";
    std::cout << (time_dfs / inc_time) << "x ";
    std::cout << (time_dfs > inc_time ? "slower" : "faster") << ")\n";
    std::cout << "Full DSU (1-pass):      " << time_dsu << " μs (";
    std::cout << (time_dsu / inc_time) << "x ";
    std::cout << (time_dsu > inc_time ? "slower" : "faster") << ")\n\n";

    if (inc_time < time_2pass && inc_time < time_bfs && 
        inc_time < time_dfs && inc_time < time_dsu) {
        double best_full = std::min({time_2pass, time_bfs, time_dfs, time_dsu});
        std::cout << "✅ Incremental DSU is FASTER for incremental updates!\n";
        std::cout << "   Advantage: " << (best_full / inc_time) << "x speedup\n";
        std::cout << "   This shows DSU is better suited for stream/dynamic updates!\n";
    } else {
        std::cout << "⚠️  Incremental DSU may not be faster in this scenario.\n";
        std::cout << "   Consider: small updates vs full recomputation overhead\n";
    }

    return 0;
}

