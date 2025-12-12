#ifndef DSU_2PASS_HPP
#define DSU_2PASS_HPP

#include <vector>
#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

class DSUInt32 {
private:
    std::vector<int32_t> parent;
    std::vector<int8_t> rank;

public:
    DSUInt32(int n) : parent(n), rank(n, 0) {
        for (int i = 0; i < n; ++i) {
            parent[i] = i;
        }
    }

    int find(int x) {
        // path compression (iteration)
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }

    void union_set(int a, int b) {
        if (a == b) return;
        int ra = find(a);
        int rb = find(b);
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
};

// Input: img as 0/1 uint8_t array, H x W
// Output: labels as int32_t array, H x W
std::vector<int32_t> label_cc_2pass(
    const uint8_t* img, int H, int W, 
    bool eight_connectivity = false
);

#endif // DSU_2PASS_HPP

