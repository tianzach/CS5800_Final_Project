#include "dsu_2pass.hpp"
#include "algorithms.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <set>
#include <algorithm>
#include <map>
#include <fstream>
#include <sstream>

struct TestResult {
    std::string strategy;
    std::string image_size;
    std::string input_type;
    double time_us;
    int components;
    double throughput_mpixels_per_sec;
    bool is_stream;
};

// Test configurations
struct TestConfig {
    int H, W;
    std::string size_name;
    bool is_stream;
    double stream_ratio;  // 0.0 = full image, >0.0 = stream ratio
    std::string input_type_name;
};


double benchmark_strategy_full(
    const uint8_t* img, int H, int W, bool eight_conn, int iterations,
    const std::string& strategy) {
    
    std::vector<int32_t> (*func)(const uint8_t*, int, int, bool) = nullptr;
    
    if (strategy == "2-Pass DSU") {
        func = label_cc_2pass;
    } else if (strategy == "BFS") {
        func = label_cc_bfs;
    } else if (strategy == "DFS") {
        func = label_cc_dfs;
    } else if (strategy == "DSU 1-pass") {
        func = label_cc_dsu;
    }
    
    if (!func) return 0.0;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        func(img, H, W, eight_conn);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / (double)iterations;
}

int get_component_count(const uint8_t* img, int H, int W, bool eight_conn, const std::string& strategy) {
    std::vector<int32_t> (*func)(const uint8_t*, int, int, bool) = nullptr;
    
    if (strategy == "2-Pass DSU") {
        func = label_cc_2pass;
    } else if (strategy == "BFS") {
        func = label_cc_bfs;
    } else if (strategy == "DFS") {
        func = label_cc_dfs;
    } else if (strategy == "DSU 1-pass") {
        func = label_cc_dsu;
    }
    
    if (!func) return 0;
    
    auto result = func(img, H, W, eight_conn);
    std::set<int32_t> comps;
    for (int i = 0; i < H * W; ++i) {
        if (result[i] > 0) comps.insert(result[i]);
    }
    return comps.size();
}


