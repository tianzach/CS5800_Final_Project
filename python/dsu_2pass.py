import numpy as np

class DSUInt32:
    def __init__(self, n: int):
        self.parent = np.arange(n, dtype=np.int32)
        self.rank = np.zeros(n, dtype=np.int8)
    def find(self, x: int) -> int:
        # path compression (iteration)
        p = self.parent
        while p[x] != x:
            p[x] = p[p[x]]
            x = p[x]
        return x
    def union(self, a: int, b: int):
        if a == b: return
        p, r = self.parent, self.rank
        ra, rb = self.find(a), self.find(b)
        if ra == rb: return
        if r[ra] < r[rb]:
            p[ra] = rb
        elif r[ra] > r[rb]:
            p[rb] = ra
        else:
            p[rb] = ra
            r[ra] += 1

def label_cc_2pass(img: np.ndarray, eight_connectivity=False) -> np.ndarray:
    """
    Two-pass CCL (Rosenfeld). Input img as 0/1.
    The first scan: give a temporary label to each foreground pixel, 
    and make a union (record equivalent) to the adjacent marked pixels.
    The second scan: map the temporary label to the representative element 
    and compress it into the final continuous label 1.. K.
    """
    assert img.ndim == 2 and img.dtype == np.uint8
    H, W = img.shape
    labels = np.zeros((H, W), dtype=np.int32)

    # temp label starts from l and then increase
    next_label = 1
    # For safety, it will not exceed H*W/2 (usually much smaller) at most and first open a slightly extra size.
    # It can also be dynamically expanded when needed; here, take the upper bound of H*W for simplicity.
    dsu = DSUInt32(H * W // 2 + 10)  # The upper boundary is large enough.

    # Neighborhood check (4 neighborhoods: left/top; 8 neighborhoods: left/top left/top right)
    for y in range(H):
        for x in range(W):
            if img[y, x] == 0:
                continue

            neighbors = []
            # left
            if x-1 >= 0 and labels[y, x-1] > 0:
                neighbors.append(labels[y, x-1])
            # upper
            if y-1 >= 0 and labels[y-1, x] > 0:
                neighbors.append(labels[y-1, x])
            if eight_connectivity:
                # upper-left
                if x-1 >= 0 and y-1 >= 0 and labels[y-1, x-1] > 0:
                    neighbors.append(labels[y-1, x-1])
                # upper-right
                if x+1 < W and y-1 >= 0 and labels[y-1, x+1] > 0:
                    neighbors.append(labels[y-1, x+1])

            if not neighbors:
                # allocate new label
                labels[y, x] = next_label
                next_label += 1
            else:
                # get min label
                m = min(neighbors)
                labels[y, x] = m
                # Make an equal union with other neighbors
                for nb in neighbors:
                    if nb != m:
                        dsu.union(m, nb)

    # The second time: Replace each temporary label with an equivalent class representative, and compress it into a continuous 1.. K
    # Find all the tags that have appeared first.
    used = np.unique(labels[labels > 0])
    if used.size == 0:
        return labels  # all background

    # Representative mapping
    rep = {}
    for lab in used:
        rep_lab = dsu.find(lab)
        rep[lab] = rep_lab

    # Continuation
    rep_vals = sorted(set(rep.values()))
    rep2final = {r: i+1 for i, r in enumerate(rep_vals)}

    out = labels.copy()
    it = np.nditer(out, flags=['multi_index'], op_flags=['readwrite'])
    while not it.finished:
        v = int(it[0])
        if v > 0:
            it[0] = rep2final[rep[v]]
        it.iternext()

    return out
