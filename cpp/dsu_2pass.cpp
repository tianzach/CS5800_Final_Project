#include "dsu_2pass.hpp"
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

std::vector<int32_t> label_cc_2pass(
    const uint8_t* img, int H, int W, 
    bool eight_connectivity
) {
    std::vector<int32_t> labels(H * W, 0);
    int next_label = 1;
    
    // Upper bound: H*W/2 + 10
    DSUInt32 dsu(H * W / 2 + 10);

    // First pass: assign temporary labels and union neighbors
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (img[y * W + x] == 0) {
                continue;
            }

            std::vector<int> neighbors;
            
            // left
            if (x - 1 >= 0 && labels[y * W + (x - 1)] > 0) {
                neighbors.push_back(labels[y * W + (x - 1)]);
            }
            // upper
            if (y - 1 >= 0 && labels[(y - 1) * W + x] > 0) {
                neighbors.push_back(labels[(y - 1) * W + x]);
            }
            
            if (eight_connectivity) {
                // upper-left
                if (x - 1 >= 0 && y - 1 >= 0 && labels[(y - 1) * W + (x - 1)] > 0) {
                    neighbors.push_back(labels[(y - 1) * W + (x - 1)]);
                }
                // upper-right
                if (x + 1 < W && y - 1 >= 0 && labels[(y - 1) * W + (x + 1)] > 0) {
                    neighbors.push_back(labels[(y - 1) * W + (x + 1)]);
                }
            }

            if (neighbors.empty()) {
                // allocate new label
                labels[y * W + x] = next_label;
                next_label++;
            } else {
                // get min label
                int m = *std::min_element(neighbors.begin(), neighbors.end());
                labels[y * W + x] = m;
                // Union with other neighbors
                for (int nb : neighbors) {
                    if (nb != m) {
                        dsu.union_set(m, nb);
                    }
                }
            }
        }
    }

    // Second pass: map to representative and compress to continuous labels
    // Find all used labels
    std::unordered_set<int> used_set;
    for (int i = 0; i < H * W; ++i) {
        if (labels[i] > 0) {
            used_set.insert(labels[i]);
        }
    }
    
    if (used_set.empty()) {
        return labels; // all background
    }

    // Representative mapping
    std::unordered_map<int, int> rep;
    for (int lab : used_set) {
        rep[lab] = dsu.find(lab);
    }

    // Compress to continuous labels
    std::unordered_set<int> rep_vals;
    for (const auto& pair : rep) {
        rep_vals.insert(pair.second);
    }
    
    std::vector<int> rep_vals_sorted(rep_vals.begin(), rep_vals.end());
    std::sort(rep_vals_sorted.begin(), rep_vals_sorted.end());
    
    std::unordered_map<int, int> rep2final;
    for (size_t i = 0; i < rep_vals_sorted.size(); ++i) {
        rep2final[rep_vals_sorted[i]] = i + 1;
    }

    // Apply final mapping
    for (int i = 0; i < H * W; ++i) {
        int v = labels[i];
        if (v > 0) {
            labels[i] = rep2final[rep[v]];
        }
    }

    return labels;
}