void run_comprehensive_test(bool eight_conn, int iterations) {
    // Define all strategies
    std::vector<std::string> strategies = {
        "2-Pass DSU", "BFS", "DFS", "DSU 1-pass"
    };
    
    // Define image sizes
    std::vector<TestConfig> image_configs = {
        {200, 200, "Small (200x200)", false, 0.0, "Full Image"},
        {200, 200, "Small (200x200)", true, 0.1, "Stream 10%"},
        {200, 200, "Small (200x200)", true, 0.5, "Stream 50%"},
        {200, 200, "Small (200x200)", true, 1.0, "Stream 100%"},
        
        {500, 500, "Medium (500x500)", false, 0.0, "Full Image"},
        {500, 500, "Medium (500x500)", true, 0.1, "Stream 10%"},
        {500, 500, "Medium (500x500)", true, 0.5, "Stream 50%"},
        {500, 500, "Medium (500x500)", true, 1.0, "Stream 100%"},
        
        {1000, 1000, "Large (1000x1000)", false, 0.0, "Full Image"},
        {1000, 1000, "Large (1000x1000)", true, 0.1, "Stream 10%"},
        {1000, 1000, "Large (1000x1000)", true, 0.5, "Stream 50%"},
        {1000, 1000, "Large (1000x1000)", true, 1.0, "Stream 100%"},
        
        {2000, 2000, "Very Large (2000x2000)", false, 0.0, "Full Image"},
        {2000, 2000, "Very Large (2000x2000)", true, 0.1, "Stream 10%"},
        {2000, 2000, "Very Large (2000x2000)", true, 0.5, "Stream 50%"},
        {2000, 2000, "Very Large (2000x2000)", true, 1.0, "Stream 100%"}
    };
    
    std::vector<TestResult> all_results;
    
    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << "COMPREHENSIVE UNIFIED TEST\n";
    std::cout << "Three Dimensions: Strategy × Image Size × Input Type (Stream/Full)\n";
    std::cout << std::string(120, '=') << "\n";
    std::cout << "Connectivity: " << (eight_conn ? "8" : "4") << "\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Total combinations: " << (strategies.size() * image_configs.size()) << "\n";
    std::cout << "Note: Stream input tests build full image and use standard algorithms\n\n";
    
    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    int test_count = 0;
    int total_tests = strategies.size() * image_configs.size();
    
    for (const auto& config : image_configs) {
        int H = config.H;
        int W = config.W;
        int image_size = H * W;
        
        std::cout << "\n" << std::string(120, '-') << "\n";
        std::cout << "Testing: " << config.size_name << " - " << config.input_type_name << "\n";
        std::cout << std::string(120, '-') << "\n";
        
        // Prepare test data
        std::vector<uint8_t> full_img(H * W);
        std::vector<std::pair<int, int>> stream_pixels;
        
        if (config.is_stream) {
            // Generate stream pixels
            int stream_size = (int)(image_size * config.stream_ratio);
            if (stream_size > image_size) stream_size = image_size;
            
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
                stream_pixels.push_back({y, x});
                full_img[y * W + x] = 1;
            }
        } else {
            // Generate full image with 30% density
            for (int i = 0; i < H * W; ++i) {
                full_img[i] = (dis(gen) < 0.3) ? 1 : 0;
            }
        }
        
        // Test each strategy
        for (const auto& strategy : strategies) {
            test_count++;
            TestResult result;
            result.strategy = strategy;
            result.image_size = config.size_name;
            result.input_type = config.input_type_name;
            result.is_stream = config.is_stream;
            
            // For stream input, build full image from stream pixels and use full algorithms
            // For full image, use full algorithms directly
            const uint8_t* img_data = config.is_stream ? full_img.data() : full_img.data();
            double time = benchmark_strategy_full(img_data, H, W, eight_conn, iterations, strategy);
            int comps = get_component_count(img_data, H, W, eight_conn, strategy);
            
            result.time_us = time;
            result.components = comps;
            if (config.is_stream) {
                int num_pixels = stream_pixels.size();
                result.throughput_mpixels_per_sec = (num_pixels / 1e6) / (time / 1e6);
            } else {
                result.throughput_mpixels_per_sec = (image_size / 1e6) / (time / 1e6);
            }
            
            all_results.push_back(result);
            
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "  [" << test_count << "/" << total_tests << "] " 
                      << std::left << std::setw(15) << strategy 
                      << ": " << std::right << std::setw(12) << result.time_us << " μs, "
                      << std::setw(8) << result.components << " components\n";
        }
    }
    
    // Print summary table
    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << "SUMMARY TABLE\n";
    std::cout << std::string(120, '=') << "\n";
    
    std::cout << std::left << std::setw(15) << "Strategy";
    std::cout << std::setw(25) << "Image Size";
    std::cout << std::setw(20) << "Input Type";
    std::cout << std::right << std::setw(15) << "Time (μs)";
    std::cout << std::setw(12) << "Components";
    std::cout << std::setw(15) << "Throughput";
    std::cout << "\n";
    std::cout << std::string(120, '-') << "\n";
    
    for (const auto& r : all_results) {
        std::cout << std::left << std::setw(15) << r.strategy;
        std::cout << std::setw(25) << r.image_size;
        std::cout << std::setw(20) << r.input_type;
        std::cout << std::right << std::fixed << std::setprecision(2);
        std::cout << std::setw(15) << r.time_us;
        std::cout << std::setw(12) << r.components;
        std::cout << std::setw(14) << std::setprecision(2) << r.throughput_mpixels_per_sec << " MP/s";
        std::cout << "\n";
    }
    
    // Find winners for each category
    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << "WINNERS BY CATEGORY\n";
    std::cout << std::string(120, '=') << "\n";
    
    // Group by image size and input type
    std::map<std::string, std::map<std::string, TestResult>> winners;
    
    for (const auto& r : all_results) {
        std::string key = r.image_size + " - " + r.input_type;
        if (winners[key].empty() || r.time_us < winners[key].begin()->second.time_us) {
            winners[key].clear();
            winners[key][r.strategy] = r;
        } else if (r.time_us == winners[key].begin()->second.time_us) {
            winners[key][r.strategy] = r;
        }
    }
    
    for (const auto& [category, strategy_map] : winners) {
        std::cout << "\n" << category << ":\n";
        for (const auto& [strategy, result] : strategy_map) {
            std::cout << "  Winner: " << strategy << " (" << result.time_us << " μs)\n";
        }
    }
    
    // Export to CSV
    std::ofstream csv_file("unified_test_results.csv");
    csv_file << "Strategy,ImageSize,InputType,Time_us,Components,Throughput_MPs,IsStream\n";
    for (const auto& r : all_results) {
        csv_file << r.strategy << ","
                 << r.image_size << ","
                 << r.input_type << ","
                 << r.time_us << ","
                 << r.components << ","
                 << r.throughput_mpixels_per_sec << ","
                 << (r.is_stream ? "true" : "false") << "\n";
    }
    csv_file.close();
    
    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << "Results exported to: unified_test_results.csv\n";
    std::cout << std::string(120, '=') << "\n\n";
}

int main(int argc, char* argv[]) {
    int iterations = 5;
    bool eight_conn = false;

    if (argc > 1) iterations = std::stoi(argv[1]);
    if (argc > 2) eight_conn = (std::stoi(argv[2]) != 0);

    run_comprehensive_test(eight_conn, iterations);

    return 0;
}

