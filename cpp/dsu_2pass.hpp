#ifndef DSU_2PASS_HPP
#define DSU_2PASS_HPP

#include <vector>
#include <cstdint>

// DSU for 2-pass algorithm
class DSUInt32 {
private:
    std::vector<int32_t> parent;
    std::vector<int32_t> rank;

public:
    DSUInt32(int n);
    int32_t find(int32_t x);
    void union_set(int32_t a, int32_t b);
};

// 2-pass connected-component labeling with DSU
std::vector<int32_t> label_cc_2pass(
    const uint8_t* img, int H, int W,
    bool eight_connectivity = false
);

#endif // DSU_2PASS_HPP

