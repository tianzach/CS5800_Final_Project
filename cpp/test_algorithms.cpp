#include "dsu_2pass.hpp"
#include "algorithms.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <set>

bool test_simple_4_connected() {
    // Test image: 2 components
    // [1, 1, 0, 0]
    // [1, 0, 0, 0]
    // [0, 0, 1, 1]
    // [0, 0, 1, 1]
    int H = 4, W = 4;
    std::vector<uint8_t> img = {
        1, 1, 0, 0,
        1, 0, 0, 0,
        0, 0, 1, 1,
        0, 0, 1, 1
    };
    
    auto result_2pass = label_cc_2pass(img.data(), H, W, false);
    auto result_bfs = label_cc_bfs(img.data(), H, W, false);
    auto result_dfs = label_cc_dfs(img.data(), H, W, false);
    auto result_dsu = label_cc_dsu(img.data(), H, W, false);
    
    // Count unique labels (excluding 0)
    std::set<int32_t> labels_2pass, labels_bfs, labels_dfs, labels_dsu;
    for (int i = 0; i < H * W; ++i) {
        if (result_2pass[i] > 0) labels_2pass.insert(result_2pass[i]);
        if (result_bfs[i] > 0) labels_bfs.insert(result_bfs[i]);
        if (result_dfs[i] > 0) labels_dfs.insert(result_dfs[i]);
        if (result_dsu[i] > 0) labels_dsu.insert(result_dsu[i]);
    }
    
    assert(labels_2pass.size() == 2);
    assert(labels_bfs.size() == 2);
    assert(labels_dfs.size() == 2);
    assert(labels_dsu.size() == 2);
    
    return true;
}

bool test_simple_8_connected() {
    // Test image: diagonal pattern
    // [1, 0, 1]
    // [0, 1, 0]
    // [1, 0, 1]
    int H = 3, W = 3;
    std::vector<uint8_t> img = {
        1, 0, 1,
        0, 1, 0,
        1, 0, 1
    };
    
    // 8-connectivity: should be 1 component
    auto result_2pass_8 = label_cc_2pass(img.data(), H, W, true);
    auto result_bfs_8 = label_cc_bfs(img.data(), H, W, true);
    
    std::set<int32_t> labels_2pass_8, labels_bfs_8;
    for (int i = 0; i < H * W; ++i) {
        if (result_2pass_8[i] > 0) labels_2pass_8.insert(result_2pass_8[i]);
        if (result_bfs_8[i] > 0) labels_bfs_8.insert(result_bfs_8[i]);
    }
    
    assert(labels_2pass_8.size() == 1);
    assert(labels_bfs_8.size() == 1);
    
    // 4-connectivity: should be 5 components
    auto result_2pass_4 = label_cc_2pass(img.data(), H, W, false);
    std::set<int32_t> labels_2pass_4;
    for (int i = 0; i < H * W; ++i) {
        if (result_2pass_4[i] > 0) labels_2pass_4.insert(result_2pass_4[i]);
    }
    
    assert(labels_2pass_4.size() == 5);
    
    return true;
}

bool test_empty_image() {
    int H = 10, W = 10;
    std::vector<uint8_t> img(H * W, 0);
    
    auto result_2pass = label_cc_2pass(img.data(), H, W);
    auto result_bfs = label_cc_bfs(img.data(), H, W);
    
    for (int i = 0; i < H * W; ++i) {
        assert(result_2pass[i] == 0);
        assert(result_bfs[i] == 0);
    }
    
    return true;
}

bool test_full_foreground() {
    int H = 5, W = 5;
    std::vector<uint8_t> img(H * W, 1);
    
    auto result_2pass = label_cc_2pass(img.data(), H, W);
    auto result_bfs = label_cc_bfs(img.data(), H, W);
    
    std::set<int32_t> labels_2pass, labels_bfs;
    for (int i = 0; i < H * W; ++i) {
        if (result_2pass[i] > 0) labels_2pass.insert(result_2pass[i]);
        if (result_bfs[i] > 0) labels_bfs.insert(result_bfs[i]);
    }
    
    assert(labels_2pass.size() == 1);
    assert(labels_bfs.size() == 1);
    
    return true;
}

bool test_consistency() {
    // Random test image
    int H = 50, W = 50;
    std::vector<uint8_t> img(H * W);
    // Simple deterministic pattern
    for (int i = 0; i < H * W; ++i) {
        img[i] = (i % 7 < 2) ? 1 : 0;
    }
    
    auto result_2pass = label_cc_2pass(img.data(), H, W);
    auto result_bfs = label_cc_bfs(img.data(), H, W);
    auto result_dfs = label_cc_dfs(img.data(), H, W);
    auto result_dsu = label_cc_dsu(img.data(), H, W);
    
    std::set<int32_t> labels_2pass, labels_bfs, labels_dfs, labels_dsu;
    for (int i = 0; i < H * W; ++i) {
        if (result_2pass[i] > 0) labels_2pass.insert(result_2pass[i]);
        if (result_bfs[i] > 0) labels_bfs.insert(result_bfs[i]);
        if (result_dfs[i] > 0) labels_dfs.insert(result_dfs[i]);
        if (result_dsu[i] > 0) labels_dsu.insert(result_dsu[i]);
    }
    
    assert(labels_2pass.size() == labels_bfs.size());
    assert(labels_bfs.size() == labels_dfs.size());
    assert(labels_dfs.size() == labels_dsu.size());
    
    return true;
}

int main() {
    std::cout << "Running C++ tests...\n\n";
    
    try {
        if (test_simple_4_connected()) {
            std::cout << "✓ Simple 4-connected test passed\n";
        }
        if (test_simple_8_connected()) {
            std::cout << "✓ Simple 8-connected test passed\n";
        }
        if (test_empty_image()) {
            std::cout << "✓ Empty image test passed\n";
        }
        if (test_full_foreground()) {
            std::cout << "✓ Full foreground test passed\n";
        }
        if (test_consistency()) {
            std::cout << "✓ Consistency test passed\n";
        }
        
        std::cout << "\n✅ All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown error occurred\n";
        return 1;
    }
}

