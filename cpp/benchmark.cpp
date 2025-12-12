#include "dsu_2pass.hpp"
#include "algorithms.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cstring>

// Simple function to generate random test image
void generate_test_image(uint8_t* img, int H, int W, double density) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (int i = 0; i < H * W; ++i) {
        img[i] = (dis(gen) < density) ? 1 : 0;
    }
}

double benchmark_function(
    std::vector<int32_t> (*func)(const uint8_t*, int, int, bool),
    const uint8_t* img, int H, int W, bool eight_conn,
    int iterations
) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func(img, H, W, eight_conn);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / (double)iterations;
}

int main(int argc, char* argv[]) {
    int H = 1000;
    int W = 1000;
    double density = 0.3;
    int iterations = 10;
    bool eight_conn = false;

    if (argc > 1) H = std::stoi(argv[1]);
    if (argc > 2) W = std::stoi(argv[2]);
    if (argc > 3) density = std::stod(argv[3]);
    if (argc > 4) iterations = std::stoi(argv[4]);
    if (argc > 5) eight_conn = (std::stoi(argv[5]) != 0);

    std::vector<uint8_t> img(H * W);
    generate_test_image(img.data(), H, W, density);

    std::cout << "C++ Benchmark Results\n";
    std::cout << "Image size: " << H << "x" << W << "\n";
    std::cout << "Density: " << density << "\n";
    std::cout << "Connectivity: " << (eight_conn ? "8" : "4") << "\n";
    std::cout << "Iterations: " << iterations << "\n\n";

    double time_2pass = benchmark_function(label_cc_2pass, img.data(), H, W, eight_conn, iterations);
    double time_bfs = benchmark_function(label_cc_bfs, img.data(), H, W, eight_conn, iterations);
    double time_dfs = benchmark_function(label_cc_dfs, img.data(), H, W, eight_conn, iterations);
    double time_dsu = benchmark_function(label_cc_dsu, img.data(), H, W, eight_conn, iterations);

    std::cout << "2-Pass (DSU): " << time_2pass << " μs\n";
    std::cout << "BFS:          " << time_bfs << " μs\n";
    std::cout << "DFS:          " << time_dfs << " μs\n";
    std::cout << "DSU (1-pass): " << time_dsu << " μs\n";

    return 0;
}

