#include "algorithms.hpp"
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <unordered_map>

// Offsets for 4-connectivity
const int OFFSETS_4[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
// Offsets for 8-connectivity
const int OFFSETS_8[8][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};

std::vector<int32_t> label_cc_bfs(
    const uint8_t* img, int H, int W,
    bool eight_connectivity
) {
    std::vector<int32_t> labels(H * W, 0);
    std::vector<bool> visited(H * W, false);
    
    const int num_offsets = eight_connectivity ? 8 : 4;
    const int (*offsets)[2] = eight_connectivity ? OFFSETS_8 : OFFSETS_4;

    int current = 0;
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (img[y * W + x] == 1 && !visited[y * W + x]) {
                current++;
                std::queue<std::pair<int, int>> q;
                q.push({y, x});
                visited[y * W + x] = true;
                labels[y * W + x] = current;

                while (!q.empty()) {
                    auto [cy, cx] = q.front();
                    q.pop();
                    
                    for (int i = 0; i < num_offsets; ++i) {
                        int ny = cy + offsets[i][0];
                        int nx = cx + offsets[i][1];
                        
                        if (ny >= 0 && ny < H && nx >= 0 && nx < W) {
                            if (img[ny * W + nx] == 1 && !visited[ny * W + nx]) {
                                visited[ny * W + nx] = true;
                                labels[ny * W + nx] = current;
                                q.push({ny, nx});
                            }
                        }
                    }
                }
            }
        }
    }
    return labels;
}

std::vector<int32_t> label_cc_dfs(
    const uint8_t* img, int H, int W,
    bool eight_connectivity
) {
    std::vector<int32_t> labels(H * W, 0);
    std::vector<bool> visited(H * W, false);
    
    const int num_offsets = eight_connectivity ? 8 : 4;
    const int (*offsets)[2] = eight_connectivity ? OFFSETS_8 : OFFSETS_4;

    int current = 0;
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (img[y * W + x] == 1 && !visited[y * W + x]) {
                current++;
                std::stack<std::pair<int, int>> stack;
                stack.push({y, x});
                visited[y * W + x] = true;
                labels[y * W + x] = current;

                while (!stack.empty()) {
                    auto [cy, cx] = stack.top();
                    stack.pop();
                    
                    for (int i = 0; i < num_offsets; ++i) {
                        int ny = cy + offsets[i][0];
                        int nx = cx + offsets[i][1];
                        
                        if (ny >= 0 && ny < H && nx >= 0 && nx < W) {
                            if (img[ny * W + nx] == 1 && !visited[ny * W + nx]) {
                                visited[ny * W + nx] = true;
                                labels[ny * W + nx] = current;
                                stack.push({ny, nx});
                            }
                        }
                    }
                }
            }
        }
    }
    return labels;
}

class DSU {
private:
    std::vector<int> parent;
    std::vector<int> rank;

public:
    DSU(int n) : parent(n), rank(n, 0) {
        for (int i = 0; i < n; ++i) {
            parent[i] = i;
        }
    }

    int find(int x) {
        while (x != parent[x]) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }

    void union_set(int a, int b) {
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

std::vector<int32_t> label_cc_dsu(
    const uint8_t* img, int H, int W,
    bool eight_connectivity
) {
    int N = H * W;
    DSU dsu(N);

    // Pass 1: union adjacent pixels
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (img[y * W + x] != 1) {
                continue;
            }
            int idx = y * W + x;

            // right
            if (x + 1 < W && img[y * W + (x + 1)] == 1) {
                dsu.union_set(idx, y * W + (x + 1));
            }
            // down
            if (y + 1 < H && img[(y + 1) * W + x] == 1) {
                dsu.union_set(idx, (y + 1) * W + x);
            }

            if (eight_connectivity) {
                if (y + 1 < H && x + 1 < W && img[(y + 1) * W + (x + 1)] == 1) {
                    dsu.union_set(idx, (y + 1) * W + (x + 1));
                }
                if (y + 1 < H && x - 1 >= 0 && img[(y + 1) * W + (x - 1)] == 1) {
                    dsu.union_set(idx, (y + 1) * W + (x - 1));
                }
            }
        }
    }

    // Pass 2: assign final labels
    std::vector<int32_t> labels(H * W, 0);
    std::unordered_map<int, int> root2label;
    int cur = 0;

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (img[y * W + x] == 1) {
                int r = dsu.find(y * W + x);
                if (root2label.find(r) == root2label.end()) {
                    cur++;
                    root2label[r] = cur;
                }
                labels[y * W + x] = root2label[r];
            }
        }
    }

    return labels;
}

