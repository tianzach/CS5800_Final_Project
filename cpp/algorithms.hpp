#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include <vector>
#include <cstdint>
#include <queue>
#include <stack>

// BFS connected-component labeling
std::vector<int32_t> label_cc_bfs(
    const uint8_t* img, int H, int W,
    bool eight_connectivity = false
);

// DFS (stack) connected-component labeling
std::vector<int32_t> label_cc_dfs(
    const uint8_t* img, int H, int W,
    bool eight_connectivity = false
);

// DSU one-pass algorithm
std::vector<int32_t> label_cc_dsu(
    const uint8_t* img, int H, int W,
    bool eight_connectivity = false
);

#endif // ALGORITHMS_HPP

