# algorithms.py
from __future__ import annotations
import numpy as np
from collections import deque


OFFSETS_4 = [(1,0),(-1,0),(0,1),(0,-1)]
OFFSETS_8 = OFFSETS_4 + [(1,1),(1,-1),(-1,1),(-1,-1)]


def label_cc_bfs(img: np.ndarray, eight_connectivity=False) -> np.ndarray:
    """
    BFS connected-component labeling.
    img: 0/1 binary array.
    """
    H, W = img.shape
    labels = np.zeros((H, W), dtype=np.int32)
    offsets = OFFSETS_8 if eight_connectivity else OFFSETS_4
    visited = np.zeros_like(img, dtype=bool)

    current = 0
    for y in range(H):
        for x in range(W):
            if img[y, x] == 1 and not visited[y, x]:
                current += 1
                q = deque([(y, x)])
                visited[y, x] = True
                labels[y, x] = current

                while q:
                    cy, cx = q.popleft()
                    for dy, dx in offsets:
                        ny, nx = cy + dy, cx + dx
                        if 0 <= ny < H and 0 <= nx < W:
                            if img[ny, nx] == 1 and not visited[ny, nx]:
                                visited[ny, nx] = True
                                labels[ny, nx] = current
                                q.append((ny, nx))
    return labels


def label_cc_dfs(img: np.ndarray, eight_connectivity=False) -> np.ndarray:
    """
    DFS (stack) connected-component labeling.
    """
    H, W = img.shape
    labels = np.zeros((H, W), dtype=np.int32)
    offsets = OFFSETS_8 if eight_connectivity else OFFSETS_4
    visited = np.zeros_like(img, dtype=bool)

    current = 0
    for y in range(H):
        for x in range(W):
            if img[y, x] == 1 and not visited[y, x]:
                current += 1
                stack = [(y, x)]
                visited[y, x] = True
                labels[y, x] = current

                while stack:
                    cy, cx = stack.pop()
                    for dy, dx in offsets:
                        ny, nx = cy + dy, cx + dx
                        if 0 <= ny < H and 0 <= nx < W:
                            if img[ny, nx] == 1 and not visited[ny, nx]:
                                visited[ny, nx] = True
                                labels[ny, nx] = current
                                stack.append((ny, nx))
    return labels


# ------------------------- DSU -------------------------
class DSU:
    def __init__(self, n: int):
        self.parent = list(range(n))
        self.rank = [0] * n

    def find(self, x: int) -> int:
        while x != self.parent[x]:
            self.parent[x] = self.parent[self.parent[x]]
            x = self.parent[x]
        return x

    def union(self, a: int, b: int):
        ra, rb = self.find(a), self.find(b)
        if ra == rb:
            return
        if self.rank[ra] < self.rank[rb]:
            self.parent[ra] = rb
        elif self.rank[ra] > self.rank[rb]:
            self.parent[rb] = ra
        else:
            self.parent[rb] = ra
            self.rank[ra] += 1


def label_cc_dsu(img: np.ndarray, eight_connectivity=False) -> np.ndarray:
    """
    One-pass "union then assign labels" algorithm.
    """
    H, W = img.shape
    N = H * W
    dsu = DSU(N)

    # pass 1: union adjacent pixels
    for y in range(H):
        for x in range(W):
            if img[y, x] != 1:
                continue
            idx = y * W + x

            # right
            if x + 1 < W and img[y, x+1] == 1:
                dsu.union(idx, y * W + (x+1))

            # down
            if y + 1 < H and img[y+1, x] == 1:
                dsu.union(idx, (y+1) * W + x)

            if eight_connectivity:
                if y + 1 < H and x + 1 < W and img[y+1, x+1] == 1:
                    dsu.union(idx, (y+1)*W + (x+1))
                if y + 1 < H and x - 1 >= 0 and img[y+1, x-1] == 1:
                    dsu.union(idx, (y+1)*W + (x-1))

    # pass 2: assign final labels
    labels = np.zeros((H, W), dtype=np.int32)
    root2label = {}
    cur = 0

    for y in range(H):
        for x in range(W):
            if img[y, x] == 1:
                r = dsu.find(y * W + x)
                if r not in root2label:
                    cur += 1
                    root2label[r] = cur
                labels[y, x] = root2label[r]

    return labels
